#include "sim.h"

////////////////////////////////////////////////////////////
//////// GENERAL FILE HANDLING FUNCTIONS 
///////////////////////////////////////////////////////////

FILE* open_file_to_read(char* fp)
{
    FILE* fh = fopen(fp, "r");
    if (fh == NULL) assert(fh != NULL && "-ERROR- Could not open file for reading!"); // Handle fopen fail
    return fh;
};

FILE* open_file_to_write(char* fp)
{
    FILE* fh = fopen(fp, "w");
    if (fh == NULL) assert(fh != NULL && "-ERROR- Could not open file for writing!"); // Handle fopen fail
    return fh;
};

FILE* open_file_to_append(char* fp)
{
    FILE* fh = fopen(fp, "a");
    if (fh == NULL) assert(fh != NULL && "-ERROR- Could not open file for appending!"); // Handle fopen fail
    return fh;
};

////////////////////////////////////////////////////////////
//////// SIMULATOR INITIALIZING FUNCTIONS  
///////////////////////////////////////////////////////////

void load_data_from_input_file(simulator* simulator, FILE* fh, int type)
{
    int i = 0;
    char line[MAX_LINE_LENGTH] = { 0 };
    char* curr_line = NULL;

    // Iterate over input file and write to the correct simulator field
    curr_line = fgets(line, MAX_LINE_LENGTH, fh);
    while (curr_line != NULL)
    {
        switch (type)
        {
        case 1: simulator->memory[i++] = (unsigned int)strtol(line, NULL, 16); break; // memin 
        case 2: simulator->disk[i++] = (unsigned int)strtol(line, NULL, 16); break; // diskin 
        case 3: simulator->cycles_of_irq2[i++] = (unsigned int)strtol(line, NULL, 10); break; // irq2in
        }
        curr_line = fgets(&(line[0]), MAX_LINE_LENGTH, fh);
    }
}

void init_simulator(simulator* simulator, char* memin_fp, char* diskin_fp, char* irq2in_fp)
{
    FILE* memin_fh = NULL;
    FILE* diskin_fh = NULL;
    FILE* irq2in_fh = NULL;


    // Validate arguments
    assert(simulator != NULL && "-ERROR- You entered invalid simulator pointer when calling init_simulator function");
    assert(memin_fp != NULL && "-ERROR- You entered invalid memin file path when calling init_simulator function");
    assert(diskin_fp != NULL && "-ERROR- You entered invalid diskin file path when calling init_simulator function");
    assert(irq2in_fp != NULL && "-ERROR- You entered invalid irq2in file path when calling init_simulator function");

    // Open input files
    memin_fh = open_file_to_read(memin_fp);
    diskin_fh = open_file_to_read(diskin_fp);
    irq2in_fh = open_file_to_read(irq2in_fp);

    // Initialize simulator fields value to 0 
    simulator->PC = 0;

    for (int i = 0; i < NUMBER_OF_REGS; i++)
    {
        simulator->regs[i] = 0;
    }
    for (int j = 0; j < NUMBER_OF_IO_REGS; j++)
    {
        simulator->io_regs[j] = 0;
    }
    memset(simulator->monitor, 0, MONITOR_SIZE * sizeof(uint8_t));
    memset(simulator->memory, 0, MEMORY_SIZE * sizeof(unsigned int));
    memset(simulator->disk, 0, DISK_SIZE * sizeof(unsigned int));
    memset(simulator->cycles_of_irq2, 0, MAX_NUM_OF_IRQ2 * sizeof(unsigned int));

    simulator->current_irq2 = 0;
    simulator->disk_dcnt = 0;
    simulator->handling_irq = false;
    simulator->run_en = false;
    simulator->curr_monitor_index = 0;

    // Load data from input files to simulator struct -> instructions from memin.txt, disk content from diskin.txt, cycles to raise irq2 from irq2in.txt
    load_data_from_input_file(simulator, memin_fh, 1);
    load_data_from_input_file(simulator, diskin_fh, 2);
    load_data_from_input_file(simulator, irq2in_fh, 3);

    // Close input files 
    fclose(memin_fh);
    fclose(diskin_fh);
    fclose(irq2in_fh);
}

