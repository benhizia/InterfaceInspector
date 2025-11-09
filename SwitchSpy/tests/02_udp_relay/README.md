# Test 02: UDP Relay & Connectionless Communication

## Overview
This test validates SwitchSpy's UDP relay capability. Unlike TCP (Test 01), UDP is **connectionless** and **unreliable** - datagrams may be lost, reordered, or duplicated. SwitchSpy must handle these characteristics gracefully while forwarding commands.

**Critical test for:**
- UDP datagram relay (no connection handshake)
- Connectionless forwarding
- Packet loss detection and tracking
- CRC checksum verification
- Out-of-order datagram handling
- Variable-length payload visualization

## Test Scenario

### Network Topology

```
┌────────┐            ┌───────────┐            ┌──────────┐
│ Sender │───UDP────→ │ SwitchSpy │───UDP────→ │ Receiver │
│        │   :8002    │   Relay   │   :9002    │          │
└────────┘            └───────────┘            └──────────┘
```

**Key differences from TCP:**
- No connection establishment (no handshake)
- No guaranteed delivery (packets may be lost)
- No ordering guarantee (packets may arrive out of order)
- No flow control (sender doesn't wait for receiver)
- Each datagram is independent

### Sender Configuration
- **Protocol**: UDP
- **Target**: SwitchSpy relay port (8002)
- **Rate**: 1 command/second
- **Payload**: CommandPacket (84 bytes with variable payload)
- **Commands**: REBOOT, STATUS, CONFIG, DATA (cycling)

### SwitchSpy Configuration
- **Mode**: Relay
- **Type**: UDP
- **Listen**: Port 8002
- **Forward**: localhost:9002

### Receiver Configuration
- **Protocol**: UDP
- **Listen**: Port 9002
- **Validation**: Sequence numbers and CRC32 checksums

### Expected Result
✅ **All commands forwarded with minimal loss**
Some loss acceptable on UDP (it's connectionless).

## Setup Instructions

### 1. Generate Metadata
```cmd
cd SwitchSpy\tests\02_udp_relay
generate_metadata.cmd
```

### 2. Build Test Programs

#### Windows:
```cmd
cd SwitchSpy\tests\02_udp_relay
g++ sender.cpp -o sender.exe -lws2_32
g++ receiver.cpp -o receiver.exe -lws2_32
```

#### Linux/macOS:
```bash
cd SwitchSpy/tests/02_udp_relay
g++ sender.cpp -o sender
g++ receiver.cpp -o receiver
```

### 3. Verify Configuration
Check `switchspy.ini`:
```ini
[connection.udp_relay_test]
type = udp
listen_port = 8002
forward_host = 127.0.0.1
forward_port = 9002
```

## Running the Test

### Step 1: Start Receiver (FIRST!)
```bash
./receiver
```

**Expected output:**
```
===== UDP COMMAND RECEIVER =====
Configuration:
  Listen port: 9002

Listening for UDP commands...
```

### Step 2: Start SwitchSpy
```bash
switchspy switchspy.ini
```

**Expected logs:**
```
[INF] [Main] Starting SwitchSpy
[INF] [Connection] udp udp_relay_test: Listening on port 8002
[INF] [Connection] udp udp_relay_test: Forwarding to 127.0.0.1:9002
```

### Step 3: Start Sender
```bash
./sender
```

**Expected output:**
```
===== UDP COMMAND SENDER =====
Configuration:
  Target: 127.0.0.1:8002
  Send rate: 1 commands/second

Sending commands via UDP...

Seq: 5 | Cmd: REBOOT | Device: 3 | Payload: 42 bytes | Rate: 1 cmd/s | CRC: 0xa3b4c5d6
Seq: 10 | Cmd: STATUS | Device: 7 | Payload: 18 bytes | Rate: 1 cmd/s | CRC: 0x12345678
```

## What to Observe in GUI

### 🔴 Connection Tab → Tree View

**What to click:**
1. Click "udp_relay_test" tab
2. Expand any command packet

**Expected visualization:**
```
📦 UDP Packet #5 (84 bytes)
├─ command_id: 1 (uint8_t)           ← 1 = REBOOT
├─ device_addr: 3 (uint8_t)          ← Target device #3
├─ payload_length: 42 (uint16_t)     ← 42 bytes of data
├─ sequence: 5 (uint32_t)            ← Command #5
├─ timestamp_ms: 1699876543210 (uint64_t)
├─ payload: [64 bytes array]         ← Expandable array
│  ├─ [0]: 0xA3 (uint8_t)
│  ├─ [1]: 0x2F (uint8_t)
│  ├─ [2]: 0x91 (uint8_t)
│  └─ ... (42 bytes used, rest zeroed)
└─ checksum: 0xa3b4c5d6 (uint32_t)   ← CRC32
```

**What to notice:**
- Command ID meanings: 1=REBOOT, 2=STATUS, 3=CONFIG, 4=DATA
- Payload is variable length (0-64 bytes)
- Unused payload bytes are zeroed
- Checksum validates data integrity

### 📊 Statistics Panel

**What to check:**
```
Connection: udp_relay_test (UDP Relay)

Packets A→B: 50      ← Sender to SwitchSpy
Packets B→A: 0       ← No responses (one-way)
Bytes A→B: 4200      ← 50 × 84 bytes
Bytes B→A: 0

Forwarded A→B: 50    ← SwitchSpy to receiver
Forwarded Bytes: 4200

Buffer Utilization: 5%
Dropped Packets: 0   ← Should be 0 for slow rate
```

**Expected:**
- Packets A→B ≈ Forwarded A→B (may differ by ±1-2 due to timing)
- Dropped Packets: 0 at 1 cmd/second (very slow rate)
- Buffer Utilization: <10% (low pressure)

### 🔍 Logs Panel

**Expected log sequence:**

#### 1. Startup
```
[INF] [Main] Starting SwitchSpy
[INF] [Config] Loading configuration from tests/02_udp_relay/switchspy.ini
[INF] [Connection] udp udp_relay_test: Creating UDP relay
[INF] [Connection] udp udp_relay_test: Bound to port 8002
[INF] [Connection] udp udp_relay_test: Forward target: 127.0.0.1:9002
[INF] [Connection] udp udp_relay_test: Ready (connectionless)
```

**Key phrase:** "Ready (connectionless)" - confirms UDP mode.

#### 2. First Command Received
```
[DBG] [Connection] udp udp_relay_test: Received datagram (84 bytes) from 127.0.0.1:54321
[DBG] [Relay] udp udp_relay_test: Forwarding 84 bytes to 127.0.0.1:9002
[DBG] [DataBuffer] Packet #1 added to buffer (size: 84 bytes)
```

**What this shows:** Receive → Forward → Buffer (correct relay flow).

#### 3. Ongoing Commands
```
[DBG] [Connection] Received datagram (84 bytes)
[DBG] [Relay] Forwarded 84 bytes
[DBG] [Parser] Command type: REBOOT (id=1)
[DBG] [Parser] Device: 3, Payload: 42 bytes
[DBG] [Parser] Checksum: 0xa3b4c5d6 (valid)
```

**Color coding:**
- White/Light blue: Normal debug messages
- No warnings expected at this slow rate

### 📈 Performance Panel

**What to observe:**

#### Network Operations
```
network.recv
  Calls: 50
  Avg: 20μs     ← GREEN (UDP recv is very fast)
  Min: 10μs
  Max: 80μs

network.send
  Calls: 50
  Avg: 18μs     ← GREEN (UDP send is instant)
  Min: 12μs
  Max: 65μs
```

**What this tells you:** UDP is faster than TCP (no handshake).

#### Relay Operations
```
relay.forward
  Calls: 50
  Avg: 120μs    ← GREEN (<500μs threshold)
  Min: 80μs
  Max: 250μs
```

**Expected:** GREEN at 1 cmd/second.

#### Buffer Operations
```
buffer.push
  Calls: 50
  Avg: 30μs     ← GREEN (<100μs threshold)
  Min: 20μs
  Max: 75μs
```

**Expected:** All operations GREEN (no stress at this rate).

### 👀 Receiver Console Output

**Expected to see:**
```
Seq: 5 | Recv: 5 | Dropped: 0 (0%) | Cmd: REBOOT | Device: 3 | Payload: 42 bytes
Seq: 10 | Recv: 10 | Dropped: 0 (0%) | Cmd: STATUS | Device: 7 | Payload: 18 bytes
Seq: 15 | Recv: 15 | Dropped: 0 (0%) | Cmd: CONFIG | Device: 5 | Payload: 31 bytes
```

**Critical validation:**
- **Dropped: 0 (0%)** at slow rate
- **Sequence matches sender**
- **No checksum errors** (CRC32 validation passes)

## Validation Checklist

### ✅ Pass Criteria

Monitor for at least 2 minutes (120+ commands):

#### UDP Relay Functionality
- [ ] SwitchSpy logs show "Bound to port 8002"
- [ ] SwitchSpy logs show "Ready (connectionless)"
- [ ] Receiver gets commands (0% drop at slow rate)
- [ ] Sequence numbers match sender (±2)
- [ ] No checksum errors reported

#### Tree View Visualization
- [ ] Can expand command packets
- [ ] All 7 fields visible (command_id, device_addr, payload_length, sequence, timestamp_ms, payload, checksum)
- [ ] Payload array is expandable
- [ ] Variable payload length displayed correctly
- [ ] Checksum shown in hex format

#### Statistics Panel
- [ ] "Packets A→B" ≈ sender's command count
- [ ] "Forwarded A→B" ≈ "Packets A→B" (should match)
- [ ] "Bytes A→B" = Packets × 84
- [ ] "Dropped Packets" = 0 at slow rate
- [ ] "Buffer Utilization" < 10%

#### Logs Panel
- [ ] Saw "[INF] Ready (connectionless)"
- [ ] Saw "[DBG] Received datagram (84 bytes)"
- [ ] Saw "[DBG] Forwarding 84 bytes"
- [ ] Saw "[DBG] Checksum: 0x... (valid)"
- [ ] No errors or warnings

#### Performance Panel
- [ ] `network.recv` green (<50μs for UDP)
- [ ] `network.send` green (<50μs for UDP)
- [ ] `relay.forward` green (<500μs)
- [ ] All operations green

#### Receiver Console
- [ ] Receives commands at ~1 cmd/second
- [ ] Dropped: 0 (0%)
- [ ] No checksum errors
- [ ] Command types cycle: REBOOT → STATUS → CONFIG → DATA → REBOOT...

## UDP-Specific Testing

### Test Packet Loss Resilience

UDP can lose packets. Let's simulate:

#### Fast sending (stress test):
```bash
./sender 127.0.0.1 8002 10  # 10ms = 100 cmd/s
```

**Expected:**
- Possible packet loss (1-5% on localhost, higher on network)
- Receiver shows ">>> DROPPED X packets!"
- SwitchSpy continues forwarding remaining packets
- No crashes or hangs

**In SwitchSpy logs:**
```
[WRN] [Connection] Possible UDP packet loss detected
[DBG] [FlowControl] Receive rate: 8400 bytes/sec (100 packets/sec)
```

**In receiver console:**
```
>>> DROPPED 3 packets! (expected 245, got 248)
Seq: 250 | Recv: 247 | Dropped: 3 (1.2%)
```

**This is NORMAL for UDP!** Loss is expected under stress.

### Test Out-of-Order Delivery

UDP doesn't guarantee ordering. Simulate network reordering:

**On Linux (using tc for network emulation):**
```bash
# Add 50ms delay with 25% reordering
sudo tc qdisc add dev lo root netem delay 50ms reorder 25% 50%

# Run test
./sender
```

**Expected receiver output:**
```
Seq: 8 | Recv: 7 | Dropped: 0
>>> OUT OF ORDER: expected 9, got 7
Seq: 10 | Recv: 9 | Dropped: 0
```

**SwitchSpy behavior:** Should still forward all packets, even if out of order.

**Clean up:**
```bash
sudo tc qdisc del dev lo root
```

### Test Variable Payload Lengths

Sender generates random payload lengths (0-64 bytes):

**In tree view, click multiple packets:**
```
Packet #5:  payload_length: 42
Packet #10: payload_length: 18
Packet #15: payload_length: 31
Packet #20: payload_length: 0   ← Empty payload!
```

**Validation:** Tree view correctly displays all lengths, including 0.

### Test Checksum Detection

Simulate corruption (requires code modification or network tool):

**If a corrupted packet arrives:**

**Receiver console:**
```
>>> CHECKSUM ERROR: expected 0x12345678, got 0xabcdefff
```

**Receiver behavior:** Skip corrupted packet, continue with next.

**SwitchSpy:** Should forward as-is (relay doesn't validate checksums, only receiver does).

## Advanced Testing

### Increase Send Rate

```bash
./sender 127.0.0.1 8002 100  # 100ms = 10 cmd/s
./sender 127.0.0.1 8002 10   # 10ms = 100 cmd/s
./sender 127.0.0.1 8002 1    # 1ms = 1000 cmd/s (very fast!)
```

**Expected behavior at different rates:**

| Rate | Expected Drop % | Buffer Utilization | Performance |
|------|----------------|-------------------|-------------|
| 1 cmd/s | 0% | <5% | All green |
| 10 cmd/s | 0% | <10% | All green |
| 100 cmd/s | 0-2% | 20-40% | Mostly green |
| 1000 cmd/s | 2-10% | 60-90% | Yellow/red warnings |

### Test Multiple Senders

Run multiple senders to the same SwitchSpy port:

**Terminal 1:**
```bash
./sender 127.0.0.1 8002 1000
```

**Terminal 2:**
```bash
./sender 127.0.0.1 8002 1000
```

**Expected:**
- SwitchSpy receives from both (UDP allows multiple sources)
- Sequences will be interleaved
- Receiver may see out-of-order messages
- No crashes

**In receiver console:**
```
>>> OUT OF ORDER: expected 15, got 8 (from different sender)
```

**This is normal:** Multiple independent senders will cause sequence conflicts.

## Troubleshooting

### No packets received
**Problem:** Receiver shows no output

**Causes:**
- SwitchSpy not running
- Wrong port configuration
- Firewall blocking UDP

**Solution:**
1. Check SwitchSpy logs for "Bound to port 8002"
2. Verify ports: sender→8002, relay 8002→9002, receiver←9002
3. Check firewall: `netstat -an | grep 8002` (should show LISTENING)

### High packet loss (>5%)
**Problem:** Receiver shows many dropped packets at slow rate

**Causes:**
- System overload (CPU/memory)
- SwitchSpy buffer too small
- Network issues (if not localhost)

**Solution:**
1. Check Performance panel for red metrics
2. Increase buffer size in switchspy.ini: `buffer_size = 2097152` (2MB)
3. Use deferred mode: `visualization = deferred`
4. Check system load: `top` or Task Manager

### Checksum errors
**Problem:** Receiver reports CRC mismatches

**Causes:**
- Memory corruption
- Buffer overflow in SwitchSpy
- Network corruption (rare on localhost)

**This is a BUG if it happens on localhost!**

**Debug:**
1. Check SwitchSpy logs for buffer overflow warnings
2. Reduce send rate: `./sender 127.0.0.1 8002 2000` (slower)
3. Compare sender CRC output with receiver errors

### Out of order packets (on localhost)
**Problem:** Receiver shows out-of-order messages frequently

**On localhost, this should be RARE (<1%).**

**If frequent (>5%):**
- System scheduler issues
- SwitchSpy buffering incorrectly (bug)
- Multiple senders (expected behavior)

## Performance Expectations

### UDP vs TCP Comparison

| Metric | UDP (Test 02) | TCP (Test 01) | Reason |
|--------|---------------|---------------|--------|
| Connection setup | Instant (0ms) | ~5-10ms | No handshake |
| Recv latency | 10-20μs | 30-50μs | Simpler protocol |
| Send latency | 10-20μs | 30-50μs | No ACK wait |
| Reliability | Best-effort | Guaranteed | No retransmission |
| Ordering | None | Guaranteed | No sequence tracking |

### Expected Performance Metrics

At 1 cmd/second:
- CPU: <1%
- Memory: <5MB
- Network recv: <20μs
- Relay forward: <100μs
- All operations: GREEN

At 100 cmd/second:
- CPU: ~5%
- Memory: <20MB
- Network recv: 20-50μs
- Relay forward: 100-300μs
- Mostly GREEN, occasional YELLOW

At 1000 cmd/second:
- CPU: ~30%
- Memory: <50MB
- Buffer pressure warnings expected
- Some packet loss acceptable (2-5%)
- YELLOW/RED warnings normal

## Interpreting Results

### Connectionless Nature

**When you see:**
```
[INF] [Connection] udp udp_relay_test: Ready (connectionless)
```

**It means:**
- No connection handshake required
- Sender can send immediately without waiting
- Each datagram is independent (no session state)
- SwitchSpy doesn't maintain connection state

**Contrast with TCP:**
```
[INF] [Connection] tcp tcp_basic_test: Connected
```

TCP requires explicit connection establishment.

### Packet Loss is Normal

**UDP characteristics:**
- No delivery guarantee
- No automatic retransmission
- Packets can be lost due to:
  - Network congestion
  - Buffer overflow
  - Routing issues
  - Receiver too slow

**When loss is acceptable:**
- <1% at low rates (1-10 cmd/s)
- <5% at high rates (100-1000 cmd/s)
- Applications expecting loss (video streaming, gaming)

**When loss indicates a problem:**
- >10% on localhost (system issue)
- >20% on LAN (network issue)
- Any loss at 1 cmd/second (severe problem)

### Checksum Validation

**Purpose:** Detect data corruption during transmission.

**Expected:** 0 checksum errors on localhost.

**If errors occur:** Investigate buffer overflow, memory corruption, or hardware issues.

## Next Steps

After validating UDP relay:
- ✅ Test 01: TCP Basic
- ✅ Test 02: UDP Relay (you are here)
- → Test 03: Multicast Broadcast
- → Test 04: High Throughput
- → Test 05: Rate Mismatch
- → Test 06: Complex Structs

## Summary

This test validates that SwitchSpy:
1. ✅ Relays UDP datagrams without requiring connections
2. ✅ Handles connectionless communication correctly
3. ✅ Forwards packets with minimal loss (<1% at normal rates)
4. ✅ Visualizes variable-length payloads correctly
5. ✅ Operates efficiently (faster than TCP due to no handshake)
6. ✅ Detects and tracks packet loss appropriately
7. ✅ Handles high-rate UDP traffic with deferred mode

**Pass criteria:** Receiver gets 98%+ of packets at normal rates, all fields visualized correctly, no crashes under stress.
