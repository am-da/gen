#include "mppl_compiler.h"

char *tokenstr[NUMOFTOKEN+1] = {
    "",
    "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
     "else", "procedure", "return", "call", "while", "do", "not", "or",
    "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
     "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
    ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read","write", "break"
};

static int is_global = GLOBAL_PARA;
static char *label_exit;
static int op_para = 0;
static int vd_para  = 0;
static int pro_ex = 0;
static int subpro = NOPARA;
static int left_para = 0;
static struct ID *ref_id = NULL;
static int Type_look(int TYPE);
static int em_para = 0;
static int repet = 0;
static int token = 0;

int Parse_program(){
    char *st_label = NULL;

    init_global_idtab();
    token = scan();
    if(token != TPROGRAM) return error("Keyword error 'program'");
    token = scan();

    if(token != TNAME) return error("not found program name");
    if(cross_proc(NULL, get_linenum()) == ERROR) return ERROR;
    if(out_ass(string_attr, &st_label) == ERROR) return ERROR;
    token = scan();
    
    if(token != TSEMI) return error("not found semicolon ");
    token = scan();

    if(block(st_label) == ERROR) return ERROR;
 
    if(token != TDOT) return error("not found period");
    token = scan();

    printstr();
    Utility_functions();
    fprintf(output, "\tEND\n");
    release_global_idtab();

    return NORMAL;
}

int block(char *st_label){
    while(token == TVAR || token == TPROCEDURE){
        if(token == TVAR){
            is_global = GLOBAL_PARA;
            if(variable_declaration() == ERROR) return ERROR;
        }else if(token == TPROCEDURE){
            is_global = LOCAL_PARA;
            if(subprogram_declaration() == ERROR) return ERROR;
        }else return error("var or procedure is not found");
        
    }
    fprintf(output, "%s", st_label);
    fprintf(output, "\tDS\t0\n");

    is_global = GLOBAL_PARA;
    if(compound_statement() == ERROR) return ERROR;
    fprintf(output, "\tRET\n");

    return NORMAL;
}

int variable_declaration(){
    if(token != TVAR) return error("Keyword error 'var' ");
    token = scan();
    
    if(variable_names() == ERROR) return ERROR;

    if(token != TCOLON) return error("Symbol error ':' ");
    token = scan();

    if(type() == ERROR) return ERROR;

    if(de_para(subpro, is_global) == ERROR) return ERROR;
  
    if (token != TSEMI) return error("Symbol error ';' ");
    token = scan();

    while(token == TNAME){
        if(variable_names() == ERROR) return ERROR;

        if(token != TCOLON) return error("Symbol error ':' ");
        token = scan();
        
        if(type() == ERROR) return ERROR;

        if(de_para(subpro, is_global) == ERROR) return ERROR;

        if (token != TSEMI) return error("Symbol error ';' ");
        token = scan();
    }
    return NORMAL;
}

int variable_names(){
    if(variable_name() == ERROR) return ERROR;
    while(token == TCOMMA){
        token = scan();
        if(variable_name() == ERROR) return ERROR;
    }
    return NORMAL;
}

int variable_name(){

    if(token != TNAME) error("not found name");
    if(cross_name(string_attr) == ERROR) return ERROR;
    if(cross_linenum(get_linenum()) == ERROR) return ERROR;
    token = scan();
    
    return NORMAL;
}

int type(){
    int TYPE = ERROR;
    if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        if((TYPE = standard_type()) == ERROR) return ERROR;
    }else if(token == TARRAY){
        if((TYPE = array_type()) == ERROR) return ERROR;
    }else{
        return error("not found type");
    }
    return TYPE;
}

int standard_type(){
    int TYPE = ERROR;
    if(token == TINTEGER){
        TYPE = TPINT;
    }else if(token == TBOOLEAN){
        TYPE = TPBOOL;
    }else if(token == TCHAR){
        TYPE = TPCHAR;
    }else{
        return error("not found Standard type " );
    }
    
    if(cross_type(TYPE, 0, NULL, NULL) == ERROR) return ERROR;
    token = scan();
    return TYPE;
}

