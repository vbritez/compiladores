/*
 *        Analizador Léxico        
 *        Curso: Compiladores y Lenguajes de Bajo de Nivel
 *        Práctica de Programación Nro. 1
 *        
 *        Descripcion:
 *        Implementa un analizador léxico que reconoce números, identificadores, 
 *         palabras reservadas, operadores y signos de puntuación para un lenguaje
 *         con sintaxis tipo Pascal.
 *        
 */

/*********** LIbrerias utilizadas **************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>

/***************** MACROS **********************/

//Codigos
#define VAR                        256
#define END                        257
#define PR_DO                258
#define TO                        259
#define THEN                260
#define OF                        261
#define PR_FOR                262
#define PR_IF                263
#define PR_ELSE                264
#define WRITELN                265
#define WRITE                266
#define STEP                267
#define NUM                        284
#define ID                        285
#define CAR                        287
#define LITERAL                288
#define OPREL                290
#define OPSUMA                291
#define OPMULT                292
#define OPASIGNA        293
#define USER_TYPE        294
// Fin Codigos
#define TAMBUFF         5
#define TAMLEX                 50
#define TAMHASH         101

/************* Definiciones ********************/

typedef struct entrada{
        //definir los campos de 1 entrada de la tabla de simbolos
        int compLex;
        char lexema[TAMLEX];        
        struct entrada *tipoDato; // null puede representar variable no declarada        
        // aqui irian mas atributos para funciones y procedimientos...
        
} entrada;

typedef struct {
        int compLex;
        entrada *pe;
} token;

/************* Variables globales **************/

int consumir;                        /* 1 indica al analizador lexico que debe devolver
                                                el sgte componente lexico, 0 debe devolver el actual */

char cad[5*TAMLEX];                // string utilizado para cargar mensajes de error
token t;                                // token global para recibir componentes del Analizador Lexico

// variables para el analizador lexico

FILE *archivo;                        // Fuente pascal
char buff[2*TAMBUFF];        // Buffer para lectura de archivo fuente
char id[TAMLEX];                // Utilizado por el analizador lexico
int delantero=-1;                // Utilizado por el analizador lexico
int fin=0;                                // Utilizado por el analizador lexico
int numLinea=1;                        // Numero de Linea

/************** Prototipos *********************/


void sigLex();                // Del analizador Lexico

/**************** Funciones **********************/

/*********************HASH************************/
entrada *tabla;                                //declarar la tabla de simbolos
int tamTabla=TAMHASH;                //utilizado para cuando se debe hacer rehash
int elems=0;                                //utilizado para cuando se debe hacer rehash

int h(const char* k, int m)
{
        unsigned h=0,g;
        int i;
        for (i=0;i<strlen(k);i++)
        {
                h=(h << 4) + k[i];
                if (g=h&0xf0000000){
                        h=h^(g>>24);
                        h=h^g;
                }
        }
        return h%m;
}
void insertar(entrada e);

void initTabla()
{        
        int i=0;
        
        tabla=(entrada*)malloc(tamTabla*sizeof(entrada));
        for(i=0;i<tamTabla;i++)
        {
                tabla[i].compLex=-1;
        }
}

int esprimo(int n)
{
        int i;
        for(i=3;i*i<=n;i+=2)
                if (n%i==0)
                        return 0;
        return 1;
}

int siguiente_primo(int n)
{
        if (n%2==0)
                n++;
        for (;!esprimo(n);n+=2);

        return n;
}

//en caso de que la tabla llegue al limite, duplicar el tamaño
void rehash()
{
        entrada *vieja;
        int i;
        vieja=tabla;
        tamTabla=siguiente_primo(2*tamTabla);
        initTabla();
        for (i=0;i<tamTabla/2;i++)
        {
                if(vieja[i].compLex!=-1)
                        insertar(vieja[i]);
        }                
        free(vieja);
}

