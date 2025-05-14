#define _CRT_SECURE_NO_WARNINGS
#include "asm.h"

FILE* open_file_to_read(char* filePath)
{
    FILE* fp = fopen(filePath, "r");
    if (fp == NULL) {
        return NULL; // file is invalid
    }
    return fp;
};

FILE* open_file_to_write(char* filePath)
{
    FILE* fp = fopen(filePath, "w");
    if (fp == NULL) {
        return NULL; // file is invalid
    }
    return fp;
};

bool contains_label(char* token, int token_len)
{
    if ((token_len > 0)
        && (token[token_len - 1] == ':')
        && (token[0] != '#')) return true;//not empty label + not opcode + not comment
    return false;
};

void set_label(int pc, int local_index, int label_len, char* token, label* label_arr)
{
    label_arr[local_index].set = true;
    label_arr[local_index].line = pc;//save current PC
    token[label_len - 1] = '\0'; // remove the colomn
    strcpy(label_arr[local_index].name, token); //save label name without ':'
};
void set_word(int pc, char* token_add, char* token_data, word* words)
{
    int addr = convertToDecimal(token_add) + 1;
    words[addr].pc = pc;
    words[addr].address = addr;
    words[addr].data = convertToDecimal(token_data);
    words[addr].set = true;
};

// Function to convert a string representation of a number to decimal
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
    long decimal = strtol(str, &endptr, base);

    // Check for conversion errors
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid input string '%s'.\n", str);
        return 0;
    }

    return (int)decimal; // Return the decimal integer
}

void find_label_words_lines(FILE* fp, label* label_arr, word* words)
{

    char line[MAX_LINE_LENGTH];
    char line_temp[MAX_LINE_LENGTH];
    int token_len = 0;
    int pc = 0; //local program counter
    int local_index = 0;


    //read assebly lines in order to detect LABELS
    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) //itereation over the file till the end
    {
        // code to handle lines with a comment only
        strcpy(line_temp, line);
        char* token_temp = strtok(line_temp, DELIMITER_C);
        if (token_temp != NULL) {
            if (token_temp[0] == '#') {
                continue;
            }
        }
        // up to here

        char* token = strtok(line, DELIMITER);
        if (token)
        {
            // Word scanning
            if (token[0] == '.') // word instruction increment
            {
                char* token_ad = strtok(NULL, DELIMITER);
                char* token_data = strtok(NULL, DELIMITER);
                set_word(pc, token_ad, token_data, words);
                pc++;
                continue;
            }

            // Label scaning
            token_len = (int)strlen(token);
            if (contains_label(token, token_len))
            {
                set_label(pc, local_index, token_len, token, label_arr);
                local_index++; //laber_arr index inc
                token = strtok(NULL, DELIMITER);
                if (token == NULL)
                {
                    continue;
                }
            }
        }
        else continue;

        char* token_rd = strtok(NULL, DELIMITER);
        char* token_rs = strtok(NULL, DELIMITER);
        char* token_rt = strtok(NULL, DELIMITER);

        // I-type check ,if there is an i-type instruction 

        if ((token_rd != NULL) && (token_rs != NULL) && (token_rt != NULL))
        {
            if (strcmp(token_rd, "$imm") == 0 || strcmp(token_rs, "$imm") == 0 || strcmp(token_rt, "$imm") == 0)
            {
                pc += 2;
                continue;
            }

        }
        pc++; //address inc
    }
};


