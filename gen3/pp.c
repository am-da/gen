#include "mppl_compiler.h"
#define true 1
#define false 0

static int exists_empty_statement = 0;
static int while_statement_level = 0;
int subprogram = 0;
int in_variable_declaration = 0;
int in_call_statement = 0;
int is_array_type = 0;
int formal_para = 0;
int definition_procedure_name = 0;
struct ID *id_procedure = NULL;

int parse_program(void) {
    if (token != TPROGRAM) {
        return error("Keyword 'program' is not found.");
    }
    token = scan();
    if (token != TNAME) {
        return error("Program name is not found.");
    }
    token = scan();
    if (token != TSEMI) return error("Semicolon is not found.");
    token = scan();

    if (block() == ERROR) return ERROR;

    if (token != TDOT) return error("Period is not found at the end of program.");
    token = scan();

    return NORMAL;
}

int block(void) {
    while (token == TVAR || token == TPROCEDURE) {
        if (token == TVAR) {
            if (variable_declaration() == ERROR) return ERROR;
        
        } else {
            if (subprogram_declaration() == ERROR) return ERROR;
        }
    }
    if (compound_statement() == ERROR) return ERROR;
    
    return NORMAL;
}

int variable_declaration(void) {
    int is_the_first_line = 1;
    if (token != TVAR) return error("Keyword 'var' is not found.");
    token = scan();
    in_variable_declaration = true;

    while (token == TNAME) {
        if (variable_names() == ERROR) return ERROR;
        
        if (token != TCOLON) return error("Colon is not found.");
        token = scan();

        if (parse_type() == ERROR) return ERROR;
        
        if (token != TSEMI) return error("Symbol ';' is not found.");
        token = scan();

        if (is_the_first_line == 1) {
            is_the_first_line = 0;
        }
    }
    in_variable_declaration = false;

    return NORMAL;
}

int variable_names(void) {
    if (token != TNAME) return error("Name is not found.");
    

    if (in_variable_declaration || formal_para) {
        if (id_register_without_type(string_attr) == ERROR) return ERROR;
    }
    else if (register_linenum(string_attr) == ERROR) return ERROR;
    
    token = scan();

    while (token == TCOMMA) {
        token = scan();

        if (token != TNAME) return error("Name is not found.");
        
        if (in_variable_declaration || formal_para) {
            if (id_register_without_type(string_attr) == ERROR) {
                return ERROR;
            }
        }
        else if (register_linenum(string_attr) == ERROR) return ERROR;
        
        token = scan();
    }
    return NORMAL;
}

int parse_type(void) {
    if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        if (
            standard_type() == ERROR) return ERROR;
        
    } else if (token == TARRAY) {
        if (formal_para) {
            return error("Array types cannot be defined in the formal parameter");
        }
        is_array_type = true;
        if (parse_array_type() == ERROR) return ERROR;
        
        is_array_type = false;
    } else {
        return error("Type is not found.");
    }
    return NORMAL;
}

int standard_type(void) {
    int standard_type = TPNONE;
    struct TYPE *type;
    if (token != TINTEGER && token != TBOOLEAN && token != TCHAR) {
        return error("Standard type is not found.");
    }

    if (in_variable_declaration || formal_para) {
        if (is_array_type) {
            switch (token) {
                case TINTEGER:
                    type = array_type(TPARRAYINT);
                    break;
                case TBOOLEAN:
                    type = array_type(TPARRAYBOOL);
                    break;
                case TCHAR:
                    type = array_type(TPARRAYCHAR);
                    break;
            }
        } else {
            switch (token) {
                case TINTEGER:
                    type = def_type(TPINT);
                    break;
                case TBOOLEAN:
                    type = def_type(TPBOOL);
                    break;
                case TCHAR:
                    type = def_type(TPCHAR);
                    break;
            }
        }
        if (div_type(&type) == ERROR) return ERROR;
    }

    switch (token) {
        case TINTEGER:
            standard_type = TPINT;
            break;
        case TBOOLEAN:
            standard_type = TPBOOL;
            break;
        case TCHAR:
            standard_type = TPCHAR;
            break;
    }
    token = scan();
    return standard_type;
}

int parse_array_type(void) {
    if (token != TARRAY) return error("Keyword 'array' is not found.");
    token = scan();

    if (token != TLSQPAREN) return error("Symbol '[' is not found.");
    token = scan();

    if (token != TNUMBER) return error("Number is not found.");
    
    if (in_variable_declaration) {
        if (num_attr < 1) {
            return error("The size of the array that can be defined is 1 <= 'array size'.");
        }
    }
    token = scan();
    if (token != TRSQPAREN) return error("Symbol ']' is not found.");
    token = scan();

    if (token != TOF) return error("Keyword 'of' is not found.");
    token = scan();

    if (standard_type() == ERROR) return ERROR;
    
    return NORMAL;
}

