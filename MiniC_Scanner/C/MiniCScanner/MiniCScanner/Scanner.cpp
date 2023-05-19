/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Scanner.h"

extern FILE *sourceFile;                       // miniC source program


int superLetter(char ch);
int superLetterOrDigit(char ch);
int getNumber(char firstCharacter, int *column);
int hexValue(char ch);
void lexicalError(int n);


const char *tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39                              */
	"while",    "{",        "||",       "}",
	//   ...........    custom symbols ............................... //
	/* 40         41          42        43           44         45     */
	"char",  "double",        "for",    "do",        "goto",    "switch",
	/* 46         47          48        49           50         51    */
	   "case",    "break",    "default","colon",     "charlit", "strlit",
	/* 52    */
	   "doublelit"	

};

const char *keyword[NO_KEYWORD] = {
	"const",  "else",    "if",    "int",    "return",  "void",    "while",
	"char",   "double" , "for" ,  "do" ,    "goto" ,   "switch" , "case" , 
	"break" , "default"
};

enum tsymbol tnum[NO_KEYWORD] = {
	tconst,    telse,     tif,     tint,     treturn,   tvoid,     twhile,
	tchar,     tdouble,   tfor,    tdo,      tgoto,     tswitch,   tcase,
	tbreak,    tdefault
};

