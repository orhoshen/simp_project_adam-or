#include "asm.h"
#define _CRT_SECURE_NO_WARNINGS

FILE* open_file_to_read(char* filePath)
{
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file %s for reading.\n", filePath);
        return NULL; // file is invalid
    }
    return fp;
}

FILE* open_file_to_write(char* filePath)
{
    FILE* fp = fopen(filePath, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not file %s for writing.\n", filePath);
        return NULL; // file is invalid
    }
    return fp;
}

void strip_comment(char* line) {
    char* comment_pos = strchr(line, '#');
    if (comment_pos != NULL) {
        *comment_pos = '\0'; // cut off the line at the '#'
    }
}

bool contains_label(char* token, int token_len) {
    return (token_len > 0 && token[token_len - 1] == ':'); 
}

void set_label(int pc, int local_index, int label_len, char* token, label* label_arr)
{
    label_arr[local_index].set = true;
    label_arr[local_index].line = pc;//save current PC
    token[label_len - 1] = '\0'; // remove the ":"
    strcpy(label_arr[local_index].name, token); //save label name without ':'
}

void set_word(char* token_add, char* token_data, word* words)
{
    int addr = convertToDecimal(token_add);
    if (addr < 0 || addr >= MEMORY_SIZE) {
        fprintf(stderr, "Error: .word address %d out of bounds (0-4095)\n", addr);
        return;
    }
    words[addr].address = (uint32_t)addr;
    words[addr].data = (uint32_t)convertToDecimal(token_data);
    words[addr].set = true;
}

// Function to convert a string representation of a number to integer
int convertToDecimal(const char* str) {
    // Check if the input string is empty
    if (str == NULL || *str == '\0') {
        fprintf(stderr, "Error: Empty input string.\n");
        return 0;
    }

    // Determine the base (hexadecimal or decimal)
    int base = 10; // Default to decimal
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        base = 16; // Hexadecimal if starts with "0x" or "0X"
        str += 2; // Skip "0x" or "0X" prefix
    }

    // Convert the string to decimal integer
    char* endptr;
    int result = strtol(str, &endptr, base);

    // Check for conversion errors
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid input string '%s'.\n", str);
        return 0;
    }

    return result; // Return the integer
}
// Helper to check if a string is a register name (starts with '$')
static inline bool is_register_name(char* s) {
    return s && s[0] == '$';
}

static inline void maybe_flush(int pc,
                               char *pending,
                               bool *has_pending,
                               int  *idx,
                               label *tbl)
{
    if (!*has_pending) return;
    tbl[*idx].set  = true;
    tbl[*idx].line = pc;
    strcpy(tbl[*idx].name, pending);
    *has_pending   = false;
    (*idx)++;
}

void parse_lines(FILE* fp, label* label_arr, word* words)
{
    char line[MAX_LINE_LENGTH];
    int token_len = 0;
    int pc = 0; //local program counter
    int local_index = 0; // for the label array
    char pending_label[MAX_LABEL_LENGTH] = {0}; //to deal with lines with label & instruction
    bool has_pending = false; //to deal with lines with label & instruction

    //read assembly lines in order to detect LABELS and words
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) //itereation over the file till the end
    {
        strip_comment(line);

        char* token = strtok(line, DELIMITER);
        if (token)
        {
            // Word scanning
            if (token[0] == '.') // word instruction increment
            {
                char* token_ad = strtok(NULL, DELIMITER);
                char* token_data = strtok(NULL, DELIMITER);
                int   addr       = convertToDecimal(token_ad);
                set_word(token_ad, token_data, words);
                if (addr >= pc)   
                    pc = addr + 1;
                continue;
            }

            // Label scaning
            token_len = (int)strlen(token);
            if (contains_label(token, token_len)) //first token is label
            {
                /* remember the label text but DON’T set it yet */
                token[token_len - 1] = '\0';         
                strcpy(pending_label, token);
                has_pending = true;
                token = strtok(NULL, DELIMITER);      
                if (token == NULL) {
                    set_label(pc, local_index++, token_len, pending_label, label_arr);
                    has_pending = false;
                    continue;
                }
            }
        }
        else continue;

        char* token_rd = strtok(NULL, DELIMITER);
        char* token_rs = strtok(NULL, DELIMITER);
        char* token_rt = strtok(NULL, DELIMITER);
        char* token_imm = strtok(NULL, DELIMITER);


        // I-type check ,check if bigimm is used and advance pc accordingly
        if ((token_rd != NULL) && (token_rs != NULL) && (token_rt != NULL))
        {
            bool uses_imm = (strcmp(token_rd, "$imm") == 0) ||
                            (strcmp(token_rs, "$imm") == 0) ||
                            (strcmp(token_rt, "$imm") == 0) ||
                            is_branch(token) ; //fixme branch does not always use $imm... only if it has labels with it..
            if (uses_imm)
            {
                bool bigimm = true;
                if (token_imm != NULL)
                {
                    char *endptr;
                    long val = strtol(token_imm, &endptr, 0);
                    if (*endptr == '\0' && val >= -128 && val <= 127)
                        bigimm = false;
                }
                maybe_flush(pc, pending_label, &has_pending, &local_index, label_arr); //setting label in imm
                pc += bigimm ? 2:1;
            }
            else 
            {
                maybe_flush(pc, pending_label, &has_pending, &local_index, label_arr); //setting label in imm
                pc++; //for 1 line I instruction
            }
            continue;
        }
    }
}