instruction* initialize_instruction(simulator* sim)
{
    instruction* inst = (instruction*)malloc(sizeof(instruction)); // Free in the end of each while loop interaction
    unsigned int opcode = 0;
    unsigned int rd = 0;
    unsigned int rs = 0;
    unsigned int rt = 0;
    unsigned int instruction_line = sim->memory[sim->PC]; // Load current instruction based on current PC

    // Extract imm8 and bigimm flag before shifting
    unsigned int imm8 = instruction_line & 0xFF;         // bits[7:0]
    unsigned int bigimm = (instruction_line >> 8) & 0x1; // bit[8]
    instruction_line >>= 12;                             // skip reserved + imm fields

    // Isolate each register and opcode field using masking
    rt = instruction_line & 0xF;
    instruction_line >>= 4;
    rs = instruction_line & 0xF;
    instruction_line >>= 4;
    rd = instruction_line & 0xF;
    instruction_line >>= 4;
    opcode = instruction_line & 0xFF;

    // Instruction initialization
    inst->i_type = bigimm ? true : false;
    if (bigimm)
    {
        inst->imm = sim->memory[sim->PC + 1];
        sim->regs[REG_IMM] = (int32_t)inst->imm;
    }
    else if (rd == REG_IMM || rs == REG_IMM || rt == REG_IMM)
    {
        // Small immediate encoded in imm8 field
        int32_t sext = (int8_t)(imm8 & 0xFF); // sign extend 8-bit
        inst->imm = sext;
        sim->regs[REG_IMM] = sext;
    }
    else
    {
        inst->imm = 0;
    }
    inst->opcode = opcode;
    inst->rd = rd;
    inst->rt = rt;
    inst->rs = rs;
    return inst;
}

////////////////////////////////////////////////////////////
//////// CLOCK FUNCTIONS  
///////////////////////////////////////////////////////////

void inc_clk(simulator* sim)
{
    sim->io_regs[IO_REG_CLKS] = (sim->io_regs[IO_REG_CLKS] == 0xFFFFFFFF) ? 0 : (sim->io_regs[IO_REG_CLKS] + 1);
}

////////////////////////////////////////////////////////////
//////// DISK HANDLING FUNCTIONS  
///////////////////////////////////////////////////////////

void disk_cmd_handler(simulator* sim, instruction* inst)
{
    sim->io_regs[IO_REG_DISK_CMD] = sim->regs[inst->rd];; // Set diskcmd to 1 for Reading and 2 for Writing
    sim->io_regs[IO_REG_DISK_STATUS] = 1; // Change status to busy 
    sim->disk_dcnt = 1024; // Re-set counter to 1024
}

void disk_main_handler(simulator* sim)
{
    if (sim->io_regs[IO_REG_DISK_STATUS]) // If diskstatus == 1 <-> if disk is currently handling read/write 
    {
        sim->disk_dcnt--;  // Count 1024 clock cycles 
        if (!sim->disk_dcnt) // 1024 cycles passed from initial read/write request -> reset status and registers + read or write to the memory
        {
            // Assert IRQ1 -> Disk is ready to receive a new command
            sim->io_regs[IO_REG_IRQ_1_STATUS] = 1;
            // Read/Write operations
            if (sim->io_regs[IO_REG_DISK_CMD] == 1) read_from_disk(sim); // Read
            if (sim->io_regs[IO_REG_DISK_CMD] == 2) write_to_disk(sim); // Write
            // Reset disk_status and disk_cmd registers
            sim->io_regs[IO_REG_DISK_STATUS] = 0;
            sim->io_regs[IO_REG_DISK_CMD] = 0;
        }
    }
}

void read_from_disk(simulator* sim)
{
    int sector = sim->io_regs[IO_REG_DISK_SECTOR];
    int buffer = sim->io_regs[IO_REG_DISK_BUFFER];

    for (int i = 0; i < DISK_SECTOR_SIZE; i++)
    {
        sim->memory[buffer + i] = sim->disk[DISK_SECTOR_SIZE * sector + i]; // DMA read
    }
}

