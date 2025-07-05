#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<stdint.h>
#include<assert.h>

#define NUMBER_OF_INPUT_ARGS (14)
#define MAX_LINE_LENGTH (500)
#define NUMBER_OF_REGS (16)
#define NUMBER_OF_IO_REGS (23)
#define MONITOR_SIZE (256*256)
#define MEMORY_SIZE (4096)
#define MAX_NUM_OF_IRQ2 (4096) 
#define DISK_NUM_OF_SECTORS (128)
#define DISK_SECTOR_SIZE (128) 
#define DISK_SIZE (128*128)
#define DISK_RW_TIME (1024) // Disk read/write time - 1024 clock cycles


typedef struct _instruction
{
    uint8_t  opcode;      // 8 bits
    uint8_t  rd   : 4;
    uint8_t  rs   : 4;
    uint8_t  rt   : 4;
    uint8_t  reserved : 3; // always 0 in memory
    uint8_t  bigimm   : 1;
    uint8_t  imm8;        // 8 bits – meaningful only when bigimm == 0
    int32_t  imm32;       // store sign-extended 32-bit constant here - meaningful only when bigimm == 1
} instruction;

// Simulator data & state
typedef struct simulator {
    unsigned int PC;
    unsigned int regs[NUMBER_OF_REGS];
    unsigned int io_regs[NUMBER_OF_IO_REGS];
    uint8_t      monitor[MONITOR_SIZE];
    unsigned int memory[MEMORY_SIZE];  // array of instructions
    unsigned int disk[DISK_SIZE];
    unsigned int cycles_of_irq2[MAX_NUM_OF_IRQ2];
    unsigned int current_irq2;
    unsigned int disk_dcnt;
    bool         handling_irq;
    bool         run_en;
    int          curr_monitor_index;
    instruction* pending_inst;
    bool    in_second_cycle_of_bigimm;  // Track if we're in second cycle
} simulator;

typedef enum _cmd_line_arg {
    CMD_LINE_ARG_MEMIN = 1,
    CMD_LINE_ARG_DISKIN = 2,
    CMD_LINE_ARG_IRQ2IN = 3,
    CMD_LINE_ARG_MEMOUT = 4,
    CMD_LINE_ARG_REGOUT = 5,
    CMD_LINE_ARG_TRACE = 6,
    CMD_LINE_ARG_HWREGTRACE = 7,
    CMD_LINE_ARG_CYCLES = 8,
    CMD_LINE_ARG_LEDS = 9,
    CMD_LINE_ARG_DISPLAY7SEG = 10,
    CMD_LINE_ARG_DISKOUT = 11,
    CMD_LINE_ARG_MONITOR = 12,
    CMD_LINE_ARG_MONITOR_YUV = 13,
} cmd_line_arg;

// Map opcode name to number 
typedef enum _opcode {
    ADD  = 0,
    SUB  = 1,
    MUL  = 2,
    AND  = 3,
    OR   = 4,
    XOR  = 5,
    SLL  = 6,
    SRA  = 7,
    SRL  = 8,
    BEQ  = 9,
    BNE  = 10,
    BLT  = 11,
    BGT  = 12,
    BLE  = 13,
    BGE  = 14,
    JAL  = 15,
    LW   = 16,
    SW   = 17,
    RETI = 18,
    IN   = 19,
    OUT  = 20,
    HALT = 21,
} opcode;

// Map register name to number 
typedef enum _reg {
    REG_ZERO = 0,
    REG_IMM  = 1,
    REG_V0   = 2,
    REG_A0   = 3,
    REG_A1   = 4,
    REG_A2   = 5,
    REG_A3   = 6,
    REG_T0   = 7,
    REG_T1   = 8,
    REG_T2   = 9,
    REG_S0   = 10,
    REG_S1   = 11,
    REG_S2   = 12,
    REG_GP   = 13,
    REG_SP   = 14,
    REG_RA   = 15,
} reg;

// Map IO register name to number 
typedef enum _io_reg {
    IO_REG_IRQ_0_ENABLE = 0,
    IO_REG_IRQ_1_ENABLE = 1,
    IO_REG_IRQ_2_ENABLE = 2,
    IO_REG_IRQ_0_STATUS = 3,
    IO_REG_IRQ_1_STATUS = 4,
    IO_REG_IRQ_2_STATUS = 5,
    IO_REG_IRQ_HANDLER = 6,
    IO_REG_IRQ_RETURN = 7,
    IO_REG_CLKS = 8,
    IO_REG_LEDS = 9,
    IO_REG_DISPLAY_7_SEG = 10,
    IO_REG_TIMER_ENABLE = 11,
    IO_REG_TIMER_CURRENT = 12,
    IO_REG_TIMER_MAX = 13,
    IO_REG_DISK_CMD = 14,
    IO_REG_DISK_SECTOR = 15,
    IO_REG_DISK_BUFFER = 16,
    IO_REG_DISK_STATUS = 17,
    IO_REG_RESERVED_1 = 18,
    IO_REG_RESERVED_2 = 19,
    IO_REG_MONITOR_ADDR = 20,
    IO_REG_MONITOR_DATA = 21,
    IO_REG_MONITOR_CMD = 22,
} io_reg;

// File handling functions
FILE* open_file_to_read(char* fp);
FILE* open_file_to_write(char* fp);
FILE* open_file_to_append(char* fp);

// Load data from input files to simulator struct: memin.txt (type 1), diskin.txt (type 2), irq2in.txt (type 3)
void load_data_from_input_file(simulator* simulator, FILE* fh, int type);

// Initialize simulation: 
// 1. Validate input files
// 2. Handle input files: load memory, load disk content, load irq2 times
// 3. Initialize parameters and monitor to zero
void init_simulator(simulator* simulator, char* memin_fp, char* diskin_fp, char* irq2in_fp);
instruction *initialize_instruction(simulator* sim);

// Run simulation:
// 2. Run fetch-decode-execute loop
// 3. Write output files 
void run_simulator(simulator* simulator, char* memout_fp, char* regout_fp, char* trace_fp, char* hwregtrace_fp, char* cycles_fp, char* leds_fp,
    char* display7seg_fp, char* diskout_fp, char* monitor_txt_fp, char* monitor_yuv_fp);

// Write output files functions (hwregtrace.txt is not written in a function, but in the main fetch-decode-execute loop)
void write_output_file_memout(simulator* simulator, char* memout_fp);
void write_output_file_regout(simulator* simulator, char* regout_fp);
void write_output_file_trace(simulator* simulator, char* trace_fh, instruction* inst);
void write_output_file_cycles(simulator* simulator, char* cycles_fp);
void write_output_file_diskout(simulator* simulator, char* diskout_fp);
void write_output_file_monitor(simulator* simulator, char* monitor_txt_fp, char* monitor_yuv_fp);

// Disk operations functions 
void disk_cmd_handler(simulator* simulator, instruction* inst);
void disk_main_handler(simulator* simulator);
void read_from_disk(simulator* simulator);
void write_to_disk(simulator* simulator);

// Clock increment function
void inc_clk(simulator* simulator);

// IRQ handling functions 
void timer(simulator* simulator); // IRQ0
void irq2(simulator* simulator); // IRQ2
void irq_handler(simulator * simulator, instruction * inst, int clock_cycles);
