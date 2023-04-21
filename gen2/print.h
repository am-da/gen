

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSTRSIZE 1024
#define NORMAL 0
#define ERROR 1
#define    TNAME        1    /* Name : Alphabet { Alphabet | Digit } */
#define    TPROGRAM    2    /* program : Keyword */
#define    TVAR        3    /* var : Keyword */
#define    TARRAY        4    /* array : Keyword */
#define    TOF        5    /* of : Keyword */
#define    TBEGIN        6    /* begin : Keyword */
#define    TEND        7    /* end : Keyword */
#define    TIF        8    /* if : Keyword */
#define    TTHEN        9    /* then : Keyword */
#define    TELSE        10    /* else : Keyword */
#define    TPROCEDURE    11    /* procedure : Keyword */
#define    TRETURN        12    /* return : Keyword */
#define    TCALL        13    /* call : Keyword */
#define    TWHILE        14    /* while : Keyword */
#define    TDO        15    /* do : Keyword */
#define    TNOT        16    /* not : Keyword */
#define    TOR        17    /* or : Keyword */
#define    TDIV        18    /* div : Keyword */
#define    TAND        19    /* and : Keyword */
#define    TCHAR        20    /* char : Keyword */
#define    TINTEGER    21    /* integer : Keyword */
#define    TBOOLEAN    22    /* boolean : Keyword */
#define    TREADLN        23    /* readln : Keyword */
#define    TWRITELN    24    /* writeln : Keyword */
#define    TTRUE        25    /* true : Keyword */
#define    TFALSE        26    /* false : Keyword */
#define    TNUMBER        27    /* unsigned integer */
#define    TSTRING        28    /* String */
#define    TPLUS        29    /* + : symbol */
#define    TMINUS        30    /* - : symbol */
#define    TSTAR        31    /* * : symbol */
#define    TEQUAL        32    /* = : symbol */
#define    TNOTEQ        33    /* <> : symbol */
#define    TLE        34    /* < : symbol */
#define    TLEEQ        35    /* <= : symbol */
#define    TGR        36    /* > : symbol */
#define    TGREQ        37    /* >= : symbol */
#define    TLPAREN        38    /* ( : symbol */
#define    TRPAREN        39    /* ) : symbol */
#define    TLSQPAREN    40    /* [ : symbol */
#define    TRSQPAREN    41    /* ] : symbol */
#define    TASSIGN        42    /* := : symbol */
#define    TDOT        43    /* . : symbol */
#define    TCOMMA        44    /* , : symbol */
#define    TCOLON        45    /* : : symbol */
#define    TSEMI        46    /* ; : symbol */
#define    TREAD        47    /* read : Keyword */
#define    TWRITE        48    /* write : Keyword */
#define    TBREAK        49    /* break : Keyword */

#define MAXTOKEN    49

#define KEYWORDSIZE    28
#define KEYWORDLENGTH    9

extern struct KEY {
    char *keyword;
    int keytoken;
} key_word[KEYWORDSIZE];

extern int error(char *mes);

// scan.c
extern int init_scan(char *filename, FILE **fp);

extern int scan(FILE *fp);

extern int not_token(char c, FILE *fp);

extern int keyword_check(const char *tokenstr);

extern int symbol_check(char *tokenstr, FILE *fp);

extern int not_token(char c, FILE *fp);

extern int num_attr;

extern char string_attr[MAXSTRSIZE];

extern char cbuf;

extern int linenum;

extern int get_linenum(void);

extern void end_scan(FILE *fp);

extern void next_buf(FILE *fp);


// print.c
extern int token;

extern int indent;

extern int parse_program(FILE *fp);

extern int block(FILE *fp);

extern int variable_declaration(FILE *fp);

extern int variable_names(FILE *fp);

extern int variable_name(FILE *fp);

extern int type(FILE *fp);

extern int standard_type(FILE *fp);

extern int array_type(FILE *fp);

extern int subprogram_declaration(FILE *fp);

extern int formal_parameters(FILE *fp);

extern int compound_statement(FILE *fp);

extern int statement(FILE *fp);

extern int condition_statement(FILE *fp);

extern int iteration_statement(FILE *fp);

extern int exit_statement(FILE *fp);

extern int call_statement(FILE *fp);

extern int expressions(FILE *fp);



extern int assignment_statement(FILE *fp);


extern int variable(FILE *fp);

extern int expression(FILE *fp);

extern int simple_expression(FILE *fp);

extern int term(FILE *fp);

extern int factor(FILE *fp);

extern int constant(FILE *fp);

extern int multiplicative_operator(FILE *fp);

extern int additive_operator(FILE *fp);

extern int relational_operator(FILE *fp);

extern int input_statement(FILE *fp);

extern int output_statement(FILE *fp);

extern int output_format(FILE *fp);

extern void indent_do(void);

