# Test 05: Rate Mismatch & Buffer Overflow

## Overview
This test validates SwitchSpy's handling of rate mismatches where the sender transmits data **2x faster** than the receiver can process. This stresses the buffer management and flow control monitoring systems.

**Critical test for:**
- Buffer overflow detection and warnings
- Rate mismatch detection
- Packet drop tracking
- Flow control monitoring
- Deferred vs Real-time visualization performance impact

## Test Scenario

### Sender Configuration
- **Rate**: 100 packets/second (10ms delay)
- **Behavior**: Fast, aggressive transmission

### Receiver Configuration
- **Rate**: 50 packets/second (20ms delay)
- **Behavior**: Slow processing (2x slower than sender)

### Expected Result
⚠️ **Buffer will fill up and drop packets**
This is intentional to test overflow handling!

## Setup Instructions

### 1. Generate Metadata
```cmd
cd SwitchSpy\tests\05_rate_mismatch
..\..\..\python header_inspector.py shared.hpp
copy ..\..\..\MetaDataGen\shared.meta.json .
```

### 2. Build Test Programs
```cmd
g++ sender.cpp -o sender.exe -lws2_32
g++ receiver.cpp -o receiver.exe -lws2_32
```

### 3. Configure SwitchSpy
File: `switchspy.ini`
```ini
[global]
mode = relay
visualization = deferred  # IMPORTANT: Use deferred mode!

[connection.rate_mismatch_test]
type = tcp
listen_port = 8005
forward_host = 127.0.0.1
forward_port = 9005
metadata_file = tests/05_rate_mismatch/shared.meta.json

[recording]
enabled = true
buffer_size = 1048576  # 1MB - will fill up!
```

## Running the Test

### Step 1: Start Receiver (FIRST!)
```bash
./receiver
```

**Expected output:**
```
===== SLOW RECEIVER - Rate Mismatch Test =====
Configuration:
  Listen port: 9005
  Process rate: 50 packets/second
  Processing delay: 20ms

Listening...
```

### Step 2: Start SwitchSpy
```bash
switchspy tests/05_rate_mismatch/switchspy.ini
```

### Step 3: Start Sender
```bash
./sender
```

**Expected output:**
```
===== FAST SENDER - Rate Mismatch Test =====
Configuration:
  Target: 127.0.0.1:8005
  Send rate: 100 packets/second
  Delay between sends: 10ms

Connected! Starting fast transmission...

Seq: 100 | Sent: 100 | Rate: 100 pkt/s | Temp: 23.5°C | Batt: 0%
Seq: 200 | Sent: 200 | Rate: 100 pkt/s | Temp: 27.2°C | Batt: 0%
...
```

## What to Observe in GUI

### 🔴 CRITICAL: Logs Panel (Bottom → "Logs" tab)

This is where you'll see the buffer overflow warnings!

**Expected log sequence:**

#### 1. Normal Startup (first few seconds)
```
[INF] [Main] Starting SwitchSpy
[INF] [Connection] tcp rate_mismatch_test: Listening on port 8005
[INF] [Connection] tcp rate_mismatch_test: Connected
```

#### 2. Buffer Pressure Warnings (after 10-20 seconds)
```
[WRN] [DataBuffer] Buffer at 75.3% capacity (786432/1048576 bytes)
```
**What to click:** Nothing, just observe
**What this means:** Buffer is filling up, system is detecting pressure

#### 3. Critical Buffer Warnings (after 30-40 seconds)
```
[ERR] [DataBuffer] Buffer at 91.2% capacity (956000/1048576 bytes)
[WRN] [DataBuffer] Packet too large for buffer, dropping
```
**Color coding:**
- Yellow/Orange: Warning level (75-90% full)
- Red: Critical level (>90% full)

#### 4. Flow Control Warnings
```
[WRN] [FlowControl] Rate mismatch detected.
Receive: 200 KB/s, Send: 100 KB/s (ratio: 2x)

[WRN] [FlowControl] Sender is significantly faster than receiver.
Consider using deferred visualization mode for better performance.
```

#### 5. Expected Pattern
You should see warnings every 5-10 seconds as buffer continues to overflow.

