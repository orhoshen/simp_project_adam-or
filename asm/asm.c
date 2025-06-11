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
        fprintf(stderr, "Error: Could not open file %s for writing.\n", filePath);
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

void set_word(int pc, char* token_add, char* token_data, word* words)
{
    int addr = convertToDecimal(token_add);
    if (addr < 0 || addr >= MEMORY_SIZE) {
        fprintf(stderr, "Error: .word address %d out of bounds (0-4095)\n", addr);
        return;
    }
    words[addr].pc = pc;
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
    long result = strtol(str, &endptr, base);

    // Check for conversion errors
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid input string '%s'.\n", str);
        return 0;
    }

    return result; // Return the decimal integer
}

void parse_lines(FILE* fp, label* label_arr, word* words)
{
    char line[MAX_LINE_LENGTH];
    int token_len = 0;
    int pc = 0; //local program counter
    int local_index = 0; // for the label array


    //read assembly lines in order to detect LABELS and calculate PC
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        strip_comment(line);

        char* token = strtok(line, DELIMITER);
        if (!token) continue;

        // Word directive handling
        if (token[0] == '.')
        {
            char* token_ad = strtok(NULL, DELIMITER);
            char* token_data = strtok(NULL, DELIMITER);
            set_word(pc, token_ad, token_data, words);
            pc++;
            continue;
        }

        // Label handling
        token_len = (int)strlen(token);
        if (contains_label(token, token_len))
        {
            set_label(pc, local_index, token_len, token, label_arr);
            local_index++;
            token = strtok(NULL, DELIMITER);
            if (token == NULL)
            {
                continue;
            }
        }

        // Parse operands
        char* token_rd = strtok(NULL, DELIMITER);
        char* token_rs = strtok(NULL, DELIMITER);
        char* token_rt = strtok(NULL, DELIMITER);
        char* token_imm = strtok(NULL, DELIMITER);

        bool need_bigimm = false;
        if (is_branch(token)) {
            need_bigimm = true;
        } else if (token_imm != NULL) {
            if (!(isdigit((unsigned char)token_imm[0]) || token_imm[0] == '-' || token_imm[0] == '+'))
            {
                // Operand is a label, always requires big immediate
                need_bigimm = true;
            }
            else
            {
                long imm_val = strtol(token_imm, NULL, 0);
                if (imm_val < -128 || imm_val > 127)
                {
                    need_bigimm = true;
                }
            }
        }

        pc += need_bigimm ? 2 : 1;
    }
}

bool is_branch(char* opcode) {
    return strcmp(opcode, "beq") == 0 ||
           strcmp(opcode, "bne") == 0 ||
           strcmp(opcode, "blt") == 0 ||
           strcmp(opcode, "bgt") == 0 ||
           strcmp(opcode, "ble") == 0 ||
           strcmp(opcode, "bge") == 0;
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

        instruction inst; //new instruction initialization
        token = strtok(line, DELIMITER); //taking the first token of line, deciding what to do accordingly
        if (token == NULL || token[0] == '\n' || token[0] == '\0') continue; //empty line or comment - ignore

        if (words[pc].set == true) // If there is a word that should be written to this address -> write word's data
        {
            fprintf(outputfp, "%08X\n", words[pc].data);
            words[pc].set = false;
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

        inst.opcode = decode_opcode(token);
        inst.rd = decode_register(token1);
        inst.rs = decode_register(token2);
        inst.rt = decode_register(token3);

        if (is_branch(token)) {
            int label_addr;

            if (token4 == NULL) {
                // Format: beq rs, rt, label
                inst.rs = decode_register(token1);
                inst.rt = decode_register(token2);
                label_addr = decode_imm(token3, label_arr);
            } else {
                // Format: beq $imm, rs, rt, label
                inst.rs = decode_register(token2);
                inst.rt = decode_register(token3);
                label_addr = decode_imm(token4, label_arr);
            }

            inst.rd = 1;  // $imm register holds the label address
            inst.bigimm = 1;
            inst.imm8 = 0;  // not used when bigimm=1

            write_i2memin(inst, outputfp, words, pc, label_addr);
            pc += 2;
            continue;
        }

        if (strcmp(token1, "$imm") == 0 || strcmp(token2, "$imm") == 0 || strcmp(token3, "$imm") == 0) {
            int imm_value = decode_imm(token4, label_arr);

            // Check range for 8-bit signed immediate: -128 ≤ x ≤ 127
            if (imm_value >= -128 && imm_value <= 127) {
                inst.bigimm = 0;
                inst.imm8 = (uint8_t)(imm_value & 0xFF);
                write_i2memin(inst, outputfp, words, pc, imm_value);
                pc += 1;
            } else { //requires second register
                inst.bigimm = 1;
                inst.imm8 = (uint8_t)(imm_value & 0xFF);
                write_i2memin(inst, outputfp, words, pc, imm_value);
                //fprintf(outputfp, "%08X\n", (uint32_t)imm_value);
                pc += 2;
            }

        } else {
            // Regular R-type instruction without immediate
            inst.bigimm = 0;
            inst.imm8 = 0;
            write_i2memin(inst, outputfp, words, pc, 0);
            pc += 1;
        }

    }
    write_rest_words(outputfp, words, pc);
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
        return 0;
    }

    for (int i = 0; i < MAX_NUMBER_OF_LABELS; i++) {
        if (labels[i].set == 0) break;
        if (strcmp(labels[i].name, imm) == 0) { // found label, return address
            return labels[i].line;
        }
    }
    return strtol(imm, NULL, 0); // number

}

void write_i2memin(instruction inst, FILE* outputfp, word* words, int pc, int imm_value) {
    // Build the full 32-bit instruction word
    uint32_t encoded =
        (inst.opcode << 24) |
        (inst.rd     << 20) |
        (inst.rs     << 16) |
        (inst.rt     << 12) |
        (0           << 9 ) |   // reserved bits = 0
        (inst.bigimm << 8 ) |
        (inst.imm8   & 0xFF);   // force to 8 bits

    fprintf(outputfp, "%08X\n", encoded);

    // Write second word only if bigimm == 1
    if (inst.bigimm == 1) {
        if (words[pc + 1].set == true) {
            fprintf(outputfp, "%08X\n", words[pc + 1].data); // make sure no word is there
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
            for (int j = temp; j + 1 < i; j++)
            {
                fprintf(memin_fp, "%08X\n", 0x0);
            }
            fprintf(memin_fp, "%08X\n", words[i].data);
            temp = i;
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

    parse_lines(assembly_fp, labels, words);
    fclose(assembly_fp);

    FILE* assembly_fp1 = open_file_to_read(argv[1]);
    write2memin(assembly_fp1, labels, memin_fp, words);
    fclose(assembly_fp1);
    fclose(memin_fp);
    return 0;
}