struct tokenType scanner(int* line, int* column) // 토큰타입을 반환하는 함수
{
	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH];
	token.number = tnull;



	do {
		while (true){
			ch = fgetc(sourceFile);
			*column += 1;
			if (!isspace(ch)){
				break;
			}
			if (ch=='\n'){
				*column = 0;
				*line += 1;
			}
		};	// state 1: skip blanks // 앞에 공백문자 무시하기
	

		if (superLetter(ch)) { // identifier or keyword // 알파벳 or _ 으로 시작하는 토큰
			i = 0;
			token.columnnumber = *column;
			token.linenumber = *line;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
				*column += 1;
			} while (superLetterOrDigit(ch)); //알파벳 or digit or _
			//하나의 토큰을 id에 저장함.
			if (i >= ID_LENGTH) 
				lexicalError(1); // ID_LENGTH보다 토큰의 캐릭터길이가 길다.
			id[i] = '\0'; //맨끝에 \0 붙여준다.
			ungetc(ch, sourceFile);  //  retract //하나 더 읽은거 다시 넣는다.		
			*column -= 1;				 
			// find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) 
					break; // 동일하면 break
			if (index < NO_KEYWORD){    // found, keyword exit
				token.number = tnum[index]; // keyword에 해당하는 tnum의 값을 number에 준다.
				
			}
			else {                     // not found, identifier exit
				token.number = tident;  // keyword가 아님
				strcpy_s(token.value.id, id); // TOKENVALUE를 저장
				
			}
		}  // end of identifier or keyword
		else if (isdigit(ch)) {  // number // digit으로 시작하는 토큰
			token.columnnumber = *column;
			token.linenumber = *line;
			token.value.num = getNumber(ch, column);
			ch = fgetc(sourceFile);
			*column+=1;
			if (ch == '.') {
				
				//token.number = tdoublelit;
				double zarisu = 1;
				while(true){
					ch = fgetc(sourceFile);
					*column += 1;
					
					if(isdigit(ch)){
						zarisu = zarisu * 0.1;
						
						token.double_sector += zarisu * (int(ch)-48);
						
						token.number = tdoublelit;
						
					}else if(ch=='e'||ch=='E'){
						ch = fgetc(sourceFile);
						*column += 1;
						if (isdigit(ch)) {
							index = getNumber(ch,column);
							token.double_sector += token.value.num;
							for (i = 0; i < index; i++)
								token.double_sector *= 10;
						}
						else if (ch == '+') {
							ch = fgetc(sourceFile);
							*column+=1;
							index = getNumber(ch,column);
							token.double_sector += token.value.num;
							for (i = 0; i < index; i++)
								token.double_sector = token.double_sector * 10;
						}
						else if (ch == '-') {
							ch = fgetc(sourceFile);
							*column+=1;
							index = getNumber(ch,column);
							token.double_sector += token.value.num;
							for (i = 0; i < index; i++)
								token.double_sector = token.double_sector / 10;
						}else{
							lexicalError(9);
						}
						token.number = tdoublelit;
						break;
					}
					else if(isspace(ch) || ch==';'){ //더블끝
						
						ungetc(ch, sourceFile);		
						*column -= 1;	
						token.number = tdoublelit;
						
						token.double_sector += token.value.num;
						
						
						break;
					}
					else{
						lexicalError(8);
						while(true){
							ch=fgetc(sourceFile);
							*column += 1;
							if (isspace(ch) | ch==';' | ch=='/') {
								ungetc(ch, sourceFile);
								*column -=1;
								break;
							}
						}
					}
				}
			
				
			}
			else // 원래코드
			{
				token.number = tnumber;
				ungetc(ch, sourceFile);
				*column -= 1;
			}
		}
		else switch (ch) {  // special character
		case '/':
			ch = fgetc(sourceFile); //다음거까지 가져온다.
			if (ch == '*')			// text comment
				do {
					while (ch != '*') ch = fgetc(sourceFile);
					ch = fgetc(sourceFile);
				} while (ch != '/'); //멀티라인주석 -> 따로 token을 건들지 않네.. tnull
			else if (ch == '/')		// line comment -> 마찬가지... tnull
				while (fgetc(sourceFile) != '\n'); //줄바꿈이 일어날때까지 스트림에서 받는다.
			else if (ch == '=')  token.number = tdivAssign; // div assign
			else {
				token.number = tdiv; //걍 나누기구나~~
				ungetc(ch, sourceFile); // retract // 하나미리 받은거 스트림에 돌려준다.
				*column -= 1;
			}
			break;
		case '!':
			ch = fgetc(sourceFile); //다음거가져온다. 
			if (ch == '=')  token.number = tnotequ; //다를때 논리연산자
			else {
				token.number = tnot; //걍 not이구나~~
				ungetc(ch, sourceFile); // retract 하나 미리 가져온거 돌려준다.
				*column -= 1;
			}
			break;
		case '%':
			ch = fgetc(sourceFile);
			if (ch == '=') {
				token.number = tremAssign;
			}
			else {
				token.number = tremainder;
				ungetc(ch, sourceFile);
				*column -= 1;
			}
			break;
		case '&':
			ch = fgetc(sourceFile);
			if (ch == '&')  token.number = tand;
			else {
				lexicalError(2); //&은 &&만 취급
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '*':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tmulAssign;
			else {
				token.number = tmul;
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '+':
			ch = fgetc(sourceFile);
			if (ch == '+')  token.number = tinc;
			else if (ch == '=') token.number = taddAssign;
			else {
				token.number = tplus;
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '-':
			ch = fgetc(sourceFile);
			if (ch == '-')  token.number = tdec;
			else if (ch == '=') token.number = tsubAssign;
			else {
				token.number = tminus;
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '<':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tlesse;
			else {
				token.number = tless;
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '=':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tequal;
			else {
				token.number = tassign;
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '>':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tgreate;
			else {
				token.number = tgreat;
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '|':
			ch = fgetc(sourceFile);
			if (ch == '|')  token.number = tor;
			else {
				lexicalError(3); // |은 ||만 취급
				ungetc(ch, sourceFile);  // retract
				*column -= 1;
			}
			break;
		case '(': 
			token.number = tlparen;
			token.columnnumber = *column;
			token.linenumber = *line;         
			        
			break;
		case ')': 
			token.number = trparen;         
			token.columnnumber = *column;
			token.linenumber = *line;         
			        
			break;
		case ',': 
			token.number = tcomma;          
			token.columnnumber = *column;
			token.linenumber = *line;         
			      
			break;
		case ';': 
			token.number = tsemicolon;      
			token.columnnumber = *column;
			token.linenumber = *line;         
			        
			break;
		case '[': 
			token.number = tlbracket;       
			token.columnnumber = *column;
			token.linenumber = *line;         
			         
			break;
		case ']': 
			token.number = trbracket;       
			token.columnnumber = *column;
			token.linenumber = *line;         
			       
			break;
		case '{': 
			token.number = tlbrace;         
			token.columnnumber = *column;
			token.linenumber = *line;         
			        
			break;
		case '}': 
			token.number = trbrace;         
			token.columnnumber = *column;
			token.linenumber = *line;         
			       
			break;
		case EOF: 
			token.number = teof;            
			token.columnnumber = *column;
			token.linenumber = *line;         
			        
			break;
		case ':': 
			token.number = tcolon;          
			token.columnnumber = *column;
			token.linenumber = *line;
			        
			break;
		case '\'': //state A
			token.columnnumber = *column;
			token.linenumber = *line;
			ch = fgetc(sourceFile);
			*column += 1;
			if (ch == '\\')  // \\, \', \" ...
			{
				id[0] = ch;
				id[1] = fgetc(sourceFile);
				id[2] = '\0'; 
				*column += 1;			
			}
			else if (ch=='\''){
				lexicalError(7);
				break;
			}
			else { 
				id[0] = ch; 
				id[1] = '\0';
			}
			ch = fgetc(sourceFile);
			*column += 1;
			if (ch == '\'') {
				token.number = tcharlit;
				strcpy_s(token.value.id, id);
				
			}
			else
				lexicalError(5);
			
			break;

		case '\"':
				i = 0;
				while (true){
					token.columnnumber = *column;
					token.linenumber = *line;
					ch = fgetc(sourceFile);
					*column+=1;
					if (ch=='\"' && (ch!='\"' || id[i-1]!='\\')){
						break;
					}

					if (i<ID_LENGTH)  
						id[i++] = ch;
					else {             
						lexicalError(1);
						break;
					}
					if (ch == '\n') {
						ungetc(ch, sourceFile);
						*column-=1;
						lexicalError(6);
						break;
					}
				}
				if (i<ID_LENGTH) {
					id[i]='\0'; 
					if (ch=='\"') {			
						strcpy_s(token.value.id,id);
						token.number=tstrlit;
						
					}
				}
				else {
					while (true){
						ch = fgetc(sourceFile);
						if (ch=='\n'){
							ungetc(ch,sourceFile);
							*column -= 1;
							break;
						
						}
							
					}
						
				}
				break;
		case '.':
				token.columnnumber = *column;
				token.linenumber = *line;
				ch = fgetc(sourceFile);
				*column += 1;
				if (isdigit(ch)) {
					token.number = tdoublelit;
					token.double_sector = getNumber(ch,column);
					while (token.double_sector >= 1){
						token.double_sector = token.double_sector * 0.1;
					}
				}
				else {
					lexicalError(4);
					ungetc(ch,sourceFile);
					*column -= 1;
				}
				break;
			


		default: {
			printf("Current character : %c", ch);
			lexicalError(4); // 우리가 모른넘이 들어왔다!!
			break;
		}

		} // switch end
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	case 5: printf("character literal needs \' at the end\n");
		break;
	case 6: printf("string literal needs \" at the end\n");
		break;
	case 7: printf("between single quotation marks need one character. cannot be blank\n");
		break;
	case 8: printf("in double type fractional part should consist of number or space.\n");
		break;
	case 9: printf("floating point error\n");
		break;
	
	}
}

int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1; //isalpha는 알파벳인지 확인 알파벳아니면 0, 맞으면 1반환
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

int getNumber(char firstCharacter, int* column)
{
	int num = 0;
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		*column += 1;
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while (true){
				ch = fgetc(sourceFile);
				*column+=1;
				value = hexValue(ch);
				if (value == -1){
					break;
				}
				num = 16 * num + value;
			}
				
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
				*column +=1;
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
			*column += 1;
		} while (isdigit(ch));
	}
	ungetc(ch, sourceFile);  /*  retract  */
	*column -= 1;
	return num;
}

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

void printToken(struct tokenType token, char* fileName)
{
	if (token.number == tident) // keyword가 아닌넘들
		printf("%s (%d, %s, %s, %d, %d)\n",tokenName[token.number], token.number, token.value.id, fileName, token.linenumber, token.columnnumber);
	else if (token.number == tnumber) //숫자
		printf("%s (%d, %d, %s, %d, %d)\n",tokenName[token.number], token.number, token.value.num, fileName, token.linenumber, token.columnnumber);
	else if (token.number == tcharlit || token.number == tstrlit)
		printf("%s (%d, %s, %s, %d, %d)\n",tokenName[token.number], token.number, token.value.id, fileName, token.linenumber, token.columnnumber);
	else if (token.number == tdoublelit)
		printf("%s (%d, %lf, %s, %d, %d)\n",tokenName[token.number], token.number, token.double_sector, fileName, token.linenumber, token.columnnumber);
	else{ // 이미 정의되있는 TN으로도 알 수 있는 것들
		printf("%s (%d, %s, %s, %d, %d)\n",tokenName[token.number], token.number, tokenName[token.number], fileName ,token.linenumber, token.columnnumber);
		}

}