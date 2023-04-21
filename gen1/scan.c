
#include "token-list.h"
FILE *fp;

void next_buf(void);
int error_num(char cbuf);

char string_attr[MAXSTRSIZE];
extern key key_keyword[KEYWORDSIZE];
extern key key_symbol[KEYWORDSIZE];

char cbuf;
int linenum;
int num_attr;

int scan(void)
{
    int index = 0;
    char token[MAXSTRSIZE];
    
    memset(token, 0, sizeof(token));
    
    //not token
  if(cbuf <= 0x26){
    while(1)
    {
      if ((cbuf == 0x0d) || (cbuf ==0x0a))
      {
        char before_cbuf = cbuf;
        next_buf();
        if ((before_cbuf == 0x0a && cbuf == 0x0d) || (before_cbuf == 0x0d && cbuf == 0x0a))
        {
          cbuf = fgetc(fp);
        }
        linenum++;
      }
      else if(cbuf >= 0x21 && cbuf <= 0x26){
          printf("Character that cannot be displayed.\n");
          return -1;
      }
      else if(cbuf == 0x00){
          printf("there is null\n.");
          return -1;
      }
      else if(cbuf < 0)
        return -1;
      else
        next_buf();

      if (cbuf >= 0x27)
        break;
    }
  }

    //number(0~9)
    if(cbuf >= 0x30 && cbuf <= 0x39)
    {
      while(1){
        snprintf(token, MAXSTRSIZE, "%s%c", token, cbuf);
        next_buf();
        error_num(cbuf);
          
        // not number
        if (!(cbuf >= 0x30&& cbuf <= 0x39))
        {
          memset(string_attr, 0, sizeof(string_attr));
          snprintf(string_attr, MAXSTRSIZE, "%s", token);
          num_attr = atoi(token);
          if (num_attr > 32767)
          {
              printf("number is too large \n");
              return -1;
          }
          return TNUMBER;
          break;
        }
      }
    }
      
    //Alphabet(large & small)  keyword or name
    else if((cbuf >= 0x41 && cbuf <= 0x5a) || (cbuf >= 0x61 && cbuf <= 0x7a))
    {
      while(1){
        snprintf(token,MAXSTRSIZE,"%s%c",token,cbuf);
        error_num(cbuf);
        next_buf();
        //next token is not alphabet
        if (!((cbuf >= 0x41 && cbuf <= 0x5a) || (cbuf >= 0x61 && cbuf <= 0x7a)))
        {
          for(index = 0;index < KEYWORDSIZE;index++)
          { // token is keyword
            if (strcmp(token, key_keyword[index].keyword) == 0)
              return key_keyword[index].keytoken;
          }
          memset(string_attr, 0, sizeof(string_attr));
          snprintf(string_attr, MAXSTRSIZE, "%s", token);
          if (strlen(string_attr) > 1022)
          {
              printf("name is too large \n");
              return -1;
          }
          id_countup(string_attr);
          return TNAME;
          break;
        }
      }
    }
    //symbol
    else if((cbuf >= 0x28 && cbuf <= 0x2e) || (cbuf >= 0x3a && cbuf <= 0x3e) || cbuf == 0x5b || cbuf == 0x5d) {
        char symbol = cbuf;
        next_buf();
        switch (symbol) {
            case '+':
                return TPLUS;
            case '-':
                return TMINUS;
            case '*':
                return TSTAR;
            case '=':
                return TEQUAL;
            case '(':
                return TLPAREN;
            case ')':
                return TRPAREN;
            case '[':
                return TLSQPAREN;
            case ']':
                return TRSQPAREN;
            case '.':
                return TDOT;
            case ',':
                return TCOMMA;
            case ';':
                return TSEMI;
            case '<':
                if (cbuf == '>') {
                    next_buf();
                    return TNOTEQ;
                } else if (cbuf== '=') {
                    next_buf();
                    return TLEEQ;
                } else {
                    return TLE;
                }
            case '>':
                if (cbuf == '=') {
                    next_buf();
                    return TGREQ;
                } else {
                    return TGR;
                }
            case ':':
                if (cbuf == '=') {
                    next_buf();
                    return TASSIGN;
                } else {
                    return TCOLON;
                }
            default:
                error("symbol error");
                return -1;
        }
    }
    //string
   else  if(cbuf == 0x27){
       next_buf();
        while(1){
            if(cbuf == 0x27){
                next_buf();
                if(cbuf == 0x27){
                    next_buf();
                    continue;
                }
                return TSTRING;
            }
            else{
                snprintf(string_attr, MAXSTRSIZE, "%s%c", string_attr, cbuf);
                if (cbuf == 0x20){
                    return TSTRING;
                }
            }
            return TSTRING;
        }
        return -1;
    }

    //comment {}
  else if(cbuf == 0x7b)
  {
    while(1)
    {
      next_buf();
      if (cbuf < 0)
      {
        error("EOF in comment．");
        return -1;
      }
      if (cbuf == 0x7d)
      {
        next_buf();
        error_num(cbuf);
        return 0;
      }
    }
  }
    
    // comment /* */
  else if(cbuf == 0x2f)// /
  {
    next_buf();
    error_num(cbuf);

    if(cbuf == 0x2a)// *
    {
      while (1)
      {
        next_buf();
        if (cbuf < 0)
        {
          error("EOF in comment．");
          return -1;
        }

        if (cbuf == 0x2a) // *
        {
          next_buf();
          error_num(cbuf);
          if(cbuf == 0x2f)// /
          {
            next_buf();
            return 0;
          }
        }
      }
    }
  }
  return -1;
}

int get_linenum(void)
{
  return linenum;
}

void end_scan(void){
  fclose(fp);
}

int error_num(char cbuf)
{
    if(cbuf < 0)
        return -1;
    return 0;
}

//Next Token
void next_buf(void)
{
    cbuf = fgetc(fp);
}

//initiate_
int init_scan(char *filename)
{
  fp = fopen(filename,"r");
  if (fp == NULL)
  {
    printf("can't open file\n");
    return -1;
  }
  next_buf();
  
    
  linenum = 1;
  init_idtab();
    
  return 0;
}









