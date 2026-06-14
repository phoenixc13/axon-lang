# AXON Language Specification
## Version 0.1.0-pre-alpha | APSE Group

---

## 1. Introduction

This document is the formal specification of the AXON language, the graph-native systems language powering the APSE 1.1 cognitive engine. AXON is designed as a **compiled, statically typed, memory-safe** language that treats knowledge graphs as first-class citizens of the type system.

---

## 2. Lexical Structure

### 2.1 Character Set
AXON source files are encoded in UTF-8. Only ASCII identifiers are accepted in the core language. Unicode string literals are supported via the `str` type.

### 2.2 Keywords

```
node    edge    concept   message   fn       let
const   use     domain    links     diverge  branch
sandbox import  pub       priv      return   if
else    loop    break     continue  match    as
```

### 2.3 Primitive Types

| Type    | Size     | Description                        |
|---------|----------|------------------------------------|
| `u8`    | 1 byte   | Unsigned 8-bit integer             |
| `u16`   | 2 bytes  | Unsigned 16-bit integer            |
| `u32`   | 4 bytes  | Unsigned 32-bit integer            |
| `u64`   | 8 bytes  | Unsigned 64-bit integer            |
| `i8`    | 1 byte   | Signed 8-bit integer               |
| `i16`   | 2 bytes  | Signed 16-bit integer              |
| `i32`   | 4 bytes  | Signed 32-bit integer              |
| `i64`   | 8 bytes  | Signed 64-bit integer              |
| `f32`   | 4 bytes  | IEEE 754 single precision float    |
| `f64`   | 8 bytes  | IEEE 754 double precision float    |
| `bool`  | 1 byte   | Boolean (true / false)             |
| `NodeRef` | 8 bytes | Pointer to a Node in shared memory |
| `EdgeRef` | 8 bytes | Pointer to an Edge in shared memory|

### 2.4 Array Types

Fixed-size arrays are declared with bracket notation:
```axon
let data: [f32; 1024]  // 1024 floats, stack or shared-mem allocated
```

Dynamic arrays are **not allowed** in `#[zero_copy]` types.

---

## 3. Graph Primitive Types

### 3.1 `node`

A `node` represents a discrete knowledge entity within the AXON cognitive graph.

**Syntax:**
```axon
node <Identifier> {
    domain: <Domain>,
    <field>: <type> [= <literal>],
    ...
    links: [Edge -> <NodeIdentifier>, ...]
}
```

**Semantics:**
- Each `node` declaration allocates a fixed-size record in the AXON shared memory segment.
- The `domain` field is mandatory and classifies the node within the MDC knowledge taxonomy.
- `links` is a compile-time-fixed list of directed `Edge` references.

**Example:**
```axon
node Thermodinamica {
    domain: Physics,
    entropy_constant: f64 = 1.380649e-23,
    temperature_k: f64 = 298.15,
    links: [Edge -> Fluidos, Edge -> MaterialScience]
}
```

### 3.2 `edge`

An `edge` encodes a directed or bidirectional relationship between two `node` entities.

**Syntax:**
```axon
edge <Identifier> {
    from: <NodeRef>,
    to:   <NodeRef>,
    weight: f64,
    bidirectional: bool
}
```

**Example:**
```axon
edge HeatTransferLink {
    from: Thermodinamica,
    to:   BiTeHeatsink,
    weight: 0.87,
    bidirectional: false
}
```

### 3.3 `concept`

A `concept` is a higher-order type that encodes a **causal or inferential relationship** between two graph entities. Concepts are the primary input/output of the MDC engine.

**Syntax:**
```axon
concept <Identifier> {
    from: <NodeRef | ConceptRef>,
    to:   <NodeRef | ConceptRef>,
    <field>: <type> [= <literal>],
    ...
}
```

**Example:**
```axon
concept SeebeckHarvest {
    from: ThermalGradient,
    to:   ElectricalOutput,
    seebeck_coefficient_uv_k: f64 = 200.0,
    estimated_power_w: f64 = 0.0,
    validated: bool = false
}
```

---

## 4. Zero-Copy Contract

### 4.1 Attribute Syntax

