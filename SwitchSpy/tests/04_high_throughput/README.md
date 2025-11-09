# Test 04: High Throughput & Performance Stress Testing

## Overview
This test validates SwitchSpy's performance under **extreme load** - simulating high-frequency market data feeds at 10,000+ ticks/second. This stress test proves that deferred visualization mode maintains relay performance even when processing massive data volumes.

**Critical test for:**
- High-throughput relay performance (>10K messages/sec)
- Deferred mode advantage under stress
- Buffer management at scale
- GUI responsiveness under load
- System resource usage (CPU, memory)
- Latency consistency (no degradation)

## Test Scenario

### Use Case: High-Frequency Trading (HFT) Market Data

This test simulates a **real-world HFT scenario**:
- Market exchanges send ticks every 100 microseconds
- Each tick contains bid/ask prices, sizes, and trade data
- Systems must process 10,000-50,000 ticks/second
- Latency must remain <1ms (critical for trading algorithms)
- Any delay can cost millions in trading losses

```
┌────────┐            ┌───────────┐            ┌──────────┐
│Exchange│─10K tps──→ │ SwitchSpy │─10K tps──→ │ Trading  │
│  Feed  │            │   Relay   │            │Algorithm │
└────────┘            └───────────┘            └──────────┘
```

### Sender Configuration
- **Protocol**: TCP (reliable delivery required)
- **Rate**: 10,000 ticks/second (100μs delay)
- **Throughput**: ~0.7 MB/second
- **Payload**: MarketTick (72 bytes: prices, sizes, timestamps)

### SwitchSpy Configuration
- **Mode**: Relay (deferred visualization REQUIRED)
- **Buffer**: 10MB (large to handle bursts)
- **Visualization**: Deferred (critical for performance)

### Receiver Configuration
- **Processing**: Fast consumption (keep up with sender)
- **Validation**: Sequence tracking, drop detection

### Expected Result
✅ **Relay maintains <1ms latency with 0-2% drops**
Higher drop rates may occur on slower systems.

## Setup Instructions

### 1. Generate Metadata
```cmd
cd SwitchSpy\tests\04_high_throughput
generate_metadata.cmd
```

### 2. Build Test Programs

#### Windows:
```cmd
cd SwitchSpy\tests\04_high_throughput
g++ sender.cpp -o sender.exe -lws2_32 -O3
g++ receiver.cpp -o receiver.exe -lws2_32 -O3
```

**IMPORTANT:** Use `-O3` optimization flag for maximum performance!

#### Linux/macOS:
```bash
cd SwitchSpy/tests/04_high_throughput
g++ sender.cpp -o sender -O3 -pthread
g++ receiver.cpp -o receiver -O3 -pthread
```

### 3. Verify Configuration
Check `switchspy.ini`:
```ini
[global]
visualization = deferred  # CRITICAL: Must be deferred!

[connection.throughput_test]
buffer_size = 10485760  # 10MB for high throughput
```

## Running the Test

### Step 1: Start Receiver (FIRST!)
```bash
./receiver
```

**Expected output:**
```
===== HIGH THROUGHPUT RECEIVER - Market Data Consumer =====
Configuration:
  Listen port: 9004

Listening for connections...
```

### Step 2: Start SwitchSpy
```bash
switchspy switchspy.ini
```

**Expected logs:**
```
[INF] [Main] Starting SwitchSpy
[INF] [Config] Visualization mode: DEFERRED
[INF] [Connection] tcp throughput_test: Listening on port 8004
[INF] [Connection] tcp throughput_test: Forwarding to 127.0.0.1:9004
[WRN] [Config] High throughput detected: Buffer size 10MB
```

### Step 3: Start Sender (High Rate!)
```bash
./sender 127.0.0.1 8004 100  # 100μs = 10,000 ticks/s
```

**Expected output:**
```
===== HIGH THROUGHPUT SENDER - Market Data Feed =====
Configuration:
  Target: 127.0.0.1:8004
  Tick rate: 10000 ticks/second
  Throughput: 0.69 MB/s
  Delay: 100 microseconds

Connected! Starting high-frequency transmission...

Seq: 1000 | Sent: 1000 | Rate: 10050 ticks/s | Avg: 10021 ticks/s | Throughput: 0.69 MB/s
Seq: 2000 | Sent: 2000 | Rate: 9980 ticks/s | Avg: 10005 ticks/s | Throughput: 0.69 MB/s
...
```