void write2memin(FILE* inputfp, label* label_arr, FILE* outputfp, word* words)
{
    char line[MAX_LINE_LENGTH];
    char line_temp[MAX_LINE_LENGTH];
    char* token;
    char* token1;
    char* token2;
    char* token3;
    char* token4;
    int pc = 0; //local program counter
    bool is_i_type; //true means i instruction, false mean R intruction

    while (fgets(line, MAX_LINE_LENGTH, inputfp) != NULL) //itereation over the file till the end
    {

        // code to handle lines with a comment only
        strcpy(line_temp, line);
        char* token_temp = strtok(line_temp, DELIMITER_C);
        if (token_temp != NULL) {
            if (token_temp[0] == '#') {
                continue;
            }
        }
        // up to here

        instruction inst; //new instruction initialization
        token = strtok(line, DELIMITER); //taking the first token of line, deciding what to do accordingly
        if (token == NULL || token[0] == '#' || token[0] == '\n' || token[0] == '\0') continue; //empty line or comment - ignore

        if (words[pc].set == true) // If there is a word that should me wrotten to this address -> write word's data
        {
            fprintf(outputfp, "%05X\n", words[pc].data);
            words[pc].set = false;
            pc++;
            continue;
        }

        if (token[0] == '.')  // Word line, ignore. Already handeled.
        {
            continue;
        }
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

        if (strcmp(token1, "$imm") == 0 || strcmp(token2, "$imm") == 0 || strcmp(token3, "$imm") == 0) {
            is_i_type = true;
            if (strcmp(token4, "0") == 0) {
                inst.imm = 0;
            }
            else {
                inst.imm = decode_imm(token4, label_arr);
            }
            write_i2memin(inst, outputfp, words, pc);
            pc += 2;

        }
        else {
            is_i_type = false;
            write_r2memin(inst, outputfp);
            pc++;
        }

    }
    write_rest_words(outputfp, words, pc);
}

bool find_if_pc_word(word* words, int pc) {

    for (int i = 0; words[i].pc != -1; ++i) {
        if (words[i].pc == pc) {
            return true; // Found a word with pc equal to 'pc'
        }
    }
    return false; // No word with pc equal to 'pc' found

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
    return -1; // asuming correct input we will not get there
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
    return -1; // asuming correct input we will not get there
}


int decode_imm(char* imm, label* labels) {

    for (int i = 0; i < MAX_NUMBER_OF_LABELS; i++) {
        if (labels[i].set = 0) break;
        if (strcmp(labels[i].name, imm) == 0) { // found label, return address
            return labels[i].line;
        }
    }
    return strtol(imm, NULL, 0); // number

}

void write_i2memin(instruction inst, FILE* outputfp, word* words, int pc) {
    fprintf(outputfp, "%02X%X%X%X\n", inst.opcode, inst.rd, inst.rs, inst.rt);
    if (words[pc + 1].set == true) // If there is a word-writing in the next line, write it before inc imm
        fprintf(outputfp, "%05X\n", words[pc + 1].data);
    else
        fprintf(outputfp, "%05X\n", inst.imm);

}

void write_r2memin(instruction inst, FILE* outputfp) {
    fprintf(outputfp, "%02X%X%X%X\n", inst.opcode, inst.rd, inst.rs, inst.rt);
}

void write_rest_words(FILE* memin_fp, word* words, int pc) {
    int temp = pc; // Last active PC from main loop, end of the instructions mem space 

    for (int i = pc; i < MEMORY_SIZE; i++)
    {
        if (words[i].set == true)
        {
            for (int j = temp; j + 1 < i; j++)
            {
                fprintf(memin_fp, "%05X\n", 0x0);
            }
            fprintf(memin_fp, "%05X\n", words[i].data);
            temp = i;
        }
    }
}



int main(int argc, char* argv[])
{
    //valid file check
    if (argc != 3) {
        printf("Wrong input, you gave %d arguments insted of 3", argc);
        return 1;
    }
    FILE* assembly_fp = open_file_to_read(argv[1]);
    FILE* memin_fp = open_file_to_write(argv[2]);
    label labels[MAX_NUMBER_OF_LABELS]; //set labels array at max mem size
    word words[MAX_NUMBER_OF_LABELS]; // words are same as labels

    find_label_words_lines(assembly_fp, labels, words);

    fclose(assembly_fp);
    FILE* assembly_fp1 = open_file_to_read(argv[1]);
    write2memin(assembly_fp1, labels, memin_fp, words);
    fclose(assembly_fp);
    fclose(memin_fp);
    return 0;
}