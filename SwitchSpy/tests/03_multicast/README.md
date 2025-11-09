# Test 03: Multicast Broadcast & Passive Monitoring

## Overview
This test validates SwitchSpy's **passive multicast monitoring** capability. Unlike relay tests, SwitchSpy joins a multicast group as a passive listener without interfering with communication between the sender and receiver.

**Critical test for:**
- Multicast group subscription
- Passive monitoring (spy mode)
- Multiple receivers on same multicast group
- Non-intrusive data capture
- Broadcast telemetry visualization

## Test Scenario

### Network Topology

```
                  ┌─────────────┐
                  │   Sender    │
                  │ (Broadcast) │
                  └──────┬──────┘
                         │
                         │ Multicast Group
                         │ 239.255.0.1:8003
                         │
         ┌───────────────┼───────────────┐
         │               │               │
    ┌────▼────┐    ┌────▼────┐    ┌────▼────┐
    │SwitchSpy│    │Receiver1│    │Receiver2│
    │  (Spy)  │    │(Monitor)│    │(Monitor)│
    └─────────┘    └─────────┘    └─────────┘
```

**Key points:**
- Sender broadcasts to multicast group
- SwitchSpy passively monitors (does NOT forward)
- Receiver(s) also listen on same group
- All listeners receive identical data
- No man-in-the-middle - pure observation

### Sender Configuration
- **Multicast group**: 239.255.0.1
- **Port**: 8003
- **Rate**: 1 broadcast/second
- **Payload**: TelemetryBroadcast (GPS, sensors, status)

### SwitchSpy Configuration
- **Mode**: Spy (passive monitoring)
- **Type**: Multicast
- **Visualization**: Deferred

### Receiver Configuration
- **Joins**: Same multicast group (239.255.0.1:8003)
- **Behavior**: Passive listener, validates sequence

### Expected Result
✅ **All listeners receive identical broadcasts**
This validates that SwitchSpy doesn't interfere!

## Setup Instructions

### 1. Generate Metadata
```cmd
cd SwitchSpy\tests\03_multicast
generate_metadata.cmd
```

**Expected output:**
```
========================================
Generating Metadata for Test 03
========================================

Running header_inspector.py on shared.hpp...
Copying metadata file to test directory...

========================================
Metadata generation complete!
========================================
```

### 2. Build Test Programs

#### Windows:
```cmd
cd SwitchSpy\tests\03_multicast
g++ sender.cpp -o sender.exe -lws2_32
g++ receiver.cpp -o receiver.exe -lws2_32
```

#### Linux/macOS:
```bash
cd SwitchSpy/tests/03_multicast
g++ sender.cpp -o sender
g++ receiver.cpp -o receiver
```

### 3. Verify Configuration
Check that `switchspy.ini` points to the correct metadata file:
```ini
[connection.multicast_test]
type = multicast
multicast_group = 239.255.0.1
multicast_port = 8003
metadata_file = tests/03_multicast/shared.meta.json
```

## Running the Test

### Step 1: Start Receiver (FIRST!)
This validates that the receiver works independently.

**Windows:**
```cmd
receiver.exe
```

**Linux/macOS:**
```bash
./receiver
```

**Expected output:**
```
===== MULTICAST RECEIVER - Passive Listener =====
Configuration:
  Multicast group: 239.255.0.1:8003

Joined multicast group successfully!
Listening for broadcasts...
```

### Step 2: Start SwitchSpy

**Windows:**
```cmd
..\..\build\windows-x64-release\Release\switchspy.exe switchspy.ini
```

**Linux/macOS:**
```bash
../../build/linux-release/switchspy switchspy.ini
```

**Expected logs in SwitchSpy:**
```
[INF] [Main] Starting SwitchSpy
[INF] [Config] Loading configuration from switchspy.ini
[INF] [Connection] multicast multicast_test: Joining group 239.255.0.1:8003
[INF] [Connection] multicast multicast_test: Listening (spy mode)
```

**What to check:** Look for "spy mode" in logs - confirms passive monitoring.

### Step 3: Start Sender

**Windows:**
```cmd
sender.exe
```

**Linux/macOS:**
```bash
./sender
```

