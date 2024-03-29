#include "token-list.h"

/* keyword list */
key key_keyword[KEYWORDSIZE] = {
    {"and",     TAND    },
    {"array",    TARRAY    },
    {"begin",    TBEGIN    },
    {"boolean",    TBOOLEAN},
    {"break",    TBREAK  },
    {"call",    TCALL    },
    {"char",    TCHAR    },
    {"div",        TDIV    },
    {"do",        TDO    },
    {"else",    TELSE    },
    {"end",        TEND    },
    {"false",    TFALSE    },
    {"if",        TIF    },
    {"integer",    TINTEGER},
    {"not",        TNOT    },
    {"of",        TOF    },
    {"or",        TOR    },
    {"procedure", TPROCEDURE},
    {"program",    TPROGRAM},
    {"read",    TREAD    },
    {"readln",    TREADLN },
    {"return",     TRETURN },
    {"then",    TTHEN    },
    {"true",    TTRUE    },
    {"var",        TVAR    },
    {"while",    TWHILE    },
    {"write",    TWRITE  },
    {"writeln",    TWRITELN}
};
/* symbol list*/
key key_symbol[SYMBOLSIZE] = {
    {"+",TPLUS},
    {"-",TMINUS},
    {"*",TSTAR},
    {"=",TEQUAL},
    {"<>",TNOTEQ},
    {"<",TLE},
    {"<=",TLEEQ},
    {">",TGR},
    {">=",TGREQ},
    {"(",TLPAREN},
    {")",TRPAREN},
    {"[",TLSQPAREN},
    {"]",TRSQPAREN},
    {":=",TASSIGN},
    {".",TDOT},
    {",",TCOMMA},
    {":",TCOLON},
    {";",TSEMI}};
/* Token counter */
int numtoken[NUMOFTOKEN + 1];

/* string of each token */
char *tokenstr[NUMOFTOKEN + 1] = {
    "",
    "NAME", "program", "var", "array", "of", "begin", "end", "if", "then",
    "else", "procedure", "return", "call", "while", "do", "not", "or",
    "div", "and", "char", "integer", "boolean", "readln", "writeln", "true",
    "false", "NUMBER", "STRING", "+", "-", "*", "=", "<>", "<", "<=", ">",
    ">=", "(", ")", "[", "]", ":=", ".", ",", ":", ";", "read", "write", "break"};

int main(int nc, char *np[])
{
    int token;
    int index;

  if (nc < 2)
  {
    printf("File name id not given.\n");
    return 0;
  }
  if (init_scan(np[1]) < 0)
  {
    printf("File %s can not open.\n", np[1]);
    return 0;
  }
  
  while ((token = scan()) >= 0)
  {
    numtoken[token]++;
  }
  end_scan();

  for(index = 1;index  < NUMOFTOKEN + 1; index++)
  {
    if(numtoken[index] >= 1)
      printf("\"%s \"   %d\n",tokenstr[index],numtoken[index]);
  }
  print_idtab();
  release_idtab();
  return 0;
}

void error(char *mes)
{
  printf("\n ERROR: %s\n", mes);
  end_scan();
}