int array_type(){
    int TYPE = TPARRAY;
    struct TYPE *TYPE_e = NULL;
    int SIZEofarray = 0;

    if(token != TARRAY) return error("Keyword error 'array' " );
    token = scan();
    if(token != TLSQPAREN) return error(" Symbol error '[' ");
    token = scan();
    if(token != TNUMBER) return error("not found number");
    if((SIZEofarray = num_attr) == 0) return error("array error");
    token = scan();
    if(token != TRSQPAREN) return error("Symbol error ']'");
    token = scan();
    if(token != TOF) return error("Keyword error 'of'");
    token = scan();

    if(standard_type() == ERROR) return ERROR;
    if((TYPE_e = judge_type()) == NULL) return error("array error");

    if(TYPE_e->ttype == TPINT) TYPE_e->ttype = TPINT;
    else if(TYPE_e->ttype == TPCHAR) TYPE_e->ttype = TPCHAR;
    else if(TYPE_e->ttype == TPBOOL) TYPE_e->ttype = TPBOOL;

    if(cross_type(TYPE, SIZEofarray, TYPE_e, NULL) == ERROR) return ERROR;
    return TYPE;
}

int subprogram_declaration(){
    struct PARA *param = NULL;
    subpro = EXPARA;
    init_local_idtab();
    
    if(token != TPROCEDURE) return error("Keyword error 'procedure'");
    token = scan();
    if (variable_name() == ERROR) return ERROR;
    if(token == TLPAREN && (formal_parameters() == ERROR)) return ERROR;

    if (token != TSEMI) return error("Symbol error ';' ");
    token = scan();
    param = propara(pre_proname());
    if((token == TVAR) && (variable_declaration() == 1)) return ERROR;
    
    cross_name(pre_proname());
    cross_linenum(pre_number());
    cross_type(TPPROC, 0, NULL, param);
    de_para(NOPARA, GLOBAL_PARA);
    para_stack(param);

    if(compound_statement() == ERROR) return ERROR;

    if (token != TSEMI) return error("Symbol error ';' ");
    token = scan();
    relocate_local_idtab();
    fprintf(output, "\tRET\n");
    subpro = NOPARA;

    return NORMAL;
}

int formal_parameters(void){
    int TYPE = ERROR;
    vd_para = 1;

    if(token != TLPAREN) return error("Symbol error '(' ");
    token = scan();
    if(variable_names() == ERROR) return ERROR;

    if(token != TCOLON) return error("Symbol error ':'");
    token = scan();
    if((TYPE = type()) == ERROR) return ERROR;
    if(Type_look(TYPE) == ERROR) return error("type error");
    
    if(de_para(subpro, is_global) == ERROR) return ERROR;

    while(token == TSEMI){
        token = scan();
        if(variable_names() == ERROR) return ERROR;
        if(token != TCOLON) return error("Symbol error ':' ");
        token = scan();

        if((TYPE = type()) == ERROR) return ERROR;
        if(Type_look(TYPE) == ERROR) return error("type error");

        if(de_para(subpro, is_global) == ERROR) return ERROR;
    }

    if(token != TRPAREN) return error("() is not found");
    token = scan();
    vd_para  = 0;

    return NORMAL;

}

int compound_statement(){
    if(token != TBEGIN) return error("Keyword error 'begin' ");
    token = scan();
    if (statement() == ERROR) return ERROR;
    
    while(token == TSEMI){
        token = scan();
        if(statement() == ERROR) return ERROR;
    }

    if(token != TEND) return error("Keyword error 'end' ");
    if(em_para == 1) em_para = 0;
    token = scan();

    return NORMAL;
}

