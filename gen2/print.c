#include "print.h"

int empty_statement = 0;
int iteration = 0;
int token = 0;
int indent = 0;

char *tokenstr[MAXTOKEN + 1] = {
        "",
        "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
        "else", "procedure", "return", "call", "while", "do", "not", "or",
        "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
        "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
        ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read", "write", "break"
};

int parse_program(FILE *fp) {
    if (token != TPROGRAM) { return (error("Keyword error 'program'")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TNAME) { return (error("not found program name")); }
    printf("%s ", string_attr);
    token = scan(fp);

    if (token != TSEMI) { return (error("not found semicolon ")); }
    printf("%s\n", tokenstr[token]);
    token = scan(fp);

    if (block(fp) == ERROR) { return ERROR; }

    if (token != TDOT) { return (error("not found period ")); }
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int block(FILE *fp) {
    while ((token == TVAR) || (token == TPROCEDURE)) {
        if (token ==  TVAR){
            indent++;
            indent_do();
            if (variable_declaration(fp) == ERROR)  return ERROR;
            indent--;
        }
        else if(token ==TPROCEDURE){
            indent++;
            indent_do();
            if (subprogram_declaration(fp) == ERROR)  return ERROR;
            indent--;
        }
        else{
            break;
        }
    }
    if (compound_statement(fp) == ERROR)  return ERROR;

    return NORMAL;
}

int variable_declaration(FILE *fp) {
    if (token != TVAR)  return (error("Keyword error 'var' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (variable_names(fp) == ERROR)  return ERROR;

    if (token != TCOLON) { return (error("Symbol error ':' ")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (type(fp) == ERROR) { return ERROR; }

    if (token != TSEMI)  return (error("Symbol error ';' "));
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    while (token == TNAME) {
        indent++;
        indent_do();

        if (variable_names(fp) == ERROR) return ERROR;

        if (token != TCOLON) return (error("Symbol error ':' "));
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (type(fp) == ERROR)  return ERROR;

        if (token != TSEMI)  return (error("Symbol error ';' "));
        printf("\b%s\n", tokenstr[token]);
        token = scan(fp);
        indent--;
    }
    return NORMAL;
}

int variable_names(FILE *fp) {
    if (variable_name(fp) == ERROR) return ERROR;

    while (token == TCOMMA) {
        printf("\b%s ", tokenstr[token]);
        token = scan(fp);

        if (variable_name(fp) == ERROR)  return ERROR;
    }
    return NORMAL;
}

int variable_name(FILE *fp) {
    if (token != TNAME) return (error("not found name"));
    printf("%s ", string_attr);
    token = scan(fp);

    return NORMAL;
}

int type(FILE *fp) {
    if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
        if (standard_type(fp) == ERROR)  return ERROR;
    }
    else if(token == TARRAY){
        if (array_type(fp) == ERROR)  return ERROR;
    }
    else{
            return (error("not found Type"));
    }
    return NORMAL;
}

int standard_type(FILE *fp) {
    if (token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("not found Standard type "));
    }
    return NORMAL;
}

int array_type(FILE *fp) {
    if (token != TARRAY) return (error("Keyword error 'array' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TLSQPAREN) return (error(" Symbol error '[' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TNUMBER)  return (error("not found number"));
    printf("%s ", string_attr);
    token = scan(fp);

    if (token != TRSQPAREN)  return (error("Symbol error ']'"));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (token != TOF) return (error("Keyword error 'of'"));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (standard_type(fp) == ERROR)  return ERROR;

    return NORMAL;
}

int subprogram_declaration(FILE *fp) {
    if (token != TPROCEDURE)  return (error("Keyword error 'procedure'"));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (variable_name(fp) == ERROR) return ERROR;

    if (token == TLPAREN) {
        if (formal_parameters(fp) == ERROR)  return ERROR;
    }

    if (token != TSEMI)  return (error("Symbol error ';' "));
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    if (token == TVAR) {
        indent_do();
        if (variable_declaration(fp) == ERROR) return ERROR;
    }
    indent_do();
    if (compound_statement(fp) == ERROR) return ERROR;

    if (token != TSEMI)  return (error("Symbol error ';' "));
    printf("\b%s\n", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int formal_parameters(FILE *fp) {
    if (token != TLPAREN) { return (error("Symbol error '(' ")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (variable_names(fp) == ERROR) { return ERROR; }

    if (token != TCOLON) { return (error("Symbol error ':'")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (type(fp) == ERROR) { return ERROR; }

    while (token == TSEMI) {
        printf("\b%s\t", tokenstr[token]);
        token = scan(fp);

        if (variable_names(fp) == ERROR)  return ERROR;

        if (token != TCOLON) { return (error("Symbol error ':' ")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (type(fp) == ERROR) return ERROR;
    }

    if (token != TRPAREN) { return (error("Symbol error ')' ")); }
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int compound_statement(FILE *fp) {
    if (token != TBEGIN)  return (error("Keyword error 'begin' "));
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    indent++;
    indent_do();
    if (statement(fp) == ERROR)  return ERROR;

    while (token == TSEMI) {
        printf("\b%s\n", tokenstr[token]); //;
        token = scan(fp);
        if (token != TEND){
            indent_do();
        }
        if (statement(fp) == ERROR) { return ERROR; }
    }

    if (token != TEND) return (error("Keyword error 'end' "));
    indent--;
    if (empty_statement == 1) {
        empty_statement = 0;
    } else {
        printf("\n");
    }
    indent_do();
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    return NORMAL;
}

int statement(FILE *fp) {
    if (token == TNAME){
        if (assignment_statement(fp) == ERROR)  return ERROR;
    }
    else if (token == TIF){
        if (condition_statement(fp) == ERROR)  return ERROR;
    }
    else if (token == TWHILE){
        if (iteration_statement(fp) == ERROR) return ERROR;
    }
    else if (token == TBREAK){
        if (exit_statement(fp) == ERROR) return ERROR;
    }
    else if (token == TCALL){
        if (call_statement(fp) == ERROR) return ERROR;
    }
    else if (token == TRETURN){
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        return NORMAL;
    }
    else if (token == TREAD || token == TREADLN){
        if (input_statement(fp) == ERROR)  return ERROR;
    }
    else if (token == TWRITE || token == TWRITELN){
        if (output_statement(fp) == ERROR)  return ERROR;
    }
    else if (token == TBEGIN){
        if (compound_statement(fp) == ERROR) return ERROR;
    }
    else{
        empty_statement = 1;
    }
    return NORMAL;
}

int condition_statement(FILE *fp) {
    if (token != TIF) return (error("Keyword error 'if' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (expression(fp) == ERROR)  return ERROR;

    if (token != TTHEN) return (error("Keyword error 'then' "));
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    indent++;
    indent_do();
    if (statement(fp) == ERROR) return ERROR;
    indent--;

    if (token == TELSE) {
        if (empty_statement == 1) {
            empty_statement = 0;
            printf("\r");
        } else {
            printf("\n");
        }
        indent_do();
        printf("%s\n", tokenstr[token]);
        token = scan(fp);
        indent++;
        indent_do();
        if (statement(fp) == ERROR)  return ERROR;
        indent--;
    }
    return NORMAL;
}

int iteration_statement(FILE *fp) {
    if (token != TWHILE) return (error("Keyword error  'while' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (expression(fp) == ERROR) return ERROR;

    if (token != TDO) return (error("Keyword error 'do'"));
    printf("%s\n", tokenstr[token]);
    token = scan(fp);
    iteration++;
    indent_do();
    if (statement(fp) == ERROR)  return ERROR;
    iteration--;

    return NORMAL;
}

int exit_statement(FILE *fp) {
    if (token != TBREAK)  return (error("Keyword error 'break'"));
    if (iteration > 0) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    } else {
        return error("Keyword error'break' ");
    }
    return NORMAL;
}

int call_statement(FILE *fp) {
    if (token != TCALL) return (error("Keyword error 'call' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (variable_name(fp) == ERROR)  return ERROR;

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (expressions(fp) == ERROR) return ERROR;

        if (token != TRPAREN) return (error("Symbol error ')'"));
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    return NORMAL;
}

int expressions(FILE *fp) {
    if (expression(fp) == ERROR)  return ERROR;

    while (token == TCOMMA) {
        if(token != TCOMMA) return(error("not found commma "));
        printf("\b%s ", tokenstr[token]);
        token = scan(fp);

        if (expression(fp) == ERROR)  return ERROR;
    }
    return NORMAL;
}

int assignment_statement(FILE *fp) {
    if (variable(fp) == ERROR)  return ERROR;

    if (token != TASSIGN) return (error("Symbol error ':=' "));
    printf("%s ", tokenstr[token]);
    token = scan(fp);

    if (expression(fp) == ERROR)  return ERROR;

    return NORMAL;
}

int variable(FILE *fp) {
    if (variable_name(fp) == ERROR)  return ERROR;

    if (token == TLSQPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (expression(fp) == ERROR)  return ERROR;

        if (token != TRSQPAREN) {
            return (error("Symbol error ']' "));
        }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    return NORMAL;
}

int expression(FILE *fp) {
    if (simple_expression(fp) == ERROR) return ERROR;

    while ((token == TEQUAL) || (token == TNOTEQ) || (token == TLE) || (token == TLEEQ) || (token == TGR) ||
           (token == TGREQ)) {
        if (relational_operator(fp) == ERROR)  return ERROR;

        if (simple_expression(fp) == ERROR)  return ERROR;
    }
    return NORMAL;
}

int simple_expression(FILE *fp) {
    if (token == TPLUS || token == TMINUS){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    if (term(fp) == ERROR) return ERROR;

    while ((token == TPLUS) || (token == TMINUS) || (token == TOR)) {
        if (additive_operator(fp) == ERROR) return ERROR;

        if (term(fp) == ERROR) return ERROR;
    }
    return NORMAL;
}

int term(FILE *fp) {
    if (factor(fp) == ERROR)  return ERROR;

    while (token == TSTAR || token == TDIV || token == TAND) {
        if (multiplicative_operator(fp) == ERROR)  return ERROR;

        if (factor(fp) == ERROR)  return ERROR;
    }
    return NORMAL;
}

int factor(FILE *fp) {
    if (token == TNAME){
        if (variable(fp) == ERROR) return ERROR;
    }
    else if (token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING){
        if (constant(fp) == ERROR) return ERROR;
    }
    else if(token == TLPAREN){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
        
        if (expression(fp) == ERROR) return ERROR;
        
        if (token != TRPAREN) {
            return (error("Symbol ')' "));
        }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else if (token == TNOT){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
        
        if (factor(fp) == ERROR) return ERROR;
    }
    else if (token == TINTEGER || token == TBOOLEAN || token == TCHAR){
        if (standard_type(fp) == ERROR)  return ERROR;
        
        if (token != TLPAREN) {
            return (error("Symbol error '(' "));
        }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
        
        if (expression(fp) == ERROR) return ERROR;
        
        if (token != TRPAREN) {
            return (error("Symbol error ')' "));
        }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("not found factor"));
    }
    return NORMAL;
}

int constant(FILE *fp) {
    if (token == TNUMBER){
        printf("%s ", string_attr);
        token = scan(fp);
    }
    else if(token == TFALSE || token == TTRUE){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else if (token == TSTRING){
        printf("'%s' ", string_attr);
        token = scan(fp);
    }
    else{
        return (error("not found Constant"));
    }
    return NORMAL;
}

int multiplicative_operator(FILE *fp) {
    if (token == TSTAR || token == TDIV || token == TAND){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("not found multiplicative operator"));
    }
    return NORMAL;
}

int additive_operator(FILE *fp) {
    if (token == TPLUS || token == TMINUS || token == TOR){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("not found additive operator"));
    }
    return NORMAL;
}
//
int relational_operator(FILE *fp) {
    if (token ==TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR|| token == TGREQ){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("not found　relational operator "));
    }
    return NORMAL;
}

int input_statement(FILE *fp) {
    if (token == TREAD || token == TREADLN){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("Keyword error not found　'read', 'readln'"));
    }

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);

        if (variable(fp) == ERROR) return ERROR;

        while (token == TCOMMA) {
            printf("%s ", tokenstr[token]);
            token = scan(fp);

            if (variable(fp) == ERROR) return ERROR;
        }
        if (token != TRPAREN) { return (error("Symbol error ) ")); }
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    return NORMAL;
}

int output_statement(FILE *fp) {
    if (token == TWRITE || token == TWRITELN){
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    else{
        return (error("Keyword error not found 'write', 'writeln'"));
    }

    if (token == TLPAREN) {
        printf("%s ", tokenstr[token]);
        token = scan(fp);
        if (output_format(fp) == ERROR) return ERROR;

        while (token == TCOMMA) {
            printf("%s ", tokenstr[token]);
            token = scan(fp);
            if (output_format(fp) == ERROR) return ERROR;
        }
        if (token != TRPAREN) return (error("Symbol error ) "));
        printf("%s ", tokenstr[token]);
        token = scan(fp);
    }
    return NORMAL;
}

int output_format(FILE *fp) {
    switch (token) {
        case TSTRING:
            if (strlen(string_attr) > 1) {
                printf("'%s' ", string_attr);
                token = scan(fp);
                break;
            }
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
            if (expression(fp) == ERROR) return ERROR;

            if (token == TCOLON) {
                printf("%s ", tokenstr[token]);
                token = scan(fp);

                if (token != TNUMBER) { return (error("not found　Number")); }
                printf("%s ", string_attr);
                token = scan(fp);
            }
            break;
        default:
            return (error("not found　Output format"));
    }
    return NORMAL;
}

void indent_do(void) {
    int k = 0;
    for (k = 0; k < indent; k++) {
        printf("    ");
    }
}