```axon
#[zero_copy]
#[shared_memory_layout]
node <Identifier> { ... }
```

### 4.2 Enforcement Rules

The AXON typechecker enforces the following invariants on `#[zero_copy]` types:

1. **No heap allocation**: All fields must have compile-time-known sizes.
2. **No pointers to heap**: `String`, `Vec<T>` (dynamic), and boxed types are **forbidden**.
3. **No padding gaps > 8 bytes**: The compiler inserts explicit padding fields if needed and emits a warning.
4. **Alignment**: All fields must be naturally aligned. The compiler reorders fields if needed and emits a diagnostic.

### 4.3 Memory Layout

Zero-copy types are written directly to the **AXON shared memory segment** (managed by the runtime). Any consumer process with permission can `mmap` the same physical pages and read the data without any copy or syscall overhead.

---

## 5. Functions and Control Flow

### 5.1 Function Declaration

```axon
fn <identifier>(<param>: <type>, ...) -> <ReturnType> {
    <body>
}
```

### 5.2 The `diverge` Instruction

`diverge` is the MDC's native parallel hypothesis branching mechanism. It spawns concurrent logical branches within the AXON runtime's deterministic scheduler (not OS threads).

```axon
fn infer(input: Concept) -> HypothesisSet {
    diverge {
        branch thermodynamics: apply_law(Thermodynamics1st, input),
        branch fluid:          apply_law(FluidDynamics,     input),
        branch material:       apply_law(MaterialStress,    input)
    } -> validate_in_sandbox()
}
```

- All branches execute concurrently within the shared memory space.
- Results are collected into a `HypothesisSet` and passed to `validate_in_sandbox()`.
- Branches that fail validation are pruned; their graph connections are marked inactive via the ENABLE local backprop.

### 5.3 `sandbox` blocks

```axon
sandbox "physics_sim" {
    // Code here runs in an isolated microVM
    // Errors are captured and returned as SandboxError, not propagated
    simulate(FluidDynamics, input_concept)
}
```

---

## 6. Domains Enum

The `domain` field of a `node` must be one of the following built-in domain classifiers:

```axon
domain Physics
domain Chemistry
domain MaterialScience
domain Robotics
domain Electronics
domain Thermodynamics
domain FluidDynamics
domain Cognition
domain Linguistics
domain Mathematics
domain ComputerScience
domain Biology
domain Custom(str)   // user-defined domain label
```

---

## 7. Module System

```axon
use stdlib::physics::Thermodinamica;
use stdlib::robotics::JointCommand;
use bridge::enable::TensorExport;
use bridge::apex::SharedBus;
```

- Modules map 1:1 to filesystem paths relative to the project root.
- There are no circular imports allowed (compile error).
- `pub` marks a declaration as exported from its module.

---

## 8. AXON Binary Format (.axb)

The AXON compiler (`axonc`) emits `.axb` files — a compact binary format:

| Section       | Description                                        |
|---------------|----------------------------------------------------|
| `AXHDR`       | Magic bytes + version + target arch               |
| `NODEMAP`     | Flat array of serialized node descriptors          |
| `EDGEMAP`     | Flat array of edge records                         |
| `CONCEPTMAP`  | Flat array of concept records                      |
| `SHMEM_INIT`  | Shared memory initialization directives            |
| `CODE`        | Native machine code or LLVM IR (configurable)      |
| `SYMTAB`      | Symbol table for runtime and APEX routing          |

---

## 9. Compiler Flags

```bash
axonc [OPTIONS] <input.axon> -o <output>

Options:
  --target <arch>       Target: x86_64, arm64, riscv64, llvm-ir
  --emit <format>       Output: axb (default), llvm-ir, asm
  --opt <level>         Optimization: 0, 1, 2, 3
  --strict-zero-copy    Treat zero-copy violations as errors (not warnings)
  --enable-bridge       Enable ENABLE tensor bridge codegen
  --apex-bridge         Enable APEX IPC bridge codegen
  --fpga-bridge         Enable VHDL/Verilog register map output
  --sandbox-mode        Enable sandbox execution wrappers
  --debug               Include debug symbols in .axb
```

---

*APSE Group — AXON Language Specification v0.1.0-pre-alpha — June 2026*