void write_to_disk(simulator* sim)
{
    int sector = sim->io_regs[IO_REG_DISK_SECTOR];
    int buffer = sim->io_regs[IO_REG_DISK_BUFFER];

    for (int i = 0; i < DISK_SECTOR_SIZE; i++)
    {
        sim->disk[DISK_SECTOR_SIZE * sector + i] = sim->memory[buffer + i]; // DMA write
    }
}

////////////////////////////////////////////////////////////
//////// IRQ HANDLING FUNCTIONS  
///////////////////////////////////////////////////////////

void timer(simulator* sim) // IRQ0 - Timer 
{
    if (sim->io_regs[IO_REG_TIMER_ENABLE])
    {
        if (sim->io_regs[IO_REG_TIMER_CURRENT] == sim->io_regs[IO_REG_TIMER_MAX]) // If timer gets max value, reset it and assert IRQ0
        {
            sim->io_regs[IO_REG_IRQ_0_STATUS] = 1; // IRQ0 is triggered
            sim->io_regs[IO_REG_TIMER_CURRENT] = 0;
        }
        else sim->io_regs[IO_REG_TIMER_CURRENT]++;
    }
}

void irq2(simulator* sim) // IRQ2 - external file 
{
    if ((sim->io_regs[IO_REG_CLKS]) == sim->cycles_of_irq2[sim->current_irq2])
    {
        sim->io_regs[IO_REG_IRQ_2_STATUS] = 1;
        sim->current_irq2++;
    }
    else (sim->io_regs[IO_REG_IRQ_2_STATUS] = 0);
}

////////////////////////////////////////////////////////////
//////// WRITE OUTPUT FILES FUNCTIONS  
///////////////////////////////////////////////////////////

void write_output_file_memout(simulator* simulator, char* memout_fp)
{
    FILE* memout_fh = open_file_to_write(memout_fp);
    for (int i = 0; i < MEMORY_SIZE; i++)
    {
        fprintf(memout_fh, "%08X\n", simulator->memory[i]);
    }
    fclose(memout_fh);
    return;
}

void write_output_file_regout(simulator* simulator, char* regout_fp)
{
    FILE* regout_fh = open_file_to_write(regout_fp);
    for (int i = 2; i < NUMBER_OF_REGS; i++) {
        fprintf(regout_fh, "%08X\n", simulator->regs[i]);
    }
    fclose(regout_fh);
    return;
}

void write_output_file_trace(simulator* simulator, char* trace_fp, instruction* inst)
{
    FILE* trace_fh = open_file_to_append(trace_fp);
    unsigned int R1 = inst->i_type ? inst->imm : 0;

    fprintf(trace_fh, "%03X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X\n",
        simulator->PC,
        simulator->memory[simulator->PC],
        simulator->regs[0], // 8 zeros in R0 field ($zero is a protected register and will always hold 0)
        R1, // if R-type 0x0, if I-type sign-extended imm 
        simulator->regs[2],
        simulator->regs[3],
        simulator->regs[4],
        simulator->regs[5],
        simulator->regs[6],
        simulator->regs[7],
        simulator->regs[8],
        simulator->regs[9],
        simulator->regs[10],
        simulator->regs[11],
        simulator->regs[12],
        simulator->regs[13],
        simulator->regs[14],
        simulator->regs[15]);
    fclose(trace_fh);
}

// hwregtrace.txt is written during the main while loop in run_simulator

void write_output_file_cycles(simulator* simulator, char* cycles_fp)
{
    FILE* cycles_fh = open_file_to_write(cycles_fp);
    fprintf(cycles_fh, "%u", simulator->io_regs[IO_REG_CLKS]);
    fclose(cycles_fh);
    return;
}

// leds.txt & display7seg.txt are written during the main while loop in run_simulator