int statement(){
    if (token == TNAME){
        if (assignment_statement() == ERROR)  return ERROR;
    }
    else if (token == TIF){
        if (condition_statement() == ERROR)  return ERROR;
    }
    else if (token == TWHILE){
        if (iteration_statement() == ERROR) return ERROR;
    }
    else if (token == TBREAK){
        if (exit_statement() == ERROR) return ERROR;
    }
    else if (token == TCALL){
        if (call_statement() == ERROR) return ERROR;
    }
    else if (token == TRETURN){
        if(return_statement() == ERROR) return ERROR;
    }
    else if (token == TREAD || token == TREADLN){
        if (input_statement() == ERROR)  return ERROR;
    }
    else if (token == TWRITE || token == TWRITELN){
        if (output_statement() == ERROR)  return ERROR;
    }
    else if (token == TBEGIN){
        if (compound_statement() == ERROR) return ERROR;
    }
    else{
        empty_statement(); return NORMAL;
    }
    return NORMAL;
}


int condition_statement(){
    int TYPE = ERROR;
    char *label1, *label2;

    if(create_label(&label1) == ERROR) return ERROR;
    if(create_label(&label2) == ERROR) return ERROR;
    if(token != TIF) return error("Keyword error 'if' ");
    token = scan();
    if((TYPE = expression()) == ERROR) return ERROR;
    if(TYPE != TPBOOL) return error("error 'BOOL'");

    fprintf(output, "\tCPA\tgr1,gr0\n");
    fprintf(output, "\tJZE\t%s\n", label1);

    if(token != TTHEN) return error("'then' is not found");
    token = scan();
    if(statement() == ERROR) return ERROR;
    
    if(token == TELSE){
        fprintf(output, "\tJUMP\t%s\n", label2);
        fprintf(output, "%s\tDS\t0\n", label1);
        token = scan();
        if(statement() == ERROR) return ERROR;
        fprintf(output, "%s", label2);
    }else{
        fprintf(output, "%s\tDS\t0\n", label1);
    }
    return NORMAL;
}

int iteration_statement(){

    int TYPE = ERROR;
    char *label = NULL, *label_te = NULL, *label_ju = NULL;

    if(token != TWHILE) return error("Keyword error  'while' ");
    token = scan();
    if(create_label(&label) == ERROR) return ERROR;
    if(create_label(&label_ju) == ERROR) return ERROR;
    label_te = label_exit;
    fprintf(output, "%s\tDS\t0\n", label);

    if((TYPE = expression()) == ERROR) return ERROR;
    if(TYPE != TPBOOL) return error("Error bool");

    if(token != TDO) return error("Keyword error 'do'");
    token = scan();

    fprintf(output, "\tCPA\tgr1,gr0\n");
    fprintf(output, "\tJZE\t%s\n", label_ju);
    label_exit = label_ju;

    repet++;
    if(statement() == ERROR) return ERROR;
    repet--;
    
    label_exit = label_te;
    fprintf(output, "\tJUMP\t%s\n", label);
    fprintf(output, "%s\tDS\t0\n", label_ju);

    return NORMAL;
}

int exit_statement(){
    if(token != TBREAK) return error("Keyword error 'break'");
    if(repet > 0){
        token = scan();
    }else{
        return error("Keyword error'break' ");
    }
    fprintf(output, "\tJUMP\t%s\n", label_exit);
    return NORMAL;
}

int call_statement(){

    struct ID *fparams;
    int line;

    if(token != TCALL) return error("Keyword error 'call' ");
    token = scan();

    line = get_linenum();
    
    if((fparams = search_global_idtab(pre_proname())) == NULL){
        return error("%s is not found", pre_proname());
    }
    if(is_global == LOCAL_PARA && strcmp(pre_proname(), fparams->name) == 0){
        return error("proc error");
    }
    if((ref_para(pre_proname(), pre_proname(), line, 0, is_global)) == ERROR) return ERROR;

    if(token == TLPAREN){
        token = scan();
        pro_ex = 1;
        if(expressions(fparams->itp->paratp) == ERROR) return ERROR;
        pro_ex = 0;
        if(token != TRPAREN) return error("Symbol error ')'");
        token = scan();
    }
    fprintf(output, "\tCALL\t$%s\n", pre_proname());
    return NORMAL;
}

