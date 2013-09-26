#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include "postfija.c"
#include "anlex.c"
#define TAM_TOKEN 30 //asumimos que el maximo tamanho de un token sera 30
#define CANT_TOKENS 100  // y que la cantidad de tokens en una linea sera 100
#define TRUE 1
#define FALSE 0
#define SEPARADOR "\n----------------------------------------------------------\n"


FILE *archivo;
int main(int cantArg, char *arg[]){
   system("clear");
   if(cantArg>1){
      if(!(archivo=fopen(arg[1],"rt"))){
          printf("No existe archivo especificado!\n\n");
      }
      else
        printf("INICIO DE ANALISIS LEXICO!\n\n");
        analisis_lexico(archivo);
        printf("FIN DE ANALISIS LEXICO!\n\n");
        printf(SEPARADOR);
        printf("\nRESULTADOS\n\n");
        archivo=fopen(arg[1],"rt");
        procesar();
        printf("\nFIN DE RESULTADOS\n\n");
                   
   }
   else
        printf("No se especifico archivo fuente!\n\n");
   return 0;
}


int procesar(){
    int caracter=0;
    char token[TAM_TOKEN]; //token puede ser numero, signo, parentesis...y se apila en tokens
    int contador_tam_token=0;//para conocer el tamanho de un token
    int i=0;
    int error=0;
    int fila=0;//indice de tokens
    int num_linea=1;
    char tokens[CANT_TOKENS][TAM_TOKEN];//por cada fila de la matriz, se va a guardar un token( numero, signo, parentesis...)
    int comentario=0; //para saber si es un comentario
      
        while((caracter=fgetc(archivo))!=EOF){
            error=0;//por defecto no existe error al inicio de linea
            i=0;//al inicio del ciclo, token es vacio, en el ciclo se carga con numeros, signos o parentesis
            contador_tam_token=1;//para verificar longitud de un token, para no admitir numeros demasiados grandes por ej
            
            if(caracter==' '||caracter=='\t'){
                  continue;        
            }
            else if(caracter=='\n'){//se llego al final de la linea, se analiza la linea
                     analizar_expresion(tokens,fila,error,num_linea,comentario);
                     fila=0;////despues de analizar  tokens, se debe vaciar 
                     num_linea++;   //aumentar numero de linea
                     comentario = 0;
                     continue;
            }
            else if (isdigit(caracter)){//es un numero
                     int estado=0;
                     int acepto=0;
                     token[i]=caracter;
                				
                     while(!acepto)	{
                          if (contador_tam_token++<=TAM_TOKEN){//evitar numeros muy grandes
                                switch(estado){
                                     case 0: //una secuencia netamente de digitos, puede ocurrir . o e
                                           caracter=fgetc(archivo);
				           if (isdigit(caracter)){
					       token[++i]=caracter;
					       estado=0;
				           }
				           else if(caracter=='.'){//si o si debe venir algun numero
					       token[++i]=caracter;
					       estado=1;
				           }
				           else if(tolower(caracter)=='e'){
					       token[++i]=caracter;
					       estado=3;
				           }
				           else{
					       estado=6;
				           }
				           break;
			
			             case 1://un punto, debe seguir un digito 
				           caracter=fgetc(archivo);						
				           if (isdigit(caracter)){
					       token[++i]=caracter;
					       estado=2;
				           }
				           else{
					       estado=-1;
				           }
				           break;

			             case 2://la fraccion decimal, pueden seguir los digitos o e
				           caracter=fgetc(archivo);
				           if (isdigit(caracter)){
					       token[++i]=caracter;
					       estado=2;
				           }
				           else if(tolower(caracter)=='e'){
                                               token[++i]=caracter;
					       estado=3;
				           }
			        	   else
					       estado=6;
				           break;

			             case 3://una e, puede seguir +, - o una secuencia de digitos
				           caracter=fgetc(archivo);
				           if (caracter=='+' || caracter=='-'){
					       token[++i]=caracter;
					       estado=4;
				           }
				           else if(isdigit(caracter)){
					       token[++i]=caracter;
					       estado=5;
				           }
				           else{
					       estado=-1;
				           }
				           break;

			            case 4://necesariamente debe venir por lo menos un digito
				          caracter=fgetc(archivo);
				          if (isdigit(caracter)){
					      token[++i]=caracter;
					      estado=5;
				           }
				          else{
					      estado=-1;
					      ungetc(caracter,archivo);
				          }
				          break;

			            case 5://una secuencia de digitos correspondiente al exponente
				          caracter=fgetc(archivo);
				          if (isdigit(caracter)){
					     token[++i]=caracter;
					     estado=5;
				          }
				          else
					     estado=6;
				          break;

			           case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
				         if (caracter!=EOF)
					     ungetc(caracter,archivo);
				         acepto=1;     
				         break;

                        	   case -1:
                                         token[++i]=caracter;
                                         token[++i]='\0';
                        		 if (caracter==EOF){
                        			printf("Linea %d: %s ---> No se esperaba el fin de archivo!",num_linea,token);
                       				printf(SEPARADOR);
                                         }
                                         else if(caracter=='\n'){
                                             printf("Linea %d: ",num_linea);
                                             int n=0;
                                             for(n=0;n<fila;n++){//imprimir expresion en pantalla
                                                  printf("%s",tokens[n]);
                                             }
                                             printf(" ---> No se esperaba fin de expresion");
                                         }
                                         else
                                             printf("Linea %d: %s ---> No se esperaba '%c'",num_linea,token,caracter);
                                         printf(SEPARADOR);
                                         error=1;
                                         acepto=1;//romper ciclo while
                                 }//fin switch
                           }//end if (contador_tam_tokens++)
                           else{
                               error=1;
                               printf("Linea %d:--> Error, numero demasiado grande -->%s",num_linea,token);
                               printf(SEPARADOR);
                               break;
                           }
                     }//fin while(!acepto)

                  }//end if (isdigit(caracter))
                  else if(caracter=='+'){
                           token[i]='+';
                           char noValido[100];//para guardar cadena invalida
                            int n=0;
                            while(caracter!='\n' && caracter!=EOF && !isdigit(caracter)){
                              caracter=fgetc(archivo);
                              noValido[n++]=caracter;
                              if(caracter == '*' || caracter == '-'|| caracter == '/' || caracter == '+')
                                break;
                            }//end while
                            
                            if(isdigit(caracter)) {
                              ungetc(caracter,archivo);
                            }else {
                              while(caracter!='\n' && caracter!=EOF){
                                  token[++i]=caracter;//se guarda cadena erronea hasta que se encuentre espacio, fin linea o fin archivo
                                  caracter=fgetc(archivo);
                              }//end  while
                              noValido[n]='\0';
                              error=1;
                              token[++i]='\0';
                              strcpy(tokens[fila++],token);
                              printf("Linea %d: ",num_linea);
                              int x;
                              for(x=0;x<fila;x++)
                                 printf("%s",tokens[x]);
                              if (n>0)
                                if(noValido[n-1]=='\n' || noValido[n-1]==EOF ) {
                                   printf("\nResultado: ERROR: No se esperaba fin de expresion"); 
                                }else {
                                  printf("\nResultado: ERROR: No se esperaba '%s'",noValido);
                                } 
                                printf(SEPARADOR);
                            }//end if
                        }
                    else if(caracter=='-'){
                            token[i]='-';
                            char noValido[100];//para guardar cadena invalida
                            int n=0;
                            while(caracter!='\n' && caracter!=EOF && !isdigit(caracter)){
                              caracter=fgetc(archivo);
                              noValido[n++]=caracter;
                              if(caracter == '*' || caracter == '-'|| caracter == '/' || caracter == '+')
                                break;
                            }//end while
                            
                            if(isdigit(caracter)) {
                              ungetc(caracter,archivo);
                            }else {
                              while(caracter!='\n' && caracter!=EOF){
                                  token[++i]=caracter;//se guarda cadena erronea hasta que se encuentre fin linea o fin archivo
                                  caracter=fgetc(archivo);
                              }//end while
                              
                              noValido[n]='\0';
                              error=1;
                              token[++i]='\0';
                              strcpy(tokens[fila++],token);
                              printf("Linea %d: ",num_linea);
                              int x;
                              for(x=0;x<fila;x++)
                                 printf("%s",tokens[x]);
                              if (n>0)
                                if(noValido[n-1]=='\n' || noValido[n-1]==EOF ) {
                                   printf("\nResultado: ERROR: No se esperaba fin de expresion"); 
                                }else {
                                  printf("\nResultado: ERROR: No se esperaba '%s'",noValido);
                                } 
                                printf(SEPARADOR);
                            } //end if
                        }
                    else if(caracter=='*'){
                            token[i]='*';
                            char noValido[100];//para guardar cadena invalida
                            int n=0;
                            while(caracter!='\n' && caracter!=EOF && !isdigit(caracter)){
                              caracter=fgetc(archivo);
                              noValido[n++]=caracter;
                              if(caracter == '*' || caracter == '-'|| caracter == '/' || caracter == '+')
                                break;
                            }//end while
                            
                            if(isdigit(caracter)) {
                              ungetc(caracter,archivo);
                            }else {
                              while(caracter!='\n' && caracter!=EOF){
                                  token[++i]=caracter;//se guarda cadena erronea hasta que se encuentre espacio, fin linea o fin archivo
                                  caracter=fgetc(archivo);
                              }//end  while
                              
                              noValido[n]='\0';
                              error=1;
                              token[++i]='\0';
                              strcpy(tokens[fila++],token);
                              printf("Linea %d: ",num_linea);
                              int x;
                              for(x=0;x<fila;x++)
                                 printf("%s",tokens[x]);
                              if (n>0)
                                if(noValido[n-1]=='\n' || noValido[n-1]==EOF ) {
                                   printf("\nResultado: ERROR: No se esperaba fin de expresion"); 
                                }else {
                                  printf("\nResultado: ERROR: No se esperaba '%s'",noValido);
                                } 
                                printf(SEPARADOR);
                            }//end if
                    }
                    else if(caracter=='/'){
                          token[i]='/';
                          char noValido[100];//para guardar cadena invalida
                            int n=0;
                            while(caracter!='\n' && caracter!=EOF && !isdigit(caracter)){
                              caracter=fgetc(archivo);
                              noValido[n++]=caracter;
                              if(caracter == '*' || caracter == '-'|| caracter == '+')
                                break;
                              
                              if(caracter== '/') {
                                comentario = 1;
                                break;
                              }
                              
                            } //end while
                            
                            if(isdigit(caracter)) {
                              ungetc(caracter,archivo);
                            }else if(comentario==0){
                              while(caracter!='\n' && caracter!=EOF){
                                  token[++i]=caracter;//se guarda cadena erronea hasta que se encuentre espacio, fin linea o fin archivo
                                  caracter=fgetc(archivo);
                              } //end while
                              
                              noValido[n++]='\0';
                              error=1;
                              token[++i]='\0';
                              strcpy(tokens[fila++],token);
                              printf("Linea %d: ",num_linea);
                              int x;
                              for(x=0;x<fila;x++)
                                 printf("%s",tokens[x]);
                              if (n>0 && comentario == 0) {
                                if(noValido[n-1]=='\n' || noValido[n-1]==EOF ) {   
                                   printf("\nResultado: ERROR: No se esperaba fin de expresion"); 
                                }else {
                                  printf("\nResultado: ERROR: No se esperaba '%s'",noValido);
                                }
                              }
                                printf(SEPARADOR);
                            }else{
                              while(caracter!='\n' && caracter!=EOF){
                                  caracter=fgetc(archivo);
                              } //end while
                              ungetc(caracter,archivo);
                            }//end if
                    }
                    else if(caracter=='('){
                            token[i]='(';
                    }
                    else if(caracter==')'){
                            token[i]=')';
                    }
                    else {//no es ningun token valido, entonces vamos leer todo el token 
                        char noValido[100];//para guardar cadena invalida
                        int n=0;
                        while(caracter!='\n' && caracter!=EOF && caracter!=' ' && caracter!='\t' ){
                            noValido[n++]=caracter;//se guarda cadena erronea hasta que se encuentre espacio, fin linea o fin archivo
                            caracter=fgetc(archivo);
                        }
                        noValido[n]='\0';
                        error=1;
                        if (n>0)
                        printf("Linea %d ---> No se esperaba '%s'", num_linea, noValido);
                        printf(SEPARADOR);
                    }   
                if(error==0){//si no existe error, se carga  tokens con token
                    token[++i]='\0';
                    strcpy(tokens[fila++],token);
                }
                else{                 
                     while(caracter!='\n' && caracter!=EOF){//para no terminar programa ante un error, se busca final de
                          caracter=fgetc(archivo);                             //linea para analizar linea siguiente....
                     }
                     num_linea++;
                     fila=0;//vaciar tokens
                }    
         }//fin while(caracter=fgetc(archivo))!=EOF))...
            
        analizar_expresion(tokens,fila,error,num_linea,comentario);//se llego a fin de archivo, tokens se cargo pero no se proceso..
               
      return 0;  
}

int analizar_expresion(char tokens[CANT_TOKENS][TAM_TOKEN],int fila,int error,int num_linea,int comentario){
     if(fila>0&&error==0&&comentario==0){//evitar analizar lineas vacias, solo si existe expresion
           /*ahora vamos a ver si sintacticamente esta bien la exp, enviamos los tokens y la 
             cantidad de tokens que tiene nuestra expresion
           */   
              printf("Linea %d: ",num_linea);
              int x;
              for(x=0;x<fila;x++)
                 printf("%s",tokens[x]);
              printf(" ");
              printf("\nResultado: %.2f", postfija(tokens,fila));//postfija devuelve el resultado
              printf(SEPARADOR);
         
     }
  return 0;
}



