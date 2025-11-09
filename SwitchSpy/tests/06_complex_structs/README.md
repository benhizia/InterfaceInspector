# Test 06: Complex Nested Structures & Tree View Visualization

## Overview
This test validates SwitchSpy's ability to visualize **complex nested data structures**. Unlike simple flat structs, this test uses **hierarchical composition** with multiple levels of nesting - structs containing other structs containing arrays.

**Critical test for:**
- Nested struct visualization (Vec3, Quaternion, Color, SensorData inside RobotTelemetry)
- Tree view hierarchical expansion
- Array visualization within structs
- Metadata parsing of complex type hierarchies
- Correct field offset calculation for nested members
- Human-readable display of compound data types

## Test Scenario

### Use Case: Industrial Robot Telemetry

This test simulates a **6-axis industrial robot** transmitting telemetry:
- Position in 3D space (Vec3)
- Orientation as quaternion (Quaternion)
- Velocity vector (Vec3)
- Status color indicator (Color RGBA)
- Environmental sensors (SensorData)
- Six joint angles (array)
- Tool state and error flags

**Data hierarchy:**
```
RobotTelemetry (96 bytes)
├─ robot_id (scalar)
├─ sequence (scalar)
├─ timestamp_ms (scalar)
├─ position (Vec3)
│  ├─ x_mm (scalar)
│  ├─ y_mm (scalar)
│  └─ z_mm (scalar)
├─ orientation (Quaternion)
│  ├─ w_e4 (scalar)
│  ├─ x_e4 (scalar)
│  ├─ y_e4 (scalar)
│  └─ z_e4 (scalar)
├─ velocity (Vec3)
│  └─ ... (same as position)
├─ status_color (Color)
│  ├─ r (scalar)
│  ├─ g (scalar)
│  ├─ b (scalar)
│  └─ a (scalar)
├─ sensors (SensorData)
│  ├─ temperature_c100 (scalar)
│  ├─ pressure_pa (scalar)
│  └─ humidity_p10 (scalar)
├─ battery_percent (scalar)
├─ error_flags (scalar)
├─ task_id (scalar)
├─ joint_angles[6] (array)
│  ├─ [0] (scalar)
│  ├─ [1] (scalar)
│  ├─ ... 
│  └─ [5] (scalar)
├─ tool_state (scalar)
└─ padding[3] (array)
```

### Sender Configuration
- **Protocol**: TCP
- **Rate**: 2 updates/second (slow for detailed inspection)
- **Motion**: Circular path with sinusoidal height variation
- **Payload**: RobotTelemetry (96 bytes with 5 nested structs + 2 arrays)

### SwitchSpy Configuration
- **Mode**: Relay (deferred visualization)
- **Metadata**: Must extract nested struct definitions

### Receiver Configuration
- **Processing**: Full telemetry parsing with nested field display

### Expected Result
✅ **Tree view shows full hierarchy with expandable nested structs**

## Setup Instructions

### 1. Generate Metadata

**IMPORTANT:** Metadata generation must recognize nested types!

```cmd
cd SwitchSpy\tests\06_complex_structs
generate_metadata.cmd
```

**Expected output:**
```
========================================
Generating Metadata for Test 06
========================================

Running header_inspector.py on shared.hpp...
Copying metadata file to test directory...

========================================
Metadata generation complete!
========================================

Preview of metadata (nested structs):
---
"RobotTelemetry"
"Vec3"
"Quaternion"
"Color"
"SensorData"
---

This metadata includes NESTED STRUCTURES!
```

**Verify nested structs in metadata:**
```cmd
type shared.meta.json
```

**Look for:**
- Definitions for Vec3, Quaternion, Color, SensorData (not just RobotTelemetry)
- Type references like `"type": "Vec3"` for nested fields
- Array type indicators for joint_angles and padding

### 2. Build Test Programs

#### Windows:
```cmd
cd SwitchSpy\tests\06_complex_structs
g++ sender.cpp -o sender.exe -lws2_32
g++ receiver.cpp -o receiver.exe -lws2_32
```

#### Linux/macOS:
```bash
cd SwitchSpy/tests/06_complex_structs
g++ sender.cpp -o sender
g++ receiver.cpp -o receiver
```

### 3. Verify Configuration
Check `switchspy.ini`:
```ini
[connection.complex_structs_test]
metadata_file = tests/06_complex_structs/shared.meta.json
```

