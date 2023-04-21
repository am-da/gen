
#ifndef MPPL_H_
#define MPPL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MAXSTRSIZE 1024
#define MAXNUMSIZE 32767
#define MAXLABELSIZE 6
#define ERROR -1
#define NORMAL 0

#define FORMAL_PARAM 1
#define NOT_FORMAL_PARAM 0

#define GLOBAL_PARAM 1
#define LOCAL_PARAM 0

#define TPINT 101
#define TPCHAR 102
#define TPBOOL 103
#define TPARRAY 104
#define TPPROC 108
#define NUMOFTYPE 8

/* Token */
#define    TNAME        1
#define    TPROGRAM    2
#define    TVAR        3
#define    TARRAY        4
#define    TOF            5
#define    TBEGIN        6
#define    TEND        7
#define    TIF            8
#define    TTHEN        9
#define    TELSE        10
#define    TPROCEDURE    11
#define    TRETURN        12
#define    TCALL        13
#define    TWHILE        14
#define    TDO            15
#define    TNOT        16
#define    TOR            17
#define    TDIV        18
#define    TAND        19
#define    TCHAR        20
#define    TINTEGER    21
#define    TBOOLEAN    22
#define    TREADLN        23
#define    TWRITELN    24
#define    TTRUE        25
#define    TFALSE        26
#define    TNUMBER        27
#define    TSTRING        28
#define    TPLUS        29
#define    TMINUS        30
#define    TSTAR        31
#define    TEQUAL        32
#define    TNOTEQ        33
#define    TLE            34
#define    TLEEQ        35
#define    TGR            36
#define    TGREQ        37
#define    TLPAREN        38
#define    TRPAREN        39
#define    TLSQPAREN    40
#define    TRSQPAREN    41
#define    TASSIGN        42
#define    TDOT        43
#define    TCOMMA        44
#define    TCOLON        45
#define    TSEMI        46
#define    TREAD        47
#define    TWRITE        48
#define    TBREAK        49    
#define KEYWORDSIZE    28

/* Instruction set */
#define EOVF 201
#define EOVF1 202
#define E0DIV 203
#define E0DIV1 204
#define EROV  205
#define EROV1 206
#define WRITECHAR 207
#define WRITESTR 208
#define BOVFCHECK 209
#define BOVFLEVEL 210
#define WRITEINT 211
#define MMINT 212
#define WRITEBOOL 213
#define WBTRUE 214
#define WBFALSE 215
#define WRITELINE 216
#define FLUSH 217
#define READCHAR 218
#define READINT 219
#define READLINE 220
#define ONE 221
#define SIX 222
#define TEN 223
#define SPACE 224
#define MINUS 225
#define TAB 226
#define ZERO 227
#define NINE 228
#define NEWLINE 229
#define INTBUF 230
#define OBUFSIZE 231
#define IBUFSIZE 232
#define INP 233
#define OBUF 234
#define IBUF 235
#define RPBBUF 236

struct KEYWORDS{
    char *keyword;
    int keytoken;
};

struct TYPE{
    int ttype;
    int arraysize;
    struct TYPE *etp;
    struct TYPE *paratp;
};

struct LINE{
    int reflinenum;
    struct LINE *nextep;
};

struct ID{
    char *name;
    char *procname;
    struct TYPE *itp;
    int ispara;
    int deflinenum;
    struct LINE *irefp;
    struct ID *nextp;
};

struct PARAM{
    struct ID *now;
    struct PARAM *nextp;
};

/* main.c */
extern FILE *input, *output;
extern int error(const char *mes, ...);

/* scan.c */
extern int num_attr;
extern int init_scan(char *filename, FILE **input);
extern int scan(void);
extern int get_linenum(void);
extern int keyword_check(const char *tokenstr);
extern int not_token(char c, FILE *input);
extern int symbol_check(char *tokenstr, FILE *fp) ;
extern void end_scan(FILE *input);
extern void next_buf(FILE *fp);
extern char string_attr[MAXSTRSIZE];

/* mppl_compiler.c */
extern int labelnum;
extern int init_outfile(char *filename, FILE **out);
extern int create_label(char **label);
extern int print_id_label(struct ID *p);
extern int register_strlabel(char *label, char *str);
extern int inst_start(char *program_name, char **st_label);
extern int inst_write_string(char *str);
extern int inst_write_value(int type, int nums);
extern int inst_expression(int opr);
extern int inst_factor_char(int type, int exp_type);
extern void Utility_functions(void);
extern void end_outfile(FILE *out);
extern void print_strlabel(void);
extern void inst_procedule_params(struct PARAM *params);

/* parser.c */
#define NUMOFTOKEN    49
extern char *tokenstr[NUMOFTOKEN+1]; 
extern int Parse_program(void);
extern int block(char *st_label);
extern int variable_declaration(void);
extern int variable_names(void);
extern int variable_name(void);
extern int type(void);
extern int standard_type(void);
extern int array_type(void);
extern int subprogram_declaration(void);
extern int formal_parameters(void);
extern int compound_statement(void);
extern int statement(void);
extern int condition_statement(void);
extern int iteration_statement(void);
extern int exit_statement(void);
extern int call_statement(void);
extern int expressions(struct TYPE *fparams);
extern int return_statement(void);
extern int assignment_statement(void);
extern int left_part(void);
extern int variable(void);
extern int expression(void);
extern int simple_expression(void);
extern int term(void);
extern int factor(void);
extern int constant(void);
extern int multiplicative_operator(void);
extern int additive_operator(void);
extern int relational_operator(void);
extern int input_statement(void);
extern int output_statement(void);
extern int output_format(void);
extern int empty_statement(void);

/* cxref.c */
extern struct ID *globalidroot;
extern struct ID *localidroot;
extern struct PARAM *param;
extern struct ID *search_global_idtab(char *name);
extern struct ID *search_local_idtab(char *name, char *pname);
extern struct TYPE *get_etp_type_structure(void);
extern struct PARAM *get_paratp(char *pname);
extern struct ID *refactor_to_lexicographical(struct ID *to, struct ID *from);
extern void init_global_idtab(void);
extern void init_local_idtab(void);
extern void release_global_idtab(void);
extern void relocate_local_idtab(void);
extern int reference_identifer(char *name, char *pname, int linenum, int refnum, int is_global);
extern int memorize_name(char *name);
extern int memorize_type(int ttype, int tsize, struct TYPE *tetp, struct PARAM *tparatp);
extern int memorize_linenum(int line);
extern int memorize_proc(char *name, int line);
extern int define_identifer(int vd_para, int is_global);
extern int get_prev_procline(void);
extern char *get_prev_procname(void);
extern char *get_prev_procname2(void);
extern char *type_str[NUMOFTYPE + 1];
#endif
