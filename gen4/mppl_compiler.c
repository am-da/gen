#include "mppl_compiler.h"

static struct SLABEL{
    char str[MAXSTRSIZE];
    struct SLABEL *nextp;
}*strlabelroot = NULL;

int labelnum = 0;

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
    char *t_label;

    if((t_label = (char *)malloc((MAXLABELSIZE +1) * sizeof(char))) == NULL) return error("can not malloc in create label");
    
    labelnum++;

    if(labelnum >= 10000) return error("num of label is too many");
    
    snprintf(t_label, MAXLABELSIZE, "L%04d", labelnum);
    *label = t_label;
    return NORMAL;
}

int print_id_label(struct ID *p){
    if(p == NULL){
        return error("ID is not found");
    }else if(p->itp == NULL){
        return error("type of ID is not defined");
    }else if(p->name == NULL){
        return error("name of ID is not defined");
    }

    if(p->procname != NULL) {
        fprintf(output, "$%s%%%s", p->name, p->procname); //?
        //np = p->name;
    }
    else fprintf(output, "$%s", p->name);

    if(p->itp->ttype == TPARRAY) fprintf(output, "\tDS\t%d\n", p->itp->arraysize);
    else if(p->itp->ttype == TPPROC) fprintf(output, "\tDS\t0\n");
    else fprintf(output, "\tDC\t0\n");
    return NORMAL;
}

void print_strlabel(){
    struct SLABEL *p;
    for(p = strlabelroot; p != NULL; p = p->nextp){
        fprintf(output, "%s", p->str);
    }
}

