#include "mppl_compiler.h"

struct ID *globalidroot = NULL;
struct ID *localidroot = NULL;
struct PARA *param = NULL;
char *type_str[NUMOFTYPE + 1] = {
    "", "integer", "char", "boolean", "array", "integer", "char", "boolean", "procedule"
};

static struct LINE *t_lineroot;
static struct TYPE *t_type ,*paratp_root = NULL;
static struct NAMES{
    char *formal_name;
    struct NAMES *nextname;
} *t_namesroot;

static struct PROC{
    char *prev_procname;
    int prev_proc_line;
} *t_proc = NULL;

static void init_formaltab(void);
static void release_formaltab(void);
int compare_to(const void *p, const void *q);

void init_global_idtab(){
    globalidroot = NULL;
    init_formaltab();
}

void init_local_idtab(){
    localidroot = NULL;
    init_formaltab();
}

static void init_formaltab(){
    t_lineroot = NULL;
    t_type = NULL;
    t_namesroot = NULL;
    paratp_root = NULL;
}

struct ID *search_global_idtab(char *name){
    struct ID *p;
    for(p = globalidroot; p != NULL; p = p->nextp){
        if(strcmp(name, p->name) == 0 && p->procname == NULL) return p;
    }
    return NULL;
}

struct ID *search_local_idtab(char *name, char *pname){
    struct ID *p;
    for(p = localidroot; p != NULL; p = p->nextp){
        if(strcmp(name, p->name) == 0 && strcmp(pname, p->procname) == 0) return p;
    }
    return NULL;
}

int cross_name(char *name){
    struct NAMES *p;
    char *cname;

    if((p = (struct NAMES *)malloc(sizeof(struct NAMES))) == NULL)return error("cannot malloc");
    if((cname = (char *)malloc(strlen(name) + 1)) == NULL)return error("cannot malloc");
    
    strcpy(cname, name);
    p->formal_name = cname;
    p->nextname = t_namesroot;
    t_namesroot = p;

    return NORMAL;
}