void write_output_file_diskout(simulator* simulator, char* diskout_fp)
{
    FILE* diskout_fh = open_file_to_write(diskout_fp);
    for (int i = 0; i < DISK_SIZE; i++)
    {
        fprintf(diskout_fh, "%08X\n", simulator->disk[i]);
    }
    fclose(diskout_fh);
    return;
}

void write_output_file_monitor(simulator* simulator, char* monitor_txt_fp, char* monitor_yuv_fp)
{
    FILE* monitor_txt_fh = open_file_to_write(monitor_txt_fp);
    FILE* monitor_yuv_fh = open_file_to_write(monitor_yuv_fp);

    // This function writes both monitor.txt and monitor.yuv output files, based on simulator->monitor array at the end of the simulator run 
    for (int i = 0; i < MONITOR_SIZE; i++)
    {
        // Write each pixel to monitor.txt in a new line, in 2 hex digits
        fprintf(monitor_txt_fh, "%02X\n", simulator->monitor[i]);
        // Write each pixel to monitor.yuv in a new line, in 8 binary digits
        fwrite(&simulator->monitor[i], 1, 1, monitor_yuv_fh);
    }

    fclose(monitor_txt_fh);
    fclose(monitor_yuv_fh);
}

////////////////////////////////////////////////////////////
//////// RUN SIMULATOR FUNCTION
////////////////////////////////////////////////////////////

