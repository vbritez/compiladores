#define TAM_TOKEN 30 //es la dimension de la columna de la matriz
#define TOKENS_IGUALES    1000  //los numeros pueden ser cualquiera, mayores a 256 para no confundir con los caracteres ascii
#define PROG        316
#define SMT         317
#define DECL_SMT    318
#define IF_SMT      319
#define FOR_SMT     320
#define ASSI_SMT    321
#define WRITE_SMT   322
#define SIMPLE_VAR  323
#define IDENT_LIST  324
#define IDENT_LIST2 325
#define FOR_CLAUSE  326
#define FOR_LIST    327
#define FOR_LIST2   328
#define IF_BLOCK    329
#define IF_BLOCK2   330
#define RELA_OP     331
#define COND        332
#define EXP         333
#define ARIT_EXP    334   
#define TERM        335
#define FACTOR      336
#define ARIT_EXP2   337
#define TER2        338
#define ADDOP       339
#define MULOP       340
#define FIN         341
#define SIGNO       342
#define ERROR       999

int analisis_sintactico(char tokens[][TAM_TOKEN],int cant_tokens, int numero_linea){
    int error=0;
    char msg[300];
    char pila[100][TAM_TOKEN];//una pila que puede almacenar como maximo 100 tokens
    int pos_pila=-1;
    int fila=0;
    strcpy(tokens[cant_tokens],"$");//marcamos fin de expresion
    strcpy(pila[++pos_pila],"$");//fin de expresion en la pila 
    strcpy(pila[++pos_pila],"PROG");// se carga simbolo inicial en la pila
    
    int analizar=1;
    while(analizar==1){
         char tope[30];
         char token_input[30];
            
         strcpy(tope,pila[pos_pila]);//se copia en la variable tope el token que esta en el tope de la pila
         strcpy(token_input,tokens[fila]);//se copia en token_input un token de la expresion
            
         if(strcmp(token_input,tope)==0){//los tokens son iguales
             if(strcmp(tope,"$")!=0){//no es fin de de expresion
                 match(token_input);
                 pos_pila--;//disminuye el tope de la pila 
                 fila++;//analizaremos el siguiente token de la matriz
                  //volvemos a cargar tope y token_input
                 strcpy(tope,pila[pos_pila]);//se copia en la variable tope el token que esta en el tope de la pila
                 strcpy(token_input,tokens[fila]);//se copia en token_input un token de la expresion
             }    
         }
            

         int estado;
         estado=TOKENS_IGUALES;//se asume que el tope de la pila y el token seran iguales
         
         
         if(strcmp(tope,"PROG")==0) estado=PROG;
         if(strcmp(tope,"SMT")==0) estado=SMT;
         if(strcmp(tope,"DECL_SMT")==0) estado=DECL_SMT;
         if(strcmp(tope,"IF_SMT")==0) estado=IF_SMT;
         if(strcmp(tope,"FOR_SMT")==0) estado=FOR_SMT;
         if(strcmp(tope,"ASSI_SMT")==0) estado=ASSI_SMT;
         if(strcmp(tope,"WRITE_SMT")==0) estado=WRITE_SMT;
         if(strcmp(tope,"SIMPLE_VAR")==0) estado=SIMPLE_VAR;
         if(strcmp(tope,"IDENT_LIST")==0) estado=IDENT_LIST;
         if(strcmp(tope,"IDENT_LIST2")==0) estado=IDENT_LIST2;
         if(strcmp(tope,"IF_BLOCK")==0) estado=IF_BLOCK;
         if(strcmp(tope,"IF_BLOCK2")==0) estado=IF_BLOCK2;
         if(strcmp(tope,"RELA_OP")==0) estado=RELA_OP;
         if(strcmp(tope,"COND")==0) estado=COND;
         if(strcmp(tope,"EXP")==0) estado=EXP;
         if(strcmp(tope,"ARIT_EXP")==0) estado=ARIT_EXP;
         if(strcmp(tope,"TERM")==0) estado=TERM;
         if(strcmp(tope,"FACTOR")==0) estado=FACTOR;
         if(strcmp(tope,"ARIT_EXP2")==0) estado=ARIT_EXP2;
         if(strcmp(tope,"TER2")==0) estado=TER2;
         if(strcmp(tope,"ADDOP")==0) estado=ADDOP;
         if(strcmp(tope,"MULOP")==0) estado=MULOP;
         if(strcmp(tope,"SIGNO")==0) estado=SIGNO;
         if(strcmp(tope,"ERROR")==0) estado=ERROR;
         if(strcmp(tope,"$")==0) estado=FIN;
         
         switch(estado){
                         
             case FIN: 
                      if(strcmp(token_input,"$")!=0)//pila ya esta vacia, pero todavia quedan tokens en la matriz
                         strcpy(pila[++pos_pila],"ERROR");
                      else{//$==$  ; se terminaron los tokens en la matriz, y tambien en la pila, entonces ok
                          analizar=0;//romper while
                      }
                      break;
                      
             case PROG: //PROG--> SMT   
                      strcpy(pila[pos_pila],"SMT");
                      break;
                      
             case SMT: //SMT--> DECL_SMT SMT | IF_SMT SMT |FOR_SMT SMT  | ASSI_SMT SMT | WRITE_SMT SMT | vacio
                     if(strcmp(token_input,"VAR")==0){  //SMT--> DECL_SMT SMT                 
                          strcpy(pila[pos_pila++],"SMT");    
                          strcpy(pila[pos_pila],"DECL_SMT"); 
                                     
                     }else if (strcmp(token_input,"PR_IF")==0){
                          strcpy(pila[pos_pila++],"SMT");    //SMT--> IF_SMT SMT 
                          strcpy(pila[pos_pila],"IF_SMT");
                          
                     }else if (strcmp(token_input,"PR_FOR")==0){
                          strcpy(pila[pos_pila++],"SMT");    //SMT--> FOR_SMT SMT 
                          strcpy(pila[pos_pila],"FOR_SMT");
                     
                     }else if (strcmp(token_input,"ID")==0){
                          strcpy(pila[pos_pila++],"SMT");    //SMT--> ASSI_SMT SMT 
                          strcpy(pila[pos_pila],"ASSI_SMT");
                          
                     }else if (strcmp(token_input,"WRITE")==0 || strcmp(token_input,"WRITELN")==0){
                          strcpy(pila[pos_pila++],"SMT");    //SMT--> WRITE_SMT SMT 
                          strcpy(pila[pos_pila],"WRITE_SMT");
                          
                     } else {//SMT toma vacio
                          pos_pila--;//y disminuye tope de la pila 
                     }      
                      break;
            
            case DECL_SMT: //DECL_SMT--> SIMPLE_VAR   
                      strcpy(pila[pos_pila],"SIMPLE_VAR");
                      break;
                      
            case IF_SMT: //IF_SMT--> #IF# COND #THEN# SMT IF_BLOCK #END IF# ;
                      if (strcmp(token_input,"PR_IF")==0){
                        strcpy(pila[pos_pila++],"PR_IF");
                        strcpy(pila[pos_pila++],"END");
                        strcpy(pila[pos_pila++],"IF_BLOCK");
                        strcpy(pila[pos_pila++],"SMT");
                        strcpy(pila[pos_pila++],"THEN");
                        strcpy(pila[pos_pila++],"COND");    
                        strcpy(pila[pos_pila],"PR_IF");
                      }
                      break;
                      
             case IF_BLOCK: //IF_BLOCK--> #ELSE# IF_BLOCK2 |vacio ;
                      if (strcmp(token_input,"PR_ELSE")==0){
                          strcpy(pila[pos_pila++],"IF_BLOCK2");
                          strcpy(pila[pos_pila],"PR_ELSE");
                      } else {//SMT toma vacio
                          pos_pila--;//y disminuye tope de la pila 
                     }      
                      break;
                      
             case IF_BLOCK2: //IF_BLOCK2--> IF_SMT | SMT |vacio ;
                if (strcmp(token_input,"PR_IF")==0){ //IF_BLOCK2--> IF_SMT
                      strcpy(pila[pos_pila],"PR_IF");
                      
                } else if (strcmp(token_input,"PR_IF")==0 strcmp(token_input,"WRITE")==0 || strcmp(token_input,"WRITELN")==0
                           || strcmp(token_input,"ID")==0 || strcmp(token_input,"PR_FOR")==0 || strcmp(token_input,"VAR")==0){//IF_BLOCK2--> SMT
                      strcpy(pila[pos_pila],"SMT");
                } else {//IF_BLOCK2 toma vacio
                          pos_pila--;//y disminuye tope de la pila 
                }      
                break;
                
             case COND: //COND--> EXP RELA_OP EXP;
                if (strcmp(token_input,"-")==0 || strcmp(token_input,"(")==0 || strcmp(token_input,"NUM")==0
                  || strcmp(token_input,"ID")==0 || strcmp(token_input,"\"")==0){  
                
                  strcpy(pila[pos_pila++],"EXP");
                  strcpy(pila[pos_pila++],"RELA_OP");
                  strcpy(pila[pos_pila],"EXP");
                }      
                break;
                
             case RELA_OP: //RELA_OP--> '==' | '<>' | '<' | '>' | '>=' | '<='
                if(strcmp(token_input,"==")==0) {
                  strcpy(pila[pos_pila],"==");        
               } else if(strcmp(token_input,"<>")==0) {
                  strcpy(pila[pos_pila],"<>); 
               } else if(strcmp(token_input,"<")==0) {
                  strcpy(pila[pos_pila],"<);
               } else if(strcmp(token_input,">")==0) {
                  strcpy(pila[pos_pila],">");
               } else if(strcmp(token_input,">")==0) {
                  strcpy(pila[pos_pila],">");
               } else if(strcmp(token_input,"<=")==0) {
                  strcpy(pila[pos_pila],"<=");
               }   else {
                   strcpy(pila[++pos_pila],"ERROR"); 
               }                    
                  break;
             
             case ARIT_EXP: //ARIT_EXP--> TERM ARIT_EXP2
                      strcpy(pila[pos_pila++],"ARIT_EXP2");    
                      strcpy(pila[pos_pila],"TERM");
                      break;
                          
             case ARIT_EXP2: //ARIT_EXP2--> ADDOP TERM ARIT_EXP2 | vacio
                      if(strcmp(token_input,"+")==0 || strcmp(token_input,"-")==0){  //ARIT_EXP2--> ADDOP TERM  ARIT_EXP2          
                           strcpy(pila[++pos_pila],"TERM");             
                           strcpy(pila[++pos_pila],"ADDOP");            
                      }
                      else//ARIT_EXP2 toma vacio
                          pos_pila--;//y disminuye tope de la pila       
                      break;
                          
             case TERM: //TERM-->  [SIGNO] FACTOR TER2  
                      strcpy(pila[pos_pila++],"TER2");            
                      strcpy(pila[pos_pila],"FACTOR"); 
                      strcpy(pila[++pos_pila],"SIGNO");    
                      break;
                          
             case TER2://TER2-->MULOP  [SIGNO] FACTOR TER2 | vacio
                      if(strcmp(token_input,"*")==0 || strcmp(token_input,"/")==0){
                          strcpy(pila[++pos_pila],"FACTOR");     
                          strcpy(pila[++pos_pila],"SIGNO");        
                          strcpy(pila[++pos_pila],"MULOP");             
                      }
                      else//TER2 toma vacio
                          pos_pila--;//y disminuye tope de la pila
                      break;
                           
             case ADDOP://ADDOP--> + | -       
                      if(strcmp(token_input,"+")==0 || strcmp(token_input,"-")==0)
                           strcpy(pila[pos_pila],token_input);//Se reemplaza ADDOP por + o por -
                      else
                           strcpy(pila[++pos_pila],"ERROR");
                      break;
                           
             case MULOP://MULOP--> * | /
                      if(strcmp(token_input,"*")==0 || strcmp(token_input,"/")==0)
                           strcpy(pila[pos_pila],token_input);//Se reemplaza MULOP por * o por /
                      else
                           strcpy(pila[++pos_pila],"ERROR");     
                      break;
                          
             case FACTOR: //FACTOR-->(ARIT_EXP) | NUM
                      if(strcmp(token_input,"(")==0) {//FACTOR-->(ARIT_EXP)
                           strcpy(pila[pos_pila++],")"); //reemplazar FACTOR por (ARIT_EXP)
                           strcpy(pila[pos_pila++],"ARIT_EXP");
                           strcpy(pila[pos_pila],"(");
                      } 
                      else if(isdigit(token_input[0]))//solo verificamos si el primer elemento es un numero
                               strcpy(pila[pos_pila],token_input);//se carga el numero en el tope de la pila   
                           else 
                               strcpy(pila[++pos_pila],"ERROR");
                                    
                      break;
                         
                case SIGNO: //SIGNO= - | vacio
                      if(tokens[fila][0]=='-')
                            fila++;

                      pos_pila--;//si existe o no signo, igual disminuye la pila
                      break;
                
                case TOKENS_IGUALES:
                      if(strcmp(tope,token_input)==0){
                          match(token_input);
                          pos_pila--;//disminuye el tope de la pila 
                          fila++;//analizamos el siguiente token
                      }
                      else 
                          strcpy(pila[++pos_pila],"ERROR");
                      break;
                    
                case ERROR:
                          error=1;//para retornar error, entonces no se va a efectuar operacion aritmetica
                          analizar=0;
                          if(strcmp(token_input,"$")==0)
                              msj_error("No se esperaba fin de expresion!",numero_linea);
                       
                               else{
                                    sprintf(msg,"No se esperaba '%s'",tokens[fila]);
                                    msj_error(msg,numero_linea);
                               }
                          
            }//fin switch      
    }//fin while
    
    return error;
}