//insertar una entrada en la tabla
void insertar(entrada e)
{
        int pos;
        if (++elems>=tamTabla/2)
                rehash();
        pos=h(e.lexema,tamTabla);
        while (tabla[pos].compLex!=-1)
        {
                pos++;
                if (pos==tamTabla)
                        pos=0;
        }
        tabla[pos]=e;

}
//busca una clave en la tabla, si no existe devuelve NULL, posicion en caso contrario
entrada* buscar(const char *clave)
{
        int pos;
        entrada *e;
        pos=h(clave,tamTabla);
        while(tabla[pos].compLex!=-1 && strcmp(tabla[pos].lexema,clave)!=0 )
        {
                pos++;
                if (pos==tamTabla)
                        pos=0;
        }
        return &tabla[pos];
}

void insertTablaSimbolos(const char *s, int n)
{
        entrada e;
        sprintf(e.lexema,s);
        e.compLex=n;
        insertar(e);
}

void initTablaSimbolos()
{
        int i;
        entrada pr,*e;
        const char *vector[]={
                "VAR",
                "END",
                "DO",
                "TO",
                "THEN",
                "FOR", 
                "IF", 
                "ELSE",  
                "WRITELN",
                "WRITE",
                "STEP"
        };
         for (i=0;i<11;i++)
        {
                insertTablaSimbolos(vector[i],i+258);
        }
  insertTablaSimbolos(",",',');
  insertTablaSimbolos(";",';');
  insertTablaSimbolos("[",'[');
  insertTablaSimbolos("]",']');
	insertTablaSimbolos("(",'(');
	insertTablaSimbolos(")",')');
	insertTablaSimbolos("+",OPSUMA);
	insertTablaSimbolos("-",OPSUMA);
	insertTablaSimbolos("*",OPMULT);
	insertTablaSimbolos("/",OPMULT);
	insertTablaSimbolos("=",OPASIGNA);
	insertTablaSimbolos("==",OPREL);
	insertTablaSimbolos("<",OPREL);
  insertTablaSimbolos("<=",OPREL);
  insertTablaSimbolos("<>",OPREL);
  insertTablaSimbolos(">",OPREL);
  insertTablaSimbolos(">=",OPREL);
}

// Rutinas del analizador lexico

void error(const char* mensaje)
{
        printf("Lin %d: Error Lexico. %s.\n",numLinea,mensaje);        
}