void run_simulator(simulator* simulator, char* memout_fp, char* regout_fp, char* trace_fp, char* hwregtrace_fp, char* cycles_fp, char* leds_fp, char* display7seg_fp, char* diskout_fp, char* monitor_txt_fp, char* monitor_yuv_fp)
{
    // Validate arguments
    assert(simulator != NULL && "-ERROR- You entered invalid simulator pointer when calling run_simulator function");
    assert(memout_fp != NULL && "-ERROR- You entered invalid memout file path when calling run_simulator function");
    assert(regout_fp != NULL && "-ERROR- You entered invalid regout file path when calling run_simulator function");
    assert(trace_fp != NULL && "-ERROR- You entered invalid trace file path when calling run_simulator function");
    assert(hwregtrace_fp != NULL && "-ERROR- You entered invalid hwregtrace file path when calling run_simulator function");
    assert(cycles_fp != NULL && "-ERROR- You entered invalid cycles file path when calling run_simulator function");
    assert(leds_fp != NULL && "-ERROR- You entered invalid leds file path when calling run_simulator function");
    assert(display7seg_fp != NULL && "-ERROR- You entered invalid display7seg file path when calling run_simulator function");
    assert(monitor_txt_fp != NULL && "-ERROR- You entered invalid monitor_txt file path when calling run_simulator function");
    assert(monitor_yuv_fp != NULL && "-ERROR- You entered invalid monitor_yuv file path when calling run_simulator function");

    // assert run_en to start simulation  
    simulator->run_en = true;
    bool branch_en = false; // Branch done and PC changed accordingly
    // Initialize IRQ
    int irq = 0;
    //simulator->io_regs[IO_REG_IRQ_2_ENABLE] = 1; // Enable irq2

    // hwregtraxe, leds & display7seg files handling
    FILE* hwregtrace_fh = open_file_to_write(hwregtrace_fp);
    FILE* leds_fh = open_file_to_write(leds_fp);
    FILE* display7seg_fh = open_file_to_write(display7seg_fp);
    // Initialize trace file 
    FILE* trace_fh = open_file_to_write(trace_fp);
    fclose(trace_fh);
    // IO names pattern
    const char* io_names[] = { "irq0enable", "irq1enable", "irq2enable", "irq0status", "irq1status", "irq2status", "irqhandler", "irqreturn", "clks", "leds", "display7seg",
                              "timerenable", "timercurrent", "timermax", "diskcmd", "disksector", "diskbuffer", "diskstatus", "reserved", "reserved", "monitoraddr", "monitordata", "monitorcmd" };

    // Run fetch-decode-execute loop until HALT
    while (simulator->run_en)
    {
        int invalid_write_attempt = 0;
        branch_en = false;

        // Fetch an instruction 
        instruction* inst = initialize_instruction(simulator);

        // Interrupts trigering
        timer(simulator); // IRQ0
        disk_main_handler(simulator); // IRQ1
        irq2(simulator);  // IRQ2
        irq = (simulator->io_regs[IO_REG_IRQ_0_ENABLE] && simulator->io_regs[IO_REG_IRQ_0_STATUS]) ||
            (simulator->io_regs[IO_REG_IRQ_1_ENABLE] && simulator->io_regs[IO_REG_IRQ_1_STATUS]) ||
            (simulator->io_regs[IO_REG_IRQ_2_ENABLE] && simulator->io_regs[IO_REG_IRQ_2_STATUS]);

        // Write trace.txt (before any changes to registers are done) 
        write_output_file_trace(simulator, trace_fp, inst); // Write registers to trace output file before executing the instruction

        // Protect $zero and $imm registers from writing 
        invalid_write_attempt = (inst->rd == 0 || inst->rd == 1) &&
            (inst->opcode == ADD || inst->opcode == SUB || inst->opcode == MUL || inst->opcode == AND || inst->opcode == OR || inst->opcode == XOR ||
                inst->opcode == SLL || inst->opcode == SRA || inst->opcode == SRL || inst->opcode == JAL || inst->opcode == LW || inst->opcode == SW || inst->opcode == IN);

        if (invalid_write_attempt)
        {
            simulator->PC++; // Increase PC once done with an instruction execution
            if (inst->i_type) simulator->PC++; // If the current instruction is I-type, increase the PC by 2
            inc_clk(simulator); // Increase clock cycle count
            continue;
        }

        // Choose operation mode based on opcode
        switch (inst->opcode) {

            // Register and logic commands:
        case ADD: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] + simulator->regs[inst->rt]; break;
        } // R[rd] = R[rs] + R[rt]
        case SUB: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] - simulator->regs[inst->rt]; break;
        } // R[rd] = R[rs] - R[rt]
        case MUL: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] * simulator->regs[inst->rt]; break;
        } // R[rd] = R[rs] * R[rt]
        case AND: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] & simulator->regs[inst->rt]; break;
        } // R[rd] = R[rs] AND R[rt]
        case OR: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] | simulator->regs[inst->rt]; break;
        } // R[rd] = R[rs] OR R[rt]
        case XOR: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] ^ simulator->regs[inst->rt]; break;
        } // R[rd] = R[rs] XOR R[rt]
        case SLL: {
            simulator->regs[inst->rd] = simulator->regs[inst->rs] << (simulator->regs[inst->rt]);
            break;
        } // R[rd] = R[rs] SLL R[rt]
        case SRA: {
            if (simulator->regs[inst->rt] < 0) {
                simulator->regs[inst->rd] = (simulator->regs[inst->rs] >> simulator->regs[inst->rt]) | ~(~(0x0) >> simulator->regs[inst->rt]);
            }
            else {
                simulator->regs[inst->rd] = (simulator->regs[inst->rs] >> simulator->regs[inst->rt]);
            }
            break;
        } // R[rd] = R[rs] SRA R[rt]
        case SRL: {
            simulator->regs[inst->rd] = (simulator->regs[inst->rs] >> simulator->regs[inst->rt]); break;
        } // R[rd] = R[rs] SRL R[rt]

