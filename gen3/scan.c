#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mppl_compiler.h"

FILE *fp;
int num_attr = 0;
char string_attr[MAXSTRSIZE];
static int cbuf;
static int nbuf = '\0';
static void next_buf(void);
static int linenum = 1;
static int token_linenum = 0;

int get_linenum(void);
void set_token_linenum(void);
static int scan_alnum(void);
static int check_digit(void);
static int scan_string(void);
static int scan_comment(void);
static int scan_symbol(void);
static int get_keyword_token_code(char *token);

int init_scan(char *filename) {
    if ((fp = fopen(filename, "r")) == NULL) {
        error("file error");
        return -1;
    }
    next_buf();
    next_buf();

    return 0;
}

int scan(void) {
    int token_code = -1;
    while (1) {
        if (cbuf == EOF) {
            return -1;
        } else if (cbuf == '\r' || cbuf == '\n') {
            if (cbuf == '\r') {
                if (nbuf == '\n') {
                    next_buf();
                }
                next_buf();
                linenum++;
            } else {
                if (nbuf == '\r') {
                    next_buf();
                }
                next_buf();
                linenum++;
            }
        } else if (cbuf == ' ' || cbuf == '\t') {
            next_buf();
        } else if (!isprint(cbuf)) {
            error("scan error");
            return -1;
        } else if (isalpha(cbuf)) {
            token_code = scan_alnum();
            break;
        } else if (isdigit(cbuf)) {
            token_code = check_digit();
            break;
        } else if (cbuf == '\'') {
            token_code = scan_string();
            break;
        } else if ((cbuf == '/' && nbuf == '*') || cbuf == '{') {
            if (scan_comment() == -1) {
                break;
            }
        } else {
            token_code = scan_symbol();
            break;
        }
    }
    set_token_linenum();
    return token_code;
}

int get_linenum(void) {
    return token_linenum;
}

void set_token_linenum(void) {
    token_linenum = linenum;
}

int end_scan(void) {
    if (fclose(fp) == EOF) {
        error("error fil close");
        ;
        return -1;
    }
    return 0;
}

static int scan_alnum() {
    memset(string_attr, '\0', sizeof(string_attr));
    string_attr[0] = cbuf;

    next_buf();
    while (isalnum(cbuf)) {
        int len = (int)strlen(string_attr);
        if (len < MAXSTRSIZE - 1) {
            string_attr[len] = cbuf;
        } else {
            error("scan error in alphabet or number");
            return -1;
        }

        next_buf();
    }
    return get_keyword_token_code(string_attr);
}


static int check_digit() {
    int num = cbuf- '0';

    memset(string_attr, '\0', sizeof(string_attr));
    string_attr[0] = cbuf;

    next_buf();
    while (isdigit(cbuf)) {
        int len = (int)strlen(string_attr);
        if (len < MAXSTRSIZE - 1) {
            string_attr[len] = cbuf;
        } else {
            error("digit check error");
            return -1;
        }

        num *= 10;
        num += cbuf - '0';
        next_buf();
    }
    if (num <= MAX_NUM) {
        num_attr = num;
        return TNUMBER;
    } else {
        error("error check digit");
    }

    return -1;
}

static int scan_string() {
    memset(string_attr, '\0', sizeof(string_attr));
    next_buf();

    while (1) {
        if (!isprint(cbuf)) {
            error("error string scan");
            return -1;
        }

        if (cbuf == '\'' && nbuf != '\'') {
            break;
        }

        if (cbuf == '\'' && nbuf == '\'') {
            int len = (int)strlen(string_attr);
            if (len < MAXSTRSIZE - 1) {
                string_attr[len] = cbuf;
            } else {
                error("error string scan");
                return -1;
            }

            next_buf();
        }

        int len = (int)strlen(string_attr);
        if (len < MAXSTRSIZE - 1) {
            string_attr[len] = cbuf;
        } else {
            error("error string scan");
            return -1;
        }

        next_buf();
    }
    next_buf();

    return TSTRING;
}

static int scan_comment() {
    if (cbuf == '/' && nbuf == '*') {
        next_buf();
        next_buf();
        while (cbuf != EOF) {
            if (cbuf == '*' && nbuf == '/') {
                next_buf();
                next_buf();
                return 0;
            }
            next_buf();
        }
    } else if (cbuf == '{') {
        next_buf();
        while (cbuf != EOF) {
            if (cbuf == '}') {
                next_buf();
                return 0;
            }
            next_buf();
        }
    }
   
    if (cbuf != EOF) {
        error("error comment scan");
    }
    return -1;
}

static int scan_symbol() {
    char symbol = cbuf;
    next_buf();
    switch (symbol) {
        case '+':
            return TPLUS;
        case '-':
            return TMINUS;
        case '*':
            return TSTAR;
        case '=':
            return TEQUAL;
        case '<':
            if (cbuf == '>') {
                next_buf();
                return TNOTEQ;
            } else if (cbuf == '=') {
                next_buf();
                return TLEEQ;
            } else {
                return TLE;
            }
        case '>':
            if (cbuf == '=') {
                next_buf();
                return TGREQ;
            } else {
                return TGR;
            }
        case '(':
            return TLPAREN;
        case ')':
            return TRPAREN;
        case '[':
            return TLSQPAREN;
        case ']':
            return TRSQPAREN;
        case ':':
            if (cbuf == '=') {
                next_buf();
                return TASSIGN;
            } else {
                return TCOLON;
            }
        case '.':
            return TDOT;
        case ',':
            return TCOMMA;
        case ';':
            return TSEMI;
        default:
            error("error scan symbol");
            return -1;
    }
}

static int get_keyword_token_code(char *token) {
    int index;
    for (index = 0; index < KEYWORDSIZE; index++) {
        if (strcmp(token, key[index].keyword) == 0) {
            return key[index].keytoken;
        }
    }

    return TNAME;
}


static void next_buf() {
    cbuf = nbuf;
    nbuf = fgetc(fp);
    return;
}

