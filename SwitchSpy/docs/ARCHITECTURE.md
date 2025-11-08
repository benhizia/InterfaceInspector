# SwitchSpy Architecture

## Overview
SwitchSpy is a network relay and visualization tool that acts as a man-in-the-middle between network processes, enabling real-time visualization, recording, and replay of network data streams.

## Architecture Pattern: MVC

### Model Layer
**Location**: `src/model/`, `include/switchspy/model/`

#### Components:
- **ConnectionState**: Manages state of individual connections
- **DataBuffer**: Ring buffer for efficient packet storage
- **SessionData**: Aggregates connection metadata and statistics
- **ReplayData**: Stores recorded session data

#### Responsibilities:
- Data storage and state management
- Statistics tracking (bytes sent/received, packet counts)
- Data buffer management for both real-time and deferred modes

### Controller Layer
**Location**: `src/controller/`, `include/switchspy/controller/`

#### Components:
- **ConnectionManager**: Orchestrates all active connections
- **RelayController**: Handles packet forwarding logic
- **RecordingController**: Manages binary serialization/deserialization
- **ReplayController**: Handles replay of recorded sessions

#### Responsibilities:
- Connection lifecycle management
- Routing and forwarding logic
- Mode switching (real-time/deferred/replay)
- Integration with networking core and serialization

### View Layer
**Location**: `src/view/`, `include/switchspy/view/`

#### Components:
- **MainWindow**: ImGui main application window
- **ConnectionTab**: Individual tab for each connection/service
- **TreeViewer**: Tree view for data visualization
- **StructRenderer**: Auto-renders structs using metadata JSON
- **StatsPanel**: Statistics and monitoring panel

#### Responsibilities:
- ImGui rendering and user interaction
- Tabbed interface management
- Real-time and deferred visualization
- Tree view for hierarchical data display
- Integration with metadata for struct visualization

## Core Layer

### Networking Core
**Location**: `src/core/`, `include/switchspy/core/`

#### Components:
- **NetworkConnection**: Base class for all connection types
- **TCPConnection**: TCP relay implementation
- **UDPConnection**: UDP relay implementation
- **MulticastConnection**: Multicast group handling
- **NetworkManager**: Central networking coordinator

#### Features:
- Non-blocking I/O with epoll/kqueue/IOCP
- Efficient packet forwarding with zero-copy where possible
- Connection pool management
- Multicast group subscription/publication

### Serialization
**Location**: `src/serialization/`, `include/switchspy/serialization/`

#### Components:
- **BinarySerializer**: Efficient binary format for recording
- **BinaryDeserializer**: Replay from binary format
- **PacketFormatter**: Packet framing for serialization

#### Binary Format:
```
[Header: 16 bytes]
  - Magic: 4 bytes (0x53574943 'SWIC')
  - Version: 2 bytes
  - Flags: 2 bytes
  - Timestamp: 8 bytes

[Session Info: Variable]
  - Connection type: 1 byte (TCP/UDP/Multicast)
  - Source address: Variable
  - Destination address: Variable
  - Port mapping: 4 bytes

[Packet Records: Repeated]
  - Timestamp: 8 bytes
  - Direction: 1 byte (A->B or B->A)
  - Length: 4 bytes
  - Data: Variable
```

### Configuration
**Location**: `src/config/`, `include/switchspy/config/`

#### Components:
- **IniParser**: Parse INI configuration files
- **ConfigManager**: Global configuration state
- **PortMapping**: Port and address mapping configuration

#### INI Format:
```ini
[global]
mode = relay              # relay, replay, record
visualization = realtime  # realtime, deferred
theme = dark

[connection.tcp_relay_1]
type = tcp
listen_port = 8001
forward_host = 192.168.1.100
forward_port = 9001
metadata_file = SimpleStruct.meta.json

[connection.multicast_spy_1]
type = multicast
multicast_group = 239.1.1.1
multicast_port = 5000
metadata_file = ComplexStruct.meta.json

[recording]
enabled = true
output_dir = ./recordings
auto_save = true
buffer_size = 10485760  # 10MB

[replay.session_1]
file = ./recordings/session_2024_11_08.bin
mode = multicast_emit
target_group = 239.1.1.1
target_port = 5000
```

## Data Flow

### Real-time Relay Mode
```
Process A -> SwitchSpy:ListenPort -> [NetworkCore] -> [RelayController]
                                                            |
                                                            v
                                                    [DataBuffer (minimal)]
                                                            |
                                                            v
                                                    [View (async update)]
                                                            |
                                                            v
                                        [NetworkCore] -> Process B:ForwardPort
```

### Deferred Visualization Mode
```
Process A -> SwitchSpy:ListenPort -> [NetworkCore] -> [RelayController]
                                                            |
                                                            v
                                                    [Fast Forward to B]
                                                            |
                                                            v
                                                    [DataBuffer (queued)]
                                                            |
                                                            v
                                        [View thread (batch processing)]
```

### Replay Mode
```
[BinaryFile] -> [ReplayController] -> [DataBuffer]
                                            |
                                            v
                                    [View (visualization)]
                                            |
                                            v
                                [NetworkCore (emit multicast)]
```

## Performance Considerations

### Real-time Mode
- Direct packet forwarding with minimal buffering
- Lock-free ring buffers for view updates
- Async view rendering on separate thread

### Deferred Mode
- Immediate forwarding to prevent latency
- Background buffering for visualization
- Batch processing of visualization data

### Zero-Copy Optimization
- Use of `sendfile()` / `splice()` on Linux
- Memory-mapped buffers for large transfers
- Scatter-gather I/O where supported

## Thread Model

### Main Thread
- ImGui rendering
- User input handling
- Configuration management

### Network Thread Pool
- Connection accept/connect
- Packet receive/send
- Non-blocking I/O multiplexing

### Serialization Thread
- Background recording
- Async file I/O

### Visualization Thread (Deferred Mode)
- Packet parsing
- Struct deserialization
- Tree view updates

## Metadata Integration

### Struct Visualization
Uses existing InterfaceInspector metadata JSON files:

```cpp
// Load metadata
MetadataParser parser("SimpleStruct.meta.json");

// Auto-render in tree view
void StructRenderer::render(const void* data) {
    for (const auto& member : parser.getMembers()) {
        ImGui::TreeNodeEx(member.name.c_str());

        // Cast data based on metadata
        const void* member_data = (char*)data + member.offset;
        renderMember(member_data, member.type, member.size);

        // Show documentation
        ImGui::TextDisabled("// %s", member.doc.c_str());
    }
}
```

## Dependencies

### External Libraries
- **nlohmann/json**: JSON parsing for metadata
- **ImGui**: GUI framework
- **inih**: INI parsing library

### Platform-Specific
- **Linux**: epoll, splice, sendfile
- **macOS**: kqueue
- **Windows**: IOCP

## Build System

CMake-based build with:
- C++17 standard
- Platform detection
- Optional ImGui backends (GLFW, SDL2)
- Submodule support for dependencies

## Extension Points

### Custom Serializers
Implement `ISerializer` interface for custom formats (JSON, Protobuf, etc.)

### Custom Renderers
Implement `IRenderer` interface for custom visualization

### Protocol Analyzers
Plugin system for protocol-specific parsing (HTTP, custom protocols)

## Security Considerations

- **Local binding only by default**: Prevent unauthorized external access
- **Recording encryption**: Optional encryption for sensitive data
- **Access control**: Configuration-based connection filtering
- **Memory safety**: Bounds checking on all buffer operations