// Branch commands: 
        case BEQ: {
            if (simulator->regs[inst->rs] == simulator->regs[inst->rt])
            {
                simulator->PC = simulator->regs[inst->rd];
                branch_en = true;
            }
            break;
        } // if (R[rs] == R[rt]) pc = R[rd]
        case BNE: {
            if (simulator->regs[inst->rs] != simulator->regs[inst->rt])
            {
                simulator->PC = simulator->regs[inst->rd];
                branch_en = true;
            }
            break;
        } // if (R[rs] != R[rt]) pc = R[rd]
        case BLT: {
            if (simulator->regs[inst->rs] < simulator->regs[inst->rt])
            {
                simulator->PC = simulator->regs[inst->rd];
                branch_en = true;
            }
            break;
        } // if (R[rs] < R[rt]) pc = R[rd]
        case BGT: {
            if (simulator->regs[inst->rs] > simulator->regs[inst->rt])
            {
                simulator->PC = simulator->regs[inst->rd];
                branch_en = true;
            }
            break;
        } // if (R[rs] > R[rt]) pc = R[rd]
        case BLE: {
            if (simulator->regs[inst->rs] <= simulator->regs[inst->rt])
            {
                simulator->PC = simulator->regs[inst->rd];
                branch_en = true;
            }
            break;
        } // if (R[rs] <= R[rt]) pc = R[rd]
        case BGE: {
            if (simulator->regs[inst->rs] >= simulator->regs[inst->rt])
            {
                simulator->PC = simulator->regs[inst->rd];
                branch_en = true;
            }
            break;
        } // if (R[rs] >= R[rt]) pc = R[rd]
    // Jump and link command:
        case JAL: {
            simulator->regs[inst->rd] = simulator->PC + (inst->i_type == true) + 1; // R[rd] = next instruction address
            simulator->PC = simulator->regs[inst->rs]; //  pc = R[rs]
            branch_en = true;
            break;
        }

                // Memory commands:
        case LW: {
            simulator->regs[inst->rd] = simulator->memory[simulator->regs[inst->rs] + simulator->regs[inst->rt]]; break;
        } // R[rd] = MEM[R[rs]+R[rt]], with sign extension
        case SW: {
            simulator->memory[simulator->regs[inst->rs] + simulator->regs[inst->rt]] = (simulator->regs[inst->rd] & 0X000FFFFF); break;
        } // MEM[R[rs]+R[rt]] = R[rd] (bits 19:0)

//IRQ command:
        case RETI: {
            simulator->PC = simulator->io_regs[IO_REG_IRQ_RETURN];
            simulator->handling_irq = false; // Disable irq handling flag
            branch_en = true;
            break;
        } // PC = IORegister[7]

