#include "switchspy/view/main_window.hpp"
#include "switchspy/config/config_manager.hpp"
#include "switchspy/controller/connection_manager.hpp"
#include "switchspy/core/network_manager.hpp"
#include <iostream>
#include <string>

using namespace switchspy;

int main(int argc, char** argv) {
    std::cout << "SwitchSpy - Network Relay and Visualization Tool" << std::endl;
    std::cout << "=================================================" << std::endl;

    // Parse command line arguments
    std::string config_file = "config/example.ini";
    if (argc > 1) {
        config_file = argv[1];
    }

    std::cout << "Loading configuration from: " << config_file << std::endl;

    // Load configuration
    auto& config = config::ConfigManager::instance();
    if (!config.loadFromFile(config_file)) {
        std::cerr << "Failed to load configuration file: " << config_file << std::endl;
        std::cerr << "Using default configuration..." << std::endl;
    }

    // Initialize networking
    std::cout << "Initializing networking..." << std::endl;
    auto& net_manager = core::NetworkManager::instance();
    net_manager.initialize();

    // Initialize connection manager
    std::cout << "Setting up connections..." << std::endl;
    auto& conn_manager = controller::ConnectionManager::instance();

    // Create connections from configuration
    const auto& port_mappings = config.getPortMappings();
    for (const auto& [name, mapping] : port_mappings) {
        std::cout << "  - " << name << " ("
                  << (mapping.type == ConnectionType::TCP ? "TCP" :
                      mapping.type == ConnectionType::UDP ? "UDP" : "Multicast")
                  << ")" << std::endl;

        conn_manager.addConnection(name);

        // TODO: Create actual network connections here
        // For now, just create the connection state model
    }

    std::cout << std::endl;
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Mode: " << (config.getMode() == ConnectionMode::Relay ? "Relay" :
                                 config.getMode() == ConnectionMode::Record ? "Record" : "Replay") << std::endl;
    std::cout << "  Visualization: " << (config.getVisualizationMode() == VisualizationMode::Realtime ?
                                          "Real-time" : "Deferred") << std::endl;
    std::cout << "  Recording: " << (config.isRecordingEnabled() ? "Enabled" : "Disabled") << std::endl;
    std::cout << std::endl;

    // Initialize and run GUI
    std::cout << "Starting GUI..." << std::endl;
    view::MainWindow main_window;

    if (!main_window.initialize()) {
        std::cerr << "Failed to initialize main window" << std::endl;
        return 1;
    }

    std::cout << "SwitchSpy is running. Close the window to exit." << std::endl;
    main_window.run();

    // Cleanup
    std::cout << "Shutting down..." << std::endl;
    main_window.shutdown();
    net_manager.shutdown();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
