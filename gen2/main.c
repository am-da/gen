#include "print.h"

struct KEY key_word[KEYWORDSIZE] = {
        {"and",       TAND},
        {"array",     TARRAY},
        {"begin",     TBEGIN},
        {"boolean",   TBOOLEAN},
        {"break",     TBREAK},
        {"call",      TCALL},
        {"char",      TCHAR},
        {"div",       TDIV},
        {"do",        TDO},
        {"else",      TELSE},
        {"end",       TEND},
        {"false",     TFALSE},
        {"if",        TIF},
        {"integer",   TINTEGER},
        {"not",       TNOT},
        {"of",        TOF},
        {"or",        TOR},
        {"procedure", TPROCEDURE},
        {"program",   TPROGRAM},
        {"read",      TREAD},
        {"readln",    TREADLN},
        {"return",    TRETURN},
        {"then",      TTHEN},
        {"true",      TTRUE},
        {"var",       TVAR},
        {"while",     TWHILE},
        {"write",     TWRITE},
        {"writeln",   TWRITELN}
};

int main(int nc, char *np[]) {
    FILE *fp;
    int judge = 0;

    if (nc < 2) {
        fprintf(stderr, "File name not given.\n");
        return -1;
    }

    if (init_scan(np[1], &fp) < 0) {
        fprintf(stderr, "File %s can not open.\n", np[1]);
        return -1;
    }

    token = scan(fp);
    judge = parse_program(fp);
    end_scan(fp);

    if (judge == NORMAL) {
        return 0;
    } else {
        return -1;
    }
}

int error(char *mes) {
    fflush(stdout);
    fprintf(stderr, "\nline%d ERROR: %s\n", get_linenum(), mes);
    return (ERROR);
}
