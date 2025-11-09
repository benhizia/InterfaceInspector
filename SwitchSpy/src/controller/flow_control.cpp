#include "switchspy/controller/flow_control.hpp"
#include "switchspy/common/logger.hpp"
#include <sstream>

namespace switchspy {
namespace controller {

FlowControlMonitor::FlowControlMonitor(const std::string& connection_name)
    : connection_name_(connection_name)
    , last_rate_update_(std::chrono::steady_clock::now())
    , last_send_(std::chrono::steady_clock::now())
    , last_receive_(std::chrono::steady_clock::now())
{
}

void FlowControlMonitor::onPacketReceived(size_t bytes) {
    total_received_bytes_ += bytes;
    bytes_received_last_second_ += bytes;
    last_receive_ = std::chrono::steady_clock::now();

    updateRates();
    checkAndLogWarnings();
}

void FlowControlMonitor::onPacketSent(size_t bytes) {
    total_sent_bytes_ += bytes;
    bytes_sent_last_second_ += bytes;
    last_send_ = std::chrono::steady_clock::now();

    updateRates();
    checkAndLogWarnings();
}

void FlowControlMonitor::onBufferPush(size_t buffer_size, size_t buffer_capacity) {
    buffer_push_count_++;

    if (buffer_capacity > 0) {
        buffer_pressure_ = static_cast<double>(buffer_size) / buffer_capacity;

        if (buffer_pressure_ >= BUFFER_PRESSURE_CRITICAL) {
            std::ostringstream oss;
            oss << connection_name_ << ": Buffer critical ("
                << (int)(buffer_pressure_ * 100) << "% full, "
                << buffer_size << "/" << buffer_capacity << " bytes)";
            LOG_ERROR("FlowControl", oss.str());
        } else if (buffer_pressure_ >= BUFFER_PRESSURE_WARNING) {
            static auto last_warning = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_warning).count() > 5) {
                std::ostringstream oss;
                oss << connection_name_ << ": Buffer pressure high ("
                    << (int)(buffer_pressure_ * 100) << "% full)";
                LOG_WARNING("FlowControl", oss.str());
                last_warning = now;
            }
        }
    }
}

void FlowControlMonitor::onBufferPop() {
    buffer_pop_count_++;
}

void FlowControlMonitor::onSendBlocked() {
    blocked_send_count_++;

    static auto last_warning = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_warning).count() > 10) {
        std::ostringstream oss;
        oss << connection_name_ << ": Send operations blocked ("
            << blocked_send_count_.load() << " times). Receiver may be slow.";
        LOG_WARNING("FlowControl", oss.str());
        last_warning = now;
    }
}

void FlowControlMonitor::onReceiveBlocked() {
    blocked_receive_count_++;
}

double FlowControlMonitor::getRateMismatch() const {
    if (receive_rate_bps_ == 0.0 && send_rate_bps_ == 0.0) {
        return 1.0;
    }

    if (receive_rate_bps_ == 0.0) {
        return send_rate_bps_ > 0.0 ? 999.0 : 1.0;
    }

    if (send_rate_bps_ == 0.0) {
        return receive_rate_bps_ > 0.0 ? 999.0 : 1.0;
    }

    return std::max(receive_rate_bps_ / send_rate_bps_,
                    send_rate_bps_ / receive_rate_bps_);
}

bool FlowControlMonitor::isBufferOverflowing() const {
    return buffer_pressure_ >= BUFFER_PRESSURE_CRITICAL;
}

bool FlowControlMonitor::isRateMismatched() const {
    return getRateMismatch() >= RATE_MISMATCH_THRESHOLD;
}

bool FlowControlMonitor::isSenderFasterThanReceiver() const {
    return receive_rate_bps_ > 0.0 && send_rate_bps_ < receive_rate_bps_ * 0.5;
}