### 📊 Performance Panel (Bottom → "Performance" tab)

**What to check:**

#### 1. Buffer Operations
Click to expand, look for:
- `buffer.push` - Should show YELLOW or RED
  - Average: 50-150μs (may spike during overflow)
  - Max: Could exceed 1000μs when dropping packets

**What this tells you:** Buffer is under stress

#### 2. Relay Operations
- `relay.forward` - Should remain GREEN
  - Average: < 500μs
  - Deferred mode keeps forwarding fast!

**What this tells you:** Relay isn't slowed down by visualization

#### 3. Network Operations
- `network.send` - Should be GREEN
- `network.recv` - Should be GREEN

**What this tells you:** Network I/O is healthy, bottleneck is receiver

### 📈 Connection Tab → Statistics Panel

**What to observe:**

```
Packets A→B: 5000      ← Sender packets
Packets B→A: 0
Bytes A→B: 120000      ← Total bytes sent
Bytes B→A: 0

Buffer Utilization: 87%  ← Should be HIGH (>75%)
Dropped Packets: 2500    ← Will increase over time!
```

**Click:** Refresh or just watch numbers update

**What HIGH buffer utilization means:**
- 75-85%: Warning zone (expected in this test)
- 85-95%: Critical zone (packet drops occurring)
- >95%: Extreme pressure (many packets dropped)

### 🌳 Tree View (Connection Tab → Left Panel)

**What to check:**

1. **Click** on any packet to expand
2. **Observe** struct members:
```
📦 Packet #1234 (24 bytes)
├─ sequence: 1234 (uint64_t)
├─ sensor_id: 12345 (uint32_t)
├─ temperature_c100: 2350 (int16_t)  ← 23.50°C
├─ humidity_p10: 652 (uint16_t)      ← 65.2%
├─ pressure_pa: 101325 (uint32_t)    ← 101325 Pa
├─ status_flags: 0x00 (uint16_t)
├─ battery_percent: 34 (uint8_t)
└─ reserved: 0 (uint8_t)
```

3. **Notice:** Sequence numbers may have gaps due to dropped packets
   - You might see: 1234 → 1235 → 1240 → 1241 (packets 1236-1239 dropped)

### Receiver Console Output

**Expected to see:**
```
>>> DROPPED 50 packets! (expected 100, got 150)
>>> DROPPED 75 packets! (expected 200, got 275)

Seq: 500 | Recv: 250 | Dropped: 250 (50%) | Rate: 50 pkt/s | Temp: 25.5°C
```

**This proves:**
- Receiver is getting packets (50%)
- But half are being dropped (50% drop rate)
- SwitchSpy is detecting and logging the issue

## Validation Checklist

### ✅ Pass Criteria

Monitor for at least 2 minutes, then verify:

#### Logs Panel
- [ ] Saw "[WRN] Buffer at 75% capacity" (yellow)
- [ ] Saw "[ERR] Buffer at 90% capacity" (red)
- [ ] Saw "[WRN] Rate mismatch detected"
- [ ] Saw "ratio: 2x" or similar in rate mismatch warning

#### Performance Panel
- [ ] `buffer.push` shows yellow/red (>100μs average)
- [ ] `relay.forward` stays green (<500μs)
- [ ] `network.send` and `network.recv` stay green

#### Statistics Panel
- [ ] Buffer Utilization > 75%
- [ ] Dropped Packets > 0 and increasing
- [ ] Packets A→B much higher than Packets B→A

#### Receiver Console
- [ ] Shows "DROPPED X packets" messages
- [ ] Drop rate approximately 50% (±10%)
- [ ] Receive rate approximately 50 pkt/s

### ⚠️ What "Good" Failure Looks Like

This test is **designed to fail gracefully**:

✅ **Correct behavior:**
- Buffer fills up → warnings logged
- Old packets dropped → new packets still forwarded
- Relay continues working → no crashes
- Statistics accurately track drops
- Receiver gets ~50% of packets (the rest dropped)