int register_strlabel(char *label, char *str){
    struct SLABEL *p, *q;
    if((p = (struct SLABEL *)malloc(sizeof(struct SLABEL))) == NULL){
        return error("cannot malloc in register strlabel");
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

int inst_start(char *program_name, char **st_label){
    fprintf(output, ";program %s;\n", program_name);
    fprintf(output, "$$%s\tSTART\n", program_name);
    fprintf(output, "\tLAD\tgr0,0\n");
    if(create_label(st_label) == ERROR) return ERROR;
    fprintf(output, "\tCALL\t%s\n", *st_label);
    fprintf(output, "\tCALL\tFLUSH\n");
    fprintf(output, "\tSVC\t0\n");
    return NORMAL;
}

void inst_procedule_params(struct PARAM *para){
    struct PARAM *p;
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

int inst_write_string(char *str){
    char *label;
    char t_str[MAXSTRSIZE];
    if(create_label(&label) == ERROR) return ERROR;
    fprintf(output, "\tLAD\tgr1,%s\n",label);
    fprintf(output, "\tLD\tgr2,gr0\n");
    fprintf(output, "\tCALL\tWRITESTR\n");
    snprintf(t_str, MAXSTRSIZE, "'%s'", str);
    if(register_strlabel(label, t_str) == ERROR) return ERROR;
    return NORMAL;
}

int inst_write_value(int type, int nums){
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

int inst_expression(int opr){
    char *label_t = NULL, *label_f = NULL;
    if(create_label(&label_t) == ERROR) return ERROR;
    if(create_label(&label_f) == ERROR) return ERROR;

    fprintf(output, "\tPOP\tgr2\n");
    fprintf(output, "\tCPA\tgr2,gr1\n");
    if (opr == TEQUAL){
        fprintf(output, "\tJZE\t%s\n", label_t);
        fprintf(output, "\tLD\tgr1,gr0\n");
    }
    else if (opr == TNOTEQ){
        fprintf(output, "\tJNZ\t%s", label_t);
        fprintf(output, "\tLD\tgr1,gr0\n");
    }
    else if(opr == TLE){
        fprintf(output, "\tJMI\t%s\n", label_t);
        fprintf(output, "\tLD\tgr1,gr0\n");
    }
    else if (opr == TLEEQ){
        fprintf(output, "\tJPL\t%s\n", label_t);
        fprintf(output, "\tLAD\tgr1,1\n");
    }
    else if (opr == TGR){
        fprintf(output, "\tJPL\t%s\n", label_t);
        fprintf(output, "\tLD\tgr1,gr0\n");
    }
    else if (opr == TGREQ){
        fprintf(output, "\tJMI\t%s", label_t);
        fprintf(output, "\tLAD\tgr1,1\n");
    }

    fprintf(output, "\tJUMP\t%s\n", label_f);
    fprintf(output, "%s", label_t);
    fprintf(output, "\tDS\t0\n");
    if(opr == TLEEQ || opr == TGREQ){
        fprintf(output, "\tLD\tgr1,gr0\n");
    }else
    {
        fprintf(output, "\tLAD\tgr1,1\n");
    }
    fprintf(output, "%s", label_f);
    fprintf(output, "\tDS\t0\n");
    return NORMAL;
}

int inst_factor_char(int type, int exp_type){
    char *label = NULL;
    switch(exp_type){
        case TPINT:
            switch(type){
                case TPINT:
                    break;
                case TPBOOL:
                    if(create_label(&label) == ERROR) return ERROR;
                    fprintf(output, "\tCPA\tgr1, gr0\n");
                    fprintf(output, "\tJZE\t%s\n", label);
                    fprintf(output, "\tLAD\tgr1,1\n");
                    fprintf(output, "%s", label);
                    fprintf(output, "\tDS\t0\n");
                    break;
                case TPCHAR:
                    fprintf(output, "\tLAD\tgr2,127\n");
                    fprintf(output, "\tAND\tgr1,gr2\n");
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
                    fprintf(output, "\tCPA\tgr1,gr0\n");
                    fprintf(output, "\tJZE\t%s\n", label);
                    fprintf(output, "\tLAD\tgr1,1\n");
                    fprintf(output, "%s", label);
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
                    fprintf(output, "\tCPA\tgr1,gr0");
                    fprintf(output, "\tJZE\t%s\n", label);
                    fprintf(output, "\tLAD\tgr1,1\n");
                    fprintf(output, "%s", label);
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
    fprintf(output, "EOVF");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "\tLAD\tgr1,EOVF1\n");
    fprintf(output, "\tLD\tgr2,gr0\n");
    fprintf(output, "\tCALL\tWRITESTR\n");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "\tSVC\t1\t;\toverflow error stop\n");
    fprintf(output, "EOVF1\tDC\t'***** Run-Time Error : Overflow *****'\n");
    fprintf(output, "E0DIV");
    fprintf(output, "\tJNZ\tEOVF\n");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "\tLAD\tgr1,E0DIV1\n");
    fprintf(output, "\tLD\tgr2,gr0\n");
    fprintf(output, "\tCALL\tWRITESTR\n");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "\tSVC\t2\t;\t0-divide error stop\n");
    fprintf(output, "E0DIV1\tDC\t'***** Run-Time Error : Zero-Divide *****'\n");
    fprintf(output, "EROV");
    fprintf(output, "\tCALL \tWRITELINE\n");
    fprintf(output, "\tLAD\tgr1,EROV1\n");
    fprintf(output, "\tLD\tgr2,gr0\n");
    fprintf(output, "\tCALL\tWRITESTR\n");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "\tSVC\t3\t;\trange-over error stop\n");
    fprintf(output, "EROV1\tDC\t'***** Run-Time ERROR : Range-Over in Array Index *****'\n");
    fprintf(output, "WRITECHAR");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tLD\tgr6,SPACE\n");
    fprintf(output, "\tLD\tgr7,OBUFSIZE\n");
    fprintf(output, "WC1");
    fprintf(output, "\tSUBA\tgr2,ONE\n");
    fprintf(output, "\tJZE\tWC2\n");
    fprintf(output, "\tJMI\tWC2\n");
    fprintf(output, "\tST\tgr6,OBUF,gr7\n");
    fprintf(output, "\tCALL\tBOVFCHECK\n");
    fprintf(output, "\tJUMP\tWC1\n");
    fprintf(output, "WC2");
    fprintf(output, "\tST\tgr1,OBUF,gr7\n");
    fprintf(output, "\tCALL\tBOVFCHECK\n");
    fprintf(output, "\tST\tgr7,OBUFSIZE\n");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "WRITESTR");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tLD\tgr6,gr1\n");
    fprintf(output, "WS1");
    fprintf(output, "\tLD\tgr4,0,gr6\n");
    fprintf(output, "\tJZE\tWS2\n");
    fprintf(output, "\tADDA\tgr6,ONE\n");
    fprintf(output, "\tSUBA\tgr2,ONE\n");
    fprintf(output, "\tJUMP\tWS1\n");
    fprintf(output, "WS2");
    fprintf(output, "\tLD\tgr7,OBUFSIZE\n");
    fprintf(output, "\tLD\tgr5,SPACE\n");
    fprintf(output, "WS3");
    fprintf(output, "\tSUBA\tgr2,ONE\n");
    fprintf(output, "\tJMI\tWS4\n");
    fprintf(output, "\tST\tgr5,OBUF,gr7\n");
    fprintf(output, "\tCALL\tBOVFCHECK\n");
    fprintf(output, "\tJUMP\tWS3\n");
    fprintf(output, "WS4");
    fprintf(output, "\tLD\tgr4,0,gr1\n");
    fprintf(output, "\tJZE\tWS5\n");
    fprintf(output, "\tST\tgr4,OBUF,gr7\n");
    fprintf(output, "\tADDA \tgr1,ONE\n");
    fprintf(output, "\tCALL\tBOVFCHECK\n");
    fprintf(output, "\tJUMP\tWS4\n");
    fprintf(output, "WS5");
    fprintf(output, "\tST\tgr7,OBUFSIZE\n");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "BOVFCHECK");
    fprintf(output, "\tADDA\tgr7,ONE\n");
    fprintf(output, "\tCPA\tgr7,BOVFLEVEL\n");
    fprintf(output, "\tJMI\tBOVF1\n");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "\tLD\tgr7,OBUFSIZE\n");
    fprintf(output, "BOVF1");
    fprintf(output, "\tRET\n");
    fprintf(output, "BOVFLEVEL\tDC\t256\n");
    fprintf(output, "WRITEINT");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tLD\tgr7,gr0\n");
    fprintf(output, "\tCPA\tgr1,gr0\n");
    fprintf(output, "\tJPL\tWI1\n");
    fprintf(output, "\tJZE\tWI1\n");
    fprintf(output, "\tLD\tgr4,gr0\n");
    fprintf(output, "\tSUBA\tgr4,gr1\n");
    fprintf(output, "\tCPA\tgr4,gr1\n");
    fprintf(output, "\tJZE\tWI6\n");
    fprintf(output, "\tLD\tgr1,gr4\n");
    fprintf(output, "\tLD\tgr7,ONE\n");
    fprintf(output, "WI1");
    fprintf(output, "\tLD\tgr6,SIX\n");
    fprintf(output, "\tST\tgr0,INTBUF,gr6\n");
    fprintf(output, "\tSUBA\tgr6,ONE\n");
    fprintf(output, "\tCPA\tgr1,gr0\n");
    fprintf(output, "\tJNZ\tWI2\n");
    fprintf(output, "\tLD \tgr4,ZERO\n");
    fprintf(output, "\tST\tgr4,INTBUF,gr6\n");
    fprintf(output, "\tJUMP\tWI5\n");
    fprintf(output, "WI2");
    fprintf(output, "\tCPA\tgr1,gr0\n");
    fprintf(output, "\tJZE\tWI3\n");
    fprintf(output, "\tLD\tgr5,gr1\n");
    fprintf(output, "\tDIVA\tgr1,TEN\n");
    fprintf(output, "\tLD\tgr4,gr1\n");
    fprintf(output, "\tMULA\tgr4,TEN\n");
    fprintf(output, "\tSUBA\tgr5,gr4\n");
    fprintf(output, "\tADDA\tgr5,ZERO\n");
    fprintf(output, "\tST\tgr5,INTBUF,gr6\n");
    fprintf(output, "\tSUBA\tgr6,ONE\n");
    fprintf(output, "\tJUMP\tWI2\n");
    fprintf(output, "WI3");
    fprintf(output, "\tCPA\tgr7,gr0\n");
    fprintf(output, "\tJZE\tWI4\n");
    fprintf(output, "\tLD\tgr4,MINUS\n");
    fprintf(output, "\tST\tgr4,INTBUF,gr6\n");
    fprintf(output, "\tJUMP\tWI5\n");
    fprintf(output, "WI4");
    fprintf(output, "\tADDA\tgr6,ONE\n");
    fprintf(output, "WI5");
    fprintf(output, "\tLAD\tgr1,INTBUF,gr6\n");
    fprintf(output, "\tCALL\tWRITESTR\n");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "WI6");
    fprintf(output, "\tLAD\tgr1,MMINT\n");
    fprintf(output, "\tCALL\tWRITESTR\n");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "MMINT\tDC\t'-32768'\n");
    fprintf(output, "WRITEBOOL");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tCPA\tgr1,gr0\n");
    fprintf(output, "\tJZE\tWB1\n");
    fprintf(output, "\tLAD\tgr1,WBTRUE\n");
    fprintf(output, "\tJUMP\tWB2\n");
    fprintf(output, "WB1");
    fprintf(output, "\tLAD\tgr1,WBFALSE\n");
    fprintf(output, "WB2");
    fprintf(output, "\tCALL\tWRITESTR\n");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "WBTRUE\tDC 'TRUE'\n");
    fprintf(output, "WBFALSE\tDC 'FALSE'\n");
    fprintf(output, "WRITELINE");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tLD\tgr7,OBUFSIZE\n");
    fprintf(output, "\tLD\tgr6,NEWLINE\n");
    fprintf(output, "\tST\tgr6,OBUF,gr7\n");
    fprintf(output, "\tADDA\tgr7,ONE\n");
    fprintf(output, "\tST\tgr7,OBUFSIZE\n");
    fprintf(output, "\tOUT\tOBUF,OBUFSIZE\n");
    fprintf(output, "\tST\tgr0,OBUFSIZE\n");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "FLUSH");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tLD\tgr7,OBUFSIZE\n");
    fprintf(output, "\tJZE\tFL1\n");
    fprintf(output, "\tCALL\tWRITELINE\n");
    fprintf(output, "FL1");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "READCHAR");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "\tLD\tgr5,RPBBUF\n");
    fprintf(output, "\tJZE\tRC0\n");
    fprintf(output, "\tST\tgr5,0,gr1\n");
    fprintf(output, "\tST\tgr0,RPBBUF\n");
    fprintf(output, "\tJUMP\tRC3\n");
    fprintf(output, "RC0");
    fprintf(output, "\tLD\tgr7,INP\n");
    fprintf(output, "\tLD\tgr6,IBUFSIZE\n");
    fprintf(output, "\tJNZ\tRC1\n");
    fprintf(output, "\tIN\tIBUF,IBUFSIZE\n");
    fprintf(output, "\tLD\tgr7,gr0\n");
    fprintf(output, "RC1");
    fprintf(output, "\tCPA\tgr7,IBUFSIZE\n");
    fprintf(output, "\tJNZ\tRC2\n");
    fprintf(output, "\tLD\tgr5,NEWLINE\n");
    fprintf(output, "\tST\tgr5,0,gr1\n");
    fprintf(output, "\tST\tgr0,IBUFSIZE\n");
    fprintf(output, "\tST\tgr0,INP\n");
    fprintf(output, "\tJUMP\tRC3\n");
    fprintf(output, "RC2");
    fprintf(output, "\tLD\tgr5,IBUF,gr7\n");
    fprintf(output, "\tADDA\tgr7,ONE\n");
    fprintf(output, "\tST\tgr5,0,gr1\n");
    fprintf(output, "\tST\tgr7,INP\n");
    fprintf(output, "RC3");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "READINT");
    fprintf(output, "\tRPUSH\n");
    fprintf(output, "RI1");
    fprintf(output, "\tCALL\tREADCHAR\n");
    fprintf(output, "\tLD\tgr7,0,gr1\n");
    fprintf(output, "\tCPA\tgr7,SPACE\n");
    fprintf(output, "\tJZE\tRI1\n");
    fprintf(output, "\tCPA\tgr7,TAB\n");
    fprintf(output, "\tJZE\tRI1\n");
    fprintf(output, "\tCPA\tgr7,NEWLINE\n");
    fprintf(output, "\tJZE\tRI1\n");
    fprintf(output, "\tLD\tgr5,ONE\n");
    fprintf(output, "\tCPA\tgr7,MINUS\n");
    fprintf(output, "\tJNZ\tRI4\n");
    fprintf(output, "\tLD\tgr5,gr0\n");
    fprintf(output, "\tCALL\tREADCHAR\n");
    fprintf(output, "\tLD\tgr7,0,gr1\n");
    fprintf(output, "RI4");
    fprintf(output, "\tLD\tgr6,gr0\n");
    fprintf(output, "RI2");
    fprintf(output, "\tCPA\tgr7,ZERO\n");
    fprintf(output, "\tJMI\tRI3\n");
    fprintf(output, "\tCPA\tgr7,NINE\n");
    fprintf(output, "\tJPL\tRI3\n");
    fprintf(output, "\tMULA\tgr6,TEN\n");
    fprintf(output, "\tADDA\tgr6,gr7\n");
    fprintf(output, "\tSUBA\tgr6,ZERO\n");
    fprintf(output, "\tCALL\tREADCHAR\n");
    fprintf(output, "\tLD\tgr7,0,gr1\n");
    fprintf(output, "\tJUMP\tRI2\n");
    fprintf(output, "RI3");
    fprintf(output, "\tST\tgr7,RPBBUF\n");
    fprintf(output, "\tST\tgr6,0,gr1\n");
    fprintf(output, "\tCPA\tgr5,gr0\n");
    fprintf(output, "\tJNZ\tRI5\n");
    fprintf(output, "\tSUBA\tgr5,gr6\n");
    fprintf(output, "\tST\tgr5,0,gr1\n");
    fprintf(output, "RI5");
    fprintf(output, "\tRPOP\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "READLINE");
    fprintf(output, "\tST\tgr0,IBUFSIZE\n");
    fprintf(output, "\tST\tgr0,INP\n");
    fprintf(output, "\tST\tgr0,RPBBUF\n");
    fprintf(output, "\tRET\n");
    fprintf(output, "ONE\tDC\t1\n");
    fprintf(output, "SIX\tDC\t6\n");
    fprintf(output, "TEN\tDC\t10\n");
    fprintf(output, "SPACE\tDC\t#0020\n");
    fprintf(output, "MINUS\tDC\t#002D\n");
    fprintf(output, "TAB\tDC\t#0009\n");
    fprintf(output, "ZERO\tDC\t#0030\n");
    fprintf(output, "NINE\tDC\t#0039\n");
    fprintf(output, "NEWLINE\tDC\t#000A\n");
    fprintf(output, "INTBUF\tDS\t8\n");
    fprintf(output, "OBUFSIZE\tDC\t0\n");
    fprintf(output, "IBUFSIZE\tDC\t0\n");
    fprintf(output, "INP\tDC\t0\n");
    fprintf(output, "OBUF\tDS\t257\n");
    fprintf(output, "IBUF\tDS\t257\n");
    fprintf(output, "RPBBUF\tDC\t0\n");
}
