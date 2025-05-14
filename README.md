# simp_project_adam-or
**Rewriting the plan**

The user has requested I condense a previous 8-9 week plan into a 6-week version with more details. I'll rework the plan to fit 6 weeks, breaking down tasks into specific phases, weeks, and even days if that helps. The uploaded file offers further context, and I'll ensure I cite it properly as “isaproject\_040425.pdf.” No need to automate reminders, though I can offer them if the user asks. I’ll focus on a clear, well-structured text response, including the tasks for each of the 6 weeks.


**Assigning citations**

For Week 1, I’ll cite references related to Visual Studio specifications (turn1file6) and diagram scope (turn1file3). In Week 2, assembler tasks can be cited with turn1file19 and turn1file18, focusing on details like memin.txt creation. For the assembler’s .word and instruction encoding, I can pull from turn1file0 and turn1file16. Week 3 will involve remaining assembler and tests; Week 3-4 covers the simulator core with turn1file2 for the instruction set. I/O and interrupts in Week 4 will rely on turn1file5 and turn1file6.


Thought for 13 seconds


Here’s a six-week, detailed work plan to deliver both the SIMP assembler and simulator in C, per the project spec:

---

**Week 1 – Project Kickoff & Environment Setup**

1. **Deep-dive into the spec**:

   * Read through the entire PDF, highlighting assumptions (label syntax, max line length, whitespace rules) and deliverables .
   * Sketch the high-level system diagram: two separate C programs (assembler + simulator), I/O devices, memory map, test programs .
2. **Dev environment**:

   * Install **Visual Studio Community 2022** on Windows; set up one solution with two projects: `asm.exe` and `sim.exe` .
   * Verify that a clean **Build Solution** succeeds with empty “Hello, world” stubs.
3. **Version control & CI**:

   * Create a Git repo; establish `assembler` and `simulator` branches.
   * Add a GitHub Actions (or Azure Pipelines) workflow to build both projects on every push.

---

**Week 2 – Assembler Pass I (Symbol Resolution)**

1. **Command-line interface**:

   * Implement `asm.exe program.asm memin.txt`.
   * Validate argument parsing and error messages.
2. **Lexing & parsing**:

   * Tokenize each line (ignore extra spaces/tabs) and strip comments.
   * Support directives: `.word <addr> <value>` in decimal, hex (0x…), or symbolic .
3. **Pass 1 – Symbol table**:

   * On first scan, record every label’s PC (instruction count).
   * Enforce label rules: start with letter, up to 50 chars .
4. **Unit tests**:

   * Feed small snippets (e.g. single `.word`, one `add` instruction) and assert symbol table contents.

---

**Week 3 – Assembler Pass II (Encoding & Output)**

1. **Instruction encoding**:

   * Implement encoding of all 22 opcodes, fields (`opcode`,`rd`,`rs`,`rt`,`bigimm`,`imm8`/`imm32`) per the two-row format .
   * Handle `bigimm`=1 case by emitting the second 32-bit word.
2. **Emit `memin.txt`**:

   * Write each instruction word (in hex) to `memin.txt`, one per line.
3. **Directive expansion**:

   * Translate `.word` entries into appropriate memory words at the given addresses.
4. **End-to-end test**:

   * Assemble a small program (e.g. `add $t0,$t1,$t2`) and inspect `memin.txt` against hand-calculated values .

---

**Week 4 – Simulator Core**

1. **Fetch–Decode–Execute loop**:

   * Initialize PC=0; read `memin.txt` into an array of 32-bit words.
   * On each iteration, fetch one or two words (based on `bigimm`), decode fields, advance PC.
2. **Register file**:

   * Implement 32×16-bit registers (`$zero`, `$imm`, `$v0`, …, `$ra`) with read/write semantics .
3. **ALU & memory ops**:

   * Support arithmetic (`add`,`sub`,`mul`), bitwise, shifts, loads/stores (`lw`,`sw`).
   * Map memory accesses to a flat `uint16_t MEM[4096]`.
4. **Branches & jumps**:

   * Update PC for `beq`,`bne`,… and implement `jal`/`reti`.
5. **Cycle counting**:

   * Count 1 cycle for single-word, 2 cycles for two-word instructions; log to `cycles.txt` .

---

**Week 5 – I/O Devices & Interrupts**

1. **Hardware registers**:

   * Model `IORegister[0…22]` for LEDs, 7-seg, timer, disk, monitor .
   * Initialize to spec’d startup values on reset.
2. **`in` / `out` instructions**:

   * On every I/O access, append a line to `hwregtrace.txt` in the format `CYCLE READ/WRITE NAME DATA`.
3. **Timer & disk DMA**:

   * Implement timer increment, IRQ0 generation after `timermax` if `timerenable`=1.
   * For disk: when `diskcmd`≠0 and `diskstatus`=0, simulate 1024-cycle DMA, then raise IRQ1; transfer 128-word sectors between memory and an array initialized from `diskin.txt`.
4. **Interrupt handling**:

   * On any IRQ (0–2), if enabled, save PC to `irqreturn`, jump to `irqhandler`, support `reti` .

---

**Week 6 – Test Programs, Documentation & Final Packaging**

1. **Write & assemble test programs** cite:

   * **`sort.asm`**: Bubble sort 16 words at 0x100–0x10F; check `memout.txt` & `regout.txt`.
   * **`factorial.asm`**: Recursive factorial with result at 0x100.
   * **`rectangle.asm`**: Fill white rectangle in frame buffer; produce `monitor.yuv`.
   * **`disktest.asm`**: Sum sectors 0–3 into sector 4; compare `diskout.txt`.
   * Automate: batch script calls `asm.exe`+`sim.exe` for each, then `diff` against expected outputs.
2. **Project report**:

   * Write `project1_id1_id2_id3.pdf` containing architecture diagrams, data structures, instruction encoding tables, I/O design, interrupt flow charts, and sample runs .
3. **Code hygiene & comments**:

   * Ensure every function/module has clear comments explaining its role and edge-case handling.
4. **Final integration & submission**:

   * Run a full “smoke test” of all four programs in sequence.
   * Zip solution folder (source, `.sln`, test scripts, PDF) and submit per course naming conventions.

---

Adjust daily subtasks as needed based on your pace. Good luck!
