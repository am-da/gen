#include "mppl_compiler.h"

static struct CLABEL{
    char str[MAXSTRSIZE];
    struct CLABEL *nextp;
}*strlabelroot = NULL;

int lanum = 0;

int init_outfile(char *filename, FILE **out){
    char outfilename[MAXSTRSIZE];
    char *tmpname;

    tmpname = strtok(filename, ".");
    snprintf(outfilename, MAXSTRSIZE, "%.30s.csl", tmpname);
    *out = fopen(outfilename, "w");
    if(*out == NULL){
        return error("%s can not open", outfilename);
    }
    return 0;
}

void end_outfile(FILE *out){
    fclose(out);
}

int create_label(char **label){
    char *label1;

    if((label1 = (char *)malloc((MAXLABELSIZE +1) * sizeof(char))) == NULL) return error("can not malloc ");
    
    lanum++;

    if(lanum >= 10000) return error("label error");
    
    snprintf(label1, MAXLABELSIZE, "L%04d", lanum);
    *label = label1;
    return NORMAL;
}

int printID(struct ID *p){
    if(p == NULL){
        return error("ID is not found");
    }else if(p->itp == NULL){
        return error("type of ID is not defined");
    }else if(p->name == NULL){
        return error("name of ID is not defined");
    }

    if(p->procname != NULL) {
        fprintf(output, "$%s%%%s", p->name, p->procname); 
    }
    else fprintf(output, "$%s", p->name);

    if(p->itp->ttype == TPARRAY) fprintf(output, "\tDS\t%d\n", p->itp->arraysize);
    else if(p->itp->ttype == TPPROC) fprintf(output, "\tDS\t0\n");
    else fprintf(output, "\tDC\t0\n");
    return NORMAL;
}

void printstr(){
    struct CLABEL *p;
    for(p = strlabelroot; p != NULL; p = p->nextp){
        fprintf(output, "%s", p->str);
    }
}

int add_label(char *label, char *str){
    struct CLABEL *p, *q;
    if((p = (struct CLABEL *)malloc(sizeof(struct CLABEL))) == NULL){
        return error("cannot malloc");
    }
    snprintf(p->str, MAXSTRSIZE, "%s\tDC\t%s\n", label, str);
    p->nextp = NULL;
    if(strlabelroot == NULL){
        strlabelroot = p;
        return NORMAL;
    }

    for(q = strlabelroot; q->nextp != NULL; q = q->nextp){}
    q->nextp = p;
    
    return NORMAL;
}

int out_ass(char *program_name, char **st_label){
    fprintf(output, ";program %s;\n", program_name);
    fprintf(output, "$$%s\tSTART\n", program_name);
    fprintf(output, "\tLAD\tgr0,0\n");
    if(create_label(st_label) == ERROR) return ERROR;
    fprintf(output, "\tCALL\t%s\n", *st_label);
    fprintf(output, "\tCALL\tFLUSH\n"
     "\tSVC\t0\n");
    return NORMAL;
}

void para_stack(struct PARA *para){
    struct PARA *p;
    if(para == NULL) return ;
    fprintf(output, "\tPOP\tgr2\n");
    for(p = para; p != NULL; p = p->nextp){
        fprintf(output, "\tPOP\tgr1\n");
        if(p->now->procname != NULL){
            fprintf(output, "\tST\tgr1,$%s%%%s\n", p->now->name, p->now->procname);
        }else{
            fprintf(output, "\tST\tgr1,$%s\n", p->now->name);
        }
    }
    fprintf(output, "\tPUSH\t0,gr2\n");
}

int para_string(char *str){
    char *label;
    char labelt[MAXSTRSIZE];
    if(create_label(&label) == ERROR) return ERROR;
    fprintf(output, "\tLAD\tgr1,%s\n",label);
    fprintf(output, "\tLD\tgr2,gr0\n"
     "\tCALL\tWRITESTR\n");
    snprintf(labelt, MAXSTRSIZE, "'%s'", str);
    if(add_label(label, labelt) == ERROR) return ERROR;
    return NORMAL;
}

int para_ass(int type, int nums){
    if(nums > 0){
        fprintf(output, "\tLAD\tgr2,%d\n", num_attr);
    }else{
        fprintf(output, "\tLD\tgr2,gr0\n");
    }
    
    if (type == TPINT){
        fprintf(output, "\tCALL\tWRITEINT\n");
    }
    else if(type == TPCHAR){
        fprintf(output, "\tCALL\tWRITECHAR\n");
    }
    else if(type == TPBOOL){
        fprintf(output, "\tCALL\tWRITEBOOL\n");
    }
    return NORMAL;
}

