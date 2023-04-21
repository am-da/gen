#include "mppl_compiler.h"

char *tokenstr[NUMOFTOKEN+1] = {
    "",
    "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
     "else", "procedure", "return", "call", "while", "do", "not", "or",
    "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
     "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
    ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read","write", "break"
};

static int is_global = GLOBAL_PARAM;
static char *label_exit;
static int is_opr = 0;
static int vd_para  = 0;
static int is_callp = 0;
static int is_subproc = NOT_FORMAL_PARAM;
static int is_left_val = 0;
static struct ID *ref_id = NULL;
static int Check_Standard_Type(int TYPE);
static int em_para = 0;
static int is_iterate = 0;
static int token = 0;

int Parse_program(){
    char *st_label = NULL;

    init_global_idtab();
    token = scan();
    if(token != TPROGRAM) return error("Keyword error 'program'");
    token = scan();

    if(token != TNAME) return error("not found program name");
    if(memorize_proc(NULL, get_linenum()) == ERROR) return ERROR;
    if(inst_start(string_attr, &st_label) == ERROR) return ERROR;
    token = scan();
    
    if(token != TSEMI) return error("not found semicolon ");
    token = scan();

    if(block(st_label) == ERROR) return ERROR;
 
    if(token != TDOT) return error("not found period");
    token = scan();

    print_strlabel();
    Utility_functions();
    fprintf(output, "\tEND\n");
    release_global_idtab();

    return NORMAL;
}