**Expected output:**
```
===== MULTICAST SENDER - Broadcast Test =====
Configuration:
  Multicast group: 239.255.0.1:8003
  Broadcast rate: 1 broadcasts/second

Broadcasting telemetry data...

Seq: 10 | Sent: 10 | Rate: 1 bcast/s | GPS: (37.5234, -122.1456) | Sats: 8 | Batt: 3850mV
Seq: 20 | Sent: 20 | Rate: 1 bcast/s | GPS: (37.5289, -122.1401) | Sats: 9 | Batt: 3820mV
Seq: 50 | Sent: 50 | Rate: 1 bcast/s | GPS: (37.5312, -122.1389) | Sats: 7 | Batt: 3790mV [EMERGENCY]
```

## What to Observe in GUI

### 🔴 Connection Tab → Tree View (Left Panel)

**What to click:**
1. Click on "multicast_test" tab at the top
2. Expand any packet in the tree view

**Expected visualization:**
```
📡 Multicast Packet #10 (44 bytes)
├─ source_id: 12345 (uint32_t)
├─ sequence: 10 (uint64_t)
├─ timestamp_ms: 1699876543210 (uint64_t)
├─ latitude_e6: 37523456 (int32_t)         ← 37.523456°N
├─ longitude_e6: -122145678 (int32_t)      ← -122.145678°W
├─ altitude_m: 125 (int16_t)               ← 125 meters
├─ speed_cms: 850 (uint16_t)               ← 8.5 m/s
├─ heading_deg: 245 (uint16_t)             ← 245° (SW)
├─ battery_mv: 3850 (uint16_t)             ← 3.85V
├─ signal_dbm: -75 (int8_t)                ← -75 dBm
├─ satellite_count: 8 (uint8_t)            ← 8 satellites
├─ status_flags: 0x01 (uint8_t)            ← GPS fix
└─ padding: 0 (uint8_t)
```

**What to notice:**
- GPS coordinates are displayed in millionths of degrees (divide by 1e6)
- Battery voltage in millivolts (3850 = 3.85V)
- Status flags: 0x01 = GPS fix, 0x03 = GPS fix + Low battery, 0x05 = GPS fix + Emergency

### 📊 Connection Tab → Statistics Panel (Right)

**What to check:**
```
Direction: Receive Only (Multicast)
Packets Received: 50
Bytes Received: 2200  (50 × 44 bytes)

Multicast Group: 239.255.0.1:8003
Mode: Passive Spy

Buffer Utilization: 2%
Dropped Packets: 0
```

**Expected values:**
- Direction: Should say "Receive Only" or "Multicast"
- Mode: Should explicitly show "Passive Spy" or "Spy Mode"
- Packets Received: Should match sender's broadcast count (±1-2)
- Bytes: Should be Packets × 44 bytes
- Dropped Packets: Should be 0 (1 broadcast/second is very slow)

**Click:** Refresh button to update counts

### 🔍 Logs Panel (Bottom → "Logs" tab)

**Expected log sequence:**

#### 1. Startup (when SwitchSpy starts)
```
[INF] [Main] Starting SwitchSpy
[INF] [Config] Loading configuration from tests/03_multicast/switchspy.ini
[INF] [Config] Found connection: multicast_test
[INF] [Connection] multicast multicast_test: Creating multicast listener
[INF] [Connection] multicast multicast_test: Joining group 239.255.0.1:8003
[INF] [Connection] multicast multicast_test: Joined successfully
[INF] [Connection] multicast multicast_test: Operating in SPY MODE
```

**What this tells you:** SwitchSpy successfully joined the multicast group as a passive observer.

#### 2. First Broadcast Received
```
[DBG] [Connection] multicast multicast_test: Received 44 bytes from 192.168.1.100:54321
[DBG] [DataBuffer] Packet #1 added to buffer (size: 44 bytes)
```

**What to check:** Source IP should be the sender's IP (may be localhost or LAN IP).

#### 3. Ongoing Monitoring (every few broadcasts)
```
[DBG] [Connection] multicast multicast_test: Received 44 bytes
[DBG] [FlowControl] Receive rate: 44 bytes/sec (1 packet/sec)
```

**Expected pattern:** Logs appear once per second as broadcasts arrive.

#### 4. Special Events (every 50th broadcast)
```
[DBG] [Parser] Status flags: 0x05 (GPS fix + EMERGENCY)
[WRN] [Connection] Emergency flag detected in broadcast!
```