int cross_type(int ttype, int tsize, struct TYPE *tetp, struct PARA *tparatp){
    struct TYPE *temp_etp;
    struct TYPE *temp_paratp, *proot = NULL;
    struct PARA *p;
    
    if((t_type = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL)return error("cannot malloc");

    if(tetp != NULL){
        if((temp_etp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL)return error("cannot malloc");
        temp_etp = tetp;
    }else{
        temp_etp = NULL;
    }

    if(tparatp != NULL){
        for(p = tparatp; p != NULL; p = p->nextp){
            temp_paratp = p->now->itp;
            temp_paratp->paratp = proot;
            proot = temp_paratp;
        }
    }

    t_type->ttype = ttype;
    t_type->arraysize = tsize;
    t_type->etp = temp_etp;
    t_type->paratp = proot;

    return NORMAL;
}

int cross_linenum(int line){
    struct LINE *p;
    if((p = (struct LINE *)malloc(sizeof(struct LINE))) == NULL)return error("cannot malloc");
    
    p->reflinenum = line;
    p->nextep = t_lineroot;
    t_lineroot = p;
    return NORMAL;
}

int cross_proc(char *pname, int line){
    struct PROC *temp_proc;
    char *tpname;
    if((temp_proc = (struct PROC *)malloc(sizeof(struct PROC))) == NULL)return error("cannot malloc");
    
    if(pname != NULL){
        if((tpname = (char *)malloc(strlen(pname) + 1)) == NULL)return error("cannot malloc");
        strcpy(tpname, pname);
    }else{
        tpname = NULL;
    }
    
    temp_proc->prev_procname = tpname;
    temp_proc->prev_proc_line = line;
    t_proc = temp_proc;

    return NORMAL;
}

int de_para(int vd_para, int is_global){
    struct ID *p;
    char *temp_name;
    char *temp_procname;
    struct NAMES *name_p, *name_q;
    struct TYPE *t_itp;
    struct LINE *t_line, *t_ir;

    for(name_p = t_namesroot; name_p != NULL; name_p = name_q){
        if(is_global == LOCAL_PARA){
            if((p = search_local_idtab(name_p->formal_name, pre_proname())) != NULL)return error("%s is already defined", name_p->formal_name);
        }else{
            if((p = search_global_idtab(name_p->formal_name)) != NULL)return error("%s is already defined", name_p->formal_name);
        }

        if((p = (struct ID *)malloc(sizeof(struct ID))) == NULL)return error("cannot malloc");
        
        if((temp_name = (char *)malloc(strlen(name_p->formal_name) + 1)) == NULL)return error("cannot malloc ");
        
        if(is_global == LOCAL_PARA){
            if((temp_procname = (char *)malloc(strlen(t_proc->prev_procname) + 1)) == NULL)return error("cannot malloc");
        }else{
            temp_procname = NULL;
        }
        
        if((t_itp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL)return error("cannot malloc");
        if((t_ir = (struct LINE *)malloc(sizeof(struct LINE))) == NULL)return error("cannot malloc");
        t_ir = NULL;
        
        strcpy(temp_name, name_p->formal_name);
        if(is_global == LOCAL_PARA){
            strcpy(temp_procname, t_proc->prev_procname);
        }else{
            temp_procname = NULL;
        }
        *t_itp = *t_type;

        p->name = temp_name;
        p->procname = temp_procname;
        p->itp = t_itp;
        p->ispara = vd_para ;
        if(t_lineroot != NULL){
            p->deflinenum = t_lineroot->reflinenum;
            t_line = t_lineroot;
            t_lineroot = t_line;
        }
        p->irefp = t_ir;

        if(is_global){
            globalidroot = to_order(globalidroot, p);
        }else{
            localidroot = to_order(localidroot, p);
        }
        printID(p);
        name_q = name_p->nextname;
    }
    release_formaltab();
    return NORMAL;
}

int ref_para(char *name, char *pname, int linenum, int refnum, int is_global){
    struct ID *p;
    struct LINE *t_ir;
    if(is_global == LOCAL_PARA){
        if((p = search_local_idtab(name, pname)) == NULL){
            if((p = search_global_idtab(name)) == NULL)return error("%s(procedule : %s) is not found", name, pname);
        }
    }else{
        if((p = search_global_idtab(name)) == NULL) return error("%s is not found", name);
    }

    if(p->itp->ttype == TPARRAY && p->itp->arraysize <= refnum) return error("overflow error");

    if((t_ir = (struct LINE *)malloc(sizeof(struct LINE))) == NULL)return error("cannnot malloc");
    
    t_ir->reflinenum = linenum;
    t_ir->nextep = p->irefp;
    p->irefp = t_ir;

    return NORMAL;
}

int compare_to(const void *p, const void *q){
    return (strcmp(((struct ID *)p)->name, ((struct ID *)q)->name));
}

struct ID *to_order(struct ID *to, struct ID *from){
    struct ID *temp = NULL, *head, *p, *q;

    if(to == NULL){
        to = from;
        return to;
    }else if(from == NULL){
        return to;
    }
    
    if(compare_to(to, from) > 0) (void)(head = from),(void)(p = to), q = from->nextp;
    else (void)(head = to),(void)(p = to->nextp), q = from;
    temp = head;
    
    while(p != NULL && q != NULL){
        if(compare_to(p,q) > 0){
            temp->nextp = q;
            q = q->nextp;
        }else{
            temp->nextp = p;
            p = p->nextp;
        }
        temp = temp->nextp;
    }

    if(p == NULL) temp->nextp = q;
    else temp->nextp = p;
    return head;
}

void release_global_idtab(){
    struct ID *p, *q;
    struct TYPE *i, *j;
    struct LINE *a, *b;
    struct PARA *c, *d;

    for(p = globalidroot; p != NULL; p = q){
        free(p->name);
        free(p->procname);
        for(i = p->itp; i != NULL; i = j){
            j = i->paratp;
            free(i);
        }
        for(a = p->irefp; a != NULL; a = b){
            b = a->nextep;
            free(a);
        }
        q = p->nextp;
        free(p);
    }
    
    for(c = param; c != NULL; c = d){
        d = c->nextp;
        free(c);
    }
    
    free(t_proc->prev_procname);
    free(t_proc);
    t_proc = NULL;
    init_global_idtab();
}

void relocate_local_idtab(){
    globalidroot = to_order(globalidroot, localidroot);

    if(paratp_root != NULL){
        if(paratp_root->etp != NULL) free(paratp_root->etp);
        if(paratp_root->paratp != NULL) free(paratp_root->paratp);
        free(paratp_root);
    }
    t_proc->prev_procname = NULL;
    t_proc->prev_proc_line = -1;
    init_local_idtab();
}

static void release_formaltab(){
    struct NAMES *p,*q;
    struct LINE *i,*j;

    for(i = t_lineroot; i != NULL; i = j){
        j = i->nextep;
        free(i);
    }
    free(t_type);
    for(p = t_namesroot; p != NULL; p = q){
        free(p->formal_name);
        q = p->nextname;
        free(p);
    }
    init_formaltab();
}

struct TYPE *judge_type(){
    if(t_type != NULL) return t_type;
    else return NULL;
}

struct PARA *propara(char *pname){
    struct PARA *tp, *param = NULL;
    struct ID *p,*q = NULL;
    for(p = localidroot; p != NULL; p = q){
        if(p->procname == NULL) continue;
        if(strcmp(p->procname, pname) == 0 && p->ispara == EXPARA){
            if((tp = (struct PARA *)malloc(sizeof(struct PARA))) == NULL){
                fprintf(stdout, "cannot malloc");
                return NULL;
            }
            tp->now = p;
            tp->nextp = param;
            param = tp;
        }
        q = p->nextp;
    }
    return param;
}

char *pre_proname(){
    return t_proc->prev_procname;
}

char *pre_proname2(){
    return t_namesroot->formal_name;
}

int pre_number(){
    return t_proc->prev_proc_line;
}