int compare_en(int opr){
    char *labela = NULL, *labelb = NULL;
    if(create_label(&labela) == ERROR) return ERROR;
    if(create_label(&labelb) == ERROR) return ERROR;

    fprintf(output, "\tPOP\tgr2\n"
    "\tCPA\tgr2,gr1\n");
    if (opr == TEQUAL){
        fprintf(output, "\tJZE\t%s\n\tLD\tgr1,gr0\n", labela);
    }
    else if (opr == TNOTEQ){
        fprintf(output, "\tJNZ\t%s\n\tLD\tgr1,gr0\n", labela);
    }
    else if(opr == TLE){
        fprintf(output, "\tJMI\t%s\n\tLD\tgr1,gr0\n", labela);
    }
    else if (opr == TLEEQ){
        fprintf(output, "\tJPL\t%s\n\tLAD\tgr1,1\n", labela);
    }
    else if (opr == TGR){
        fprintf(output, "\tJPL\t%s\n\tLD\tgr1,gr0\n", labela);
    }
    else if (opr == TGREQ){
        fprintf(output, "\tJMI\t%s\n\tLAD\tgr1,1\n", labela);
    }

    fprintf(output, "\tJUMP\t%s\n", labelb);
    fprintf(output, "%s", labela);
    fprintf(output, "\tDS\t0\n");
    if(opr == TLEEQ || opr == TGREQ){
        fprintf(output, "\tLD\tgr1,gr0\n");
    }else
    {
        fprintf(output, "\tLAD\tgr1,1\n");
    }
    fprintf(output, "%s", labelb);
    fprintf(output, "\tDS\t0\n");
    return NORMAL;
}