## Running the Test

### Step 1: Start Receiver (FIRST!)
```bash
./receiver
```

**Expected output:**
```
===== ROBOT TELEMETRY RECEIVER - Complex Structs Test =====
Configuration:
  Listen port: 9006

Listening for robot telemetry...
```

### Step 2: Start SwitchSpy
```bash
switchspy switchspy.ini
```

**Expected logs:**
```
[INF] [Main] Starting SwitchSpy
[INF] [MetadataParser] Loading tests/06_complex_structs/shared.meta.json
[INF] [MetadataParser] Found 5 type definitions (Vec3, Quaternion, Color, SensorData, RobotTelemetry)
[INF] [MetadataParser] Nested struct 'Vec3' (12 bytes, 3 fields)
[INF] [MetadataParser] Nested struct 'Quaternion' (8 bytes, 4 fields)
[INF] [MetadataParser] Root struct 'RobotTelemetry' (96 bytes, 14 fields)
[INF] [Connection] tcp complex_structs_test: Ready
```

**Critical validation:** Look for "Found 5 type definitions" - proves nested types were parsed.

### Step 3: Start Sender
```bash
./sender
```

**Expected output:**
```
===== ROBOT TELEMETRY SENDER - Complex Structs Test =====
Configuration:
  Target: 127.0.0.1:8006
  Update rate: 2 updates/second

Connected! Sending robot telemetry...

Seq: 1 | Robot ID: 42 | Pos: (3000, 0, 1000) mm | Battery: 99% | Task: 0 | Tool: 0
Seq: 2 | Robot ID: 42 | Pos: (2985, 224, 1250) mm | Battery: 99% | Task: 0 | Tool: 1
Seq: 3 | Robot ID: 42 | Pos: (2940, 445, 1479) mm | Battery: 99% | Task: 0 | Tool: 2
```

## What to Observe in GUI

### 🔴 CRITICAL: Tree View (Connection Tab → Left Panel)

**This is THE most important panel for this test!**

**What to click:**
1. Click "complex_structs_test" tab
2. Expand packet #1 in tree view
3. **Expand nested structs** (position, orientation, velocity, status_color, sensors)
4. **Expand array fields** (joint_angles, padding)

**Expected hierarchical visualization:**

```
🤖 RobotTelemetry Packet #1 (96 bytes)
│
├─ robot_id: 42 (uint32_t)
├─ sequence: 1 (uint64_t)
├─ timestamp_ms: 1699876543210 (uint64_t)
│
├─ 📍 position (Vec3) ◄─── NESTED STRUCT (click to expand)
│  ├─ x_mm: 3000 (int32_t)
│  ├─ y_mm: 0 (int32_t)
│  └─ z_mm: 1000 (int32_t)
│
├─ 🔄 orientation (Quaternion) ◄─── NESTED STRUCT (click to expand)
│  ├─ w_e4: 10000 (int16_t)     ← 1.0000 (normalized)
│  ├─ x_e4: 0 (int16_t)          ← 0.0000
│  ├─ y_e4: 0 (int16_t)          ← 0.0000
│  └─ z_e4: 0 (int16_t)          ← 0.0000
│
├─ 💨 velocity (Vec3) ◄─── NESTED STRUCT (click to expand)
│  ├─ x_mm: 0 (int32_t)
│  ├─ y_mm: 1500 (int32_t)
│  └─ z_mm: 500 (int32_t)
│
├─ 🎨 status_color (Color) ◄─── NESTED STRUCT (click to expand)
│  ├─ r: 0 (uint8_t)             ← Red channel
│  ├─ g: 255 (uint8_t)           ← Green (fully on)
│  ├─ b: 0 (uint8_t)             ← Blue
│  └─ a: 255 (uint8_t)           ← Alpha (opaque)
│  └─ Interpreted: RGB(0, 255, 0) = GREEN
│
├─ 🌡️ sensors (SensorData) ◄─── NESTED STRUCT (click to expand)
│  ├─ temperature_c100: 2000 (int16_t)  ← 20.00°C
│  ├─ pressure_pa: 101325 (uint32_t)     ← 101325 Pa (1 atm)
│  └─ humidity_p10: 450 (uint16_t)       ← 45.0%
│
├─ battery_percent: 99 (uint8_t)
├─ error_flags: 0x00 (uint8_t)
├─ task_id: 0 (uint16_t)
│
├─ 📊 joint_angles[6] (int16_t array) ◄─── ARRAY (click to expand)
│  ├─ [0]: 0 (int16_t)    ← 0.0°
│  ├─ [1]: 450 (int16_t)  ← 45.0° (decidegrees)
│  ├─ [2]: 720 (int16_t)  ← 72.0°
│  ├─ [3]: -300 (int16_t) ← -30.0°
│  ├─ [4]: 150 (int16_t)  ← 15.0°
│  └─ [5]: -890 (int16_t) ← -89.0°
│
├─ tool_state: 0 (uint8_t)  ← 0=IDLE, 1=GRIPPING, 2=RELEASING
│
└─ 🔒 padding[3] (uint8_t array)
   ├─ [0]: 0
   ├─ [1]: 0
   └─ [2]: 0
```