int expressions(struct TYPE *fparams){

    int TYPE = ERROR;
    struct TYPE *tfparams = fparams;
    int opr = token;
    op_para = 0;

    if((TYPE = expression()) == ERROR) return ERROR;
    if(tfparams == NULL) return error("number error");
    if(TYPE != tfparams->ttype) return error("type error");

    char *label = NULL;
    if(opr == TNAME && op_para == 0){
        fprintf(output, "\tPUSH\t0,gr1\n");
    }else{
        if(create_label(&label) == ERROR) return ERROR;
        fprintf(output, "\tLAD\tgr2,%s\n", label); //0125
        fprintf(output, "\tST\tgr1,0,gr2\n");
        fprintf(output, "\tPUSH\t0,gr2\n");
        add_label(label, "0");
    }
    
    while(token == TCOMMA){
        token = scan();
        opr = token;
        tfparams = tfparams->paratp;
        op_para = 0;

        if((TYPE = expression()) == ERROR) return ERROR;
        if(tfparams == NULL) return error("number error");
        if(TYPE != tfparams->ttype) return error("type error");

        char *label = NULL;
        if(opr == TNAME && op_para == 0){
            fprintf(output, "\tPUSH\t0,gr1\n");
        }else{
            if(create_label(&label) == ERROR) return ERROR;
            fprintf(output, "\tLAD\tgr2,%s\n", label); //0125
            fprintf(output, "\tST\tgr1,0,gr2\n");
            fprintf(output, "\tPUSH\t0,gr2\n");
            add_label(label, "0");
        }
        return NORMAL;
    }

    return NORMAL;
}

int return_statement(){
    if(token != TRETURN) return error("'return' is not found");
    token = scan();
    
    /* write 'RET' in output */
    fprintf(output, "\tRET\n");

    return NORMAL;
}

int assignment_statement(){
    
    int TYPE_left_part = ERROR;
    int TYPE_expression = ERROR;

    if((TYPE_left_part = left_part()) == ERROR) return ERROR;
    if(Type_look(TYPE_left_part) == ERROR) return error("type error");

    if(token != TASSIGN) return error("Symbol error ':=' ");
    token = scan();
    if(subpro == EXPARA && ref_id->ispara == EXPARA){
        fprintf(output, "\tPUSH\t0,gr1\n");
    }
    if((TYPE_expression = expression()) == ERROR) return ERROR;
    if(Type_look(TYPE_expression) == ERROR) return error("type error");
    
    if(TYPE_left_part != TYPE_expression) return error("type error");

    if(ref_id->ispara == EXPARA && subpro == 1){
        fprintf(output, "\tPOP\tgr2\n");
        fprintf(output, "\tST\tgr1,0,gr2\n");
    }else{
        if(ref_id->itp->ttype == TPARRAY){
            fprintf(output, "\tPOP\tgr2\n");
        }
        fprintf(output, "\tST\tgr1,$%s", ref_id->name);
        if(ref_id->procname != NULL){
            fprintf(output, "%%%s",ref_id->procname);
        }
        if(ref_id->itp->ttype == TPARRAY){
            fprintf(output, ",gr2");
        }
        fprintf(output, "\n");
    }
    
    return NORMAL;
}

int left_part(){
    int TYPE = ERROR;
    left_para = 1;
    if((TYPE = variable()) == ERROR) return ERROR;
    left_para = 0;

    return TYPE;
}

