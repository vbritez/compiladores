#include<math.h>
#define TAM_TOKEN 30 //es la dimension de la columna de la matriz
float postfija(char tokens[][TAM_TOKEN],int cant_tokens){
     char operadores[100][TAM_TOKEN];//en esta matriz se cargaran los operadores (+, -, *, e, ( , )) 
     char salida[100][TAM_TOKEN];
     int pos_salida=0;
     int pos_operadores=0;
     int pos,fila,total_operadores;
     int ind=0;
     char numero[10];
     for(fila=0;fila<cant_tokens;fila++){
          switch(tokens[fila][0]){//averiguamos si es numero, signo o parentesis
               case '(':
                        strcpy(operadores[pos_operadores++],"(");//se apila directamente en operadores
                        break;
               case ')':
                        total_operadores=pos_operadores-1;
                        for(pos=total_operadores;pos>=0;pos--){
                           if(operadores[pos][0]!='('){//mientras no se encuentre '(', desapilar operadores
                              strcpy(salida[pos_salida++],operadores[pos]);//y enviar a la salida
                              pos_operadores--; //disminuye la cantidad de operadores
                           }
                           else{
                              pos_operadores--; 
                              break;
                           }
                           
                        }                                       
                        break;
               case '+':
                        reordenar_salida("+",salida,operadores,&pos_salida,&pos_operadores);
                        break;
               case '-'://caso especial               
                        if( fila==0 || tokens[fila-1][0]=='*' || tokens[fila-1][0]=='/' || (isdigit(tokens[fila-1][0])==0 && tokens[fila-1][0]!=')') ){
                            strcpy(salida[pos_salida++],"0");//cargar un cero en la salida
                            strcpy(operadores[pos_operadores++],"-");//apilar signo sin tener en cuenta precedencias
                        }else
                            reordenar_salida("-",salida,operadores,&pos_salida,&pos_operadores);
                       
                        break;
               case '*'://usamos '**' para identificar que es de mayor precedencia q '+' o '-'
                        reordenar_salida("**",salida,operadores,&pos_salida,&pos_operadores);
                        break;
               case '/':
                        reordenar_salida("//",salida,operadores,&pos_salida,&pos_operadores);
                        break;

               default: //por defecto se trata de un numero
                        //es posible que existan token como 2e2, 3.4e2.1, etc, entonces debemos separar base de exponente
                        numero[0]='\0';
                        ind=0;
                        while(tokens[fila][ind]!='\0' && tolower(tokens[fila][ind])!='e'){
                            numero[ind]=tokens[fila][ind];
                            ind++;
                        }
                        numero[ind]='\0';
                        strcpy(salida[pos_salida],numero);//apilar numero a la salida
                        pos_salida++;//aumentar indice de salida

                        if(tolower(tokens[fila][ind])=='e'){
                           reordenar_salida("ee",salida,operadores,&pos_salida,&pos_operadores);
                           int pos2=0;
                           ind++;
                           while(tokens[fila][ind]!='\0'){//buscamos el exponente;
                               if(tokens[fila][ind]!='+')
                                  numero[pos2++]=tokens[fila][ind];
                               ind++;
                           }
                           numero[pos2]='\0';  
                           strcpy(salida[pos_salida++],numero);//apilar exponente a la salida
                        }
          }//fin switch
     }//fin for


     //desapilar todos los operadores si es que sobran, y apilarlos en la salida
      pos_operadores--;
      while(pos_operadores>=0){
         strcpy(salida[pos_salida++],operadores[pos_operadores--]);
     }
           int pos_signo=0; 
     float resultado=0;
     if(pos_salida<2){//es un solo numero, ej 98,(((357))),...
        resultado=atof(salida[0]);
        pos_salida=0;
     }
     int pos_oper2,pos_oper1,n;
                             
     while(pos_signo<pos_salida){
          for(n=pos_signo;n<pos_salida;n++){//buscar un operador
              if((salida[n][0]=='-'&&salida[n][1]=='\0')||salida[n][0]=='+'||salida[n][0]=='*'||salida[n][0]=='/'||salida[n][0]=='e'){
                 pos_signo=n;
                 break;
              }
          }

          for(n=pos_signo-1;n>=0;n--){//buscar operando 2
              if(salida[n][0]!='#'){//una vez que usamos como operando una posicion del vector, le cargamos # para no volver a usar
                 pos_oper2=n;
                 break;
              }
          }

          for(n=pos_oper2-1;n>=0;n--){//buscar operando 1
              if(salida[n][0]!='#'){
                 pos_oper1=n;
                 break;
              }
          }   
            
          char oper2[TAM_TOKEN], oper1[TAM_TOKEN];
                                            
          strcpy(oper1,salida[pos_oper1]);//guardar operandos en variables
          strcpy(oper2,salida[pos_oper2]);    
          float operando1=atof(salida[pos_oper1]);
          float operando2=atof(salida[pos_oper2]);
                                        
          strcpy(salida[pos_oper1],"#");//para no volver a usar mismo operando
          strcpy(salida[pos_oper2],"#");//cuando se encuentra '#',significa que operando ya se uso
                                     
          char signo=salida[pos_signo][0];//reconocer operador
                switch(signo){
                       case '-':resultado=operando1-operando2;
                                break;

                       case '+':resultado=operando1+operando2;
                                break;
                                            
                       case '*':resultado=operando1*operando2;
                                break;
                                          
                       case '/':  if(operando2==0){
                                     printf("Division por 0 no existe! -->");
                                     resultado=0;
                                     pos_signo=pos_salida;//romper ciclo
                                  }
                                  else
                                     resultado=operando1/operando2;
                                  break;

                       case 'e':resultado=(float)pow(operando1,operando2);
                                  break;
                 }//fin switch

                 char cadena[TAM_TOKEN];//en cadena se va a guardar el resultado float a char
                 sprintf(cadena, "%.2f", resultado);
                 strcpy(salida[pos_signo],cadena);//donde habia operador, se guarda el resultado
                 pos_signo++;
     }//fin while()
     char aux[10];
     sprintf(aux,"%f",resultado);
     aux[9]='\0';
     if(strcmp(aux,"-0.000000")==0) 
         resultado=0.00f;

    return resultado;
}


    //esta funcion recibe la pila de operadores  y la pila de salida, y reordena de acuerdo a la precedencia de operadores
int reordenar_salida(char *signo,  char salida[][TAM_TOKEN], char operadores[][TAM_TOKEN],int *pos_salida, int *pos_operadores){
        if(*pos_operadores>0){// verificar si pila de operadores esta vacia
            char signoTope[3];signoTope[0]='\0';//para guardar el ultimo signo que esta en operadores
            int i;
            for(i=*pos_operadores-1;i>=0;i--){
                 strcpy(signoTope,operadores[i]);//se debe averiguar cual es el ultimo operador cargado en la pila
                 if(strlen(signoTope)>=2|| ((signoTope[0]=='-'||signoTope[0]=='+')&&(signo[0]=='-'||signo[0]=='+')) ){
                      strcpy(salida[*pos_salida],signoTope);//se saca operador de mayor precedencia de la pila de operadores y va a la salida
                      *pos_salida=*pos_salida+1;
                      *pos_operadores=*pos_operadores-1;
                 }
                 else//ultimo operador es + o -
                       break;
            }
        }
        strcpy(operadores[*pos_operadores],signo);
        *pos_operadores=*pos_operadores+1;
   return 0; 
}