**What to validate:**

#### Nested Struct Expansion
1. **Click the ▶ arrow next to "position (Vec3)"**
   - Should expand to show: x_mm, y_mm, z_mm
   - Each field should have its own value and type
   - Values should match sender console output

2. **Click the ▶ arrow next to "orientation (Quaternion)"**
   - Should expand to show: w_e4, x_e4, y_e4, z_e4
   - Values represent normalized quaternion (w²+x²+y²+z²≈1 when divided by 10000)

3. **Click the ▶ arrow next to "status_color (Color)"**
   - Should expand to show: r, g, b, a
   - Green status = (0, 255, 0, 255)
   - Orange warning = (255, 165, 0, 255)
   - Red error = (255, 0, 0, 255)

4. **Click the ▶ arrow next to "sensors (SensorData)"**
   - Should expand to show: temperature_c100, pressure_pa, humidity_p10
   - Temperature ~2000-2500 = 20-25°C
   - Pressure typically 101325 (1 atm)
   - Humidity ~350-550 = 35-55%

#### Array Expansion
1. **Click the ▶ arrow next to "joint_angles[6]"**
   - Should expand to show: [0], [1], [2], [3], [4], [5]
   - Each element is int16_t in decidegrees (divide by 10 for degrees)
   - Values should oscillate ±90° (~±900 decidegrees)

2. **Verify array bounds:**
   - Array should show exactly 6 elements (not 5, not 7)
   - Indices should be [0] through [5]

#### Type Annotations
- Each field should show its type: `(uint32_t)`, `(int16_t)`, etc.
- Nested structs should show their type name: `(Vec3)`, `(Quaternion)`, etc.
- Arrays should show: `(int16_t array)`, `(uint8_t array)`, etc.

### 📊 Statistics Panel

**What to check:**
```
Connection: complex_structs_test (TCP Relay)

Packets A→B: 120      ← Robot telemetry updates (2/sec × 60 sec)
Bytes A→B: 11520      ← 120 × 96 bytes
Forwarded A→B: 120

Buffer Utilization: 8%  ← Low (slow rate)
Dropped Packets: 0
```

**Expected:** 100% delivery (0 drops at 2 updates/second).

### 🔍 Logs Panel

**Expected log sequence:**

#### Startup with Nested Type Loading
```
[INF] [Main] Starting SwitchSpy
[INF] [MetadataParser] Loading tests/06_complex_structs/shared.meta.json
[DBG] [MetadataParser] Parsing type definition: Vec3
[DBG] [MetadataParser]   Field: x_mm (type: int32_t, offset: 0)
[DBG] [MetadataParser]   Field: y_mm (type: int32_t, offset: 4)
[DBG] [MetadataParser]   Field: z_mm (type: int32_t, offset: 8)
[DBG] [MetadataParser] Parsing type definition: Quaternion
[DBG] [MetadataParser]   Field: w_e4 (type: int16_t, offset: 0)
[DBG] [MetadataParser]   Field: x_e4 (type: int16_t, offset: 2)
[DBG] [MetadataParser]   Field: y_e4 (type: int16_t, offset: 4)
[DBG] [MetadataParser]   Field: z_e4 (type: int16_t, offset: 6)
[DBG] [MetadataParser] Parsing type definition: Color
[DBG] [MetadataParser] Parsing type definition: SensorData
[DBG] [MetadataParser] Parsing type definition: RobotTelemetry
[DBG] [MetadataParser]   Field: position (type: Vec3, offset: 20)  ← Nested!
[INF] [MetadataParser] Successfully loaded 5 types with nested hierarchies
[INF] [Connection] tcp complex_structs_test: Ready
```