int variable(void){
    int TYPE = ERROR;
    int TYPE_statement = ERROR;
    int point_to_array = -1;
    struct ID *p;
    if(is_global == LOCAL_PARA){
        if((p = search_local_idtab((char *)string_attr, pre_proname())) == NULL){
            if((p = search_global_idtab((char *)string_attr)) == NULL){
                return error("%s is not found", string_attr);
            }
        }
    }else{
        if((p = search_global_idtab((char *)string_attr)) == NULL) return error("%s is not found", string_attr);
    }
    if(left_para) ref_id = p;
    TYPE = p->itp->ttype;

    if(variable_name() == ERROR) return ERROR;

    if(token == TLSQPAREN){
        if(TYPE != TPARRAY) return error("type error");
        if(p->itp->etp == NULL) return error("array error");
        TYPE = p->itp->etp->ttype;

        token = scan();

        if(token == TNUMBER) point_to_array = num_attr;

        if((TYPE_statement = expression()) == ERROR) return ERROR;
        if(TYPE_statement != TPINT) return error("type error");

        if(token != TRSQPAREN) return error("Symbol error ']' ");
        token = scan();
    }

    if(ref_para(p->name, pre_proname(), get_linenum(), point_to_array, is_global) == ERROR) return ERROR;
    
    char *label = NULL;
    if(point_to_array >= 0){
        if(create_label(&label) == ERROR) return ERROR;

        fprintf(output, "CPA\tgr2,gr0\n\tJMI\tEROV\n\tLAD\tgr1,%d\n", p->itp->arraysize);
        fprintf(output, "\tCPA\tgr2,gr1\n\tJMI\t%s\n", label);
        fprintf(output, "\tJUMP\tEROV\n%s\n", label);
        fprintf(output, "\tPOP\tgr2\n\tADDA\tgr1,gr2\n");
        return NORMAL;
    }

    if(p->ispara == EXPARA) fprintf(output, "\tLD\tgr1,  \t$%s%%%s\n", p->name, p->procname);
    
   
    return TYPE;
}

int expression(){

    int TYPE = ERROR;
    int TYPE_operator = ERROR;
    int TYPE_operand = ERROR;
    int opr = ERROR;

    if((TYPE = simple_expression()) == ERROR) return ERROR;

    TYPE_operator = TYPE;

    while(token == TEQUAL || token == TNOTEQ || token == TLE ||
        token == TLEEQ || token == TGR || token == TGREQ ){
            
            opr = token;
        op_para = 1;
            fprintf(output, "\tPUSH\t0,gr1\n");

            if((TYPE = relational_operator()) == ERROR) return ERROR;
            if((TYPE_operand = simple_expression()) == ERROR) return ERROR;
            if(TYPE_operator != TYPE_operand) return error("type error");
            TYPE_operator = TYPE_operand;
        compare_en(opr);
    }

    return TYPE;
}

int simple_expression(){
    int TYPE = ERROR;
    int TYPE_operator = ERROR;
    int TYPE_operand = ERROR;
    
    int is_plus_or_minus = 0;
    int is_minus = 0;
    int opr = ERROR;

    if(token == TPLUS){
        is_plus_or_minus = 1;
        op_para = 1;
        token = scan();
    }else if(token == TMINUS){
        is_plus_or_minus = 1;
        is_minus = 1;
        op_para = 1;
        token = scan();
    }
    
    if((TYPE = term()) == ERROR) return ERROR;
    if(is_plus_or_minus == 1 && TYPE != TINTEGER) return error("type error");
    TYPE_operator = TYPE;

    if(is_minus == 1) {
        fprintf(output, "\tLAD\tgr2,-1\n\tMULA\tgr1,gr2\n\tJOV\tEOVF\n");
    }

    while(token == TPLUS || token == TMINUS || token == TOR){
        opr = token;
        op_para = 1;
        fprintf(output, "\tPUSH\t0,gr1\n");

        if((TYPE = additive_operator()) == ERROR) return ERROR;
        if((TYPE_operand = term()) == ERROR) return ERROR;

        fprintf(output, "\tPOP\tgr2\n");
        if (opr == TPLUS){
            fprintf(output, "\tADDA\tgr1,gr2\n\tJOV\tEOVF\n");
        }
        else if(opr == TMINUS){
            fprintf(output, "\tSUBA\tgr2,gr1\n\tJOV\tEOVF\n\tLD\tgr1,gr2\n");
        }
        else if(opr == TOR){
                fprintf(output, "\tOR\tgr1,gr2\n");
        }

        if(TYPE_operator != TYPE_operand) return error("type error");
        TYPE_operator = TYPE_operand;
    }
    return TYPE;
}