## What to Observe in GUI

### 🔴 CRITICAL: Performance Panel (Bottom → "Performance" tab)

**This is THE most important panel for this test!**

**What to observe:**

#### Network Operations
```
network.recv
  Calls: 10000
  Avg: 25μs     ← Must stay GREEN
  Min: 10μs
  Max: 150μs    ← Occasional spikes OK
  Total: 250ms

network.send
  Calls: 10000
  Avg: 30μs     ← Must stay GREEN
  Min: 12μs
  Max: 180μs
```

**Expected:** GREEN (averages <50μs).

**If YELLOW/RED:** System is struggling - consider:
- Slower sender rate: `./sender 127.0.0.1 8004 200` (5K ticks/s)
- More powerful hardware
- Disable other applications

#### Relay Operations (CRITICAL METRIC!)
```
relay.forward
  Calls: 10000
  Avg: 350μs    ← Should be GREEN (target <500μs)
  Min: 200μs
  Max: 850μs    ← Occasional spikes acceptable
```

**Color thresholds:**
- GREEN: <500μs (GOOD - relay is keeping up)
- YELLOW: 500-1000μs (WARNING - approaching limit)
- RED: >1000μs (CRITICAL - relay is bottlenecked)

**If consistently RED:** Switch to deferred mode (should already be set).

#### Buffer Operations
```
buffer.push
  Calls: 10000
  Avg: 75μs     ← May be YELLOW (acceptable under stress)
  Min: 40μs
  Max: 250μs    ← High spikes expected
```

**Expected:** YELLOW is acceptable at 10K ticks/s.

**If RED (>100μs average):** Buffer pressure is high - expected at this rate.

#### GUI Render (Check this to see visualization cost)
```
gui.render
  Calls: 60     ← ~60 FPS
  Avg: 8000μs   ← ~8ms per frame (120 FPS = 8.3ms budget)
  Min: 5000μs
  Max: 15000μs
```

**What this proves:** GUI rendering is SEPARATE from relay (deferred mode).

**If gui.render affects relay.forward:** Deferred mode is NOT working (bug!).

### 📊 Statistics Panel

**What to check:**
```
Connection: throughput_test (TCP Relay)

Packets A→B: 50000       ← Sender ticks
Packets B→A: 0
Bytes A→B: 3600000       ← 50K × 72 bytes
Bytes B→A: 0

Forwarded A→B: 49800     ← May be slightly less
Forwarded Bytes: 3585600

Buffer Utilization: 45%  ← Should stabilize between 30-70%
Dropped Packets: 200     ← Some drops expected under stress

Rate: 10021 pkt/s        ← Should match sender (~10K)
Throughput: 0.69 MB/s
```

**Expected behavior:**
- Buffer Utilization: 30-70% (stable, not growing)
- Dropped Packets: 0-2% of total (acceptable under stress)
- Rate: Within 5% of sender rate