**Critical validation:**
- "Successfully loaded 5 types" (not just 1!)
- Nested fields show correct type references (e.g., `type: Vec3` not `type: struct`)
- Offsets calculated correctly for nested fields

#### During Telemetry Reception
```
[DBG] [Connection] Received 96 bytes
[DBG] [Parser] RobotTelemetry packet #1
[DBG] [Parser]   robot_id: 42
[DBG] [Parser]   position.x_mm: 3000 (nested field)
[DBG] [Parser]   position.y_mm: 0 (nested field)
[DBG] [Parser]   position.z_mm: 1000 (nested field)
[DBG] [Parser]   orientation: quaternion (1.0, 0.0, 0.0, 0.0)
[DBG] [Parser]   status_color: RGB(0, 255, 0) GREEN
[DBG] [Parser]   sensors.temperature_c100: 2000 (20.0°C)
```

**What to check:** Parser logs show nested field access with dot notation (e.g., `position.x_mm`, `sensors.temperature_c100`).

### 📈 Performance Panel

**What to observe:**

All operations should be GREEN (2 updates/second is very slow):

```
network.recv
  Avg: 18μs     ← GREEN

relay.forward
  Avg: 95μs     ← GREEN

parser.visualize
  Calls: 120
  Avg: 150μs    ← May be slightly higher due to nested parsing
  Max: 350μs    ← Acceptable for complex structs
```

**Expected:** Nested struct parsing adds ~50-100μs overhead compared to flat structs (Test 01).

### 👀 Receiver Console Output

**Expected to see:**
```
Connected from: 127.0.0.1:54321

Seq: 1 | Recv: 1 | Dropped: 0 (0%)
  Robot ID: 42 | Battery: 99% | Task: 0
  Position: (3000, 0, 1000) mm
  Orientation (quaternion): (1.0, 0.0, 0.0, 0.0)
  Velocity: (0, 1500, 500) mm/s
  Status Color: RGB(0, 255, 0, 255)
  Sensors: Temp=20.0°C, Pressure=101325 Pa, Humidity=45.0%
  Joint Angles: [0.0°, 45.0°, 72.0°, -30.0°, 15.0°, -89.0°]
  Tool: IDLE

Seq: 2 | Recv: 2 | Dropped: 0 (0%)
  Robot ID: 42 | Battery: 99% | Task: 0
  Position: (2985, 224, 1250) mm
  ...
```

**What to validate:**
- All nested fields displayed correctly
- Values match sender console
- No parsing errors

## Validation Checklist

### ✅ Pass Criteria

Monitor for at least 2 minutes (240+ updates):

#### Metadata Loading
- [ ] Logs show "Found 5 type definitions"
- [ ] Logs show nested types: Vec3, Quaternion, Color, SensorData, RobotTelemetry
- [ ] Logs show correct offsets for nested fields
- [ ] No metadata parsing errors

#### Tree View - Nested Struct Expansion
- [ ] Can expand "position (Vec3)" to see x_mm, y_mm, z_mm
- [ ] Can expand "orientation (Quaternion)" to see w_e4, x_e4, y_e4, z_e4
- [ ] Can expand "velocity (Vec3)" to see x_mm, y_mm, z_mm
- [ ] Can expand "status_color (Color)" to see r, g, b, a
- [ ] Can expand "sensors (SensorData)" to see temperature_c100, pressure_pa, humidity_p10

#### Tree View - Array Expansion
- [ ] Can expand "joint_angles[6]" to see [0] through [5]
- [ ] Array shows exactly 6 elements
- [ ] Can expand "padding[3]" to see [0] through [2]
- [ ] Array indices start at [0] not [1]

#### Tree View - Type Annotations
- [ ] Nested structs show type name: (Vec3), (Quaternion), etc.
- [ ] Arrays show type: (int16_t array), (uint8_t array)
- [ ] Scalar fields show primitive type: (uint32_t), (int16_t), etc.

#### Value Validation
- [ ] position values match sender console (circular motion)
- [ ] orientation quaternion normalized (w²+x²+y²+z²≈1)
- [ ] status_color shows GREEN (0, 255, 0, 255) most of the time
- [ ] sensors.temperature_c100 ~2000-2500 (20-25°C)
- [ ] joint_angles oscillate ±90° (~±900)