void sigLex()
{
        int i=0, longid=0;
        char c=0;
        int acepto=0;
        int estado=0;
        char msg[41];
        entrada e;

        while((c=fgetc(archivo))!=EOF)
        {
                
                if (c==' ' || c=='\t')
                        continue;        //eliminar espacios en blanco
                else if(c=='\n')
                {
                        //incrementar el numero de linea
                        numLinea++;
                        continue;
                }
                else if (isalpha(c))
                {
                        //es un identificador (o palabra reservada)
                        i=0;
                        do{
                                id[i]=c;
                                i++;
                                c=fgetc(archivo);
                                if (i>=TAMLEX)
                                        error("Longitud de Identificador excede tamaño de buffer");
                        }while(isalpha(c) || isdigit(c));
                        id[i]='\0';
                        if (c!=EOF)
                                ungetc(c,archivo);
                        else
                                c=0;
                        t.pe=buscar(id);
                        t.compLex=t.pe->compLex;
                        if (t.pe->compLex==-1)
                        {
                                sprintf(e.lexema,id);
                                e.compLex=ID;
                                insertar(e);
                                t.pe=buscar(id);
                                t.compLex=ID;
                        }
                        break;
                }
                else if (isdigit(c))
                {
                                //es un numero
                                i=0;
                                estado=0;
                                acepto=0;
                                id[i]=c;
                                
                                while(!acepto)
                                {
                                        switch(estado){
                                        case 0: //una secuencia netamente de digitos, puede ocurrir . o e
                                                c=fgetc(archivo);
                                                if (isdigit(c))
                                                {
                                                        id[++i]=c;
                                                        estado=0;
                                                }
                                                else if(c=='.'){
                                                        id[++i]=c;
                                                        estado=1;
                                                }
                                                else if(tolower(c)=='e'){
                                                        id[++i]=c;
                                                        estado=3;
                                                }
                                                else{
                                                        estado=6;
                                                }
                                                break;
                                        
                                        case 1://un punto, debe seguir un digito (caso especial de array, puede venir otro punto)
                                                c=fgetc(archivo);                                                
                                                if (isdigit(c))
                                                {
                                                        id[++i]=c;
                                                        estado=2;
                                                }
                                                else if(c=='.')
                                                {
                                                        i--;
                                                        fseek(archivo,-1,SEEK_CUR);
                                                        estado=6;
                                                }
                                                else{
                                                        sprintf(msg,"No se esperaba '%c'",c);
                                                        estado=-1;
                                                }
                                                break;
                                        case 2://la fraccion decimal, pueden seguir los digitos o e
                                                c=fgetc(archivo);
                                                if (isdigit(c))
                                                {
                                                        id[++i]=c;
                                                        estado=2;
                                                }
                                                else if(tolower(c)=='e')
                                                {
                                                        id[++i]=c;
                                                        estado=3;
                                                }
                                                else
                                                        estado=6;
                                                break;
                                        case 3://una e, puede seguir +, - o una secuencia de digitos
                                                c=fgetc(archivo);
                                                if (c=='+' || c=='-')
                                                {
                                                        id[++i]=c;
                                                        estado=4;
                                                }
                                                else if(isdigit(c))
                                                {
                                                        id[++i]=c;
                                                        estado=5;
                                                }
                                                else{
                                                        sprintf(msg,"No se esperaba '%c'",c);
                                                        estado=-1;
                                                }
                                                break;
                                        case 4://necesariamente debe venir por lo menos un digito
                                                c=fgetc(archivo);
                                                if (isdigit(c))
                                                {
                                                        id[++i]=c;
                                                        estado=5;
                                                }
                                                else{
                                                        sprintf(msg,"No se esperaba '%c'",c);
                                                        estado=-1;
                                                }
                                                break;
                                        case 5://una secuencia de digitos correspondiente al exponente
                                                c=fgetc(archivo);
                                                if (isdigit(c))
                                                {
                                                        id[++i]=c;
                                                        estado=5;
                                                }
                                                else{
                                                        estado=6;
                                                }break;
                                        case 6://estado de aceptacion, devolver el caracter correspondiente a otro componente lexico
                                                if (c!=EOF)
                                                        ungetc(c,archivo);
                                                else
                                                        c=0;
                                                id[++i]='\0';
                                                acepto=1;
                                                t.pe=buscar(id);
                                                if (t.pe->compLex==-1)
                                                {
                                                        sprintf(e.lexema,id);
                                                        e.compLex=NUM;
                                                        insertar(e);
                                                        t.pe=buscar(id);
                                                }
                                                t.compLex=NUM;
                                                break;
                                        case -1:
                                                if (c==EOF)
                                                        error("No se esperaba el fin de archivo");
                                                else
                                                        error(msg);
                                                exit(1);
                                        }
                                }
                        break;
                }
                else if (c=='<') 
                {
                        //es un operador relacional, averiguar cual
                        c=fgetc(archivo);
                        if (c=='>'){
                                t.compLex=OPREL;
                                t.pe=buscar("<>");
                        }
                        else if (c=='='){
                                t.compLex=OPREL;
                                t.pe=buscar("<=");
                        }
                        else{
                                ungetc(c,archivo);
                                t.compLex=OPREL;
                                t.pe=buscar("<");
                        }
                        break;
                }
                else if (c=='>')
                {
                        //es un operador relacional, averiguar cual
                                c=fgetc(archivo);
                        if (c=='='){
                                t.compLex=OPREL;
                                t.pe=buscar(">=");
                        }
                        else{
                                ungetc(c,archivo);
                                t.compLex=OPREL;
                                t.pe=buscar(">");
                        }
                        break;
                }
                else if (c=='=')
                {
                        //puede ser un : o un operador de asignacion
                        c=fgetc(archivo);
                        if (c=='='){
                                t.compLex=OPREL;
                                t.pe=buscar("==");
                        }
                        else{
                                ungetc(c,archivo);
                                t.compLex=OPASIGNA;
                                t.pe=buscar("=");
                        }
                        break;
                }
                else if (c=='+')
                {
                        t.compLex=OPSUMA;
                        t.pe=buscar("+");
                        break;
                }
                else if (c=='-')
                {
                        t.compLex=OPSUMA;
                        t.pe=buscar("-");
                        break;
                }
                else if (c=='*')
                {
                        t.compLex=OPMULT;
                        t.pe=buscar("*");
                        break;
                }
                else if (c=='/')
		            {
		            
		            if ((c=fgetc(archivo))=='/')
                        {//es un comentario
                                while(c!=EOF)
                                {
                                        c=fgetc(archivo);
                                        if (c=='\n')
                                        {
                                                ungetc(c,archivo);
                                                 break;
                                        }
                                }
                        }
               else
                {
                ungetc(c,archivo);
                t.compLex=OPMULT;
                t.pe=buscar("/");
                break;
              }
                        
                }
                
                else if (c==',')
                {
                        t.compLex=',';
                        t.pe=buscar(",");
                        break;
                }
                else if (c==';')
                {
                        t.compLex=';';
                        t.pe=buscar(";");
                        break;
                }
                
                else if (c=='(')
                {
                        t.compLex='(';
                        t.pe=buscar("(");
                        break;
                }
                else if (c==')')
                {
                        t.compLex=')';
                        t.pe=buscar(")");
                        break;
                }
                else if (c=='[')
                {
                        t.compLex='[';
                        t.pe=buscar("[");
                        break;
                }
                else if (c==']')
                {
                        t.compLex=']';
                        t.pe=buscar("]");
                        break;
                }
                else if (c=='\'')
                {//un caracter o una cadena de caracteres
                        i=0;
                        id[i]=c;
                        i++;
                        do{
                                c=fgetc(archivo);
                                if (c=='\'')
                                {
                                        c=fgetc(archivo);
                                        if (c=='\'')
                                        {
                                                id[i]=c;
                                                i++;
                                                id[i]=c;
                                                i++;
                                        }
                                        else
                                        {
                                                id[i]='\'';
                                                i++;
                                                break;
                                        }
                                }
                                else if(c==EOF)
                                {
                                        error("Se llego al fin de archivo sin finalizar un literal");
                                }
                                else{
                                        id[i]=c;
                                        i++;
                                }
                        }while(isascii(c));
                        id[i]='\0';
                        if (c!=EOF)
                                ungetc(c,archivo);
                        else
                                c=0;
                        t.pe=buscar(id);
                        t.compLex=t.pe->compLex;
                        if (t.pe->compLex==-1)
                        {
                                sprintf(e.lexema,id);
                                if (strlen(id)==3 || strcmp(id,"''''")==0)
                                        e.compLex=CAR;
                                else
                                        e.compLex=LITERAL;
                                insertar(e);
                                t.pe=buscar(id);
                                t.compLex=e.compLex;
                        }
                        break;
                }
                else if (c!=EOF)
                {
                        sprintf(msg,"%c no esperado",c);
                        error(msg);
                }
        }
        if (c==EOF)
        {
                t.compLex=EOF;
                sprintf(e.lexema,"EOF");
                t.pe=&e;
        }
        
}