int subprogram_declaration(void) {
    if (token != TPROCEDURE) return error("Keyword 'procedure' is not found.");
    token = scan();

    subprogram = true;
    definition_procedure_name = true;

    if (procedure_name() == ERROR) return ERROR;
    
    definition_procedure_name = false;

    if (token == TLPAREN && formal_parameters() == ERROR) return ERROR;
    
    if (token != TSEMI) return error("Symbol ';' is not found.");
    token = scan();

    if (token == TVAR) {
        if (variable_declaration() == ERROR) return ERROR;
    }

    if (compound_statement() == ERROR) return ERROR;
    
    if (token != TSEMI) return error("Symbol ';' is not found.");
    token = scan();
    subprogram = false;
    release_local();

    return NORMAL;
}


int procedure_name(void) {
    if (token != TNAME) return error("Procedure name is not found.");
    
    if (definition_procedure_name) {
        struct TYPE *type;
        id_register_without_type(string_attr);
        type = def_type(TPPROC);
        if (div_type(&type) == ERROR) return ERROR;
        
        strncpy(current_procedure_name, string_attr, MAXSTRSIZE);
    }
    else {
        if (register_linenum(string_attr) == ERROR) return ERROR;
        id_procedure = search_procedure(string_attr);
    }
    token = scan();

    return NORMAL;
}

int formal_parameters(void) {
    if (token != TLPAREN) return error("Symbol '(' is not found.");
    
    token = scan();
    formal_para = true;

    if (variable_names() == ERROR) return ERROR;

    if (token != TCOLON) return error("Symbol ':' is not found.");
    token = scan();

    if (parse_type() == ERROR) return ERROR;

    while (token == TSEMI) {
        token = scan();

        if (variable_names() == ERROR) return ERROR;

        if (token != TCOLON) return error("Symbol ':' is not found.");
        token = scan();

        if (parse_type() == ERROR) return ERROR;
    }
    if (token != TRPAREN) return error("Sybmol ')' is not found.");
    token = scan();
    formal_para = false;

    return NORMAL;
}

int compound_statement(void) {
    if (token != TBEGIN) return error("Keyword 'begin' is not found.");
    token = scan();

    if (statement() == ERROR) return ERROR;
    while (token == TSEMI) {
        token = scan();

        if (statement() == ERROR) return ERROR;
    }

    if (token != TEND) return error("Keyword 'end' is not found.");
    
    if (exists_empty_statement) {
        exists_empty_statement = 0;
    }
    token = scan();

    return NORMAL;
}

int statement(void) {
    switch (token) {
        case TNAME:
            if (assignment_statement() == ERROR) return ERROR;
            break;
        case TIF:
            if (condition_statement() == ERROR) return ERROR;
            break;
        case TWHILE:
            if (iteration_statement() == ERROR) return ERROR;
            break;
        case TBREAK:
            if (while_statement_level == 0) {
                error("Keyword 'break' is written outside of a while statement.");
                return ERROR;
            }
            token = scan();
            break;
        case TCALL:
            if (call_statement() == ERROR) return ERROR;
            break;
        case TRETURN:
            token = scan();
            break;
        case TREAD:
        case TREADLN:
            if (input_statement() == ERROR) return ERROR;
            break;
        case TWRITE:
        case TWRITELN:
            if (output_statement() == ERROR) return ERROR;
            break;
        case TBEGIN:
            if (compound_statement() == ERROR) return ERROR;
            break;
        default:
            exists_empty_statement = 1;
            break;
    }
    return NORMAL;
}

int assignment_statement(void) {
    int var_type = TPNONE;
    int exp_type = TPNONE;
    if ((var_type = variable()) == ERROR) return ERROR;
    if (token != TASSIGN) return error("Symbol ':=' is not found.");
    token = scan();

    if ((exp_type = expression()) == ERROR) return ERROR;
    
    if (var_type != exp_type) return error("The types of the operand1 and operand2 do not match");

    return NORMAL;
}

int condition_statement(void) {
    int exp_type = TPNONE;
    if (token != TIF) return error("Keyword 'if' is not found.");
    token = scan();

    if ((exp_type = expression()) == ERROR) return ERROR;
    
    if (exp_type != TPBOOL) return error("The type of the condition must be boolean.");
    
    if (token != TTHEN) return error("Keyword 'then' is not found.");
    token = scan();

    if (statement() == ERROR) return ERROR;
    
    if (token == TELSE) {
        token = scan();

        if (token != TIF) {
            if (statement() == ERROR) return ERROR;
            
        } else {
            if (statement() == ERROR) return ERROR;
            
        }
    }
    return NORMAL;
}