int block(char *st_label){
    while(token == TVAR || token == TPROCEDURE){
        if(token == TVAR){
            is_global = GLOBAL_PARAM;
            if(variable_declaration() == ERROR) return ERROR;
        }else if(token == TPROCEDURE){
            is_global = LOCAL_PARAM;
            if(subprogram_declaration() == ERROR) return ERROR;
        }else return error("var or procedure is not found");
        
    }
    fprintf(output, "%s", st_label);
    fprintf(output, "\tDS\t0\n");

    is_global = GLOBAL_PARAM;
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

    if(define_identifer(is_subproc, is_global) == ERROR) return ERROR;
  
    if (token != TSEMI) return error("Symbol error ';' ");
    token = scan();

    while(token == TNAME){
        if(variable_names() == ERROR) return ERROR;

        if(token != TCOLON) return error("Symbol error ':' ");
        token = scan();
        
        if(type() == ERROR) return ERROR;

        if(define_identifer(is_subproc, is_global) == ERROR) return ERROR;

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
    if(memorize_name(string_attr) == ERROR) return ERROR;
    if(memorize_linenum(get_linenum()) == ERROR) return ERROR;
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
    
    if(memorize_type(TYPE, 0, NULL, NULL) == ERROR) return ERROR;
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
    if((TYPE_e = get_etp_type_structure()) == NULL) return error("array error");

    if(TYPE_e->ttype == TPINT) TYPE_e->ttype = TPINT;
    else if(TYPE_e->ttype == TPCHAR) TYPE_e->ttype = TPCHAR;
    else if(TYPE_e->ttype == TPBOOL) TYPE_e->ttype = TPBOOL;

    if(memorize_type(TYPE, SIZEofarray, TYPE_e, NULL) == ERROR) return ERROR;
    return TYPE;
}

int subprogram_declaration(){
    struct PARAM *param = NULL;
    is_subproc = FORMAL_PARAM;
    init_local_idtab();
    
    if(token != TPROCEDURE) return error("Keyword error 'procedure'");
    token = scan();
    if (variable_name() == ERROR) return ERROR;
    if(token == TLPAREN && (formal_parameters() == ERROR)) return ERROR;

    if (token != TSEMI) return error("Symbol error ';' ");
    token = scan();
    param = get_paratp(get_prev_procname());
    if((token == TVAR) && (variable_declaration() == 1)) return ERROR;
    
    memorize_name(get_prev_procname());
    memorize_linenum(get_prev_procline());
    memorize_type(TPPROC, 0, NULL, param);
    define_identifer(NOT_FORMAL_PARAM, GLOBAL_PARAM);
    inst_procedule_params(param);

    if(compound_statement() == ERROR) return ERROR;

    if (token != TSEMI) return error("Symbol error ';' ");
    token = scan();
    relocate_local_idtab();
    fprintf(output, "\tRET\n");
    is_subproc = NOT_FORMAL_PARAM;

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
    if(Check_Standard_Type(TYPE) == ERROR) return error("type error");
    
    if(define_identifer(is_subproc, is_global) == ERROR) return ERROR;

    while(token == TSEMI){
        token = scan();
        if(variable_names() == ERROR) return ERROR;
        if(token != TCOLON) return error("Symbol error ':' ");
        token = scan();

        if((TYPE = type()) == ERROR) return ERROR;
        if(Check_Standard_Type(TYPE) == ERROR) return error("type error");

        if(define_identifer(is_subproc, is_global) == ERROR) return ERROR;
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

    is_iterate++;
    if(statement() == ERROR) return ERROR;
    is_iterate--;
    
    label_exit = label_te;
    fprintf(output, "\tJUMP\t%s\n", label);
    fprintf(output, "%s\tDS\t0\n", label_ju);

    return NORMAL;
}

int exit_statement(){
    if(token != TBREAK) return error("Keyword error 'break'");
    if(is_iterate > 0){
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
    
    if((fparams = search_global_idtab(get_prev_procname())) == NULL){
        return error("%s is not found", get_prev_procname());
    }
    if(is_global == LOCAL_PARAM && strcmp(get_prev_procname(), fparams->name) == 0){
        return error("proc error");
    }
    if((reference_identifer(get_prev_procname(), get_prev_procname(), line, 0, is_global)) == ERROR) return ERROR;

    if(token == TLPAREN){
        token = scan();
        is_callp = 1;
        if(expressions(fparams->itp->paratp) == ERROR) return ERROR;
        is_callp = 0;
        if(token != TRPAREN) return error("Symbol error ')'");
        token = scan();
    }
    fprintf(output, "\tCALL\t$%s\n", get_prev_procname());
    return NORMAL;
}

int expressions(struct TYPE *fparams){

    int TYPE = ERROR;
    struct TYPE *tfparams = fparams;
    int opr = token;
    is_opr = 0;

    if((TYPE = expression()) == ERROR) return ERROR;
    if(tfparams == NULL) return error("number error");
    if(TYPE != tfparams->ttype) return error("type error");

    char *label = NULL;
    if(opr == TNAME && is_opr == 0){
        fprintf(output, "\tPUSH\t0,gr1\n");
    }else{
        if(create_label(&label) == ERROR) return ERROR;
        fprintf(output, "\tLAD\tgr2,%s\n", label); //0125
        fprintf(output, "\tST\tgr1,0,gr2\n");
        fprintf(output, "\tPUSH\t0,gr2\n");
        register_strlabel(label, "0");
    }
    
    while(token == TCOMMA){
        token = scan();
        opr = token;
        tfparams = tfparams->paratp;
        is_opr = 0;

        if((TYPE = expression()) == ERROR) return ERROR;
        if(tfparams == NULL) return error("number error");
        if(TYPE != tfparams->ttype) return error("type error");

        char *label = NULL;
        if(opr == TNAME && is_opr == 0){
            fprintf(output, "\tPUSH\t0,gr1\n");
        }else{
            if(create_label(&label) == ERROR) return ERROR;
            fprintf(output, "\tLAD\tgr2,%s\n", label); //0125
            fprintf(output, "\tST\tgr1,0,gr2\n");
            fprintf(output, "\tPUSH\t0,gr2\n");
            register_strlabel(label, "0");
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
    if(Check_Standard_Type(TYPE_left_part) == ERROR) return error("type error");

    if(token != TASSIGN) return error("Symbol error ':=' ");
    token = scan();
    if(is_subproc == FORMAL_PARAM && ref_id->ispara == FORMAL_PARAM){
        fprintf(output, "\tPUSH\t0,gr1\n");
    }
    if((TYPE_expression = expression()) == ERROR) return ERROR;
    if(Check_Standard_Type(TYPE_expression) == ERROR) return error("type error");
    
    if(TYPE_left_part != TYPE_expression) return error("type error");

    if(ref_id->ispara == FORMAL_PARAM && is_subproc == 1){
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
    is_left_val = 1;
    if((TYPE = variable()) == ERROR) return ERROR;
    is_left_val = 0;

    return TYPE;
}

int variable(void){
    int TYPE = ERROR;
    int TYPE_statement = ERROR;
    int point_to_array = -1;
    struct ID *p;
    if(is_global == LOCAL_PARAM){
        if((p = search_local_idtab((char *)string_attr, get_prev_procname())) == NULL){
            if((p = search_global_idtab((char *)string_attr)) == NULL){
                return error("%s is not found", string_attr);
            }
        }
    }else{
        if((p = search_global_idtab((char *)string_attr)) == NULL) return error("%s is not found", string_attr);
    }
    if(is_left_val) ref_id = p;
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

    if(reference_identifer(p->name, get_prev_procname(), get_linenum(), point_to_array, is_global) == ERROR) return ERROR;
    
    char *label = NULL;
    if(point_to_array >= 0){
        if(create_label(&label) == ERROR) return ERROR;

        fprintf(output, "CPA\tgr2,gr0\n");
        fprintf(output, "\tJMI\tEROV\n");
        fprintf(output, "\tLAD\tgr1,%d\n", p->itp->arraysize);
        fprintf(output, "\tCPA\tgr2,gr1\n");
        fprintf(output, "\tJMI\t%s\n", label);
        fprintf(output, "\tJUMP\tEROV\n");
        fprintf(output, "%s\n", label);
        fprintf(output, "\tPOP\tgr2\n");
        fprintf(output, "\tADDA\tgr1,gr2\n");
        return NORMAL;
    }

    if(p->ispara == FORMAL_PARAM) fprintf(output, "\tLD\tgr1,  \t$%s%%%s\n", p->name, p->procname);
    
   
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
            is_opr = 1;
            fprintf(output, "\tPUSH\t0,gr1\n");

            if((TYPE = relational_operator()) == ERROR) return ERROR;
            if((TYPE_operand = simple_expression()) == ERROR) return ERROR;
            if(TYPE_operator != TYPE_operand) return error("type error");
            TYPE_operator = TYPE_operand;
            inst_expression(opr);
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
        is_opr = 1;
        token = scan();
    }else if(token == TMINUS){
        is_plus_or_minus = 1;
        is_minus = 1;
        is_opr = 1;
        token = scan();
    }
    
    if((TYPE = term()) == ERROR) return ERROR;
    if(is_plus_or_minus == 1 && TYPE != TINTEGER) return error("type error");
    TYPE_operator = TYPE;

    if(is_minus == 1) {
        fprintf(output, "\tLAD\tgr2,-1\n");
        fprintf(output, "\tMULA\tgr1,gr2\n");
        fprintf(output, "\tJOV\tEOVF\n");
    }

    while(token == TPLUS || token == TMINUS || token == TOR){
        opr = token;
        is_opr = 1;
        fprintf(output, "\tPUSH\t0,gr1\n");

        if((TYPE = additive_operator()) == ERROR) return ERROR;
        if((TYPE_operand = term()) == ERROR) return ERROR;

        fprintf(output, "\tPOP\tgr2\n");
        if (opr == TPLUS){
            fprintf(output, "\tADDA\tgr1,gr2\n");
            fprintf(output, "\tJOV\tEOVF\n");
        }
        else if(opr == TMINUS){
            fprintf(output, "\tSUBA\tgr2,gr1\n");
            fprintf(output, "\tJOV\tEOVF\n");
            fprintf(output, "\tLD\tgr1,gr2\n");
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
        is_opr = 1;
        fprintf(output, "\tPUSH\t0,gr1\n");

        if((TYPE = multiplicative_operator()) == ERROR) return ERROR;
        if((TYPE_operand = factor()) == ERROR) return ERROR;

        fprintf(output, "\tPOP\tgr2\n");
        if (opr == TSTAR){
            fprintf(output, "\tMULA\tgr1,gr2\n");
            fprintf(output, "\tJOV\tEOVF\n");
        }
        else if (opr == TAND){
            fprintf(output, "\tAND\tgr1,gr2\n");
        }
        else if (opr == TDIV){
            fprintf(output, "\tDIVA\tgr2,gr1\n");
            fprintf(output, "\tJOV\tE0DIV\n");
            fprintf(output, "\tLD\tgr1,gr2\n");
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
        if(!(!is_opr && (vd_para  || is_callp) && (token == TCOMMA || token == TRPAREN))){
            fprintf(output, "\tLD\tgr1,\t$%s\n",get_prev_procname2()); //0127
        }
    }
            
    else if(token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        if((TYPE = constant()) == ERROR) return ERROR;
    }

    else if (token == TLPAREN){
        is_opr = 1;
        
        token = scan();
        if((TYPE = expression()) == ERROR) return ERROR;
        if(token != TRPAREN) return error("() error )");
        
        token = scan();
    }

    else if (token == TNOT){
        is_opr = 1;
        token = scan();
        if((TYPE = factor()) == ERROR) return ERROR;
        if(TYPE != TPBOOL) return error("type error in factor");
        fprintf(output, "\tLAD\tgr2,1\n");
        fprintf(output, "\tXOR\tgr1,gr2\n");
    }
    else if(token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        is_opr = 1;
        if((TYPE = standard_type()) == ERROR) return ERROR;
        if(token != TLPAREN) return error("( error in factor");
        token = scan();
        
        if((TYPE_expression = expression()) == ERROR) return ERROR;
        if(token != TRPAREN) return error(") error in factor");
        if(Check_Standard_Type(TYPE_expression) == ERROR) return error("type error in factor");
        
        if(inst_factor_char(TYPE, TYPE_expression) == ERROR) return ERROR;
        
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
        inst_write_string(i);
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
            if(Check_Standard_Type(TYPE) == ERROR) return error("type error in output_format");

            if(token == TCOLON){
                token = scan();
        
                if(token != TNUMBER) return error("not found　Number");
                inst_write_value(TYPE, num_attr);
                token = scan();
            }else{
                inst_write_value(TYPE, 0);
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

static int Check_Standard_Type(int TYPE){
    if(TYPE == TPINT || TYPE == TPBOOL || TYPE == TPCHAR ) return NORMAL;
    else return ERROR;
}