//checks if a string is a registered label
bool is_label_name(const char* token, label* labels) {
    if (token == NULL) return false;

    for (int i = 0; i < MAX_NUMBER_OF_LABELS; i++) {
        if (!labels[i].set) break;
        if (strcmp(labels[i].name, token) == 0) {
            return true;
        }
    }
    return false;
}

bool is_branch(char* opcode) {
    return strcmp(opcode, "beq") == 0 ||
           strcmp(opcode, "bne") == 0 ||
           strcmp(opcode, "blt") == 0 ||
           strcmp(opcode, "bgt") == 0 ||
           strcmp(opcode, "ble") == 0 ||
           strcmp(opcode, "bge") == 0;
}

static inline int decode_opcode_wrapper(char *opc, int pc)
{
    int code = decode_opcode(opc);
    if (code == -1) {
        fprintf(stderr, "Error: unknown opcode '%s' at PC %d\n", opc, pc);
        exit(1);
    }
    return code;
}

static inline int decode_reg_wrapper(char *reg, int pc)
{
    //if (reg == NULL) return 0;            /* caller decides if NULL legal   */
    int r = decode_register(reg);
    if (reg == NULL || r == -1) {
        fprintf(stderr, "Error: unknown or NULL register '%s' at PC %d\n", reg, pc);
        exit(1);
    }
    return r;
}