/**** 
 * 
 * Analizador Sintactico
 * 
 ****/

/* funcion para imprimir errores sintacticos*/
void errorSint(const char* mensaje){
	printf("Lin %d: Error Sintactico. %s.\n",numLinea,mensaje);
	sigLex();
	manejo();
}
/* manejo de errores, avanza hasta el siguiente ; */
void manejo(){
	while( t.compLex != ';'){
		sigLex();
	}
}

/* inicio de la pila de llamadas recursivas de las funciones*/
void parser(){
	sigLex();   /* llama al analizador lexico, el cual le provee de los tokens*/
	program();
}


/** Cuerpo de las Funciones **/
/* No terminal program*/
void program(){   /* <program> ::= <statement> */
	statement();
}
/* No terminal statement*/
 /* <statement> ::= <decla_statement> <statement>
		| <if_stm> <statement>
		| <for_clause> <statement>
		| <assignment_statement> <statement>
		| <write_stmt> <statement>
		| <arithmetic_expression> <statement>
		| vacio  */
void statement(){      
	switch(t.compLex){
	case 263: //IF
					if_stm();
					statement();
					break;
	case 262:  //FOR
					for_clause();
					 statement();
					break;
	case 266:  //WRITE
	case 265:  //WRITELN
					write_stmt();
					statement();
					break;
	case 256:  //VAR
					decla_statement();
					statement();
					break;
	case 285:    //ID
					assignment_statement();
					statement();
					break;
	case '(':   //si es una expresion aritmetica entre ()
		arithmetic_expression();
		break;
	default:
				if(isdigit(t.pe->lexema))  //si posee numeros se asume seria parte de una expresion aritmetica
					arithmetic_expression();
				break; // en caso de que sea vacío
	}
}
/* no terminal decla_statement*/
/* <decla_statement> ::= <simple_variable> ';'  */
void decla_statement(){
	simple_variable();
	if(t.pe->lexema == ';')
		sigLex();//consume token
	else
		errorSint("Se esperaba ; ");  //falta el ; 
}
/* no terminal simple_variable*/ 
/* <simple_variable> ::= "VAR" <identifier_list>  */
void simple_variable(){
	sigLex(); //consume VAR
	identifier_list();
}
/* no terminal identifier_list*/
/* <identifier_list> ::= ID <identifier_list2>  */
void identifier_list(){
	sigLex(); //consume ID
	identifier_list2();	
}
/* no terminal indentifier_list2*/
/* <identifier_list2> ::= ',' ID <identifier_list2>
		| '[' NUM | ID ']' <identifier_list2>
		| vacio     */