❌ **Incorrect behavior (bugs):**
- SwitchSpy crashes
- Relay stops forwarding (freezes)
- No warnings in logs
- Receiver gets 0 packets
- Buffer reports 0% utilization (broken monitoring)

## Experimentation

### Adjust Rates

#### Make it worse (3x mismatch):
**Sender:**
```bash
./sender 127.0.0.1 8005 5   # 5ms = 200 pkt/s
```

**Receiver:**
```bash
./receiver 9005 15          # 15ms = 66 pkt/s
```

**Expected:** Even more drops, higher buffer pressure

#### Make it balanced:
**Sender:**
```bash
./sender 127.0.0.1 8005 20  # 20ms = 50 pkt/s
```

**Receiver:**
```bash
./receiver 9005 20          # 20ms = 50 pkt/s
```

**Expected:** No drops, buffer stays low (<25%)

### Test Deferred vs Real-time

#### Current (Deferred):
- Relay latency: < 1ms
- Visualization impact: Minimal
- Buffer: Separate thread

#### Switch to Real-time:
Edit `switchspy.ini`:
```ini
visualization = realtime
```

**Expected differences:**
- `relay.forward` may turn yellow/red (slower)
- More buffer pressure
- Higher drop rate

**This proves deferred mode works better under stress!**

## Interpreting Results

### Buffer Behavior

**Timeline of a typical run:**

| Time | Buffer % | Status | Action |
|------|----------|--------|--------|
| 0-10s | 0-25% | OK | Building up |
| 10-20s | 25-75% | Warning | First warning |
| 20-30s | 75-90% | Critical | Dropping old packets |
| 30s+ | 85-95% | Steady state | Continuous drops |

**Why it stabilizes at 85-95%:**
Ring buffer drops old packets to make room → never exceeds max

### Flow Control Detection

**When you see:**
```
[WRN] Rate mismatch: Receive 200 KB/s, Send 100 KB/s (ratio: 2x)
```

**It means:**
- SwitchSpy is receiving data at 200 KB/s from sender
- SwitchSpy can only forward at 100 KB/s to receiver
- The 2x mismatch triggers the warning (threshold: 2x)

### Performance Impact

**Deferred mode advantage:**

| Operation | Real-time | Deferred |
|-----------|-----------|----------|
| Relay latency | 2-5ms | <1ms |
| Buffer push | 200μs | 50μs |
| GUI responsiveness | Laggy | Smooth |
| Packet loss | Higher | Lower |

**This test proves deferred mode is essential for high-throughput!**

## Troubleshooting

### No warnings in logs
**Problem:** Buffer stays at 0%, no warnings

**Causes:**
- Receiver processing too fast
- Sender not running
- Rate mismatch not significant enough

**Solution:**
- Increase sender rate: `./sender 127.0.0.1 8005 5`
- Decrease receiver rate: `./receiver 9005 50`

### SwitchSpy crashes
**Problem:** Application terminates

**This is a BUG!** Buffer overflow should never crash.

**Report:**
- Check Performance panel for red metrics before crash
- Check Logs panel for errors
- Note the crash timing

### 100% packet loss
**Problem:** Receiver gets 0 packets

**Causes:**
- Receiver not connected
- Wrong ports
- SwitchSpy forward configuration incorrect

**Solution:**
- Check logs for "Connected" message
- Verify ports: sender→8005, SwitchSpy 8005→9005, receiver←9005
- Restart all components in correct order

## Next Steps

After validating buffer overflow behavior:
- ✅ Test 01: TCP Basic (simple case)
- ✅ Test 05: Rate Mismatch (you are here)
- → Test 03: Multicast (one-to-many)
- → Test 06: Complex Structs (nested visualization)

## Summary

This test validates that SwitchSpy:
1. ✅ Detects rate mismatches automatically
2. ✅ Logs buffer pressure warnings at correct thresholds
3. ✅ Handles overflow gracefully (drops old packets)
4. ✅ Continues relaying despite buffer stress
5. ✅ Tracks dropped packets accurately
6. ✅ Performs better in deferred mode under stress

**Pass criteria:** System handles overflow gracefully with appropriate warnings, no crashes, and relay continues functioning.