int convert_siki(int type, int exp_type){
    char *label = NULL;
    switch(exp_type){
        case TPINT:
            switch(type){
                case TPINT:
                    break;
                case TPBOOL:
                    if(create_label(&label) == ERROR) return ERROR;
                    fprintf(output, "\tCPA\tgr1, gr0\n"
                    "\tJZE\t%s\n", label);
                    fprintf(output, "\tLAD\tgr1,1\n"
                    "%s", label);
                    fprintf(output, "\tDS\t0\n");
                    break;
                case TPCHAR:
                    fprintf(output, "\tLAD\tgr2,127\n"
                            "\tAND\tgr1,gr2\n");
                    break;
                default:
                    break;
            }
            break;
        case TPCHAR:
            switch(type){
                case TPINT:
                    break;
                case TPBOOL:
                    if(create_label(&label) == ERROR) return ERROR;
                    fprintf(output, "\tCPA\tgr1,gr0\n"
                     "\tJZE\t%s\n", label);
                    fprintf(output, "\tLAD\tgr1,1\n"
                     "%s", label);
                    fprintf(output, "\tDS\t0\n");
                    break;
                case TPCHAR:
                    break;
                default:
                    break;
            }
            break;
        case TPBOOL:
            switch(type){
                case TPINT:
                    break;
                case TPBOOL:
                    break;
                case TPCHAR:
                    if(create_label(&label) == ERROR) return ERROR;
                    fprintf(output, "\tCPA\tgr1,gr0"
                    "\tJZE\t%s\n", label);
                    fprintf(output, "\tLAD\tgr1,1\n"
                     "%s", label);
                    fprintf(output, "\tDS\t0\n");
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return NORMAL;
}

void Utility_functions(){
    fprintf(output,
  "; ------------------------\n"
  "; Utility functions\n"
  "; ------------------------\n"
 "EOVF\tCALL\tWRITELINE\n"
 "\tLAD\tgr1,EOVF1\n"
 "\tLD\tgr2,gr0\n"
 "\tCALL\tWRITESTR\n"
 "\tCALL\tWRITELINE\n"
 "\tSVC\t1\t;\toverflow error stop\n"
 "EOVF1\tDC\t'***** Run-Time Error : Overflow *****'\n"
 "E0DIV"
 "\tJNZ\tEOVF\n"
 "\tCALL\tWRITELINE\n"
 "\tLAD\tgr1,E0DIV1\n"
 "\tLD\tgr2,gr0\n"
 "\tCALL\tWRITESTR\n"
 "\tCALL\tWRITELINE\n"
 "\tSVC\t2\t;\t0-divide error stop\n"
 "E0DIV1\tDC\t'***** Run-Time Error : Zero-Divide *****'\n"
 "EROV"
 "\tCALL \tWRITELINE\n"
  "\tLAD\tgr1,EROV1\n"
 "\tLD\tgr2,gr0\n"
 "\tCALL\tWRITESTR\n"
 "\tCALL\tWRITELINE\n"
 "\tSVC\t3\t;\trange-over error stop\n"
 "EROV1\tDC\t'***** Run-Time ERROR : Range-Over in Array Index *****'\n"
 "WRITECHAR"
 "\tRPUSH\n"
 "\tLD\tgr6,SPACE\n"
 "\tLD\tgr7,OBUFSIZE\n"
 "WC1"
 "\tSUBA\tgr2,ONE\n"
 "\tJZE\tWC2\n"
"\tJMI\tWC2\n"
"\tST\tgr6,OBUF,gr7\n"
 "\tCALL\tBOVFCHECK\n"
"\tJUMP\tWC1\n"
 "WC2"
 "\tST\tgr1,OBUF,gr7\n"
 "\tCALL\tBOVFCHECK\n"
 "\tST\tgr7,OBUFSIZE\n"
 "\tRPOP\n"
 "\tRET\n"
 "WRITESTR"
 "\tRPUSH\n"
 "\tLD\tgr6,gr1\n"
 "WS1"
 "\tLD\tgr4,0,gr6\n"
 "\tJZE\tWS2\n"
 "\tADDA\tgr6,ONE\n"
 "\tSUBA\tgr2,ONE\n"
 "\tJUMP\tWS1\n"
 "WS2"
 "\tLD\tgr7,OBUFSIZE\n"
 "\tLD\tgr5,SPACE\n"
 "WS3"
 "\tSUBA\tgr2,ONE\n"
 "\tJMI\tWS4\n"
"\tST\tgr5,OBUF,gr7\n"
 "\tCALL\tBOVFCHECK\n"
 "\tJUMP\tWS3\n"
 "WS4"
 "\tLD\tgr4,0,gr1\n"
 "\tJZE\tWS5\n"
 "\tST\tgr4,OBUF,gr7\n"
 "\tADDA \tgr1,ONE\n"
 "\tCALL\tBOVFCHECK\n"
 "\tJUMP\tWS4\n"
 "WS5"
 "\tST\tgr7,OBUFSIZE\n"
 "\tRPOP\n"
 "\tRET\n"
 "BOVFCHECK"
 "\tADDA\tgr7,ONE\n"
 "\tCPA\tgr7,BOVFLEVEL\n"
 "\tJMI\tBOVF1\n"
 "\tCALL\tWRITELINE\n"
 "\tLD\tgr7,OBUFSIZE\n"
 "BOVF1"
 "\tRET\n"
 "BOVFLEVEL\tDC\t256\n"
 "WRITEINT"
 "\tRPUSH\n"
"\tLD\tgr7,gr0\n"
 "\tCPA\tgr1,gr0\n"
 "\tJPL\tWI1\n"
 "\tJZE\tWI1\n"
 "\tLD\tgr4,gr0\n"
 "\tSUBA\tgr4,gr1\n"
 "\tCPA\tgr4,gr1\n"
 "\tJZE\tWI6\n"
 "\tLD\tgr1,gr4\n"
 "\tLD\tgr7,ONE\n"
 "WI1"
 "\tLD\tgr6,SIX\n"
 "\tST\tgr0,INTBUF,gr6\n"
 "\tSUBA\tgr6,ONE\n"
 "\tCPA\tgr1,gr0\n"
"\tJNZ\tWI2\n"
 "\tLD \tgr4,ZERO\n"
 "\tST\tgr4,INTBUF,gr6\n"
 "\tJUMP\tWI5\n"
 "WI2"
"\tCPA\tgr1,gr0\n"
 "\tJZE\tWI3\n"
 "\tLD\tgr5,gr1\n"
"\tDIVA\tgr1,TEN\n"
 "\tLD\tgr4,gr1\n"
 "\tMULA\tgr4,TEN\n"
 "\tSUBA\tgr5,gr4\n"
 "\tADDA\tgr5,ZERO\n"
 "\tST\tgr5,INTBUF,gr6\n"
 "\tSUBA\tgr6,ONE\n"
 "\tJUMP\tWI2\n"
 "WI3"
 "\tCPA\tgr7,gr0\n"
 "\tJZE\tWI4\n"
 "\tLD\tgr4,MINUS\n"
 "\tST\tgr4,INTBUF,gr6\n"
 "\tJUMP\tWI5\n"
 "WI4"
 "\tADDA\tgr6,ONE\n"
 "WI5"
 "\tLAD\tgr1,INTBUF,gr6\n"
 "\tCALL\tWRITESTR\n"
 "\tRPOP\n"
 "\tRET\n"
 "WI6"
 "\tLAD\tgr1,MMINT\n"
 "\tCALL\tWRITESTR\n"
"\tRPOP\n"
"\tRET\n"
 "MMINT\tDC\t'-32768'\n"
 "WRITEBOOL"
 "\tRPUSH\n"
 "\tCPA\tgr1,gr0\n"
 "\tJZE\tWB1\n"
 "\tLAD\tgr1,WBTRUE\n"
"\tJUMP\tWB2\n"
 "WB1"
 "\tLAD\tgr1,WBFALSE\n"
"WB2"
 "\tCALL\tWRITESTR\n"
 "\tRPOP\n"
"\tRET\n"
 "WBTRUE\tDC 'TRUE'\n"
 "WBFALSE\tDC 'FALSE'\n"
 "WRITELINE"
 "\tRPUSH\n"
 "\tLD\tgr7,OBUFSIZE\n"
 "\tLD\tgr6,NEWLINE\n"
 "\tST\tgr6,OBUF,gr7\n"
 "\tADDA\tgr7,ONE\n"
 "\tST\tgr7,OBUFSIZE\n"
 "\tOUT\tOBUF,OBUFSIZE\n"
   "\tST\tgr0,OBUFSIZE\n"
 "\tRPOP\n"
 "\tRET\n"
 "FLUSH"
 "\tRPUSH\n"
"\tLD\tgr7,OBUFSIZE\n"
 "\tJZE\tFL1\n"
 "\tCALL\tWRITELINE\n"
 "FL1"
 "\tRPOP\n"
 "\tRET\n"
 "READCHAR"
 "\tRPUSH\n"
"\tLD\tgr5,RPBBUF\n"
 "\tJZE\tRC0\n"
"\tST\tgr5,0,gr1\n"
    "\tST\tgr0,RPBBUF\n"
 "\tJUMP\tRC3\n"
 "RC0"
 "\tLD\tgr7,INP\n"
"\tLD\tgr6,IBUFSIZE\n"
 "\tJNZ\tRC1\n"
 "\tIN\tIBUF,IBUFSIZE\n"
"\tLD\tgr7,gr0\n"
 "RC1"
 "\tCPA\tgr7,IBUFSIZE\n"
 "\tJNZ\tRC2\n"
 "\tLD\tgr5,NEWLINE\n"
 "\tST\tgr5,0,gr1\n"
 "\tST\tgr0,IBUFSIZE\n"
 "\tST\tgr0,INP\n"
 "\tJUMP\tRC3\n"
 "RC2"
"\tLD\tgr5,IBUF,gr7\n"
 "\tADDA\tgr7,ONE\n"
 "\tST\tgr5,0,gr1\n"
 "\tST\tgr7,INP\n"
 "RC3"
"\tRPOP\n"
"\tRET\n"
"READINT"
 "\tRPUSH\n"
 "RI1"
   "\tCALL\tREADCHAR\n"
   "\tLD\tgr7,0,gr1\n"
 "\tCPA\tgr7,SPACE\n"
 "\tJZE\tRI1\n"
 "\tCPA\tgr7,TAB\n"
 "\tJZE\tRI1\n"
    "\tCPA\tgr7,NEWLINE\n"
"\tJZE\tRI1\n"
 "\tLD\tgr5,ONE\n"
"\tCPA\tgr7,MINUS\n"
 "\tJNZ\tRI4\n"
 "\tLD\tgr5,gr0\n"
  "\tCALL\tREADCHAR\n"
 "\tLD\tgr7,0,gr1\n"
 "RI4"
 "\tLD\tgr6,gr0\n"
 "RI2"
 "\tCPA\tgr7,ZERO\n"
 "\tJMI\tRI3\n"
 "\tCPA\tgr7,NINE\n"
 "\tJPL\tRI3\n"
 "\tMULA\tgr6,TEN\n"
"\tADDA\tgr6,gr7\n"
 "\tSUBA\tgr6,ZERO\n"
"\tCALL\tREADCHAR\n"
 "\tLD\tgr7,0,gr1\n"
 "\tJUMP\tRI2\n"
 "RI3"
 "\tST\tgr7,RPBBUF\n"
 "\tST\tgr6,0,gr1\n"
 "\tCPA\tgr5,gr0\n"
"\tJNZ\tRI5\n"
 "\tSUBA\tgr5,gr6\n"
"\tST\tgr5,0,gr1\n"
 "RI5"
 "\tRPOP\n"
 "\tRET\n"
  "READLINE"
"\tST\tgr0,IBUFSIZE\n"
 "\tST\tgr0,INP\n"
 "\tST\tgr0,RPBBUF\n"
 "\tRET\n"
 "ONE\tDC\t1\n"
 "SIX\tDC\t6\n"
"TEN\tDC\t10\n"
 "SPACE\tDC\t#0020\n"
 "MINUS\tDC\t#002D\n"
 "TAB\tDC\t#0009\n"
 "ZERO\tDC\t#0030\n"
 "NINE\tDC\t#0039\n"
 "NEWLINE\tDC\t#000A\n"
 "INTBUF\tDS\t8\n"
 "OBUFSIZE\tDC\t0\n"
  "IBUFSIZE\tDC\t0\n"
 "INP\tDC\t0\n"
 "OBUF\tDS\t257\n"
  "IBUF\tDS\t257\n"
 "RPBBUF\tDC\t0\n");
}
