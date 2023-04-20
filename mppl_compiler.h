#ifndef _MPPL_COMPILER_H_
#define _MPPL_COMPILER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR -1
#define NORMAL 0

#define MAXSTRSIZE 1024
#define NUMOFTOKEN 49

#define TNAME 1
#define TPROGRAM 2
#define TVAR 3
#define TARRAY 4
#define TOF 5
#define TBEGIN 6
#define TEND 7
#define TIF 8
#define TTHEN 9
#define TELSE 10
#define TPROCEDURE 11
#define TRETURN 12
#define TCALL 13
#define TWHILE 14
#define TDO 15
#define TNOT 16
#define TOR 17
#define TDIV 18
#define TAND 19
#define TCHAR 20
#define TINTEGER 21
#define TBOOLEAN 22
#define TREADLN 23
#define TWRITELN 24
#define TTRUE 25
#define TFALSE 26
#define TNUMBER 27
#define TSTRING 28
#define TPLUS 29
#define TMINUS 30
#define TSTAR 31
#define TEQUAL 32
#define TNOTEQ 33
#define TLE 34
#define TLEEQ 35
#define TGR 36
#define TGREQ 37
#define TLPAREN 38
#define TRPAREN 39
#define TLSQPAREN 40
#define TRSQPAREN 41
#define TASSIGN 42
#define TDOT 43
#define TCOMMA 44
#define TCOLON 45
#define TSEMI 46
#define TREAD 47
#define TWRITE 48
#define TBREAK 49

#define NUMOFTYPE 8
#define TPNONE 0
#define TPINT 1
#define TPCHAR 2
#define TPBOOL 3
#define TPARRAY 4
#define TPARRAYINT 5
#define TPARRAYCHAR 6
#define TPARRAYBOOL 7
#define TPPROC 8
#define MAX_NUM 32767
#define KEYWORDSIZE 28



extern struct KEY {
    char *keyword;
    int keytoken; 
} key[KEYWORDSIZE];

struct TYPE {
    int ttype;
    int arraysize;       /*! size of array, if TPARRAY */
    struct TYPE *etp;    /*! pointer to element type if TPARRAY */
    struct TYPE *paratp; /*! pointer to parameter's type list if ttype is TPPROC */
};

struct LINE {
    int reflinenum;         /*! the line number */
    struct LINE *nextlinep; /*! pointer to next struct */
};

struct ID {
    char *name;
    char *procname;     /* procedure name within which this name is defined, NULL if global name */
    struct TYPE *itp;   /*! Type for the name */
    int ispara;         /*! 1:formal parameter, 0:else(variable) */
    int deflinenum;     /*! Name defined line number */
    struct LINE *irefp; /*! List of line numbers where the name was referenced */
    struct ID *nextp;   /*! pointer next struct */
};

extern struct ID *allroot;
extern int error(char *mes);

/* scan.c*/
extern FILE *fp;
extern int num_attr;
extern char string_attr[MAXSTRSIZE];
extern int init_scan(char *filename);
extern int scan(void);
extern int get_linenum(void);
extern int end_scan(void);

/* cr.c */
extern char current_procedure_name[MAXSTRSIZE];
extern int parse_program(void);
extern int subprogram;
extern int definition_procedure_name;
extern int formal_para;
extern void init_cr(void);
extern void release_all(void);
extern int release_local(void);
extern int id_register_without_type(char *name);
extern int div_type(struct TYPE **type);
extern struct TYPE *def_type(int type);
extern struct TYPE *array_type(int type);
extern int register_linenum(char *name);
extern struct ID *search_procedure(char *procname);
extern void print_cr(struct ID *root);

/* pp.c */
extern int block(void);
extern int variable_declaration(void);
extern int variable_names(void);
extern int compound_statement(void);
extern int parse_type(void);
extern int standard_type(void);
extern int parse_array_type(void);
extern int subprogram_declaration(void);
extern int procedure_name(void);
extern int formal_parameters(void);
extern int statement(void);
extern int assignment_statement(void);
extern int condition_statement(void);
extern int iteration_statement(void);
extern int call_statement(void);
extern int variable(void);
extern int input_statement(void);
extern int output_statement(void);
extern int output_format(void);
extern int compound_statement(void);
extern int expression(void);
extern int simple_expression(void);
extern int is_relational_operator(int token);
extern int term(void);
extern int factor(void);
extern int constant(void);
extern int expressions(void);

/* main.c */
extern char *tokenstr[NUMOFTOKEN + 1];
extern char *typestr[NUMOFTYPE + 1];
extern int token;

#endif
