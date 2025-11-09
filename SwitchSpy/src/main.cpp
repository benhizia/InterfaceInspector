#include "switchspy/view/main_window.hpp"
#include "switchspy/view/log_panel.hpp"
#include "switchspy/config/config_manager.hpp"
#include "switchspy/controller/connection_manager.hpp"
#include "switchspy/core/network_manager.hpp"
#include "switchspy/common/logger.hpp"
#include "switchspy/common/benchmark.hpp"
#include <iostream>
#include <string>
#include <memory>

using namespace switchspy;

int main(int argc, char** argv) {
    std::cout << "SwitchSpy - Network Relay and Visualization Tool" << std::endl;
    std::cout << "=================================================" << std::endl;

    // Initialize logging system
    auto& logger = logging::Logger::instance();
    logger.setLevel(logging::LogLevel::Debug);

    // Add console sink
    auto console_sink = std::make_shared<view::ConsoleSink>();
    logger.addSink(console_sink);

    LOG_INFO("Main", "Starting SwitchSpy");

    // Parse command line arguments
    std::string config_file = "config/example.ini";
    if (argc > 1) {
        config_file = argv[1];
    }

    LOG_INFO("Main", "Loading configuration from: " + config_file);

    // Load configuration
    auto& config = config::ConfigManager::instance();
    if (!config.loadFromFile(config_file)) {
        LOG_WARNING("Main", "Failed to load configuration file: " + config_file);
        LOG_INFO("Main", "Using default configuration");
    }

    // Initialize networking
    LOG_INFO("Main", "Initializing networking");
    auto& net_manager = core::NetworkManager::instance();
    net_manager.initialize();

    // Initialize connection manager
    LOG_INFO("Main", "Setting up connections");
    auto& conn_manager = controller::ConnectionManager::instance();

    // Create connections from configuration
    const auto& port_mappings = config.getPortMappings();
    for (const auto& [name, mapping] : port_mappings) {
        std::string type_str = (mapping.type == ConnectionType::TCP ? "TCP" :
                                mapping.type == ConnectionType::UDP ? "UDP" : "Multicast");
        LOG_INFO("Main", "Configuring connection: " + name + " (" + type_str + ")");

        conn_manager.addConnection(name);

        // TODO: Create actual network connections here
        // For now, just create the connection state model
    }

    // Log configuration
    std::string mode_str = (config.getMode() == ConnectionMode::Relay ? "Relay" :
                            config.getMode() == ConnectionMode::Record ? "Record" : "Replay");
    std::string viz_str = (config.getVisualizationMode() == VisualizationMode::Realtime ?
                           "Real-time" : "Deferred");
    std::string rec_str = (config.isRecordingEnabled() ? "Enabled" : "Disabled");

    LOG_INFO("Main", "Mode: " + mode_str);
    LOG_INFO("Main", "Visualization: " + viz_str);
    LOG_INFO("Main", "Recording: " + rec_str);

    // Initialize and run GUI
    LOG_INFO("Main", "Starting GUI");
    view::MainWindow main_window;

    if (!main_window.initialize()) {
        LOG_ERROR("Main", "Failed to initialize main window");
        return 1;
    }

    LOG_INFO("Main", "SwitchSpy is running. Close the window to exit.");
    main_window.run();

    // Cleanup
    LOG_INFO("Main", "Shutting down");
    main_window.shutdown();
    net_manager.shutdown();

    logger.flush();
    LOG_INFO("Main", "Goodbye!");

    return 0;
}
