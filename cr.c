#include "mppl_compiler.h"
#define NAME_SIZE 30
#define TYPE_SIZE 30
#define INDENT_SIZE 5

static struct ID *search_tab(struct ID **root, char *name, char *procname);
static int id_register_to_tab(struct ID **root, char *name, char *procname, struct TYPE **type, int ispara, int deflinenum);
static int add_type_to_parameter_list(struct ID **root, char *procname, struct TYPE **type);
static void free_strcut_ID(struct ID **root);
struct ID *globalidroot, *localidroot, *allroot, *id_without_type_root;
char current_procedure_name[MAXSTRSIZE];

void init_cr() {
    globalidroot = NULL;
    localidroot = NULL;
    allroot = NULL;
    id_without_type_root = NULL;
    return;
}

int id_register_without_type(char *name) {
    if (subprogram) return id_register_to_tab(&id_without_type_root, name, current_procedure_name, NULL, formal_para, get_linenum());
    else return id_register_to_tab(&id_without_type_root, name, NULL, NULL, formal_para, get_linenum());
}

int div_type(struct TYPE **type) {
    int register_g, register_lg;
    struct ID *p;
    if (type == NULL) return error("struct type is NULL\n");
    
    for (p = id_without_type_root; p != NULL; p = p->nextp) {
        char *name = p->name;
        char *current_procedure_name = p->procname;
        int ispara = p->ispara;
        int deflinenum = p->deflinenum;
        
        if (definition_procedure_name) {
            register_g = id_register_to_tab(&globalidroot, name, NULL, type, ispara, deflinenum);
            register_lg = id_register_to_tab(&allroot, name, NULL, type, ispara, deflinenum);
        } else if (subprogram) {
            register_g = id_register_to_tab(&localidroot, name, current_procedure_name, type, ispara, deflinenum);
            register_lg = id_register_to_tab(&allroot, name, current_procedure_name, type, ispara, deflinenum);
        } else {
            register_g = id_register_to_tab(&globalidroot, name, NULL, type, ispara, deflinenum);
            register_lg = id_register_to_tab(&allroot, name, NULL, type, ispara, deflinenum);
        }
        if (register_g == ERROR || register_lg == ERROR) return ERROR;
        
        if (formal_para) {
            add_type_to_parameter_list(&globalidroot, current_procedure_name, type);
            add_type_to_parameter_list(&allroot, current_procedure_name, type);
        }
    }
    free_strcut_ID(&id_without_type_root);
    free(*type);
    type = NULL;
    return 0;
}

