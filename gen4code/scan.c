#include "mppl_compiler.h"
char string_attr[MAXSTRSIZE];
int num_attr;
char cbuf = '\0';
int linenum = 0;

/* keyword list */
struct KEYWORDS key[KEYWORDSIZE] = {
    {"and",     TAND    },
    {"array",    TARRAY    },
    {"begin",    TBEGIN    },
    {"boolean",    TBOOLEAN},
    {"break",    TBREAK  },
    {"call",    TCALL    },
    {"char",    TCHAR    },
    {"div",        TDIV    },
    {"do",        TDO    },
    {"else",    TELSE    },
    {"end",        TEND    },
    {"false",    TFALSE    },
    {"if",        TIF    },
    {"integer",    TINTEGER},
    {"not",        TNOT    },
    {"of",        TOF    },
    {"or",        TOR    },
    {"procedure", TPROCEDURE},
    {"program",    TPROGRAM},
    {"read",    TREAD    },
    {"readln",    TREADLN },
    {"return",     TRETURN },
    {"then",    TTHEN    },
    {"true",    TTRUE    },
    {"var",        TVAR    },
    {"while",    TWHILE    },
    {"write",    TWRITE  },
    {"writeln",    TWRITELN}
};


int init_scan(char *filename, FILE **input) {
   
    *input = fopen(filename, "r");
    if (*input == NULL) {
        return -1;
    }
    linenum = 1;
    cbuf = (char) fgetc(*input);

    return 0;
}

int scan() {
    char token[MAXSTRSIZE];
    int i = 0, r_value = 0, r_val_not= 0;

    memset(token, 0, sizeof(token));

    while ((r_val_not = not_token(cbuf, input)) != 0) {
        if (r_val_not == -1) {
            return -1;
        }
    }
    if (cbuf == EOF) {
        return -1;
    }

    if ((cbuf >= 0x28 && cbuf <= 0x2e) || (cbuf >= 0x3a && cbuf <= 0x3e) ||
        cbuf == 0x5b || cbuf == 0x5d) {
        token[0] = cbuf;
        next_buf(input);
        return symbol_check(token, input);
    } else if (cbuf >= 0x30 && cbuf <= 0x39) {
        token[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(input)) != EOF; i++) {
            if (i >= MAXSTRSIZE) {
                error("token is too long.");
                return -1;
            }
            if (cbuf >= 0x30 && cbuf <= 0x39) {
                token[i] = cbuf;
            } else {
                break;
            }
        }
     
        num_attr = atoi(token);
        if (num_attr <= 32767) {
          
            memset(string_attr, 0, sizeof(string_attr));
            snprintf(string_attr, MAXSTRSIZE, "%s", token);
        } else {
            error("number is too big 32767.");
            return -1;
        }

        return TNUMBER;
    } else if (cbuf == '\'') {

        int i = 0;
        char before_cbuf[MAXSTRSIZE];
        memset(before_cbuf, 0, sizeof(before_cbuf));

        for (i = 0; (cbuf = (char) fgetc(input)) != EOF; i++) {
            if (i >= MAXSTRSIZE) {
                error("token is too long.");
                return -1;
            }
            if (cbuf == '\'') {
                next_buf(input);
                if (cbuf == '\'') {
                    before_cbuf[i] = '\'';
                    i++;
                    before_cbuf[i] = '\'';
                } else {
                    memset(string_attr, 0, sizeof(string_attr));
                    snprintf(string_attr, MAXSTRSIZE, "%s", before_cbuf);
                    return TSTRING;
                }
            } else {
                before_cbuf[i] = cbuf;
            }
        }
        error("string end error.");
        return -1;
    } else if ((cbuf >= 0x41 && cbuf <= 0x5a) || (cbuf >= 0x61 && cbuf <= 0x7a)) {

        token[0] = cbuf;
        for (i = 1; (cbuf = (char) fgetc(input)) != EOF; i++) {
            if (i >= MAXSTRSIZE) {
                error("token is too long.");
                return -1;
            }
            if (((cbuf >= 0x41 && cbuf <= 0x5a) || (cbuf >= 0x61 && cbuf <= 0x7a)) || (cbuf >= 0x30 && cbuf <= 0x39)) {
                token[i] = cbuf;
            } else {
                break;
            }
        }
        if ((r_value = keyword_check(token)) != -1) {
            return r_value;
        } else {
            memset(string_attr, 0, sizeof(string_attr));
            snprintf(string_attr, MAXSTRSIZE, "%s", token);
            return TNAME;
        }
    }
    error("unexpected token.");
    return -1;
}