#### Receiver Console
- [ ] Receives all nested fields correctly
- [ ] No parsing errors
- [ ] Values match sender and tree view
- [ ] 0% drops

## Advanced Testing

### Test Nested Struct Value Changes

Watch how tree view updates as robot moves:

**At t=0s:**
```
position (Vec3)
├─ x_mm: 3000  ← Starting at (3000, 0, 1000)
├─ y_mm: 0
└─ z_mm: 1000
```

**At t=10s:**
```
position (Vec3)
├─ x_mm: 2245  ← Circular motion
├─ y_mm: -2070
└─ z_mm: 1479  ← Sinusoidal height
```

**Validation:** Tree view values update in real-time as robot moves.

### Test Color Status Changes

Monitor status_color based on battery:

**Battery >25%:**
```
status_color (Color)
├─ r: 0
├─ g: 255   ← GREEN
├─ b: 0
└─ a: 255
```

**Battery <25%:**
```
status_color (Color)
├─ r: 255   ← RED
├─ g: 0
├─ b: 0
└─ a: 255
```

**Task change (every 10 seconds):**
```
status_color (Color)
├─ r: 255   ← ORANGE
├─ g: 165
├─ b: 0
└─ a: 255
```

**Validation:** Color changes reflected immediately in tree view.

### Test Error Flags

Watch error_flags field:

**Normal operation:**
```
error_flags: 0x00 (uint8_t)  ← No errors
```

**Low battery:**
```
error_flags: 0x02 (uint8_t)  ← Bit 1 set
```

**Sensor fault (every 50th packet):**
```
error_flags: 0x04 (uint8_t)  ← Bit 2 set
```

**Both errors:**
```
error_flags: 0x06 (uint8_t)  ← Bits 1 and 2 set (0x02 | 0x04)
```

**Validation:** Tree view shows hex value, receiver console decodes bits.

### Test Joint Angles Array

**At packet #1:**
```
joint_angles[6]
├─ [0]: 0      ← 0.0°
├─ [1]: 450    ← 45.0°
├─ [2]: 720    ← 72.0°
├─ [3]: -300   ← -30.0°
├─ [4]: 150    ← 15.0°
└─ [5]: -890   ← -89.0°
```

**At packet #20:**
```
joint_angles[6]
├─ [0]: 720    ← Oscillating
├─ [1]: 300
├─ [2]: -450
├─ [3]: 600
├─ [4]: -750
└─ [5]: 120
```

**Validation:** All 6 elements visible and updating independently.

## Troubleshooting

### Nested structs not expandable
**Problem:** Tree view shows "position (Vec3)" but no ▶ arrow to expand

**Cause:** Metadata doesn't include nested type definitions.

**Debug:**
1. Check shared.meta.json for Vec3 definition
2. Verify metadata generation ran successfully
3. Check SwitchSpy logs for "Found 5 type definitions"

**Solution:**
```bash
# Regenerate metadata
generate_metadata.cmd

# Check output
type shared.meta.json | findstr "Vec3"
```

**Expected:** Separate type definition for Vec3 with x_mm, y_mm, z_mm fields.

### Arrays show as single value
**Problem:** "joint_angles[6]" shows one number instead of 6 elements

**Cause:** Array not recognized as array type in metadata.

**Debug:**
1. Check shared.meta.json for joint_angles field
2. Look for array indicator: `"is_array": true` or `"count": 6`

**Solution:** Verify header_inspector.py correctly detects array syntax: `int16_t joint_angles[6];`

### Wrong field offsets
**Problem:** Tree view shows correct structure but values are garbage

**Cause:** Nested struct offsets calculated incorrectly.

**Debug:**
1. Check receiver console - if it shows correct values, problem is in SwitchSpy parsing
2. Compare tree view values with receiver console
3. Check logs for "Field: position (type: Vec3, offset: X)"

**Expected offsets for RobotTelemetry:**
- robot_id: offset 0 (size 4)
- sequence: offset 4 (size 8)
- timestamp_ms: offset 12 (size 8)
- position: offset 20 (size 12 = Vec3)
- orientation: offset 32 (size 8 = Quaternion)
- ...

**Solution:** Verify metadata has correct sizes for nested types.