bool FlowControlMonitor::isReceiverNotConnected() const {
    auto now = std::chrono::steady_clock::now();
    auto ms_since_last_send = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_send_).count();

    // If we've received data but haven't sent any in a while
    return total_received_bytes_ > 0 &&
           total_sent_bytes_ == 0 &&
           ms_since_last_send > CONNECTION_TIMEOUT_MS;
}

bool FlowControlMonitor::isSenderNotConnected() const {
    auto now = std::chrono::steady_clock::now();
    auto ms_since_last_receive = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_receive_).count();

    // If we're trying to send but haven't received any data
    return total_sent_bytes_ > 0 &&
           total_received_bytes_ == 0 &&
           ms_since_last_receive > CONNECTION_TIMEOUT_MS;
}

FlowControlMonitor::DiagnosticInfo FlowControlMonitor::getDiagnostics() const {
    DiagnosticInfo info;
    info.connection_name = connection_name_;
    info.receive_rate_bps = receive_rate_bps_;
    info.send_rate_bps = send_rate_bps_;
    info.rate_mismatch = getRateMismatch();
    info.buffer_pressure = buffer_pressure_;
    info.blocked_send_count = blocked_send_count_;
    info.blocked_receive_count = blocked_receive_count_;
    info.total_received_bytes = total_received_bytes_;
    info.total_sent_bytes = total_sent_bytes_;
    info.buffer_overflow_warning = isBufferOverflowing();
    info.rate_mismatch_warning = isRateMismatched();
    info.receiver_disconnected_warning = isReceiverNotConnected();
    info.sender_disconnected_warning = isSenderNotConnected();

    return info;
}

void FlowControlMonitor::reset() {
    total_received_bytes_ = 0;
    total_sent_bytes_ = 0;
    bytes_received_last_second_ = 0;
    bytes_sent_last_second_ = 0;
    receive_rate_bps_ = 0.0;
    send_rate_bps_ = 0.0;
    buffer_pressure_ = 0.0;
    buffer_push_count_ = 0;
    buffer_pop_count_ = 0;
    blocked_send_count_ = 0;
    blocked_receive_count_ = 0;
}

void FlowControlMonitor::updateRates() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_rate_update_).count();

    // Update rates every second
    if (elapsed >= 1000) {
        double elapsed_sec = elapsed / 1000.0;

        receive_rate_bps_ = bytes_received_last_second_.load() / elapsed_sec;
        send_rate_bps_ = bytes_sent_last_second_.load() / elapsed_sec;

        bytes_received_last_second_ = 0;
        bytes_sent_last_second_ = 0;
        last_rate_update_ = now;
    }
}

void FlowControlMonitor::checkAndLogWarnings() {
    static auto last_check = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();

    // Check warnings every 5 seconds
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_check).count() < 5) {
        return;
    }

    last_check = now;

    // Check for rate mismatch
    if (isRateMismatched()) {
        std::ostringstream oss;
        oss << connection_name_ << ": Rate mismatch detected. "
            << "Receive: " << (int)(receive_rate_bps_ / 1024) << " KB/s, "
            << "Send: " << (int)(send_rate_bps_ / 1024) << " KB/s "
            << "(ratio: " << (int)getRateMismatch() << "x)";
        LOG_WARNING("FlowControl", oss.str());
    }

    // Check if sender is much faster than receiver
    if (isSenderFasterThanReceiver()) {
        std::ostringstream oss;
        oss << connection_name_ << ": Sender is significantly faster than receiver. "
            << "Consider using deferred visualization mode for better performance.";
        LOG_INFO("FlowControl", oss.str());
    }

    // Check for disconnected endpoints
    if (isReceiverNotConnected()) {
        std::ostringstream oss;
        oss << connection_name_ << ": Receiver may not be connected. "
            << "Data is being received but not forwarded.";
        LOG_WARNING("FlowControl", oss.str());
    }

    if (isSenderNotConnected()) {
        std::ostringstream oss;
        oss << connection_name_ << ": Sender may not be connected. "
            << "No data is being received from source.";
        LOG_WARNING("FlowControl", oss.str());
    }
}

} // namespace controller
} // namespace switchspy