int iteration_statement(void) {
    int exp_type = TPNONE;
    if (token != TWHILE) return error("Keyword 'while' is not found.");
    
    while_statement_level++;
    token = scan();

    if ((exp_type = expression()) == ERROR) return ERROR;
    
    if (exp_type != TPBOOL) return error("The type of the condition must be boolean.");

    if (token != TDO) return error("Keyword 'do' is not found.");
    token = scan();

    if (statement() == ERROR) return ERROR;
    
    while_statement_level--;
    return NORMAL;
}

int call_statement(void) {
    if (token != TCALL) return error("Keyword 'call' is not found.");
    token = scan();

    in_call_statement = true;
    if (procedure_name() == ERROR) return ERROR;
    
    if (token == TLPAREN) {
        token = scan();

        if (expressions() == ERROR) return ERROR;
        
        if (token != TRPAREN) return error("Symbol ')' is not found.");
        token = scan();
    } else {
        struct TYPE *para_type = id_procedure->itp->paratp;
        if (para_type != NULL) return error("There are a few arguments.");
    }
    in_call_statement = false;

    return NORMAL;
}

int expressions(void) {
    int exp_type = TPNONE;
    int num_of_exp = 0;
    struct TYPE *para_type = id_procedure->itp->paratp;
    if ((exp_type = expression()) == ERROR) return ERROR;
    
    num_of_exp++;

    if (in_call_statement) {
        if (para_type == NULL) return error("This procedure takes no arguments.");
        
        if (para_type->ttype != exp_type) return error("The type of the argument1 does not match.");
    }
    while (token == TCOMMA) {
        token = scan();

        if ((exp_type = expression()) == ERROR) return ERROR;
        
        num_of_exp++;
        if (in_call_statement) {
            para_type = para_type->paratp;
            if (para_type == NULL) return error("There are a lot of arguments.");
            
            if (para_type->ttype != exp_type) return error("The type of the argument does not match.");
        }
    }
    if (in_call_statement) {
        if (para_type->paratp != NULL) return error("There are a few arguments.");
    }
    return NORMAL;
}

int variable(void) {
    int id_type = TPNONE;
    if (token != TNAME) return error("Name is not found.");
    
    if ((id_type = register_linenum(string_attr)) == ERROR) return ERROR;
    token = scan();

    if (token == TLSQPAREN) {
        int exp_type = TPNONE;
        if (!(id_type & TPARRAY)) {
            fprintf(stderr, "%s is not Array type.", string_attr);
            return error("id is not Array type.");
        }
        token = scan();

        if ((exp_type = expression()) == ERROR) {
            return ERROR;
        } else if (exp_type != TPINT) {
            return error("The array index type must be an integer.");
        }

        if (token != TRSQPAREN) return error("Sybmol ']' is not found.");
        token = scan();
        switch (id_type) {
            case TPARRAYINT:
                id_type = TPINT;
                break;
            case TPARRAYCHAR:
                id_type = TPCHAR;
                break;
            case TPARRAYBOOL:
                id_type = TPBOOL;
                break;
        }
    }
    return id_type;
}

int input_statement(void) {
    if (token != TREAD && token != TREADLN) return error("Keyword 'read' or 'readln' is not found.");
    token = scan();

    if (token == TLPAREN) {
        int var_type = TPNONE;
        token = scan();

        if ((var_type = variable()) == ERROR) return ERROR;
        
        if (var_type != TPINT && var_type != TPCHAR) return error("The type of the variable must be integer or char.");
        
        while (token == TCOMMA) {
            token = scan();

            if ((var_type = variable()) == ERROR) return ERROR;
            
            if (var_type != TPINT && var_type != TPCHAR) return error("The type of the variable must be integer or char.");
        }
        if (token != TRPAREN) return error("Sybmol ')' is not found.");
        token = scan();
    }
    return NORMAL;
}

int output_statement(void) {
    if (token != TWRITE && token != TWRITELN) return error("Keyword 'write' or 'writeln' is not found.");
    token = scan();

    if (token == TLPAREN) {
        token = scan();

        if (output_format() == ERROR) return ERROR;
        
        while (token == TCOMMA) {
            token = scan();

            if (output_format() == ERROR) return ERROR;
        }
        if (token != TRPAREN) return error("Symbol ')' is not found.");
        token = scan();
    }
    return NORMAL;
}

int output_format(void) {
    int exp_type = TPNONE;
    if (token == TSTRING && strlen(string_attr) > 1) {
        token = scan();
        return NORMAL;
    }

    switch (token) {
        case TPLUS:
        case TMINUS:
        case TNAME:
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
        case TLPAREN:
        case TNOT:
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if ((exp_type = expression()) == ERROR) return ERROR;
            
            if (exp_type & TPARRAY) return error("The type must be a standard type.");
            
            if (token == TCOLON) {
                token = scan();

                if (token != TNUMBER) return error("Number is not found.");
                token = scan();
            }
            break;
        default:
            return error("Output format is not found.");
    }
    return NORMAL;
}

