# SIMP Processor Project

This repository contains a complete toolchain for the **SIMP** processor, a 16-register RISC-like architecture designed for educational purposes in the "Computer Organization" course (0512.4400) at Tel Aviv University.

## Contents

* `assembler/`: C program that assembles `.asm` source files into machine code (`memin.txt`)
* `simulator/`: C program that simulates the SIMP CPU, executing `memin.txt` and generating detailed output
* `tests/`: Four functional assembly test programs with full I/O input/output files

  * `sort/`
  * `factorial/`
  * `rectangle/`
  * `disktest/`

---

## SIMP Architecture Summary

### Registers (16 x 32-bit)

| Register | Name    | Purpose                 |
| -------- | ------- | ----------------------- |
| \$zero   | R0      | Constant 0              |
| \$imm    | R1      | Sign-extended immediate |
| \$v0     | R2      | Return value            |
| \$a0-a3  | R3-R6   | Function arguments      |
| \$t0-t2  | R7-R9   | Temporaries             |
| \$s0-s2  | R10-R12 | Saved values            |
| \$gp     | R13     | Global pointer          |
| \$sp     | R14     | Stack pointer           |
| \$ra     | R15     | Return address          |

### Memory

* 4096 words x 32-bit each
* PC is 12-bit, increments by 1 (single-word instruction) or 2 (double-word instruction with big immediate)

### Instruction Encoding

* 32-bit instruction format (with optional second word for 32-bit immediate)
* Immediate values handled via `bigimm` field

### Instruction Set

Includes `add`, `sub`, `mul`, logic ops, shifts, branches (`beq`, `bne`, etc.), `jal`, memory access (`lw`, `sw`), `in/out`, `halt`, and `reti`

---

## Assembler

* **Input**: Assembly program (`.asm`)
* **Output**: Machine code image (`memin.txt`)
* **CLI Usage**: `asm.exe program.asm memin.txt`
* Two-pass parsing: resolves symbolic labels, assigns instruction encodings
* Supports `.word` directives, label jumps, and all SIMP instructions

---

## Simulator

* **Input**: `memin.txt`, `diskin.txt`, `irq2in.txt`
* **Output**:

  * `memout.txt`, `regout.txt`
  * `trace.txt`, `hwregtrace.txt`, `cycles.txt`
  * `leds.txt`, `display7seg.txt`, `diskout.txt`
  * `monitor.txt`, `monitor.yuv`
* **CLI Usage**:

```bash
sim.exe memin.txt diskin.txt irq2in.txt memout.txt regout.txt trace.txt \
        hwregtrace.txt cycles.txt leds.txt display7seg.txt \
        diskout.txt monitor.txt monitor.yuv
```

### Features

* Cycle-accurate fetch-decode-execute loop
* Handles interrupts, IO memory-mapped registers
* Supports: timer, LEDs, 7-segment, monitor display, disk DMA

---

## Test Programs

Each test case resides in its own directory and contains the following files:

* `*.asm`: source file
* `memin.txt`, `diskin.txt`, `irq2in.txt`
* Outputs: `memout.txt`, `regout.txt`, `trace.txt`, `hwregtrace.txt`, etc.

### 1. `sort.asm`

* Bubble sort 16 numbers (at memory 0x100–0x10F)
* Sorted output in same memory addresses

### 2. `factorial.asm`

* Recursive factorial of value at 0x100
* Result written to 0x101

### 3. `rectangle.asm`

* Draws a white-outlined rectangle on screen
* Framebuffer pixel coordinates are in 0x100–0x103 (A–D corners)

### 4. `disktest.asm`

* Reads sectors 0–3 from disk, sums them, writes result to sector 4

---

## Build & Run Requirements

* Platform: **Windows**, Visual Studio Community Edition 2022
* Submit two separate C projects:

  * `assembler` with `.sln` file
  * `simulator` with `.sln` file
* Each project must compile and run independently via **Build Solution**
* Do not hardcode filenames — read them from CLI arguments

---

## Submission Guidelines

* Documentation file: `project1_<ID1>_<ID2>_<ID3>.pdf`
* Include all `.asm`, I/O, and C source files
* Place each test case in its own folder with required inputs/outputs
* Submit a `.zip` containing all components, clearly structured

---

## Notes

* The assembler and simulator will be tested with additional unseen programs
* All I/O files must exist even if unused (can be empty)
* Avoid hardcoding; support command line parameter parsing

---

## Tools & References

* Monitor viewer: [yuvplayer](https://github.com/Tee0125/yuvplayer) with settings:

  * `size = 256x256`, `color = Y`

---

## Contact

For any project-related questions, consult the course forum or reach out to the teaching staff.