int not_token(char c, FILE *input) {
    if (c =='\t' || c =='\v' || c ==0x20){
        next_buf(input);
        return 1;
    }
    else if (c == '\r'){
        next_buf(input);
        if (cbuf == '\n') {
            next_buf(input);
        }
        linenum++;
        return 1;
    }
    else if(c == '\n'){
        next_buf(input);
        if (cbuf == '\r') {
            next_buf(input);
        }
        linenum++;
        return 1;
    }
    else if(c ==  '{'){
        int r_val_not = 2;
        while ((cbuf = (char) fgetc(input)) != EOF) {
            if (cbuf == '}') {
                if (r_val_not == 2) {
                    cbuf = (char) fgetc(input);
                    return 2;
                }
            } else if (cbuf == '*') {
                if ((cbuf = (char) fgetc(input)) == '/') {
                    cbuf = (char) fgetc(input);
                    return 3;
                }
            }
        }
        error("comment end error.");
        return -1;
    }
    else if(c == '/'){
        if ((cbuf = (char) fgetc(input)) == '*') {
            int r_val_not = 3;
            while ((cbuf = (char) fgetc(input)) != EOF) {
                if (cbuf == '}') {
                    if (r_val_not == 2) {
                        cbuf = (char) fgetc(input);
                        return 2;
                    }
                } else if (cbuf == '*') {
                    if ((cbuf = (char) fgetc(input)) == '/') {
                        next_buf(input);
                        return 3;
                    }
                }
            }
            error("comment end error.");
            return -1;
        }
        error("unexpected token.");
        return -1;
    }
    else{
        return 0;
    }
}

int keyword_check(const char *tokenstr) {
    int i = 0;
    if (strlen(tokenstr) <= KEYWORDSIZE) {
        for (i = 0; i < KEYWORDSIZE; i++) {
            if (strcmp(tokenstr, key[i].keyword) == 0) {
                return key[i].keytoken;
            }
        }
    }
    return -1;
}

int symbol_check(char *tokenstr, FILE *fp) {
    switch (tokenstr[0]) {
        case '(':
            return TLPAREN;
        case ')':
            return TRPAREN;
        case '*':
            return TSTAR;
        case '+':
            return TPLUS;
        case ',':
            return TCOMMA;
        case '-':
            return TMINUS;
        case '.':
            return TDOT;
        case ':':
            switch (cbuf) {
                case '=':
                    next_buf(fp);
                    return TASSIGN;
                case EOF:
                    return -1;
                default:
                    return TCOLON;
            }
        case ';':
            return TSEMI;
        case '<':
            switch (cbuf) {
                case '>':
                    next_buf(fp);
                    return TNOTEQ;
                case '=':
                    next_buf(fp);
                    return TLEEQ;
                case EOF:
                    return -1;
                default:
                    return TLE;
            }
        case '=':
            return TEQUAL;
        case '>':
            switch (cbuf) {
                case '=':
                    next_buf(fp);
                    return TGREQ;
                case EOF:
                    return -1;
                default:
                    return TGR;
            }
        case '[':
            return TLSQPAREN;
        case ']':
            return TRSQPAREN;
        default:
            error("symbol error.");
            return -1;
    }
}

int get_linenum(void) {
    return linenum;
}

void end_scan(FILE *fp) {
    if (fclose(fp) == EOF) {
        error("File can not close.");
    };
}

void next_buf(FILE *fp)
{
    cbuf = (char) fgetc(fp);
}