void identifier_list2(){
	switch(t.compLex){
		case ',':
			sigLex(); //consume , 
			if(t.pe->lexema == "ID")
				identifier_list2();
			else
				errorSint("Se esperaba ID "); 
			break;
		case '[':
			sigLex(); //consume [
			if( (t.pe->lexema == "NUM") || (t.pe->lexema == "ID")){
				sigLex(); //consume NUM | ID
				if(t.pe->lexema == ']'){
					sigLex(); //consume ]
					identifier_list2();
				}
				else
					errorSint("Se esperaba ] "); // falto el ]
			} 
			else
				errorSint("Se esperaba NUM o ID "); //no fue NUM ni ID
			break;
		case ';':  //encuentra ; no lo consume
		case '=':  //si es una asignacion (en la llamada recursiva desde assignment_statement())
			break; //finaliza la sentencia, asumimos que es vacio
		default:
			errorSint("Error en la declaracion de la variable ");
			break;
	}
}
/* no terminal assignment_statement*/ 
/* <assignment_statement> ::= <identifier_list> '=' <assignment_statement2> ';'  */
void assignment_statement(){
	identifier_list();
	if(t.pe->lexema != '=')  //si no es una asignacion
		errorSint("Se esperaba = ");  //error
	else if(t.pe->lexema == '='){
		sigLex(); //consume =
		assignment_statement2();
		//sigLex();
		if(t.pe->lexema == ';')
			sigLex(); //se consume el ;
		else 
			errorSint("Se esperaba ; "); //falto el ;
	}
}
/* no terminal assignment_statement2*/
/* <assignment_statement2> ::= <arithmetic_expression>
		| ID 
		| NUM
		| LITERAL 		*/
void assignment_statement2(){
	switch(t.compLex){
		case 285: //ID
			sigLex(); //consume ID
			break;
		case 284:  //NUM
			sigLex(); //consume NUM
		case 288:   //LITERAL
			sigLex(); //consume LITERAL
			break;
		default:		/* como valor por defecto enviamos a arithmetic_expression(), si hay errores los tomaremos ahi*/
			arithmetic_expression();
			break;
	}
	if(t.pe->lexema == ';')
		sigLex(); //consume el ;
	else
		errorSint("Se esperaba ; ");
}
/* no terminal arithmetic_expression()*/
/* <arithmetic_expression> ::= <termino> <arithmetic_expression2>  */
void arithmetic_expression(){
	termino();
	arithmetic_expression2();
	if(t.pe->lexema == ';')
		sigLex(); //consume el ;
	else
		errorSint("Se esperaba el ; "); //falta el ;	
}
/* no terminal arithmetic_expression2 */
/* <arithmetic_expression2> ::= <addop> <termino> <arithmetic_expression2>
		| vacio				*/ 
void arithmetic_expression2(){
	addop();
	termino();
	arithmetic_expression2();
	/* */
}
/* no terminal addop */ 
/* <addop> ::= '+' 
		| '-'			*/