// IO commands:
        case IN: {
            if ((simulator->regs[inst->rs] + simulator->regs[inst->rt]) == IO_REG_MONITOR_CMD) { simulator->regs[inst->rd] = 0; }  // Reading from monitorcmd register will return 0
            else simulator->regs[inst->rd] = simulator->io_regs[simulator->regs[inst->rs] + simulator->regs[inst->rt]];        // R[rd] = IORegister[R[rs] + R[rt]]
            fprintf(hwregtrace_fh, "%d READ %s %08X\n", simulator->io_regs[IO_REG_CLKS], io_names[simulator->regs[inst->rs] + simulator->regs[inst->rt]], simulator->io_regs[simulator->regs[inst->rs] + simulator->regs[inst->rt]]);
            break;
        }

        case OUT: {
            if (((simulator->regs[inst->rs] + simulator->regs[inst->rt]) == IO_REG_DISK_CMD) && (!simulator->io_regs[IO_REG_DISK_STATUS])) // Handle Disk R/W command only if disk status is 0
            {
                disk_cmd_handler(simulator, inst);
            }

            else if (((simulator->regs[inst->rs] + simulator->regs[inst->rt]) == IO_REG_MONITOR_CMD) && (simulator->regs[inst->rd])) // If instruction writes 1 to monitorcmd, update relevant pixel
            {
                simulator->monitor[simulator->io_regs[IO_REG_MONITOR_ADDR]] = simulator->io_regs[IO_REG_MONITOR_DATA];
                simulator->curr_monitor_index = simulator->io_regs[IO_REG_MONITOR_ADDR];
            }

            else {

                if (((simulator->regs[inst->rs] + simulator->regs[inst->rt]) == IO_REG_LEDS) && (simulator->regs[inst->rd] != simulator->io_regs[IO_REG_LEDS])) // If leds are changed, update leds output file
                {
                    simulator->io_regs[simulator->regs[inst->rs] + simulator->regs[inst->rt]] = simulator->regs[inst->rd]; // IORegister [R[rs]+R[rt]] = R[rd]
                    fprintf(leds_fh, "%d %08X\n", simulator->io_regs[IO_REG_CLKS], simulator->io_regs[IO_REG_LEDS]);
                }
                if (((simulator->regs[inst->rs] + simulator->regs[inst->rt]) == IO_REG_DISPLAY_7_SEG) && (simulator->regs[inst->rd] != simulator->io_regs[IO_REG_DISPLAY_7_SEG])) // If seg diaplay is changed, update display7seg output file
                {
                    simulator->io_regs[simulator->regs[inst->rs] + simulator->regs[inst->rt]] = simulator->regs[inst->rd]; // IORegister [R[rs]+R[rt]] = R[rd]
                    fprintf(display7seg_fh, "%d %08X\n", simulator->io_regs[IO_REG_CLKS], simulator->io_regs[IO_REG_DISPLAY_7_SEG]);
                }
                simulator->io_regs[simulator->regs[inst->rs] + simulator->regs[inst->rt]] = simulator->regs[inst->rd]; // IORegister [R[rs]+R[rt]] = R[rd]
            } // End of else - main writing

            fprintf(hwregtrace_fh, "%d WRITE %s %08X\n", simulator->io_regs[IO_REG_CLKS], io_names[simulator->regs[inst->rs] + simulator->regs[inst->rt]], simulator->regs[inst->rd]);
            break;
        }

        case HALT: {
            simulator->run_en = false; break;
        } // Halt execution, exit simulator

        default:
            break;
        } // End of switch(inst->opcode)

        if (!branch_en)
        {
            simulator->PC++; // Increase PC once done with an instruction execution
            if (inst->i_type) simulator->PC++; // If the current instruction is I-type, increase the PC by 2
        }

        // Check for & handle interrupts
        if (irq && !simulator->handling_irq) // IRQ asserted and CPU is not handling another IRQ
        {
            simulator->io_regs[IO_REG_IRQ_RETURN] = simulator->PC; // Save current PC into irq_return hwreg
            simulator->PC = simulator->io_regs[IO_REG_IRQ_HANDLER];// Change current PC to irq_handler
            simulator->handling_irq = true; // Assert irq handling flag
        }
        inc_clk(simulator); // Increase clock cycle counter
        free(inst);
    } // End of while(simulator->run_en)

    // When the simulator stops running (after a halt instruction), write the rest of the output files (that aren't written during the simulator run)
    write_output_file_memout(simulator, memout_fp);
    write_output_file_regout(simulator, regout_fp);
    write_output_file_cycles(simulator, cycles_fp);
    write_output_file_diskout(simulator, diskout_fp);
    write_output_file_monitor(simulator, monitor_txt_fp, monitor_yuv_fp);
    fclose(hwregtrace_fh);
    fclose(display7seg_fh);
    fclose(leds_fh);
}

////////////////////////////////////////////////////////////
//////// MAIN  FUNCTION
///////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    simulator* sim = (simulator*)malloc(sizeof(simulator));

    // Check if the number of input arguments is valid 
    if (NUMBER_OF_INPUT_ARGS != argc)
    {
        printf("Wrong input, you gave %d arguments instead of %d", argc, NUMBER_OF_INPUT_ARGS);
        return 1;
    }

    // Initialize simulation, open input files & load data to memory and disk   
    init_simulator(sim, argv[CMD_LINE_ARG_MEMIN], argv[CMD_LINE_ARG_DISKIN], argv[CMD_LINE_ARG_IRQ2IN]);

    // Run simulation, handle instructions & write output files 
    run_simulator(sim, argv[CMD_LINE_ARG_MEMOUT], argv[CMD_LINE_ARG_REGOUT], argv[CMD_LINE_ARG_TRACE], argv[CMD_LINE_ARG_HWREGTRACE],
        argv[CMD_LINE_ARG_CYCLES], argv[CMD_LINE_ARG_LEDS], argv[CMD_LINE_ARG_DISPLAY7SEG], argv[CMD_LINE_ARG_DISKOUT], argv[CMD_LINE_ARG_MONITOR], argv[CMD_LINE_ARG_MONITOR_YUV]);
    free(sim);
    return 0;
}