int term(){
    int TYPE = ERROR;
    int TYPE_operator = ERROR;
    int TYPE_operand = ERROR;
    int opr = ERROR;

    if((TYPE = factor()) == ERROR) return ERROR;
    TYPE_operator = TYPE;

    while(token == TSTAR || token == TDIV || token == TAND){
        opr = token;
        op_para = 1;
        fprintf(output, "\tPUSH\t0,gr1\n");

        if((TYPE = multiplicative_operator()) == ERROR) return ERROR;
        if((TYPE_operand = factor()) == ERROR) return ERROR;

        fprintf(output, "\tPOP\tgr2\n");
        if (opr == TSTAR){
            fprintf(output, "\tMULA\tgr1,gr2\n\tJOV\tEOVF\n");
        }
        else if (opr == TAND){
            fprintf(output, "\tAND\tgr1,gr2\n");
        }
        else if (opr == TDIV){
            fprintf(output, "\tDIVA\tgr2,gr1\n\tJOV\tE0DIV\n\tLD\tgr1,gr2\n");
        }
        if(TYPE_operator != TYPE_operand) return error("type error in term");
        TYPE_operator = TYPE_operand;
    }
    return TYPE;
}

int factor(){
    int TYPE_expression = ERROR;
    int TYPE = ERROR;

    if(token == TNAME){
        if((TYPE = variable()) == ERROR) return ERROR;
        if(!(!op_para && (vd_para  || pro_ex) && (token == TCOMMA || token == TRPAREN))){
            fprintf(output, "\tLD\tgr1,\t$%s\n",pre_proname2()); 
        }
    }
            
    else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        if((TYPE = constant()) == ERROR) return ERROR;
    }

    else if (token == TLPAREN){
        op_para = 1;
        
        token = scan();
        if((TYPE = expression()) == ERROR) return ERROR;
        if(token != TRPAREN) return error("() error )");
        
        token = scan();
    }

    else if (token == TNOT){
        op_para = 1;
        token = scan();
        if((TYPE = factor()) == ERROR) return ERROR;
        if(TYPE != TPBOOL) return error("type error in factor");
        fprintf(output, "\tLAD\tgr2,1\n\tXOR\tgr1,gr2\n");
    }
    else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        op_para = 1;
        if((TYPE = standard_type()) == ERROR) return ERROR;
        if(token != TLPAREN) return error("( error in factor");
        token = scan();
        
        if((TYPE_expression = expression()) == ERROR) return ERROR;
        if(token != TRPAREN) return error(") error in factor");
        if(Type_look(TYPE_expression) == ERROR) return error("type error in factor");
        
        if(convert_siki(TYPE, TYPE_expression) == ERROR) return ERROR;
        
        token = scan();
    }
    else{
            return error("factor error");
    }
    return TYPE;
}

int constant(void){
    int TYPE = ERROR;

    if(token == TNUMBER){
        fprintf(output, "\tLAD\tgr1,%d\n", num_attr);
        TYPE = TPINT;
    }else if(token == TFALSE){
        fprintf(output, "\tLD\tgr1,gr0\n");
        TYPE = TPBOOL;
    }else if(token == TTRUE){
        fprintf(output, "\tLAD\tgr1,1\n");
        TYPE = TPBOOL;
    }else if(token == TSTRING){
        if(strlen(string_attr) == 1){
            fprintf(output, "\tLAD\tgr1,%d\n", string_attr[0]);
            TYPE = TPCHAR;
        }else{
            return error("length error in constant");
        }
    }else{
        return error("input error in constant");
    }
    token = scan();

    return TYPE;
}