**What this means:** Sender triggers emergency flag every 50 broadcasts (simulated distress signal).

### 📈 Performance Panel (Bottom → "Performance" tab)

**What to observe:**

#### Network Operations
Click to expand "Network Operations":
```
network.recv
  Calls: 50
  Avg: 15μs     ← GREEN (very fast for multicast)
  Min: 8μs
  Max: 45μs
  Total: 750μs
```

**What this tells you:** Multicast receive is extremely fast (no TCP handshake overhead).

#### Buffer Operations
```
buffer.push
  Calls: 50
  Avg: 25μs     ← GREEN (well below 100μs threshold)
  Min: 18μs
  Max: 60μs
```

**What this tells you:** Buffer has no pressure (1 broadcast/sec is very slow).

#### Parsing Operations
```
parser.visualize
  Calls: 50
  Avg: 80μs     ← GREEN
  Min: 60μs
  Max: 120μs
```

**What this tells you:** Deferred mode keeps parsing fast without blocking reception.

#### Expected Color Codes
- **All operations**: GREEN (< threshold)
- **If any yellow/red**: Something is wrong (multicast should be very efficient)

### 👀 Receiver Console Output

**Expected to see:**
```
Seq: 10 | Recv: 10 | Dropped: 0 (0%) | GPS: (37.5234, -122.1456) | Alt: 125m | Sats: 8
Seq: 20 | Recv: 20 | Dropped: 0 (0%) | GPS: (37.5289, -122.1401) | Alt: 132m | Sats: 9
Seq: 50 | Recv: 50 | Dropped: 0 (0%) | GPS: (37.5312, -122.1389) | Alt: 118m | Sats: 7 [EMERGENCY]
```

**Critical validation:**
- **Dropped: 0 (0%)** - Receiver should get ALL broadcasts
- **Sequence matches sender** - Receiver sequence should match sender exactly
- **GPS coordinates match** - Coordinates in receiver should match sender console

**This proves:**
1. SwitchSpy doesn't interfere with multicast communication
2. Multiple listeners can coexist on the same multicast group
3. All listeners receive identical data

## Validation Checklist

### ✅ Pass Criteria

Monitor for at least 2 minutes (120+ broadcasts), then verify:

#### Multicast Functionality
- [ ] SwitchSpy logs show "Joined group 239.255.0.1:8003"
- [ ] SwitchSpy logs show "Operating in SPY MODE"
- [ ] Receiver successfully receives broadcasts (0% drop rate)
- [ ] Receiver and sender sequence numbers match
- [ ] No "forwarding" messages in logs (spy mode doesn't forward)

#### Tree View Visualization
- [ ] Can expand broadcast packets in tree view
- [ ] All 12 fields of TelemetryBroadcast are displayed
- [ ] GPS coordinates are visible (latitude_e6, longitude_e6)
- [ ] Status flags displayed (0x01 = GPS fix, 0x05 = GPS fix + emergency)
- [ ] Field types are correct (uint32_t, int32_t, etc.)

#### Statistics Panel
- [ ] "Packets Received" matches sender's broadcast count (±2)
- [ ] "Bytes Received" = Packets × 44 bytes
- [ ] "Dropped Packets" = 0
- [ ] "Mode" shows "Spy" or "Passive"
- [ ] "Direction" shows "Receive Only" or similar

#### Logs Panel
- [ ] Saw "[INF] Joined group 239.255.0.1:8003"
- [ ] Saw "[INF] Operating in SPY MODE"
- [ ] Saw "[DBG] Received 44 bytes" messages
- [ ] Saw "[WRN] Emergency flag detected" every ~50 broadcasts
- [ ] No errors or warnings about multicast join failures

#### Performance Panel
- [ ] `network.recv` shows green (<50μs average for multicast)
- [ ] `buffer.push` shows green (<100μs)
- [ ] All operations green (no yellow/red)

#### Receiver Console
- [ ] Shows "Joined multicast group successfully!"
- [ ] Receive rate approximately 1 broadcast/second
- [ ] Dropped: 0 (0%) throughout the test
- [ ] GPS coordinates displayed and changing
- [ ] Emergency flag appears every 50th broadcast

### ⚠️ What Success Looks Like

This test validates **non-intrusive passive monitoring**:

✅ **Correct behavior:**
- SwitchSpy joins multicast group successfully
- Receiver also joins and receives all broadcasts
- Both SwitchSpy and receiver get identical data
- 0% packet loss for receiver (nothing is blocked)
- No "relay" or "forward" operations (spy mode only observes)
- All broadcasts visualized correctly in tree view
- Emergency flags detected and logged appropriately

❌ **Incorrect behavior (bugs):**
- SwitchSpy fails to join multicast group
- Receiver can't join when SwitchSpy is running (port conflict)
- Receiver drops packets when SwitchSpy is active (interference)
- SwitchSpy logs show "forwarding" messages (wrong mode)
- Tree view doesn't show all 12 fields
- GPS coordinates not displayed or incorrect

## Advanced Testing

### Test with Multiple Receivers

You can run multiple receiver instances to prove they all coexist:

**Terminal 1:**
```bash
./receiver
```

**Terminal 2:**
```bash
./receiver 239.255.0.1 8003
```

**Terminal 3:**
```bash
# Run SwitchSpy GUI
switchspy switchspy.ini
```

**Expected:** All three receive identical broadcasts with 0% drop rate.

### Adjust Broadcast Rate

#### Faster broadcasts (10 broadcasts/second):
```bash
./sender 239.255.0.1 8003 100  # 100ms delay = 10 bcast/s
```

**Expected:**
- Still 0% drops (multicast is efficient)
- Buffer utilization remains low (<10%)
- Network operations stay green

#### Very fast (100 broadcasts/second):
```bash
./sender 239.255.0.1 8003 10  # 10ms delay = 100 bcast/s
```

**Expected:**
- Possible drops on receiver (depends on system)
- SwitchSpy should still receive all (deferred mode advantage)
- Buffer utilization increases but stays manageable

### Emergency Broadcast Detection

Watch for broadcasts with emergency flags:

**In sender console:**
```
Seq: 50 | ... | [EMERGENCY]
Seq: 100 | ... | [EMERGENCY]
Seq: 150 | ... | [EMERGENCY]
```

**In SwitchSpy logs:**
```
[WRN] [Connection] Emergency flag detected in broadcast #50
[WRN] [Connection] Emergency flag detected in broadcast #100
```

**In tree view:**
Click on packet #50:
```
├─ status_flags: 0x05 (uint8_t)  ← 0x05 = 0x01 (GPS) | 0x04 (Emergency)
```

**This validates:** Flag parsing and special event detection work correctly.

### Low Battery Detection

Sender occasionally sets low battery flag when battery_mv < 3300:

**In tree view:**
```
├─ battery_mv: 3250 (uint16_t)    ← Below 3.3V
├─ status_flags: 0x03 (uint8_t)   ← 0x03 = 0x01 (GPS) | 0x02 (Low battery)
```

**In logs:**
```
[WRN] [Connection] Low battery detected: 3250mV
```

## Troubleshooting

### Multicast join fails
**Problem:** SwitchSpy logs show "Failed to join multicast group"

**Causes:**
- Firewall blocking multicast traffic
- No network interface available
- Multicast routing not enabled

**Solution (Windows):**
```cmd
# Check Windows Firewall
# Allow multicast traffic for switchspy.exe
```

**Solution (Linux):**
```bash
# Check multicast routing
ip route show | grep 224.0.0.0
# Should show: 224.0.0.0/4 dev eth0 ...

# If missing, add multicast route:
sudo ip route add 224.0.0.0/4 dev eth0
```

### Receiver can't join when SwitchSpy is running
**Problem:** Receiver shows "Bind failed" or "Address already in use"

**Cause:** SO_REUSEADDR not set properly (bug in code)

**Validation:** Both SwitchSpy and receiver should set SO_REUSEADDR to allow multiple binds to the same multicast port. Check both codebases for this socket option.

### Receiver drops packets when SwitchSpy is active
**Problem:** Receiver shows dropped packets, but sender console shows continuous broadcasts

**This is a BUG!** SwitchSpy should NOT interfere with multicast traffic.

**Debug steps:**
1. Run sender + receiver WITHOUT SwitchSpy → Should be 0% drops
2. Run sender + receiver WITH SwitchSpy → If drops appear, SwitchSpy is interfering
3. Check SwitchSpy logs for "forwarding" messages (wrong mode)
4. Verify switchspy.ini has `mode = spy` (not relay)

### No broadcasts received
**Problem:** SwitchSpy and receiver both show no packets

**Causes:**
- Sender not running
- Wrong multicast group/port
- Firewall blocking

**Solution:**
1. Verify sender is running and printing "Broadcasting..."
2. Check sender, receiver, and switchspy.ini all use same group (239.255.0.1:8003)
3. Try pinging multicast group: `ping 239.255.0.1` (some systems support this)
4. Check `netstat` for multicast memberships

### SwitchSpy shows "Relay" or "Forward" in logs
**Problem:** Logs show relaying/forwarding messages

**This is a BUG!** Spy mode should ONLY observe, never forward.

**Cause:** Configuration error or mode detection bug

**Solution:**
1. Verify switchspy.ini has `mode = spy`
2. Check connection type is `multicast` (not tcp/udp)
3. No `forward_host` or `forward_port` should be specified for multicast spy

## Performance Expectations

### Network Performance
| Metric | Expected | Reason |
|--------|----------|--------|
| Multicast join time | <100ms | Fast group membership |
| Receive latency | <50μs | Direct kernel delivery |
| Buffer push | <50μs | Low rate, no contention |
| Parse & visualize | <100μs | Simple struct, deferred mode |

### System Load
- **CPU usage**: <1% (1 broadcast/second is very light)
- **Memory**: <10MB (minimal buffering needed)
- **Network**: <50 bytes/sec (44 bytes/broadcast)

### Scaling Expectations
| Broadcast Rate | Expected Behavior |
|----------------|-------------------|
| 1 bcast/s | All green, 0% drops, <1% CPU |
| 10 bcast/s | All green, 0% drops, <5% CPU |
| 100 bcast/s | Mostly green, possible drops on slow systems |
| 1000 bcast/s | Buffer pressure warnings, drops expected |

## Interpreting Results

### Multicast Group Membership

**When you see:**
```
[INF] [Connection] multicast multicast_test: Joined group 239.255.0.1:8003
[INF] [Connection] multicast multicast_test: Operating in SPY MODE
```

**It means:**
- SwitchSpy successfully subscribed to multicast group
- Network interface configured kernel-level multicast filters
- All broadcasts to this group will be delivered to SwitchSpy
- SwitchSpy will NOT forward (spy mode is passive only)

### Multiple Receivers Coexistence

**Key principle:** Multicast allows multiple receivers on the same group.

**Evidence of correct behavior:**
1. Both receiver console and SwitchSpy show identical sequence numbers
2. Both show 0% drop rate
3. Both receive at same rate (~1 broadcast/second)
4. No conflicts or "address in use" errors

**If this fails:** There's a fundamental multicast implementation bug.

### Spy Mode vs Relay Mode

**Spy Mode (this test):**
- Joins multicast group as passive observer
- ONLY receives (no forwarding)
- Logs should show "Operating in SPY MODE"
- Statistics show "Receive Only"

**Relay Mode (other tests):**
- Acts as man-in-the-middle (A → SwitchSpy → B)
- Receives AND forwards
- Logs show "Forwarding X bytes"
- Statistics show bidirectional counts

**This test validates:** SwitchSpy can operate in pure observation mode without becoming a relay.

## Next Steps

After validating multicast spy behavior:
- ✅ Test 01: TCP Basic (simple relay)
- → Test 02: UDP Relay (connectionless)
- ✅ Test 03: Multicast Broadcast (you are here)
- → Test 04: High Throughput (stress test)
- ✅ Test 05: Rate Mismatch (buffer overflow)
- → Test 06: Complex Structs (nested visualization)

## Summary

This test validates that SwitchSpy:
1. ✅ Joins multicast groups successfully
2. ✅ Operates in passive spy mode (no forwarding)
3. ✅ Doesn't interfere with other multicast receivers
4. ✅ Visualizes broadcast data correctly in tree view
5. ✅ Detects and logs special events (emergency, low battery)
6. ✅ Handles multicast efficiently with minimal overhead
7. ✅ Supports multiple receivers on the same group

**Pass criteria:** SwitchSpy and receiver both receive 100% of broadcasts with no interference or conflicts. All broadcasts visualized correctly with proper flag detection.
