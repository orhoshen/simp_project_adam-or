#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<ctype.h>

#define MAX_LINE_LENGTH (500)
#define MEMORY_SIZE (4096)
#define MAX_LABEL_LENGTH (50)
#define MAX_NUMBER_OF_LABELS (4096)
#define DELIMITER (", #\t\n\r")
#define DELIMITER_C (", \t\n\r")

typedef struct label {
    char name[MAX_LABEL_LENGTH];
    uint32_t line;
    bool set;
}label;

typedef struct instruction {
    uint32_t opcode     : 8;
    uint32_t rd         : 4;
    uint32_t rs         : 4;
    uint32_t rt         : 4;
    uint32_t reserved   : 3;
    uint32_t bigimm     : 1;
    uint32_t imm8       : 8;
}instruction;

typedef struct word_inst {
    uint32_t address; //for word inst
    uint32_t data;
    int pc;
    bool set;

}word;

FILE* open_file_to_read(char* fp);
FILE* open_file_to_write(char* fp);
void strip_comment(char* line);
void parse_lines(FILE* fp, label* label_arr, word* words);
bool contains_lable(char* label, int label_len);
void set_lable(int pc, int local_index, int lable_len, char* token, label* label_arr);
bool is_branch(char* opcode);
void write2memin(FILE* inputfp, label* label_arr, FILE* outputfp, word* words);
void write_i2memin(instruction inst, FILE* outputfp, word* words, int pc, int imm_value);
int decode_opcode(char* opcode);
int decode_register(char* rd);
int decode_imm(char* imm, label* label_arr);
int convertToDecimal(const char* str);
void write_rest_words(FILE* memin_fp, word* words, int pc);

