# Test 01: TCP Basic Relay

## Overview
This test validates basic TCP relay functionality with simple struct visualization.

**What it tests:**
- TCP connection relay through SwitchSpy
- Automatic struct visualization using metadata
- Real-time packet display in tree view
- Deferred visualization mode performance
- Connection statistics tracking

## Test Components

### Data Structure
```cpp
struct SimpleMessage {
    uint32_t sequence_number;    // Message counter
    uint64_t timestamp_ms;       // When message was sent
    float temperature;           // Simulated sensor data
    uint8_t status;              // 0=OK, 1=Warning, 2=Error
    uint8_t padding[3];          // Alignment padding
};
```

### Network Topology
```
Sender (127.0.0.1:random)
    ↓
SwitchSpy Relay (listen: 8001 → forward: 9001)
    ↓
Receiver (127.0.0.1:9001)
```

## Setup Instructions

### Step 1: Generate Metadata

Run the metadata generation script:

**Windows:**
```cmd
generate_metadata.cmd
```

**Linux/macOS:**
```bash
cd ../../..
python header_inspector.py SwitchSpy/tests/01_tcp_basic/shared.hpp
cp MetaDataGen/shared.meta.json SwitchSpy/tests/01_tcp_basic/
```

This will create `shared.meta.json` containing struct layout information.

### Step 2: Build Test Programs

**Windows:**
```cmd
cd build
cmake --build . --config Release
```

Or manually compile:
```cmd
cl /EHsc sender.cpp /Fe:sender.exe Ws2_32.lib
cl /EHsc receiver.cpp /Fe:receiver.exe Ws2_32.lib
```

**Linux/macOS:**
```bash
g++ -std=c++17 sender.cpp -o sender
g++ -std=c++17 receiver.cpp -o receiver
```

### Step 3: Start SwitchSpy

Navigate to SwitchSpy root and run:

**Windows:**
```cmd
switchspy.exe tests\01_tcp_basic\switchspy.ini
```

**Linux/macOS:**
```bash
./switchspy tests/01_tcp_basic/switchspy.ini
```

## Running the Test

### Terminal 1: Start Receiver
```bash
./receiver
# Or with custom port:
./receiver 9001
```

**Expected output:**
```
TCP Receiver - Test 01
======================
Listening on port: 9001
Waiting for connection...
```

### Terminal 2: Start Sender
```bash
./sender
# Or with custom host/port:
./sender 127.0.0.1 8001
```

**Expected output:**
```
TCP Sender - Test 01
====================
Connecting to: 127.0.0.1:8001
Connected!
Sending messages (Ctrl+C to stop)...
Sent #0 | Temp: 20.0°C | Status: 0
Sent #1 | Temp: 20.5°C | Status: 0
...
```

## SwitchSpy GUI Testing

### What to Check

#### 1. Connection Tab
**Location:** Top panel → `tcp_test_01` tab

**Expected to see:**
- ✅ Connection status: "Connected"
- ✅ Data flowing indicator
- ✅ Packet counter increasing

#### 2. Tree View
**Location:** Connection tab → Left panel

**What to click:**
1. Expand a packet node (click the ▶ arrow)
2. Look for struct members:
   - `sequence_number` (incrementing)
   - `timestamp_ms` (current time)
   - `temperature` (20-70°C range)
   - `status` (0 or 1)

**Expected visualization:**
```
📦 Packet #0 (16 bytes)
├─ sequence_number: 0 (uint32_t, offset: 0)
├─ timestamp_ms: 1699564123456 (uint64_t, offset: 4)
├─ temperature: 20.0 (float, offset: 12)
└─ status: 0 (uint8_t, offset: 16)
   // Status flags (0=OK, 1=Warning, 2=Error)
```

#### 3. Hex View
**Location:** Connection tab → Right panel

**What to check:**
- Raw packet data in hexadecimal
- Should show 16 bytes per SimpleMessage
- Verify byte order matches struct layout

#### 4. Statistics Panel
**Location:** Connection tab → Bottom

**Expected values:**
- Packets A→B: Increasing (sender to receiver)
- Packets B→A: 0 (no reverse traffic)
- Bytes A→B: Increasing by 16 bytes per message
- Total Packets: Matching sender count

#### 5. Logs Panel
**Location:** Bottom panel → "Logs" tab

**What to look for:**
- `[INF] Connection: tcp_test_01 connected`
- `[INF] Relay: Forwarding packets`
- No `[ERR]` or `[WRN]` messages (in normal operation)

**If you see warnings:**
- `[WRN] Buffer at 75% capacity` → Normal if sender is fast
- `[WRN] Rate mismatch detected` → Check if receiver is slow