int multiplicative_operator(){
    int TYPE = ERROR;

    if(token == TSTAR || token == TDIV) TYPE = TPINT;
    else if(token == TAND) TYPE = TPBOOL;
    else return error("not found multiplicative operator");
    token = scan();
    return TYPE;
}

int additive_operator(){
    int TYPE = ERROR;

    if(token == TPLUS || token == TMINUS) TYPE = TPINT;
    else if(token == TOR) TYPE = TPBOOL;
    else return error("not found additive operator");
    token = scan();
    
    return TYPE;
}

int relational_operator(){
    if(!( token == TEQUAL || token == TNOTEQ || token == TLE   ||
          token == TLEEQ  || token == TGR    || token == TGREQ )){
            return error("not found　relational operator");
    }
    token = scan();

    return TPBOOL;
}

int input_statement(){
    int TYPE = ERROR;
    int is_readln = 0;

    if(token == TREADLN) is_readln = 1;
    if(token == TREAD || token == TREADLN){
        token = scan();
    }
    else return error("Keyword error in input_statement");

    if(token == TLPAREN){
        token = scan();

        if((TYPE = variable()) == ERROR) return ERROR;
        if(TYPE != TPINT && TYPE != TPCHAR){
            return error("type error in input_statement");
        }
        switch(TYPE){
            case TPINT:
                fprintf(output, "\tCALL\tREADINT\n");
                break;
            case TPCHAR:
                fprintf(output, "\tCALL\tREADCHAR\n");
                break;
            default :
                break;
        }

        while(token == TCOMMA){
            token = scan();
            if((TYPE = variable()) == ERROR) return ERROR;
            if(TYPE != TPINT && TYPE != TPCHAR){
                return error("type error in input_statement");
            }
            switch(TYPE){
                case TPINT:
                    fprintf(output, "\tCALL\tREADINT\n");
                    break;
                case TPCHAR:
                    fprintf(output, "\tCALL\tREADCHAR\n");
                    break;
                default :
                    break;
            }
        }

        if(token != TRPAREN) return error("Symbol error ) ");
        token = scan();
    }
    if(is_readln) fprintf(output, "\tCALL\tREADLINE\n");
    return NORMAL;
}

int output_statement(){
    int is_writeln = 0;
    if(token == TWRITELN) is_writeln = 1;
    if(token == TWRITE || token == TWRITELN){
        token = scan();
    }
    else return error("type error in output_statement");

    if(token == TLPAREN){
        token = scan();
        if(output_format() == ERROR) return ERROR;
        
        while(token == TCOMMA){
            token = scan();
            if(output_format() == ERROR) return ERROR;
        }

        if(token != TRPAREN) return error("Symbol error ) ");
        token = scan();
    }
    if(is_writeln) fprintf(output, "\tCALL\tWRITELINE\n");
    return NORMAL;
}

int output_format(){
    int TYPE = ERROR;

    char *i = " ";
    if(token == TSTRING && strlen(string_attr) > 1){
        para_string(i);
        token = scan();
        return NORMAL;
    }

    switch(token){
        case TSTRING:
        case TPLUS:
        case TMINUS:
        case TNAME:
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TLPAREN:
        case TNOT:
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if((TYPE = expression()) == ERROR) return ERROR;
            if(Type_look(TYPE) == ERROR) return error("type error in output_format");

            if(token == TCOLON){
                token = scan();
        
                if(token != TNUMBER) return error("not found　Number");
                para_ass(TYPE, num_attr);
                token = scan();
            }else{
                para_ass(TYPE, 0);
            }
            break;
        default:
            return error("not found Output format");
            break;
    }
    return NORMAL;
}
int empty_statement(){
    em_para = 1;
    return NORMAL;
}

static int Type_look(int TYPE){
    if(TYPE == TPINT || TYPE == TPBOOL || TYPE == TPCHAR ) return NORMAL;
    else return ERROR;
}
