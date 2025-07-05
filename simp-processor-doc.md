# SIMP Processor Project Documentation

## Executive Summary

This document presents the implementation of the SIMP (Simple MIPS) processor project, which encompasses the development of two primary components: an assembler and a cycle-accurate simulator. Through careful implementation in C, we have created a complete toolchain that translates assembly language programs into machine code and executes them with precise hardware simulation. The project demonstrates comprehensive understanding of computer architecture principles, including instruction processing, memory management, interrupt handling, and I/O device interaction. Our implementation successfully passes all test cases, validating arithmetic operations, memory access patterns, graphics rendering, and disk I/O functionality.

## Table of Contents

1. [Project Overview and Objectives](#project-overview-and-objectives)
2. [SIMP Architecture](#simp-architecture)
3. [Assembler Implementation](#assembler-implementation)
4. [Simulator Implementation](#simulator-implementation)
5. [Test Programs Analysis](#test-programs-analysis)

## 1. Project Overview and Objectives

### 1.1 Project Scope

The SIMP processor project required us to develop a complete computer system simulation environment. Our implementation consists of two standalone C programs that work together to form a complete development pipeline. The assembler translates human-readable assembly code into machine language, while the simulator executes these programs with full hardware emulation.

### 1.2 Development Environment

Our development was conducted using Visual Studio 2022 Community Edition on Windows, targeting 64-bit architecture. The project structure maintains separation between the assembler and simulator components, with each residing in its own directory containing source files and Visual Studio project configurations.

### 1.3 Project Structure

```
project/
├── asm/
│   ├── asm.c
│   ├── asm.h
│   └── asm.vcxproj
├── sim/
│   ├── sim.c
│   ├── sim.h
│   └── sim.vcxproj
├── sort/
│   ├── sort.asm
│   └── [generated files]
├── factorial/
│   ├── factorial.asm
│   └── [generated files]
├── rectangle/
│   ├── rectangle.asm
│   └── [generated files]
└── disktest/
    ├── disktest.asm
    └── [generated files]
```

## 2. SIMP Architecture

### 2.1 Processor Design

The SIMP processor implements a reduced instruction set architecture with 16 general-purpose registers, each holding 32-bit values. The design philosophy prioritizes simplicity while maintaining sufficient functionality for meaningful programs. Two registers deserve special attention: R0 ($zero) remains constantly zero through hardware protection, while R1 ($imm) automatically loads with immediate values during instruction decode.

### 2.2 Memory Organization

Our processor utilizes a unified memory model where instructions and data coexist in the same 4,096-word address space. This design choice simplifies the memory interface while requiring careful program layout to prevent code-data conflicts. The 12-bit program counter adequately addresses this memory range, advancing by one or two words depending on instruction length.

### 2.3 Instruction Encoding

SIMP instructions employ a variable-length encoding scheme. Single-word instructions execute in one cycle and encode small immediates directly. Two-word instructions, indicated by the bigimm flag, provide full 32-bit immediate values at the cost of an additional cycle. This design balances code density with the flexibility to handle large constants and addresses.

### 2.4 I/O Architecture

The processor interfaces with external devices through 23 memory-mapped registers. This approach maintains architectural simplicity while supporting diverse peripherals including timers, disk storage, display devices, and interrupt controllers. Each device register serves a specific function, from enabling interrupts to controlling pixel rendering on the monitor.

### 2.5 Interrupt Mechanism

Our implementation supports three interrupt sources with straightforward priority handling. The timer generates periodic interrupts for time-based operations, the disk signals completion of I/O operations, and external interrupts arrive through a predetermined schedule. The interrupt system prevents recursion by masking further interrupts until the current handler completes, ensuring predictable execution flow.

## 3. Assembler Implementation

### 3.1 Input/Output Specification

| File Type | Format | Description |
|-----------|--------|-------------|
| **Input** | program.asm | Assembly source with instructions, labels, comments, and directives |
| **Output** | memin.txt | Memory image with 8-digit hexadecimal values per line |

The assembler processes text files containing assembly instructions and produces memory images suitable for simulator consumption. Each line in the output represents one 32-bit word, formatted as exactly eight hexadecimal digits.

### 3.2 Core Functions

Our assembler implementation centers around several key functions that handle different aspects of the translation process:

**parse_lines()**: This function implements the first pass of our two-pass algorithm. It scans the entire source file, building a symbol table of labels and their addresses while calculating proper memory positions for each instruction. The function must account for variable instruction lengths when computing label addresses.

**write2memin()**: The second pass function generates actual machine code. It reads the source again, now armed with complete label information, and produces binary encodings for each instruction. This function handles immediate value sizing, register name translation, and proper instruction formatting.

**decode_opcode()**: Translates mnemonic operation names into their numeric representations. The function uses string comparison to map each instruction name to its corresponding opcode value.

**decode_register()**: Converts register names (like $t0 or $sp) into their numeric indices. This function recognizes both symbolic names and direct numeric references.

**decode_imm()**: Processes immediate values, handling decimal numbers, hexadecimal values, and label references. For labels, it consults the symbol table built during the first pass.

### 3.3 Data Structures

The assembler maintains several critical data structures:

```c
typedef struct {
    char name[MAX_LABEL_LENGTH];
    int address;
    bool is_valid;
} label;

typedef struct {
    int address;
    int data;
    bool has_data;
} word;
```

**label_arr[MAX_NUMBER_OF_LABELS]**: Stores all labels discovered during the first pass, mapping names to addresses.

**words[MEMORY_SIZE]**: Holds data values from .word directives, tracking both the target address and value.

### 3.4 Two-Pass Algorithm Flow

```
First Pass - Symbol Resolution:
┌─────────────────────────────────────────┐
│ Initialize: PC = 0, label_count = 0     │
│                                         │
│ For each source line:                   │
│   1. Remove comments                    │
│   2. Skip empty lines                   │
│   3. If label definition:               │
│      - Store (name, PC) in symbol table │
│   4. If .word directive:                │
│      - Record in words array            │
│   5. If instruction:                    │
│      - Determine size (1 or 2 words)    │
│      - Increment PC accordingly        │
└─────────────────────────────────────────┘

Second Pass - Code Generation:
┌─────────────────────────────────────────┐
│ Reset: PC = 0, rewind input file        │
│                                         │
│ For each instruction:                   │
│   1. Parse opcode and operands          │
│   2. Convert to numeric values          │
│   3. Resolve label references           │
│   4. Determine immediate encoding       │
│   5. Pack fields into instruction word  │
│   6. Write to output file               │
│                                         │
│ Fill gaps and write .word data          │
└─────────────────────────────────────────┘
```

### 3.5 Implementation Highlights

The assembler implements automatic immediate sizing, examining each immediate value to determine whether single-word encoding suffices. Values fitting within -128 to 127 use the compact form, while larger values or label references trigger two-word encoding. This optimization reduces code size without programmer intervention.

Error handling throughout the assembler ensures robust operation. Unknown opcodes, invalid register names, and out-of-bounds memory addresses all generate appropriate error messages. The implementation validates input thoroughly while maintaining clear error reporting for debugging assistance.

## 4. Simulator Implementation

### 4.1 Input/Output Specification

| File | Type | Description |
|------|------|-------------|
| memin.txt | Input | Initial memory contents from assembler |
| diskin.txt | Input | Initial disk contents (128 sectors × 128 words) |
| irq2in.txt | Input | External interrupt schedule (cycle numbers) |
| memout.txt | Output | Final memory state |
| regout.txt | Output | Final register values (R2-R15) |
| trace.txt | Output | Instruction execution log with register states |
| hwregtrace.txt | Output | I/O register access log |
| cycles.txt | Output | Total execution cycles |
| leds.txt | Output | LED state changes |
| display7seg.txt | Output | Seven-segment display changes |
| diskout.txt | Output | Final disk contents |
| monitor.txt | Output | Final framebuffer (text format) |
| monitor.yuv | Output | Final framebuffer (binary format) |

### 4.2 Core Functions

**run_simulator()**: The main execution loop that implements the fetch-decode-execute cycle. This function coordinates all aspects of simulation, from instruction processing to device updates and interrupt handling.

**initialize_instruction()**: Fetches and decodes the instruction at the current PC. It handles both single and two-word instructions, automatically loading the $imm register with the appropriate immediate value.

**timer()**: Updates the timer device each cycle. When enabled, it increments the counter and triggers interrupts upon reaching the programmed maximum value.

**disk_main_handler()**: Manages disk operations, counting down the 1024-cycle delay and performing DMA transfers upon completion. This function simulates realistic disk timing behavior.

**irq2()**: Checks for external interrupts by comparing the current cycle count against the scheduled interrupt times from the input file.

### 4.3 Data Structures

```c
typedef struct {
    unsigned int PC;
    unsigned int regs[16];
    unsigned int io_regs[23];
    unsigned int memory[4096];
    unsigned int disk[16384];
    uint8_t monitor[65536];
    unsigned int cycles_of_irq2[4096];
    unsigned int current_irq2;
    unsigned int disk_dcnt;
    bool handling_irq;
    bool run_en;
    instruction* pending_inst;
    bool in_second_cycle_of_bigimm;
} simulator;

typedef struct {
    unsigned int opcode;
    unsigned int rd, rs, rt;
    unsigned int imm8;
    unsigned int imm32;
    bool bigimm;
} instruction;
```

### 4.4 Key Global Variables

**simulator* sim**: The primary simulation state structure containing all processor and device state.

**FILE* handles**: Multiple file pointers for output generation (trace_fp, hwreg_fp, etc.).

### 4.5 Execution Flow

```
Main Simulation Loop:
┌──────────────────────────────────────┐
│ While (run_en == true):              │
│                                      │
│ 1. Check second cycle flag           │
│    - If set, complete instruction    │
│    - Skip to device updates          │
│                                      │
│ 2. Fetch & decode instruction        │
│    - Read from memory[PC]            │
│    - Extract fields                  │
│    - Load immediate to $imm          │
│                                      │
│ 3. Increment clock                   │
│                                      │
│ 4. Update devices                    │
│    - Timer tick                      │
│    - Disk progress                   │
│    - External interrupts             │
│                                      │
│ 5. Execute instruction               │
│    - Perform operation               │
│    - Update registers/memory         │
│    - Handle branches                 │
│                                      │
│ 6. Write trace entry                 │
│                                      │
│ 7. Update PC                         │
│                                      │
│ 8. Check interrupts                  │
│    - Save PC if interrupt pending    │
│    - Jump to handler                 │
└──────────────────────────────────────┘
```

### 4.6 Implementation Highlights

Cycle accuracy forms the cornerstone of our simulator design. Single-word instructions complete in one cycle, while two-word instructions stall the pipeline for an additional cycle. The implementation carefully tracks these timing details, ensuring realistic program execution.

Device simulation operates in parallel with instruction execution. Each cycle, the simulator updates all hardware devices, checking for timer expiration, disk operation completion, and external interrupt arrival. This approach accurately models the concurrent nature of hardware operations.

The simulator protects special registers through explicit checks. Any attempt to modify $zero gets ignored, maintaining its constant zero value. Similarly, $imm updates occur only during instruction decode, preventing corruption by program code.

## 5. Test Programs Analysis

### 5.1 sort.asm - Array Sorting

This program implements bubble sort on a 16-element array, demonstrating fundamental programming constructs. The implementation uses nested loops where the outer loop controls passes through the array while the inner loop performs pairwise comparisons and swaps. Memory access patterns involve sequential array traversal with calculated addressing. The program validates our processor's ability to handle complex control flow with multiple branch conditions, perform address arithmetic for array indexing, and execute numerous load/store operations efficiently.

### 5.2 factorial.asm - Arithmetic Computation

The factorial program computes n! through iterative multiplication, showcasing arithmetic capabilities. Starting with the input value from memory, it repeatedly multiplies by decreasing values until reaching zero. This simple yet effective test verifies the multiplication instruction's correctness, validates loop termination conditions, and demonstrates basic parameter passing through memory. The iterative approach, while differing from the mathematical recursive definition, provides a practical implementation given our architecture's constraints.

### 5.3 rectangle.asm - Graphics Rendering

Our graphics test draws a filled rectangle on the monochrome display, exercising the I/O subsystem extensively. The program reads corner coordinates from memory, calculates dimensions, then systematically fills every pixel within the bounds. Each pixel requires three I/O operations: setting the address, providing the color value, and triggering the draw command. This intensive I/O pattern thoroughly tests our memory-mapped register implementation and validates the monitor device's functionality across its entire address space.

### 5.4 disktest.asm - Storage Operations

The disk test program performs comprehensive I/O operations, reading multiple sectors, processing their contents, and writing results back. It demonstrates sector-by-sector reading with proper status polling, element-wise addition across multiple buffers, and DMA-based data transfer. The implementation uses a polling subroutine to wait for disk readiness, showcasing a common I/O synchronization pattern. This test validates our disk controller's timing accuracy, DMA functionality, and status reporting mechanisms while demonstrating practical data processing techniques.

Each test program targets specific architectural features while collectively providing comprehensive validation of our SIMP processor implementation. Together, they confirm correct operation across arithmetic operations, memory management, control flow, and I/O handling.