# SwitchSpy - Network Relay and Visualization Tool

A sophisticated network relay and visualization tool that acts as a man-in-the-middle between network processes, providing real-time visualization, recording, and replay of network data streams with automatic struct visualization.

## Features

### Core Capabilities
- **Network Relay**: Act as a transparent proxy between two network endpoints
- **Multiple Connection Types**: TCP, UDP, and Multicast support
- **Real-time Visualization**: View data flowing through connections in real-time
- **Binary Recording**: Record network sessions to binary files for later replay
- **Replay Mode**: Replay recorded sessions, including multicast emission
- **Struct Visualization**: Automatic rendering of C++ structs using metadata from InterfaceInspector

### Architecture
- **MVC Design**: Clean separation of Model, View, and Controller
- **ImGui Interface**: Modern, responsive GUI with tabbed connections
- **Efficient Networking**: Non-blocking I/O with platform-specific optimizations
- **Dual Visualization Modes**:
  - **Real-time**: Immediate visualization (may impact performance)
  - **Deferred**: Buffered visualization (optimal performance)

### Advanced Features
- **Multicast Spy**: Join multicast groups as passive receiver
- **Tree View**: Hierarchical visualization of data structures
- **Auto-rendering**: Automatic struct member visualization using metadata JSON
- **Statistics Tracking**: Bytes sent/received, packet counts, uptime
- **Configurable**: INI-based configuration for all connections and settings

## Quick Start

### Prerequisites
```bash
# Install dependencies
brew install cmake glfw3  # macOS
# or
sudo apt-get install cmake libglfw3-dev libgl1-mesa-dev  # Linux

# Initialize submodules
git submodule update --init --recursive
```

### Building
```bash
cd SwitchSpy
mkdir build && cd build
cmake ..
cmake --build .
```

### Running
```bash
# Use example configuration
./switchspy ../config/example.ini

# Or specify a custom config
./switchspy /path/to/your/config.ini
```

## Configuration

### Configuration File Format

SwitchSpy uses INI files for configuration. See `config/example.ini` for a complete example.

#### Global Settings
```ini
[global]
mode = relay              # relay, record, replay
visualization = deferred  # realtime, deferred
theme = dark             # dark, light
```

#### Connection Configuration

**TCP Relay:**
```ini
[connection.my_tcp_relay]
type = tcp
listen_port = 8001
forward_host = 192.168.1.100
forward_port = 9001
metadata_file = path/to/struct.meta.json
```

**UDP Relay:**
```ini
[connection.my_udp_relay]
type = udp
listen_port = 8002
forward_host = 192.168.1.100
forward_port = 9002
metadata_file = path/to/struct.meta.json
```

**Multicast Spy:**
```ini
[connection.my_multicast_spy]
type = multicast
multicast_group = 239.1.1.1
multicast_port = 5000
metadata_file = path/to/struct.meta.json
```

#### Recording Configuration
```ini
[recording]
enabled = true
output_dir = ./recordings
auto_save = true
buffer_size = 10485760  # 10MB
```

#### Replay Configuration
```ini
[replay.my_replay]
file = ./recordings/session_2024_11_08.bin
mode = multicast_emit
target_group = 239.1.1.1
target_port = 5000
```

## Usage Scenarios

### Scenario 1: TCP Relay with Visualization

Monitor traffic between a client and server:

1. Configure a TCP relay in `config/my_relay.ini`:
```ini
[connection.client_server_relay]
type = tcp
listen_port = 8080
forward_host = localhost
forward_port = 3000
metadata_file = ../MetaDataGen/MyStruct.meta.json
```

2. Run SwitchSpy:
```bash
./switchspy config/my_relay.ini
```

3. Connect your client to `localhost:8080` instead of `localhost:3000`

4. View data flowing in real-time in the GUI

### Scenario 2: Multicast Spy

Passively monitor multicast traffic:

1. Configure multicast spy:
```ini
[connection.multicast_monitor]
type = multicast
multicast_group = 239.1.1.1
multicast_port = 5000
metadata_file = ../MetaDataGen/DataPacket.meta.json
```

2. Run SwitchSpy - it will join the multicast group and display all packets

### Scenario 3: Record and Replay

Record a session and replay it later:

1. Enable recording:
```ini
[global]
mode = record

[recording]
enabled = true
output_dir = ./recordings
auto_save = true
```

2. Run the relay - data will be recorded to `./recordings/`

