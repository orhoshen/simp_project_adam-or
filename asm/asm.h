#ifndef ASM_H
#define ASM_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_LINE_LENGTH (500)
#define MEMORY_SIZE (4096)
#define MAX_LABEL_LENGTH (50)
#define MAX_NUMBER_OF_LABELS (4096)
#define DELIMITER     ", \t\n\r#"
#define DELIMITER_C   ", \t\n\r"

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
    bool set;

}word;

// IO helpers 
FILE *open_file_to_read (char *path);
FILE *open_file_to_write(char *path);

// parsing helpers
void  strip_comment(char *line);
bool  contains_label(char *token, int len);
void  set_label(int pc, int idx, int len, char *token, label *tbl);
void  set_word (char *addr_tok, char *data_tok, word *tbl);
int   convertToDecimal(const char *str);

// first pass – collect labels, reserve .word addresses, compute PC
void  parse_lines(FILE *fp, label *tbl, word *words);

// second pass + encoding
bool  is_branch(char *op);
void  write2memin(FILE *in, label *tbl, FILE *out, word *words);

int   decode_opcode  (char *opc);
int   decode_register(char *reg);
int   decode_imm     (char *imm, label *tbl);

void  write_i2memin   (instruction inst, FILE *out, word *words,
                       int pc, int imm32);
void  write_rest_words(FILE *out, word *words, int pc);


#endif // ASM_H 