### Nested fields not showing dot notation in logs
**Problem:** Logs show "x_mm: 3000" instead of "position.x_mm: 3000"

**This is OK** - depends on logging implementation.

**Ideal:** Logs use dot notation for clarity.

**Acceptable:** Logs show flat field names (as long as values are correct).

### Parser performance RED for complex structs
**Problem:** Performance panel shows parser.visualize >500μs

**Expected!** Complex nested parsing takes longer than flat structs.

**Acceptable range:**
- Flat structs (Test 01): 50-100μs
- Complex structs (Test 06): 100-300μs
- Very complex: 300-500μs

**If >500μs consistently:** Parsing algorithm may be inefficient (investigate).

## Performance Expectations

### Parsing Overhead Comparison

| Test | Struct Type | Fields | Nesting | Parse Time |
|------|-------------|--------|---------|------------|
| 01 | SimpleMessage | 5 | Flat | 50-80μs |
| 05 | SensorData | 8 | Flat | 60-90μs |
| 06 | RobotTelemetry | 14 + nested | 5 levels | 150-300μs |

**Nested parsing overhead:** 2-3x slower than flat (acceptable tradeoff for visualization).

### Tree View Responsiveness

**Expected behavior:**
- Expand/collapse animation: <100ms (smooth)
- Click → expand delay: <50ms (instant)
- Scroll performance: 60 FPS (smooth)

**If laggy:**
- Deferred mode may not be working
- System under load
- Too many packets in buffer (increase buffer size)

## Interpreting Results

### Nested Struct Hierarchy

**Correct tree structure:**
```
RobotTelemetry
├─ position (Vec3)        ← Parent struct
│  ├─ x_mm               ← Child field
│  ├─ y_mm               ← Child field
│  └─ z_mm               ← Child field
```

**Incorrect (flat):**
```
RobotTelemetry
├─ position              ← Shows as single field (wrong!)
├─ x_mm                  ← Should be nested under position
├─ y_mm
└─ z_mm
```

**If flat instead of nested:** Metadata parsing failed to recognize nested types.

### Array Visualization

**Correct array display:**
```
joint_angles[6]           ← Array header with size
├─ [0]: 0                ← Indexed elements
├─ [1]: 450
├─ [2]: 720
├─ [3]: -300
├─ [4]: 150
└─ [5]: -890
```

**Incorrect:**
```
joint_angles[6]: [0, 450, 720, -300, 150, -890]  ← Inline (not expandable)
```

**If inline:** Array expansion not implemented or metadata missing array info.

### Metadata Completeness

**Complete metadata includes:**
1. **Type definitions for all structs** (Vec3, Quaternion, Color, SensorData, RobotTelemetry)
2. **Field type references** (position field has type Vec3, not just "struct")
3. **Array metadata** (joint_angles has count=6)
4. **Correct sizes** (Vec3=12 bytes, Quaternion=8 bytes, etc.)
5. **Correct offsets** (accounting for alignment and padding)

**Check with:**
```bash
type shared.meta.json
```

**Look for:**
- `"name": "Vec3"`
- `"type": "Vec3"` (in RobotTelemetry.position field)
- `"count": 6` (in joint_angles field)
- `"size": 12` (for Vec3)

## Next Steps

After validating complex struct visualization:
- ✅ Test 01: TCP Basic
- ✅ Test 02: UDP Relay
- ✅ Test 03: Multicast Broadcast
- ✅ Test 04: High Throughput
- ✅ Test 05: Rate Mismatch
- ✅ Test 06: Complex Structs (you are here)

**All tests complete!** 

Next: Create master test suite documentation explaining how to run all tests sequentially.

## Summary

This test validates that SwitchSpy:
1. ✅ Parses nested struct type hierarchies from metadata
2. ✅ Visualizes nested structs as expandable tree nodes
3. ✅ Displays arrays with individual element expansion
4. ✅ Calculates correct field offsets for nested members
5. ✅ Shows type annotations for nested structs and arrays
6. ✅ Updates nested field values in real-time
7. ✅ Handles complex data structures (5 nested types, 14 fields, arrays)
8. ✅ Maintains performance (<300μs parse time for complex struct)

**Pass criteria:** Tree view shows full hierarchical structure with all nested structs and arrays expandable. All field values correct and updating. Receiver gets 100% of packets with all nested data intact.
