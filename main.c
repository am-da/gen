#include <stdio.h>
#include "mppl_compiler.h"

struct KEY key[KEYWORDSIZE] = {
    {"and", TAND},
    {"array", TARRAY},
    {"begin", TBEGIN},
    {"boolean", TBOOLEAN},
    {"break", TBREAK},
    {"call", TCALL},
    {"char", TCHAR},
    {"div", TDIV},
    {"do", TDO},
    {"else", TELSE},
    {"end", TEND},
    {"false", TFALSE},
    {"if", TIF},
    {"integer", TINTEGER},
    {"not", TNOT},
    {"of", TOF},
    {"or", TOR},
    {"procedure", TPROCEDURE},
    {"program", TPROGRAM},
    {"read", TREAD},
    {"readln", TREADLN},
    {"return", TRETURN},
    {"then", TTHEN},
    {"true", TTRUE},
    {"var", TVAR},
    {"while", TWHILE},
    {"write", TWRITE},
    {"writeln", TWRITELN}};

char *tokenstr[NUMOFTOKEN + 1] = {
    "", "NAME", "program", "var", "array", "of", "begin",
    "end", "if", "then", "else", "procedure", "return", "call",
    "while", "do", "not", "or", "div", "and", "char",
    "integer", "boolean", "readln", "writeln", "true", "false", "NUMBER",
    "STRING", "+", "-", "*", "=", "<>", "<",
    "<=", ">", ">=", "(", ")", "[", "]",
    ":=", ".", ",", ":", ";", "read", "write",
    "break"};

char *typestr[NUMOFTYPE + 1] = {"", "integer", "char", "boolean", "array", "integer", "char", "boolean", "procedure"};

int token;
static char *file_name;

int main(int nc, char *np[]) {
    int ret;
    if (nc < 2) {
        error("error main");
        fprintf(stderr, "File name id not given.\n");
        return -1;
    }

    file_name = np[1];

    if (init_scan(file_name) < 0) {
        fprintf(stderr, "File %s can not open.\n", file_name);
        return -1;
    }

    init_cr();
    token = scan();
    ret = parse_program();

    if (end_scan() < 0) {
        error("function main()");
        fprintf(stderr, "File %s can not close.\n", file_name);
        return -1;
    }

    print_cr(allroot);
    fflush(stdout);
    release_all();
    return ret;
}

int error(char *mes) {
    fprintf(stderr, "error line :%d\n", get_linenum());
    fprintf(stderr, "error message:%s\n", mes);
    return ERROR;
}