struct TYPE *def_type(int type) {
    struct TYPE *p_type;
    if ((p_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("malloc error in def_type\n");
        return NULL;
    }
    p_type->ttype = type;
    p_type->arraysize = 0;
    p_type->etp = NULL;
    p_type->paratp = NULL;
    return p_type;
}

struct TYPE *array_type(int type) {
    struct TYPE *p_type;
    struct TYPE *p_etp;
    if ((p_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("malloc error in array_type\n");
        return (NULL);
    }
    p_type->ttype = type;
    p_type->arraysize = num_attr;

    if ((p_etp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
        error("array malloc error in array_type\n");
        return (NULL);
    }
    
    if (type == TPARRAYINT) p_etp->ttype = TPINT;
    else if(type == TPARRAYCHAR) p_etp->ttype = TPCHAR;
    else if(type == TPARRAYBOOL) p_etp->ttype = TPBOOL;
    else{
        fprintf(stderr, "[%d] is not array\n", type);
        error("array type error.\n");
        return NULL;
    }
    
    p_etp->arraysize = 0;
    p_etp->etp = NULL;
    p_etp->paratp = NULL;
    p_type->etp = p_etp;
    p_type->paratp = NULL;
    return p_type;
}

int register_linenum(char *name) {
    struct ID *id;
    struct ID *allid;
    struct LINE *line;
    struct LINE *allline;
    struct LINE *line_end;
    struct LINE *allline_end;
    int id_type;

    //subprogram exists
    if (subprogram) {
        char *procname = current_procedure_name;
        if ((id = search_tab(&localidroot, name, procname)) == NULL) {
            if ((id = search_tab(&globalidroot, name, NULL)) == NULL) return error("undefined name error.");
            else allid = search_tab(&allroot, name, NULL);
            
        } else {
            allid = search_tab(&allroot, name, procname);
        }
        
        if (strcmp(name, procname) == 0 && id->itp->ttype == TPPROC) return error(" recursively called error.");
        
    } else {
        if ((id = search_tab(&globalidroot, name, NULL)) == NULL) return error("undefined name error.");
       else allid = search_tab(&allroot, name, NULL);
        
    }
    id_type = id->itp->ttype;

    if ((line = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) return error("line malloc error\n");
    
    line->reflinenum = get_linenum();
    line->nextlinep = NULL;
    line_end = id->irefp;
    if (line_end == NULL) {
        id->irefp = line;
    } else {
        while (line_end->nextlinep != NULL) {
            line_end = line_end->nextlinep;
        }
        line_end->nextlinep = line;
    }

    if ((allline = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) return error("all line malloc error\n");
    
    allline->reflinenum = get_linenum();
    allline->nextlinep = NULL;
    allline_end = allid->irefp;
    if (allline_end == NULL) {
        allid->irefp = allline;
    } else {
        while (allline_end->nextlinep != NULL) {
            allline_end = allline_end->nextlinep;
        }
        allline_end->nextlinep = allline;
    }
    return id_type;
}

struct ID *search_procedure(char *procname) {
    struct ID *p;
    p = search_tab(&globalidroot, procname, NULL);
    return p;
}

void print_cr(struct ID *root) {
    struct ID *p;
    struct LINE *q;

    fprintf(stdout, "%-*s", NAME_SIZE, "Name");
    fprintf(stdout, "%-*s", TYPE_SIZE, "Type");
    fprintf(stdout, "Def.  /  Ref.\n");
    for (p = root; p != NULL; p = p->nextp) {
        if (p->procname != NULL) {
            char name_procname[NAME_SIZE];
            sprintf(name_procname, "%s:%s", p->name, p->procname);
            fprintf(stdout, "%-*s", NAME_SIZE, name_procname);
        } else fprintf(stdout, "%-*s", NAME_SIZE, p->name);
        
        if (p->itp->ttype == TPPROC) {
            struct TYPE *arg;
            int add_space = TYPE_SIZE;
            add_space -= strlen(typestr[p->itp->ttype]) + 1;
            fprintf(stdout, "%s", typestr[p->itp->ttype]);

            if (p->itp->paratp!= NULL) fprintf(stdout, "(");
            
            for (arg = p->itp->paratp; arg != NULL; arg = arg->paratp) {
                add_space -= strlen(typestr[arg->ttype]);
                
                fprintf(stdout, "%s", typestr[arg->ttype]);
                if (arg->paratp != NULL) {
                    add_space -= 1;
                    fprintf(stdout, ",");
                }
                else fprintf(stdout, ")");
            }
            
            add_space -= 1;
            fprintf(stdout, "%*s", 0 < add_space ? add_space : 0, "  ");
        } else if (p->itp->ttype & TPARRAY) {
            struct TYPE *p_type = p->itp;
            char arraytype[TYPE_SIZE];
            sprintf(arraytype, "array[%d] of %s", p_type->arraysize, typestr[p_type->ttype]);
            fprintf(stdout, "%-*s", TYPE_SIZE, arraytype);
        } else {
            fprintf(stdout, "%-*s", TYPE_SIZE, typestr[p->itp->ttype]);
        }
        
        fprintf(stdout, "%*d", INDENT_SIZE, p->deflinenum);
        fprintf(stdout, "  / ");
        for (q = p->irefp; q != NULL; q = q->nextlinep) {
            fprintf(stdout, "%d", q->reflinenum);
            fprintf(stdout, "%s", q->nextlinep == NULL ? "" : ",");
        }
        fprintf(stdout, "\n");
    }
    return;
}

static struct ID *search_tab(struct ID **root, char *name, char *procname) {
    struct ID *p;

    for (p = *root; p != NULL; p = p->nextp) {
        if (strcmp(name, p->name) == 0) {
            if (procname == NULL && p->procname == NULL) {
                return (p);
            }
            else if (procname != NULL && p->procname != NULL && strcmp(procname, p->procname) == 0) {
                return (p);
            }
        }
    }
    return NULL;
}

static int add_type_to_parameter_list(struct ID **root, char *procname, struct TYPE **type) {
    struct ID *p_id;
    struct TYPE *p_paratp;
    if ((p_id = search_tab(root, procname, NULL)) == NULL) {
        return error("procedure name is not found.");
    }
    p_paratp = p_id->itp;
    while (p_paratp->paratp != NULL) {
        p_paratp = p_paratp->paratp;
    }
    if ((p_paratp->paratp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) return  error("parameter error\n");;
    
    p_paratp->paratp->ttype = (*type)->ttype;
    p_paratp->paratp->arraysize = (*type)->arraysize;

    if ((*type)->ttype & TPARRAY) {
        struct TYPE *p_etype;
        if ((p_etype = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) return error("array parameter error\n");
        
        p_etype->ttype = (*type)->ttype;
        p_etype->arraysize = (*type)->arraysize;
        p_etype->etp = NULL;
        p_etype->paratp = NULL;
        p_paratp->paratp->etp = p_etype;
    } else {
        p_paratp->paratp->etp = NULL;
    }
    p_paratp->paratp->paratp = NULL;

    return 0;
}

static int id_register_to_tab(struct ID **root, char *name, char *procname, struct TYPE **type, int ispara, int deflinenum) {
    struct ID *p_id;
    struct ID *id_end;
    struct TYPE *p_type;
    char *p_name;
    char *p_procname;

    if ((p_id = search_tab(root, name, procname)) != NULL) {
        return error("error multiple definition");
    }
    
    if ((p_id = (struct ID *)malloc(sizeof(struct ID))) == NULL) return error("malloc id error \n");
    
    if ((p_name = (char *)malloc(strlen(name) + 1)) == NULL) return error("malloc name error\n");
    
    strcpy(p_name, name);
    p_id->name = p_name;

    if (procname == NULL) {
        p_id->procname = NULL;
    } else {
        if ((p_procname = (char *)malloc(strlen(procname) + 1)) == NULL) return error("malloc procname error");
        strcpy(p_procname, procname);
        p_id->procname = p_procname;
    }

    if (type == NULL) {
        p_id->itp = NULL;
    } else {
        if ((p_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) return error("malloc type error");
        
        p_type->ttype = (*type)->ttype;
        p_type->arraysize = (*type)->arraysize;
        if ((*type)->ttype & TPARRAY) {
            struct TYPE *p_etype;
            if ((p_etype = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
                return error("malloc array type error");
            }
            p_etype->ttype = (*type)->ttype;
            p_etype->arraysize = (*type)->arraysize;
            p_etype->etp = NULL;
            p_etype->paratp = NULL;
            p_type->etp = p_etype;
        } else {
            p_type->etp = NULL;
        }
        p_type->paratp = NULL;
        p_id->itp = p_type;
    }
    p_id->ispara = ispara;
    p_id->deflinenum = deflinenum;
    p_id->irefp = NULL;
    p_id->nextp = NULL;
    id_end = *root;
    if (id_end == NULL) {
        *root = p_id;
    } else {
        while (id_end->nextp != NULL) {
            id_end = id_end->nextp;
        }
        id_end->nextp = p_id;
    }
    return 0;
}

static void free_strcut_ID(struct ID **root) {
    struct ID *p, *q;

    for (p = *root; p != NULL; p = q) {
        free(p->name);
        free(p->procname);
        free(p->itp);
        free(p->irefp);
        q = p->nextp;
        free(p);
    }
    *root = NULL;
    return;
}

void release_all(void) {
    free_strcut_ID(&globalidroot);
    free_strcut_ID(&localidroot);
    free_strcut_ID(&allroot);
    free_strcut_ID(&id_without_type_root);

    init_cr();
    return;
}

int release_local(void) {
    free_strcut_ID(&localidroot);
    return 0;
}