void addop(){
	if((t.pe->lexema == '+') || (t.pe->lexema == '-'))
		sigLex(); // consume + o -
	else
		errorSint("Se esperaba + o - "); 
}
/* no terminal termino */
/* <termino> ::= <signo> <factor> <termino2>  */
void termino(){
	signo();
	factor();
	termino2();	
}
/* no terminal termino2 */
/* <termino2> ::= <mulop> <signo> <factor> <termino2>
		| vacio						*/ 
void termino2(){
	mulop();
	signo();
	factor();
	termino2();
	/* */
}
/* no terminal signo*/
/* <signo> ::= '-' 
		| vacio				*/ 
void signo(){
	if(t.pe->lexema == '-')
		sigLex();  //consume -
}
/* no terminal mulop */
/* <mulop> ::= '*' 
		| '/' 				*/
void mulop(){
	if( (t.pe->lexema == '*') || (t.pe->lexema == '/')) 
		sigLex(); //consume * o /
	else
		errorSint("Se esperaba * o / ");
}
/* no terminal factor */
/* <factor> ::= '(' <arithmetic_expression> ')' 
		| NUM 				*/
void factor(){
	switch(t.compLex){
		case 284:  //NUM
			sigLex(); //consume NUM
			break;
		case '(':
			sigLex(); //consume (
			arithmetic_expression();
			if(t.pe->lexema == ')')
				sigLex(); // verifica el balanceo de ()
			else
				errorSint("Se esperaba ) "); //falto )
			break;
		default:
			errorSint( "Se esperaba NUM o expresion aritmetica "); //no fue num ni (arithmetic_expresion)
			break;
	}
}
/* no terminal for_clausure */
/* <for_clause> ::= "FOR" ID '=' <for_list> "TO" <for_list> "STEP" <for_list>  "DO" <statement> "END" "FOR" ';'  */
void for_clause(){
	if(t.pe->lexema == "FOR"){
		sigLex(); //consume FOR
		if(t.pe->lexema == "ID"){
			sigLex(); //consume ID
			if(t.pe->lexema == '='){
				sigLex(); // consume =
				for_list();
				if(t.pe->lexema == "TO"){
					sigLex(); //consume TO
					for_list();
					if(t.pe->lexema == "STEP"){
						sigLex(); //consume STEP
						for_list();
						if(t.pe->lexema == "DO"){
							sigLex();  //consume DO
							statement();
							if(t.pe->lexema == "END"){
								sigLex(); // consume END
								if(t.pe->lexema == "FOR"){
									getchar();  // consume FOR
									if(t.pe->lexema == ';'){
										getchar(); //sonsume ;
									}
									else{
										errorSint("Se esperaba ; "); // se esperaba ;
									}
								}
								else{
									errorSint("Se esperaba FOR "); // se esperaba FOR
								}
								
							}
							else{
								errorSint("Se esperaba END "); // se esperaba END
							}
						}
						else{
							errorSint("Se esperaba DO "); //se esperaba DO
						}
					}
					else{
						errorSint("Se esperaba STEP "); //se esperaba STEP
					}
				}
				else{
					errorSint("Se esperaba TO "); // se esperaba TO
				}
			}
			else{
				errorSint("Se esperaba = "); // se esperaba =
			}
		}
		else{
			errorSint("Se esperaba ID "); // se esperaba ID
		}
	}
	else{
		errorSint("Se esperaba FOR "); //se esperaba FOR
	}
} /* fin for_clausure*/

/* no terminal for_list
/* <for_list> ::= <arithmetic_expression>
		| ID
		| NUM					*/
void for_list(){
	switch(t.compLex){
		case 285:  //ID
		case 284:	//NUM
			sigLex(); //consume ID o NUM
			break;
		default:
			arithmetic_expression();
			break;
	}
}
/* no terminal if_stm */
/* <if_stm> ::= "IF" <condition> "THEN" <statement> <if_blocks> "END" "IF" ';'    */
void if_stm(){
	if(t.pe->lexema == "IF"){
		sigLex(); //consume IF
		condition();
		if(t.pe->lexema == "THEN"){
			sigLex(); //consume THEN
			statement();
			sigLex();  /**/
			if_blocks();
			sigLex();   /**/
			if(t.pe->lexema == "END"){
				sigLex(); // consume END
				if(t.pe->lexema == "IF"){
					sigLex(); // consume IF
					if(t.pe->lexema == ';'){
						sigLex(); // consume ;
					}
					else{
						errorSint("Se esperaba ; "); // se esperaba ;
					}
				}
				else{
					errorSint("Se esperaba IF "); //se esperaba IF
				}
			}
			else{
				errorSint("Se esperaba END "); // se esperaba END
			}
		}
		else{
			errorSint("Se esperaba THEN "); //se esperaba THEN		
		}
	}	
	else{
		errorSint("Se esperaba IF "); //se esperaba IF
	}
} // fin if_stm