int expression(void) {
    int exp_type1 = TPNONE;

    if ((exp_type1 = simple_expression()) == ERROR) return ERROR;
    
    while (is_relational_operator(token)) {
        int exp_type2 = TPNONE;
        token = scan();

        if ((exp_type2 = simple_expression()) == ERROR) return ERROR;
        
        if (exp_type1 != exp_type2) return error("The types of the operand1 and operand2 do not match");
        exp_type1 = TPBOOL;
    }
    return exp_type1;
}

int is_relational_operator(int _token) {
    switch (_token) {
        case TEQUAL:
        case TNOTEQ:
        case TLE:
        case TLEEQ:
        case TGR:
        case TGREQ:
            return 1;
        default:
            return 0;
    }
}

int simple_expression(void) {
    int term_type1 = TPNONE;
    int term_type2 = TPNONE;
    if (token == TPLUS || token == TMINUS) {
        term_type1 = TPINT;
        token = scan();
    }

    if ((term_type2 = term()) == ERROR) return ERROR;
    
    if (term_type1 == TPINT && term_type2 != TPINT) return error("The type of the term must be integer.");
    term_type1 = term_type2;

    while (token == TPLUS || token == TMINUS || token == TOR) {
        if ((token == TPLUS || token == TMINUS) && term_type1 != TPINT) {
            return error("The type of the operand must be integer.");
        } else if (token == TOR && term_type1 != TPBOOL) {
            return error("The type of the operand must be boolean.");
        }
        token = scan();
        if ((term_type2 = term()) == ERROR) return ERROR;
        
        if (term_type1 == TPINT && term_type2 != TPINT) {
            return error("The type of the operand must be integer.");
        } else if (term_type1 == TPBOOL && term_type2 != TPBOOL) {
            return error("The type of the operand must be boolean.");
        }
    }
    return term_type1;
}

int term(void) {
    int term_type1 = TPNONE;
    int term_type2 = TPNONE;

    if ((term_type1 = factor()) == ERROR) return ERROR;

    while (token == TSTAR || token == TDIV || token == TAND) {
        if ((token == TSTAR || token == TDIV) && term_type1 != TPINT) {
            return error("The type of the operand must be integer.");
        } else if (token == TAND && term_type1 != TPBOOL) {
            return error("The type of the operand must be boolean.");
        }
        token = scan();

        if ((term_type2 = factor()) == ERROR) return ERROR;
        
        if (term_type1 == TPINT && term_type2 != TPINT) {
            return error("The type of the operand must be integer.");
        } else if (term_type1 == TPBOOL && term_type2 != TPBOOL) {
            return error("The type of the operand must be boolean.");
        }
    }
    return term_type1;
}

int factor(void) {
    int factor_type = TPNONE;
    int exp_type = TPNONE;
    switch (token) {
        case TNAME:
            if ((factor_type = variable()) == ERROR) return ERROR;
            break;
        case TNUMBER:
        case TFALSE:
        case TTRUE:
        case TSTRING:
            if ((factor_type = constant()) == ERROR) return ERROR;
            break;
        case TLPAREN:
            token = scan();

            if ((factor_type = expression()) == ERROR) return ERROR;
            if (token != TRPAREN) return error("Symbol ')' is not found.");
            token = scan();
            break;
        case TNOT:
            token = scan();

            if ((factor_type = factor()) == ERROR) return ERROR;
            
            if (factor_type != TPBOOL) return error("The type of the operand must be boolean.");
            break;
        case TINTEGER:
        case TBOOLEAN:
        case TCHAR:
            if ((factor_type = standard_type()) == ERROR) return ERROR;
            if (token != TLPAREN) return error("Symbol '(' is not found");
            token = scan();

            if ((exp_type = expression()) == ERROR) return ERROR;
            
            if (exp_type & TPARRAY) return error("The type must be a standard type.");
            
            if (token != TRPAREN) return error("Symbol ')' is not found.");
            token = scan();
            break;
        default:
            return error("Factor is not found.");
    }
    return factor_type;
}

int constant(void) {
    int constant_type = NORMAL;

    switch (token) {
        case TNUMBER:
            constant_type = TPINT;
            break;
        case TFALSE:
        case TTRUE:
            constant_type = TPBOOL;
            break;
        case TSTRING:
            if (strlen(string_attr) != 1) return error("Constant string length != 1");
            constant_type = TPCHAR;
            break;
        default:
            return error("Constant is not found.");
    }
    token = scan();
    return constant_type;
}
