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
    int line;
    bool set;
}label;

typedef struct instruction {
    unsigned int opcode : 8;
    unsigned int rd : 4;
    unsigned int rs : 4;
    unsigned int rt : 4;
    unsigned int imm : 20;
}instruction;

typedef struct word_inst {
    long address; //for word inst
    unsigned int data : 20;
    int pc;
    bool set;

}word;

FILE* open_file_to_read(char* fp);
FILE* open_file_to_write(char* fp);
void find_label_words_lines(FILE* fp, label* label_arr, word* words);
bool contains_label(char* label, int label_len);
void set_lable(int pc, int local_index, int lable_len, char* token, label* label_arr);

void write2memin(FILE* inputfp, label* label_arr, FILE* outputfp, word* words);
void write_i2memin(instruction inst, FILE* outputfp, word* words, int pc);
void write_r2memin(instruction inst, FILE* outputfp);
bool find_if_pc_word(word* words, int pc);
int decode_opcode(char* opcode);
int decode_register(char* rd);
int decode_imm(char* imm, label* label_arr);
int convertToDecimal(const char* str);
void write_rest_words(FILE* memin_fp, word* words, int pc);