/* no terminal if_bloks */
/* <if_blocks> ::= "ELSE" <if_blocks2>
		| vacio				*/ 
void if_blocks(){
	if(t.pe->lexema == "ELSE"){
		sigLex(); // consume ELSE
		if_blocks2();
	}
	else {
		if(t.pe->lexema != ';'){
			error("Se esperaba ;");
		}
		else{
			sigLex(); // consume ;
		}
	}
}
/* no terminal if_blocks2 */
/* <if_blocks2> ::= <statement>
		| <if_stm> 				*/
void if_blocks2(){
	if(t.pe->lexema == "IF"){
		if_stm();
	}
	else{
		statement();
	}	
}
/* no terminal relational_op */
/* <relational_op> ::= "==" | "<>" | '<' | '>' | ">="  | "<=" 		*/
void relational_op(){
	if((t.pe->lexema == "<>") || (t.pe->lexema == '<') || (t.pe->lexema == '>') || (t.pe->lexema == ">=") || (t.pe->lexema == "<=") || (t.pe->lexema == "==")){
		sigLex(); // consume el token
	}
	else
		errorSint("Se esperaba uno de los simbolos: = o <> o < o > o >= o <= "); // se esperaba <> o < o > o >= o <=
}
/* no terminal condition */
/* <condition> ::= <expresion> <relational_op expresion> */
void condition(){
	expresion();
	relational_op();
	expresion();
}
/* no terminal expresion */
/* <expresion> ::= <arithmetic_expression>
		| ID 
		| NUM 
		| LITERAL 			*/
void expresion(){
	switch(t.compLex){
	case 285: //ID
	case 284:  //NUM
	case 288:  //LITERAL
		sigLex(); // consume NUM, ID o LITERAL
		break;
	default:  // de lo contrario es una expresion aritmetica
		arithmetic_expression();
		break;
	}
}
/* no terminal write_stmt */
/* <write_stmt> ::= "WRITE" '(' <expresion> ')' ';' 
		| "WRITELN" '(' <expresion> ')' ';'				*/
void write_stmt(){
	switch(t.compLex){
		case 266:  //WRITE
		case 265: //WRITELN
			sigLex(); // consume WRITE o WRITELN
			if(t.pe->lexema == '('){
				sigLex(); // consume (
				expresion();
				if(t.pe->lexema == ')'){
					sigLex(); // consume )
					if(t.pe->lexema == ';'){
						sigLex(); // consume ;
					}
					else{
						errorSint("Se esperaba ; "); // se esperaba ;
					}
				}
				else{
					errorSint("Se esperaba ) "); // se esperaba )
				}
			}
			else{
				errorSint("Se esperaba ( "); // se esperaba (
			}
			break;
		default:
			errorSint("Error en sentencia Write o Writeln "); // error en sentencia
			break;
	}
}



/* Programa Principal */
int main(int argc,char* args[])
{
        // inicializar analizador lexico
        int complex=0;

        initTabla();
        initTablaSimbolos();
        
        if(argc > 1)
        {
                if (!(archivo=fopen(args[1],"rt")))
                {
                        printf("Archivo no encontrado.\n");
                        exit(1);
                }
              //  while (t.compLex!=EOF){
                        //sigLex();
                        parser();			//llama al analizador lexico
                       // printf("Lin %d: %s -> %d\n",numLinea,t.pe->lexema,t.compLex);
              //  }
                fclose(archivo);
        }else{
                printf("Debe pasar como parametro el path al archivo fuente.\n");
                exit(1);
        }

        return 0;
}