#### 6. Performance Panel
**Location:** Bottom panel → "Performance" tab

**What to check:**
- `network.send` times: Should be < 1ms (green)
- `network.recv` times: Should be < 1ms (green)
- `relay.forward` times: Should be < 500μs (green)
- `buffer.push/pop` times: Should be < 100μs (green)

**Warning indicators:**
- Yellow: Operation above threshold but acceptable
- Red: Operation significantly slow, may indicate issues

## Expected Behavior

### Normal Operation
1. **Sender connects** to SwitchSpy port 8001
2. **SwitchSpy accepts** connection and forwards to receiver port 9001
3. **Receiver accepts** connection from SwitchSpy
4. **Messages flow**: Sender → SwitchSpy → Receiver
5. **Receiver validates**: Sequence numbers are consecutive
6. **GUI updates**: Tree view shows struct members in real-time

### What Success Looks Like

**Sender console:**
```
Sent #0 | Temp: 20.0°C | Status: 0
Sent #1 | Temp: 20.5°C | Status: 0
Sent #2 | Temp: 21.0°C | Status: 0
...
```

**Receiver console:**
```
Recv #0 | Temp: 20.0°C | Status: 0 | Total: 1 | Errors: 0
Recv #1 | Temp: 20.5°C | Status: 0 | Total: 2 | Errors: 0
Recv #2 | Temp: 21.0°C | Status: 0 | Total: 3 | Errors: 0
...
```

**SwitchSpy GUI:**
- Tree view shows expanding packet list
- Each packet displays all struct members correctly
- Statistics show matching packet counts
- No errors in logs
- Performance metrics all green

## Troubleshooting

### Sender can't connect
**Problem:** `Failed to connect`

**Solutions:**
- Check SwitchSpy is running first
- Verify port 8001 is configured in switchspy.ini
- Check firewall isn't blocking port 8001

### Receiver can't bind
**Problem:** `Bind failed`

**Solutions:**
- Port 9001 may be in use
- Wait 60 seconds for OS to release port
- Use different port: `./receiver 9002` and update switchspy.ini

### No struct visualization
**Problem:** Tree view shows hex data only

**Solutions:**
- Run `generate_metadata.cmd` first
- Check `shared.meta.json` exists in test directory
- Verify `metadata_file` path in switchspy.ini is correct
- Check SwitchSpy logs for metadata loading errors

### Sequence mismatch errors
**Problem:** Receiver reports sequence mismatches

**Possible causes:**
- Buffer overflow (check logs for warnings)
- Network packet loss (unlikely on localhost)
- SwitchSpy crashed/restarted mid-test

**Solutions:**
- Restart all components
- Increase buffer size in switchspy.ini
- Check Performance panel for slow operations

## Test Validation

### Pass Criteria
✅ Sender sends at least 100 messages
✅ Receiver receives all messages with no errors
✅ Sequence numbers are consecutive (no gaps)
✅ SwitchSpy tree view shows struct members correctly
✅ No errors in SwitchSpy logs
✅ Performance metrics are all green
✅ Statistics match (sender count = receiver count)

### Performance Expectations
- **Latency**: < 1ms added by SwitchSpy relay
- **Throughput**: Should handle 2000+ messages/second
- **Memory**: Buffer should stay < 50% full
- **CPU**: SwitchSpy should use < 5% CPU

## Advanced Testing

### Test Reconnection
1. Stop sender (Ctrl+C)
2. Observe SwitchSpy logs: Should show disconnect
3. Restart sender
4. Should reconnect and continue

### Test High Rate
Modify sender to send faster:
```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(1));  // 1ms instead of 500ms
```

- Watch buffer utilization in logs
- Performance panel should show increased load
- Deferred mode should prevent latency impact

### Test Recording
1. Messages are auto-recorded to `./recordings/`
2. Check file exists after test
3. Stop and replay in replay mode

## Cleanup

To stop the test:
1. Press Ctrl+C in sender terminal
2. Receiver will detect disconnect and exit
3. Close SwitchSpy GUI

Files generated:
- `shared.meta.json` - Metadata (keep for future runs)
- `recordings/*.bin` - Session recordings (can delete)
- `build/` - Compiled executables

## Next Steps

After validating this test:
- ✅ Test 02: UDP Relay (connectionless communication)
- ✅ Test 03: Multicast Broadcast (one-to-many)
- ✅ Test 04: High Throughput (stress testing)
- ✅ Test 05: Rate Mismatch (buffer overflow scenarios)
- ✅ Test 06: Complex Structs (nested data structures)