void write2memin(FILE* inputfp, label* label_arr, FILE* outputfp, word* words)
{
    char line[MAX_LINE_LENGTH];
    char* token;
    char* token1;
    char* token2;
    char* token3;
    char* token4;
    int pc = 0; //local program counter

    while (fgets(line, MAX_LINE_LENGTH, inputfp) != NULL) //itereation over the file till the end
    {
        strip_comment(line); //remove everything after '#' in assembly line

        instruction inst = {0}; //new instruction initialization
        token = strtok(line, DELIMITER); //taking the first token of line, deciding what to do accordingly
        if (token == NULL || token[0] == '\n' || token[0] == '\0') continue; //empty line or comment - ignore

        if (words[pc].set == true) // If there is a word that should be written to this address -> write word's data
        {
            fprintf(outputfp, "%08X\n", words[pc].data);
            words[pc].set = false; //clear it so it prints only once
            pc++;
            continue;
        }

        if (token[0] == '.') continue; // Word line, ignore. Already handled.

        if (contains_label(token, (int)strlen(token))) // we have a label
        {
            token = strtok(NULL, DELIMITER); //taking the first real token 
            if (token == NULL) continue; // only label in line, else we just continue normally 
        }

        // Handling assembly - regular commands
        token1 = strtok(NULL, DELIMITER); //taking second token-will represent rd reg
        token2 = strtok(NULL, DELIMITER); //taking third token-will represent rs reg
        token3 = strtok(NULL, DELIMITER); //taking fourth token-will represent rt reg
        token4 = strtok(NULL, DELIMITER); //taking fifth token-will represent const
        #ifdef DEBUG
            printf("tototoken: %s %s %s %s %s at PC = %d\n", token, token1, token2, token3, token4, pc);
        #endif
        

        inst.opcode = decode_opcode_wrapper(token, pc);

        if (is_branch(token)) { // not all branches are bigimm for example beq $ra, $zero, $zero, 0
            if (is_label_name(token4, label_arr)) {
                inst.rd = decode_reg_wrapper(token1, pc);
                inst.rs = decode_reg_wrapper(token2, pc);     
                inst.rt = decode_reg_wrapper(token3, pc);
                int label_addr = decode_imm(token4, label_arr);
                inst.bigimm = 1;
                inst.imm8   = 0;
                #ifdef DEBUG
                    printf("Writing branch instruction1: %s %s %s %s %s at PC = %d\n", token, token1, token2, token3, token4, pc);
                #endif
                print_line2memin(inst, outputfp, words, pc, label_addr);
                pc += 2;
                continue;
            } else {
                inst.rd = decode_reg_wrapper(token1, pc);
                inst.rs = decode_reg_wrapper(token2, pc);     //code is needlessly duplicated here...
                inst.rt = decode_reg_wrapper(token3, pc);
                int label_addr = decode_imm(token4, label_arr);
                inst.bigimm = 0;
                inst.imm8   = label_addr;
                #ifdef DEBUG
                    printf("Writing branch instruction2: %s %s %s %s %s at PC = %d\n", token, token1, token2, token3, token4, pc);
                #endif
                print_line2memin(inst, outputfp, words, pc, 0);
                pc += 1;
                continue;
            }
        }

        if (strcmp(token, "jal") == 0) {

            inst.rd = decode_reg_wrapper(token1, pc);   // caller’s rd
            // ---- jump-via-imm (bigimm == 1) ----------------------- 
            if (token2 && strcmp(token2, "$imm") == 0) {
                inst.rs     = 1;                          // $imm      
                inst.rt     = decode_reg_wrapper(token3, pc); // 3rd op    
                inst.bigimm = 1;
                inst.imm8   = 0;

                int target  = decode_imm(token4, label_arr); // label/num
                print_line2memin(inst, outputfp, words, pc, target);
                pc += 2;
            }
            // ---- jump-via-register (single 32-bit word) ------------ 
            else {
                inst.rs     = decode_reg_wrapper(token2, pc);  // target reg
                inst.rt     = decode_reg_wrapper(token3, pc);
                inst.bigimm = 0;
                inst.imm8   = 0;
                print_line2memin(inst, outputfp, words, pc, 0);
                pc += 1;
            }
            continue;
        }

        inst.rd = decode_reg_wrapper(token1, pc);
        inst.rs = decode_reg_wrapper(token2, pc);
        inst.rt = decode_reg_wrapper(token3, pc);

        bool uses_imm =
            (token1 && strcmp(token1,"$imm")==0) ||
            (token2 && strcmp(token2,"$imm")==0) ||
            (token3 && strcmp(token3,"$imm")==0);
        if (uses_imm) {

            int imm_value = 0;
            if (token4 != NULL && !is_register_name(token4)) {
                imm_value = decode_imm(token4, label_arr);
            }

            // Check range for 8-bit signed immediate: -128 ≤ x ≤ 127 or LABEL otherwise it is bigimm...
            if (imm_value >= -128 && imm_value <= 127 && !is_label_name(token4, label_arr)) {
                inst.bigimm = 0;
                inst.imm8 = (uint8_t)(imm_value & 0xFF);
                //printf("Writing instruction writing memin bigimm=0: %s %s %s %s %s at PC = %d\n", token, token1, token2, token3, token4, pc);
                print_line2memin(inst, outputfp, words, pc, imm_value);
                pc += 1;
                continue;
            } else { //requires second register
                inst.bigimm = 1;
                inst.imm8 = 0;
                //printf("Writing instruction4 writing memin bigimm=1: %s %s %s %s %s at PC = %d\n", token, token1, token2, token3, token4, pc);
                print_line2memin(inst, outputfp, words, pc, imm_value);
                //fprintf(outputfp, "%08X\n", (uint32_t)imm_value);
                pc += 2;
                continue;
            }
        }
        else { //no imm
            inst.bigimm = 0;
            inst.imm8   = 0;
            //printf("Writing instruction no imm in command: %s %s %s %s %s at PC = %d\n", token, token1, token2, token3, token4, pc);
            print_line2memin(inst, outputfp, words, pc, 0);
            pc += 1;
        }

    }
    write_rest_words(outputfp, words, pc); //only after finish looping all lines
}


int decode_opcode(char* opcode) {
    if (strcmp(opcode, "add") == 0) return 0;
    if (strcmp(opcode, "sub") == 0) return 1;
    if (strcmp(opcode, "mul") == 0) return 2;
    if (strcmp(opcode, "and") == 0) return 3;
    if (strcmp(opcode, "or") == 0) return 4;
    if (strcmp(opcode, "xor") == 0) return 5;
    if (strcmp(opcode, "sll") == 0) return 6;
    if (strcmp(opcode, "sra") == 0) return 7;
    if (strcmp(opcode, "srl") == 0) return 8;
    if (strcmp(opcode, "beq") == 0) return 9;
    if (strcmp(opcode, "bne") == 0) return 10;
    if (strcmp(opcode, "blt") == 0) return 11;
    if (strcmp(opcode, "bgt") == 0) return 12;
    if (strcmp(opcode, "ble") == 0) return 13;
    if (strcmp(opcode, "bge") == 0) return 14;
    if (strcmp(opcode, "jal") == 0) return 15;
    if (strcmp(opcode, "lw") == 0) return 16;
    if (strcmp(opcode, "sw") == 0) return 17;
    if (strcmp(opcode, "reti") == 0) return 18;
    if (strcmp(opcode, "in") == 0) return 19;
    if (strcmp(opcode, "out") == 0) return 20;
    if (strcmp(opcode, "halt") == 0) return 21;
    return -1; // assuming correct input we will not get there
}