3. Later, replay the session:
```ini
[global]
mode = replay

[replay.session_playback]
file = ./recordings/session_2024_11_08.bin
mode = multicast_emit
target_group = 239.1.1.1
target_port = 5000
```

### Scenario 4: Multiple Connections

Monitor multiple connections simultaneously - each gets its own tab in the GUI:

```ini
[connection.web_api]
type = tcp
listen_port = 8080
forward_host = api.example.com
forward_port = 443

[connection.database]
type = tcp
listen_port = 5433
forward_host = localhost
forward_port = 5432

[connection.events]
type = multicast
multicast_group = 239.1.1.1
multicast_port = 5000
```

## GUI Interface

### Main Window
- **Menu Bar**: File, View, Settings
- **Connection Tabs**: One tab per active connection
- **Status Bar**: Global statistics

### Connection Tab
- **Tree View**: Hierarchical view of data packets
- **Hex View**: Raw packet data in hexadecimal
- **Struct View**: Auto-rendered struct members
- **Statistics Panel**: Connection-specific stats

### Struct Visualization

When a metadata file is provided, SwitchSpy automatically visualizes struct members:

```
📦 SimpleStruct (24 bytes)
├─ value: 42 (int, offset: 0, size: 4)
├─ price: 19.99 (double, offset: 8, size: 8)
├─ is_active: true (bool, offset: 16, size: 1)
└─ category: 'A' (char, offset: 17, size: 1)
```

## Architecture

### Components

```
SwitchSpy/
├── src/
│   ├── core/           # Networking (TCP, UDP, Multicast)
│   ├── config/         # INI parsing and configuration
│   ├── model/          # Data models (buffers, state)
│   ├── controller/     # Business logic (relay, recording)
│   ├── view/           # ImGui interface
│   └── serialization/  # Binary format for recording
├── include/switchspy/  # Public headers
├── config/             # Example configurations
└── docs/               # Documentation
```

### Data Flow

```
Process A -> [Listen] -> [Relay Controller] -> [Forward] -> Process B
                              |
                              v
                         [Data Buffer]
                              |
                              v
                    [View (Tree/Hex/Struct)]
                              |
                              v
                    [Binary Serializer] (optional)
```

## Performance

### Real-time Mode
- Direct forwarding with minimal latency
- Visualization may impact performance
- Best for low-throughput connections

### Deferred Mode (Recommended)
- Immediate forwarding (no latency impact)
- Background visualization processing
- Handles high-throughput connections

### Benchmarks
- TCP Relay: < 1ms added latency (deferred mode)
- UDP Relay: < 0.5ms added latency
- Multicast: No impact on original senders
- Recording: ~10-20MB/s sustained write performance

## Development

### Building from Source
```bash
git clone --recurse-submodules <repository-url>
cd SwitchSpy
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### Running Tests
```bash
cd build
ctest --output-on-failure
```

### Adding New Connection Types

1. Inherit from `NetworkConnection` in `include/switchspy/core/`
2. Implement connection logic
3. Register in `NetworkManager`
4. Add configuration parsing in `ConfigManager`

## Troubleshooting

### "Failed to bind": Port already in use
- Check if another process is using the listen port
- Use `lsof -i :<port>` (macOS/Linux) or `netstat -ano` (Windows)

### "Failed to join multicast group"
- Verify multicast address is in range 224.0.0.0 - 239.255.255.255
- Check firewall settings
- Ensure network interface supports multicast

### "Metadata file not found"
- Verify path to `.meta.json` file is correct
- Use absolute paths or paths relative to SwitchSpy executable
- Generate metadata using InterfaceInspector first

### High CPU usage in real-time mode
- Switch to deferred visualization mode
- Reduce visualization update frequency
- Disable auto-rendering for large structs

## Integration with InterfaceInspector

SwitchSpy leverages metadata generated by InterfaceInspector:

1. Use InterfaceInspector to generate metadata:
```bash
cd InterfaceInspector
python header_inspector.py MyStruct.hpp
```

2. Reference the generated JSON in SwitchSpy config:
```ini
[connection.my_relay]
metadata_file = ../MeterfaceInspector/MetaDataGen/MyStruct.meta.json
```

3. SwitchSpy will automatically visualize struct members based on metadata

## License

See LICENSE file in repository root.

## Contributing

Contributions welcome! Please see CONTRIBUTING.md for guidelines.

## Support

For issues and questions:
- GitHub Issues: [repository-url]/issues
- Documentation: See `docs/ARCHITECTURE.md`
