# AXON Language

> **Zero-Copy Cognitive Graph Language** — Core of the APSE 1.1 Ecosystem

[![License: Proprietary](https://img.shields.io/badge/License-Proprietary-red.svg)](LICENSE)
[![Status: Pre-Alpha](https://img.shields.io/badge/Status-Pre--Alpha-orange.svg)]()
[![Ecosystem: APSE 1.1](https://img.shields.io/badge/Ecosystem-APSE%201.1-blue.svg)]()
[![Lang: C + Rust](https://img.shields.io/badge/Compiler-C%20%2B%20Rust-lightgrey.svg)]()

---

## Overview

AXON is a systems programming language engineered from scratch to eliminate the serialization/deserialization bottlenecks that cripple modern AI and robotics stacks. Where conventional pipelines rely on JSON, Protobuf, or similar text-structured interchange formats, AXON introduces **native binary graph types** that map directly to physical memory — enabling zero-copy data transfer between processes at nanosecond latency.

AXON is not a general-purpose language. It is the **lingua franca of the APSE cognitive engine**, purpose-built to express:
- Cognitive graph structures (knowledge, causality, physical laws)
- Zero-latency inter-process messaging between AXON, ENABLE, and APEX
- Hardware-near robotics control logic with deterministic timing guarantees

---

## APSE 1.1 Ecosystem Architecture

AXON is the first pillar of a three-part software triangle:

```
+--------------------------------------------------+
| APSE 1.1 ECOSYSTEM |
| |
| [AXON] <-----> [ENABLE] <-----> [APEX] |
| Graph Lang Dynamic Tensors Middleware |
| Zero-Copy Local Backprop Async Orchestrator |
| Node/Edge/Concept Cognitive Refinement Semantic Router |
| |
| [MDC - Massive Disruptive Connection Engine] |
| Concept Graph Builder / Hypothesis Diverger |
| Concurrent Sandbox Validator / Feedback Loop |
+--------------------------------------------------+
 | |
 [Hardware Layer - Bismuth Chips / FPGAs / PCBs]
```

### The Three Pillars

| Component | Role | Language | Status |
|-----------|------|----------|--------|
| **AXON** | Graph language + compiler + runtime | C, Rust | Pre-Alpha |
| **ENABLE** | Dynamic tensor library replacing PyTorch/TF | C, Rust | Planned |
| **APEX** | Ultra-low latency middleware replacing ROS 2 | C, Rust | Planned |

---

## Why AXON Exists

### The Problem with JSON/Protobuf

Every modern AI and robotics system wastes millions of CPU cycles per second on:
1. **Serializing** structured data (graphs, tensors, states) into byte strings
2. **Transmitting** those byte strings across IPC channels
3. **Deserializing** them back at the consumer side

This overhead adds **milliseconds** of latency per hop — unacceptable in real-time robotic systems where control loops run at 1kHz and cognitive inference must be deterministic.

### The AXON Solution

```axon
// AXON code — direct graph type, no serialization
node Thermodinamica {
 domain: Physics,
 entropy: f64 = 1.38e-23,
 links: [Edge -> Fluidos, Edge -> MaterialScience]
}

concept HeatDissipation {
 from: Thermodinamica,
 to: BiTe_Heatsink,
 efficiency: f64 = 0.87,
 seebeck_coefficient: f64 = 200e-6
}
```

- Types `Node`, `Edge`, and `Concept` are **first-class primitives**
- Memory layout is fixed at compile time — no heap allocation per message
- The compiler enforces **Zero-Copy Serialization**: data is written **once** into shared memory and read by any consumer without copying
- Transfer times drop from milliseconds to **nanoseconds**

---

## Language Primitives

### Core Types

```axon
// --- PRIMITIVE TYPES ---
node <identifier> {
 domain: <DomainEnum>,
 [field: <type> = <value>]*
 links: [Edge -> <NodeRef>]*
}

edge <identifier> {
 from: <NodeRef>,
 to: <NodeRef>,
 weight: f64,
 bidirectional: bool
}

concept <identifier> {
 from: <NodeRef | ConceptRef>,
 to: <NodeRef | ConceptRef>,
 [properties]*
}
```

### Zero-Copy Serialization Guarantee

AXON enforces the `#[zero_copy]` contract at the type level:

```axon
#[zero_copy]
#[shared_memory_layout]
node SensorReading {
 timestamp_ns: u64,
 lidar_points: [f32; 4096],
 rgb_frame: [u8; 921600],
 depth_map: [f32; 307200]
}
```

The compiler will **reject** any type marked `#[zero_copy]` that contains heap-allocated members (String, Vec with dynamic size, etc.).

### Async Divergence Instruction

For the MDC reasoning engine, AXON provides the native `diverge` instruction — spawning parallel hypothesis branches without allocating new threads:

```axon
fn reason(input: Concept) -> HypothesisSet {
 diverge {
 branch A: apply(LawOfThermodynamics, input),
 branch B: apply(FluidDynamics, input),
 branch C: apply(MaterialStress, input)
 } -> validate_in_sandbox()
}
```

---

## Repository Structure

```
axon-lang/
|-- compiler/ # AXON compiler (C + Rust)
| |-- src/
| | |-- lexer/ # Tokenizer
| | |-- parser/ # AST builder
| | |-- typechecker/ # Zero-copy contract enforcement
| | |-- codegen/ # Native binary output + LLVM IR
| | `-- optimizer/ # Graph layout optimizer
| |-- include/ # Public C headers (axon.h)
| `-- tests/ # Compiler unit tests
|
|-- runtime/ # AXON runtime (Rust)
| |-- src/
| | |-- memory/ # Shared memory allocator
| | |-- scheduler/ # Deterministic task scheduler
| | |-- ipc/ # Zero-copy IPC bus
| | `-- sandbox/ # Isolated execution environments
| `-- tests/
|
|-- stdlib/ # Standard library
| |-- axon/ # Core graph types
| |-- physics/ # Physics domain primitives
| |-- robotics/ # Robot control primitives
| `-- cognition/ # MDC cognitive graph types
|
|-- bridge/ # Integration layer
| |-- enable/ # AXON <-> ENABLE tensor bridge
| |-- apex/ # AXON <-> APEX middleware bridge
| `-- hardware/ # AXON <-> FPGA/PCB signal bridge
|
|-- examples/ # Sample AXON programs
| |-- hello_graph.axon
| |-- sensor_fusion.axon
| |-- mdc_diverge.axon
| `-- seebeck_monitor.axon
|
|-- docs/ # Documentation
| |-- SPEC.md # Full language specification
| |-- GRAMMAR.ebnf # Formal AXON grammar
| |-- MEMORY_MODEL.md # Zero-copy memory model
| |-- INTEGRATION.md # ENABLE + APEX integration guide
| `-- ROADMAP.md # Development roadmap
|
|-- tools/ # Developer tooling
| |-- axon-fmt/ # Code formatter
| |-- axon-lsp/ # Language Server Protocol
| `-- axon-viz/ # Graph visualizer (terminal)
|
|-- .github/
| `-- workflows/ # CI/CD pipelines
|
|-- Cargo.toml # Rust workspace
|-- CMakeLists.txt # C compiler build
|-- LICENSE
`-- README.md
```

---

## Build Requirements

| Tool | Version | Purpose |
|------|---------|---------|
| Rust | >= 1.78 | Runtime + codegen |
| GCC / Clang | >= 13 | Core compiler |
| LLVM | >= 17 | IR backend |
| CMake | >= 3.25 | C build system |
| cargo | >= 1.78 | Rust build system |

### Quick Build

```bash
# Clone
git clone https://github.com/phoenixc13/axon-lang.git
cd axon-lang

# Build compiler (C + CMake)
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Build runtime (Rust)
cargo build --release

# Run tests
cargo test
ctest --output-on-failure
```

### Compile your first AXON program

```bash
./build/axonc examples/hello_graph.axon -o hello_graph.axb
./build/axon-runtime hello_graph.axb
```

---

## Integration with the APSE Ecosystem

### AXON -> ENABLE (Tensor Bridge)

The `bridge/enable/` module exposes AXON graph nodes as **Dynamic Connection Tensors** consumable by the ENABLE library:

```axon
use bridge::enable::TensorExport;

#[enable_tensor_export]
node ConceptualWeight {
 dimensions: [u32; 4],
 values: [f32; 65536],
 gradient_mask: [bool; 65536] // local backprop mask
}
```

### AXON -> APEX (IPC Bridge)

The `bridge/apex/` module maps AXON message types to the APEX shared-memory bus:

```axon
use bridge::apex::SharedBus;

#[apex_channel(priority = REALTIME)]
message RobotCommand {
 target_joint: u8,
 torque_nm: f32,
 timestamp_ns: u64
}
```

### AXON -> Hardware (FPGA Bridge)

For edge processing, `bridge/hardware/` generates **VHDL/Verilog register interfaces** from AXON sensor types, eliminating raw telemetry transmission to the APEX middleware.

---

## Development Roadmap

### Phase 1 — Foundation (Current)
- [ ] Lexer + Parser (full grammar coverage)
- [ ] Type system with zero-copy contract enforcement
- [ ] Shared memory allocator (runtime)
- [ ] Basic codegen (x86_64 native + LLVM IR)
- [ ] `hello_graph.axon` end-to-end working

### Phase 2 — APSE Integration
- [ ] ENABLE tensor bridge
- [ ] APEX IPC bridge
- [ ] `diverge` instruction (MDC async reasoning)
- [ ] Sandbox execution environment
- [ ] AXON LSP (VS Code extension)

### Phase 3 — Hardware
- [ ] FPGA signal bridge (VHDL/Verilog codegen)
- [ ] ARM64 / RISC-V backend
- [ ] Bismuth chip ISA support (post-silicon)
- [ ] Seebeck energy monitoring integration

---

## Contributing

This is a **proprietary internal project** of the APSE Group. External contributions are not accepted at this stage.

For ecosystem partners and investors, contact the Engineering Directorate through official APSE channels.

---

## License

Proprietary — APSE Group. All rights reserved.
See [LICENSE](LICENSE) for details.

---

*APSE Group — Engineering Directorate of Strategic Semiconductors*
*Classification: Public Repository (Ecosystem Overview Only)*