int decode_register(char* reg_name) {
    if (strcmp(reg_name, "$zero") == 0) return 0;
    if (strcmp(reg_name, "$imm") == 0) return 1;
    if (strcmp(reg_name, "$v0") == 0) return 2;
    if (strcmp(reg_name, "$a0") == 0) return 3;
    if (strcmp(reg_name, "$a1") == 0) return 4;
    if (strcmp(reg_name, "$a2") == 0) return 5;
    if (strcmp(reg_name, "$a3") == 0) return 6;
    if (strcmp(reg_name, "$t0") == 0) return 7;
    if (strcmp(reg_name, "$t1") == 0) return 8;
    if (strcmp(reg_name, "$t2") == 0) return 9;
    if (strcmp(reg_name, "$s0") == 0) return 10;
    if (strcmp(reg_name, "$s1") == 0) return 11;
    if (strcmp(reg_name, "$s2") == 0) return 12;
    if (strcmp(reg_name, "$gp") == 0) return 13;
    if (strcmp(reg_name, "$sp") == 0) return 14;
    if (strcmp(reg_name, "$ra") == 0) return 15;
    return -1; // assuming correct input we will not get there
}


int decode_imm(char* imm, label* labels) {
    if (imm == NULL) {
        fprintf(stderr, "Error: NULL immediate or label.\n");
        exit(1);
    }
    for (int i = 0; i < MAX_NUMBER_OF_LABELS; i++) {
        if (labels[i].set == 0) break;
        if (strcmp(labels[i].name, imm) == 0) { // found label, return address
            return labels[i].line;
        }
    }

    // Strip trailing newline
    size_t len = strlen(imm);
    if (len > 0 && imm[len - 1] == '\n') {
        imm[len - 1] = '\0';
    }
    return strtol(imm, NULL, 0); // number

}

void print_line2memin(instruction inst, FILE* outputfp, word* words, int pc, int imm_value) {
    // Build the full 32-bit instruction word
    uint32_t encoded =
        (inst.opcode << 24) |
        (inst.rd     << 20) |
        (inst.rs     << 16) |
        (inst.rt     << 12) |
        (0           << 9 ) |   // reserved bits = 0
        (inst.bigimm << 8 ) |
        (inst.imm8   & 0xFF);   // force to 8 bits
    //printf("Encoded instruction at PC = %d: %08X\n", pc, encoded);
    fprintf(outputfp, "%08X\n", encoded);

    // Write second word only if bigimm == 1
    if (inst.bigimm == 1) {
        if (words[pc + 1].set == true) {
            fprintf(outputfp, "%08X\n", words[pc + 1].data); // make sure no word is there
            words[pc + 1].set = false; //prevent duplicate
        } else {
            fprintf(outputfp, "%08X\n", (uint32_t)imm_value); //write the imm32
        }
    }
}

void write_rest_words(FILE* memin_fp, word* words, int pc) {
    int temp = pc; // Last active PC from main loop, end of the instructions mem space 

    for (int i = pc; i < MEMORY_SIZE; i++)
    {
        if (words[i].set == true)
        {
            for (int j = temp; j < i; j++)
            {
                fprintf(memin_fp, "%08X\n", 0x0);
            }
            fprintf(memin_fp, "%08X\n", words[i].data);
            temp = i + 1;
        }
    }
}



int main(int argc, char* argv[])
{
    //valid file check
    if (argc != 3) {
        fprintf(stderr, "Wrong input, expected 2 arguments (input.asm output.memin), got %d\n", argc - 1);
        return 1;
    }
    FILE* assembly_fp = open_file_to_read(argv[1]);
    FILE* memin_fp = open_file_to_write(argv[2]);
    label labels[MAX_NUMBER_OF_LABELS]; //set labels array at max mem size
    word words[MAX_NUMBER_OF_LABELS]; // words are same as labels

    memset(labels, 0, sizeof(labels));
    memset(words,  0, sizeof(words));

    parse_lines(assembly_fp, labels, words);
    //for (int i = 0; i < MAX_NUMBER_OF_LABELS && labels[i].set; i++) {
    //    printf("Label '%s' is at PC = %d\n", labels[i].name, labels[i].line);
    //}
    fclose(assembly_fp);

    FILE* assembly_fp1 = open_file_to_read(argv[1]);
    write2memin(assembly_fp1, labels, memin_fp, words);
    fclose(assembly_fp1);
    fclose(memin_fp);
    return 0;
}