**WARNING SIGNS:**
- Buffer Utilization climbing toward 100% (will crash eventually)
- Dropped Packets >5% (relay can't keep up)
- Rate <9000 pkt/s (falling behind)

### 🔍 Logs Panel

**Expected log patterns:**

#### Startup
```
[INF] [Main] Starting SwitchSpy
[INF] [Config] Visualization mode: DEFERRED
[WRN] [Config] Buffer size 10MB (high throughput mode)
[INF] [Connection] tcp throughput_test: Ready for high-frequency data
```

#### During High Load (first 10 seconds)
```
[DBG] [FlowControl] Receive rate: 720000 bytes/sec (10000 packets/sec)
[DBG] [FlowControl] Send rate: 720000 bytes/sec (10000 packets/sec)
[INF] [FlowControl] Rates balanced (1.00x ratio)
```

**Good sign:** Receive and send rates match (relay keeping up).

#### Buffer Pressure (after 20-30 seconds)
```
[WRN] [DataBuffer] Buffer at 75.3% capacity (7897088/10485760 bytes)
```

**Expected at high rates.** Buffer should stabilize, not keep growing.

#### If System Struggles
```
[ERR] [DataBuffer] Buffer at 91.2% capacity (9563136/10485760 bytes)
[WRN] [DataBuffer] Dropping old packets to make room
[WRN] [FlowControl] Rate mismatch detected: Receive 10K pkt/s, Send 8K pkt/s (1.25x)
```

**If you see this:** System can't keep up at 10K rate. Try slower rate.

### 🌳 Tree View (Connection Tab)

**What to click:**
1. Click "throughput_test" tab
2. Expand any tick packet

**Expected visualization:**
```
📈 Market Tick #5000 (72 bytes)
├─ symbol_id: 3 (uint64_t)           ← Stock symbol hash
├─ sequence: 5000 (uint64_t)
├─ timestamp_ns: 1699876543210000000 (uint64_t)  ← Nanosecond precision
├─ bid_price_e4: 2345678 (uint64_t)  ← $234.5678
├─ ask_price_e4: 2345890 (uint64_t)  ← $234.5890 (spread: $0.0212)
├─ bid_size: 1500 (uint32_t)         ← 1500 shares
├─ ask_size: 2300 (uint32_t)         ← 2300 shares
├─ last_price_e4: 2345750 (uint64_t) ← Last trade: $234.5750
├─ last_size: 850 (uint32_t)         ← 850 shares traded
├─ flags: 0x01 (uint8_t)             ← 0x01 = is_trade
├─ market_phase: 2 (uint8_t)         ← 2 = Open
└─ padding: 0 (uint16_t)
```

**Note:** At 10K ticks/s, tree view will lag behind (deferred rendering). This is EXPECTED and GOOD (doesn't slow relay).

**To verify deferred mode:**
- Scroll through tree view rapidly
- Relay latency should NOT change in Performance panel
- If relay.forward spikes when scrolling, deferred mode isn't working

### 👀 Receiver Console Output

**Expected to see:**
```
Connected from: 127.0.0.1:54321

Seq: 1000 | Recv: 1000 | Dropped: 0 (0%) | Rate: 10050 ticks/s | Avg: 10021 ticks/s | Throughput: 0.69 MB/s | Trades: 100 | Quotes: 900
Seq: 2000 | Recv: 1995 | Dropped: 5 (0.2%) | Rate: 9980 ticks/s | Avg: 9998 ticks/s | Throughput: 0.69 MB/s | Trades: 199 | Quotes: 1796
Seq: 10000 | Recv: 9850 | Dropped: 150 (1.5%) | Rate: 10010 ticks/s | Avg: 9850 ticks/s | Throughput: 0.68 MB/s | Trades: 985 | Quotes: 8865
```

**Expected:**
- Dropped: 0-2% (acceptable under high load)
- Rate: ~10,000 ticks/s (matching sender)
- Trades: ~10% of ticks (every 10th tick is a trade)
- Quotes: ~90% of ticks

**If Dropped >5%:**
- System can't handle 10K rate
- Try slower: `./sender 127.0.0.1 8004 200` (5K ticks/s)

## Validation Checklist

### ✅ Pass Criteria

Run for at least 5 minutes (3+ million ticks), then verify:

#### High Throughput Performance
- [ ] Sender maintains 10,000 ± 500 ticks/second
- [ ] Receiver gets 98%+ of ticks (≤2% drop rate)
- [ ] Buffer utilization stabilizes (doesn't grow unbounded)
- [ ] No crashes or freezes

#### Performance Panel Metrics
- [ ] `network.recv` average <50μs (GREEN)
- [ ] `network.send` average <50μs (GREEN)
- [ ] `relay.forward` average <500μs (GREEN)
- [ ] `buffer.push` average <100μs (GREEN or YELLOW acceptable)
- [ ] Occasional spikes to 200-500μs are OK (max should be <1ms)

#### Deferred Mode Validation
- [ ] `gui.render` takes 5-15ms per frame (NOT affecting relay)
- [ ] Scrolling tree view does NOT spike relay latency
- [ ] Performance panel shows relay operations independent of GUI

#### System Stability
- [ ] CPU usage <50% (if higher, expected on slow systems)
- [ ] Memory usage <200MB
- [ ] Buffer utilization stable (not climbing to 100%)
- [ ] No memory leaks (check with task manager after 10 minutes)

#### Logs Panel
- [ ] Saw "[INF] Visualization mode: DEFERRED"
- [ ] Saw "[DBG] Receive rate: ~720000 bytes/sec"
- [ ] Buffer warnings (if any) stabilize (not continuously increasing)
- [ ] No errors except occasional "Dropped old packets" (acceptable)

## Performance Testing Scenarios

### Scenario 1: Baseline (10K ticks/second)

```bash
./sender 127.0.0.1 8004 100
```

**Expected:**
- Drop rate: 0-2%
- CPU: 20-40%
- relay.forward: <500μs (GREEN)

**Pass:** System handles baseline load.

### Scenario 2: Moderate Stress (20K ticks/second)

```bash
./sender 127.0.0.1 8004 50
```

**Expected:**
- Drop rate: 2-5%
- CPU: 40-60%
- relay.forward: 500-800μs (YELLOW)
- Buffer warnings likely

**Pass:** System handles 2x load with graceful degradation.

### Scenario 3: Extreme Stress (50K ticks/second)

```bash
./sender 127.0.0.1 8004 20
```

**Expected:**
- Drop rate: 5-15%
- CPU: 70-90%
- relay.forward: 800-1500μs (RED)
- Buffer continuously at 90%+
- Many dropped packets

**Pass:** System doesn't crash, drops packets gracefully.

### Scenario 4: Sustained Load (10K for 30 minutes)

```bash
./sender 127.0.0.1 8004 100
# Let run for 30 minutes
```

**Check after 30 minutes:**
- Memory usage stable (not growing)
- CPU usage stable (not increasing)
- Drop rate consistent (not worsening)
- No memory leaks

**Pass:** System maintains performance over time.

### Scenario 5: Deferred vs Real-time Comparison

**Step 1: Deferred mode (current):**
```ini
visualization = deferred
```

Run for 2 minutes, note `relay.forward` average in Performance panel.

**Step 2: Real-time mode:**
Edit `switchspy.ini`:
```ini
visualization = realtime
```

Restart SwitchSpy, run for 2 minutes, note `relay.forward` average.

**Expected difference:**
- Deferred: relay.forward <500μs
- Real-time: relay.forward >1000μs (2-5x slower!)

**This proves:** Deferred mode is ESSENTIAL for high throughput.

## Troubleshooting

### High drop rate (>5% at 10K rate)
**Problem:** Receiver shows many dropped ticks

**Causes:**
- System too slow (CPU/RAM)
- Real-time mode instead of deferred
- Other applications consuming resources

**Solution:**
1. Verify deferred mode in switchspy.ini
2. Close other applications
3. Try slower rate: `./sender 127.0.0.1 8004 200` (5K rate)
4. Increase buffer: `buffer_size = 20971520` (20MB)
5. Upgrade hardware (SSD, more RAM)

### Buffer grows to 100% and crashes
**Problem:** Buffer utilization climbs continuously

**This is a BUG!** Buffer should stabilize by dropping old packets.

**Cause:** Ring buffer drop mechanism not working.

**Temporary fix:**
1. Reduce send rate
2. Increase buffer size massively: `buffer_size = 104857600` (100MB)
3. Report bug to developers

### relay.forward spikes to RED (>1ms)
**Problem:** Performance panel shows red relay latency

**Causes:**
- Real-time visualization mode (bug if config says deferred)
- System under heavy load
- Disk I/O (recording enabled)

**Solution:**
1. Verify deferred mode: Check logs for "[INF] Visualization mode: DEFERRED"
2. Disable recording temporarily:
   ```ini
   [recording]
   enabled = false
   ```
3. Check disk usage (if SSD is full, it slows down)
4. Close other apps

### GUI freezes or lags
**Problem:** Tree view or GUI becomes unresponsive

**Expected at high rates!** Deferred mode means GUI lags behind.

**If GUI completely freezes (no updates for >10 seconds):**
- Check CPU usage (should be <80%)
- Check memory (if >90% RAM used, system is swapping)
- Reduce send rate

**Note:** Lag in GUI is OK as long as relay keeps working (check Performance panel).

### Memory leak (RAM usage grows)
**Problem:** Memory increases continuously over time

**This is a BUG!**

**Debug:**
1. Run for 10 minutes, note memory usage
2. Check Logs panel for errors
3. Monitor Buffer Utilization (should stabilize, not grow forever)
4. Report to developers with Performance panel screenshot

## Performance Expectations

### Throughput Comparison

| Rate | Ticks/sec | MB/sec | Drop % | CPU % | Relay Latency |
|------|-----------|--------|--------|-------|---------------|
| Slow | 1,000 | 0.07 | 0% | 5% | <100μs |
| Normal | 10,000 | 0.69 | 0-2% | 30% | 300-500μs |
| Fast | 20,000 | 1.37 | 2-5% | 50% | 500-800μs |
| Extreme | 50,000 | 3.43 | 5-15% | 80% | 1-2ms |

### Hardware Requirements

**Minimum (10K ticks/s, 0-2% drops):**
- CPU: 2.0 GHz dual-core
- RAM: 4GB
- Disk: HDD (recording may lag)
- OS: Modern Linux/Windows

**Recommended (50K ticks/s, <5% drops):**
- CPU: 3.5 GHz quad-core or better
- RAM: 16GB
- Disk: SSD (for recording)
- OS: Linux (better networking performance)

### Latency Targets

For HFT applications:
- **Target**: relay.forward <500μs (GREEN)
- **Acceptable**: 500-1000μs (YELLOW)
- **Unacceptable**: >1000μs (RED)

At 10K ticks/s with deferred mode, target should be achievable on modern hardware.

## Interpreting Results

### Buffer Utilization Behavior

**Healthy pattern:**
```
Time 0s:   Buffer 10%
Time 10s:  Buffer 30%
Time 20s:  Buffer 50%
Time 30s:  Buffer 55%  ← Stabilizes
Time 60s:  Buffer 55%
```

**Unhealthy pattern (BUG):**
```
Time 0s:   Buffer 10%
Time 10s:  Buffer 40%
Time 20s:  Buffer 70%
Time 30s:  Buffer 90%  ← Continuously growing!
Time 60s:  Buffer 98%  ← Will crash soon
```

**If buffer grows unbounded:** Ring buffer isn't dropping old packets (bug).

### Drop Rate Interpretation

**Normal drops (expected):**
- Occasional bursts (sender sends 20 ticks at once)
- System scheduler delays
- Network buffering

**Example:** 150 drops out of 100,000 ticks = 0.15% (excellent).

**Abnormal drops (problem):**
- Continuous steady drops (relay can't keep up)
- Drop rate increasing over time
- Drops at slow rates (<1K ticks/s)

**Example:** 5000 drops out of 10,000 ticks = 50% (system failing).

### Deferred Mode Advantage

**Proof that deferred mode works:**

1. **Performance panel shows:**
   - `relay.forward`: 300-500μs (GREEN)
   - `gui.render`: 8000-15000μs (much slower)

2. **Key observation:** GUI is 20-50x slower than relay, yet relay isn't affected!

3. **If real-time mode:**
   - `relay.forward`: 2000-5000μs (RED)
   - Relay would wait for GUI rendering

**This proves:** Deferred mode decouples visualization from relay.

### Throughput Ceiling

**When you hit the ceiling:**
```
Seq: 100000 | Rate: 8500 ticks/s | Avg: 8200 ticks/s (sender: 10000)
```

**Receiver falls behind sender** - system maxed out.

**Ceiling depends on:**
- CPU speed (single-threaded performance critical)
- Network stack (Linux faster than Windows)
- System load (other apps)

**Typical ceilings:**
- Low-end laptop: 5-10K ticks/s
- Mid-range desktop: 20-30K ticks/s
- High-end server: 50-100K ticks/s

## Next Steps

After validating high throughput:
- ✅ Test 01: TCP Basic
- ✅ Test 02: UDP Relay
- ✅ Test 03: Multicast Broadcast
- ✅ Test 04: High Throughput (you are here)
- → Test 05: Rate Mismatch
- → Test 06: Complex Structs

## Summary

This test validates that SwitchSpy:
1. ✅ Handles 10,000+ messages/second with <2% loss
2. ✅ Maintains relay latency <500μs under load
3. ✅ Deferred mode keeps GUI separate from relay (critical!)
4. ✅ Buffer management scales to high throughput
5. ✅ System remains stable over sustained load (30+ minutes)
6. ✅ Gracefully degrades under extreme stress (drops old packets, doesn't crash)
7. ✅ Suitable for real-world HFT applications

**Pass criteria:** At 10K ticks/s, receiver gets 98%+ ticks, relay latency <500μs, buffer stabilizes, no crashes or memory leaks.
