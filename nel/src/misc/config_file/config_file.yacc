%{

/* Includes */

#pragma warning (disable : 4786)

#include <stdio.h>
#include <vector>
#include <string>


using namespace std;

#include "nel/misc/config_file.h"
#include "nel/misc/common.h"

/* Constantes */

#define YYPARSE_PARAM pvararray

//#define DEBUG_PRINTF	printf
#define DEBUG_PRINTF	// printf

/* Types */

enum cf_operation { OP_PLUS, OP_MINUS, OP_MULT, OP_DIVIDE, OP_NEG };

struct cf_value
{
	NLMISC::CConfigFile::CVar::TVarType	Type;
	int						Int;
	double					Real;
	char					String[1024];
};

/* Externals */

extern bool cf_Ignore;

extern FILE *yyin;

/* Variables */

NLMISC::CConfigFile::CVar		cf_CurrentVar;

int		cf_CurrentLine;

/* Prototypes */

int yylex (void);

cf_value cf_op (cf_value a, cf_value b, cf_operation op);

void cf_print (cf_value Val);

void cf_setVar (NLMISC::CConfigFile::CVar &Var, cf_value Val);

int yyerror (const char *);

%}

%start ROOT

%union	{
			cf_value Val;
		}

%token <Val> ASSIGN VARIABLE STRING SEMICOLON
%token <Val> PLUS MINUS MULT DIVIDE
%token <Val> RPAREN LPAREN RBRACE LBRACE
%token <Val> COMMA INT REAL

%type <Val> inst
%type <Val> expression
%type <Val> expr2
%type <Val> expr3
%type <Val> expr4
%type <Val> exprbrace
%type <Val> variable

%%

ROOT:		{ cf_CurrentLine = 1; cf_Ignore = false; } instlist | { }
			;

instlist:	instlist inst { }
			| inst { }
			;

inst:		VARIABLE ASSIGN expression SEMICOLON
			{
				DEBUG_PRINTF("                                   (VARIABLE=");
				cf_print ($1);
				DEBUG_PRINTF("), (VALUE=");
				cf_print ($3);
				DEBUG_PRINTF(")\n");
				int i;
				// on recherche l'existence de la variable
				for(i = 0; i < (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size()); i++)
				{
					if ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Name == $1.String)
					{
						DEBUG_PRINTF("Variable '%s' existe deja, ecrasement\n", $1.String);
						break;
					}
				}
				NLMISC::CConfigFile::CVar Var;
				Var.Comp = false;
				Var.Callback = NULL;
				if (cf_CurrentVar.Comp) Var = cf_CurrentVar;
				else cf_setVar (Var, $3);
				Var.Name = $1.String;
				if (i == (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size ()))
				{
					// nouvelle variable
					DEBUG_PRINTF ("yacc: new assign var '%s'\n", $1.String);
					(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).push_back (Var);
				}
				else
				{
					// reaffectation d'une variable
					Var.Callback = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Callback;
					DEBUG_PRINTF ("yacc: reassign var '%s'\n", $1.String);
					if (Var != (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i] && Var.Callback != NULL)
						(Var.Callback)(Var);
					(*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i] = Var;
				}

				cf_CurrentVar.IntValues.clear ();
				cf_CurrentVar.RealValues.clear ();
				cf_CurrentVar.StrValues.clear ();
				cf_CurrentVar.Comp = false;
			}
			;

expression:	expr2 { $$ = $1; cf_CurrentVar.Comp = false; DEBUG_PRINTF("false\n"); }
			| LBRACE exprbrace RBRACE { $$ = $2; cf_CurrentVar.Comp = true; DEBUG_PRINTF("true\n"); }
			;

exprbrace:	expr2 { $$ =  $1; cf_CurrentVar.Type = $1.Type; cf_setVar (cf_CurrentVar, $1); }
			| expr2 { $$ = $1; cf_CurrentVar.Type = $1.Type; cf_setVar (cf_CurrentVar, $1); } COMMA exprbrace
			| { }
			;

expr2:		expr3 { $$ = $1; }
			| expr2 PLUS expr3 { $$ = cf_op($1, $3, OP_PLUS); }
			| expr2 MINUS expr3 { $$ = cf_op($1, $3, OP_MINUS); }
			;

expr3:		expr4 { $$ = $1; }
			| expr3 MULT expr4 { $$ = cf_op($1, $3, OP_MULT); }
			| expr3 DIVIDE expr4 { $$ = cf_op ($1, $3, OP_DIVIDE); }
			;

expr4:		PLUS expr4 { $$ = $2; }
			| MINUS expr4 { cf_value v; v.Type=NLMISC::CConfigFile::CVar::T_INT; /* just to avoid a warning, I affect 'v' with a dummy value */ $$ = cf_op($2,v,OP_NEG); }
			| LPAREN expression RPAREN { $$ = $2; }
			| INT { $$ = yylval.Val; }
			| REAL { $$ = yylval.Val; }
			| STRING { $$ = yylval.Val; }
			| variable { $$ = $1; }
			;

variable:	VARIABLE
			{
				DEBUG_PRINTF("yacc: cont\n");
				bool ok=false;
				int i;
				for(i = 0; i < (int)((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM))).size()); i++)
				{
					if ((*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Name == $1.String)
					{
						ok = true;
						break;
					}
				}
				if (ok)
				{
					cf_value Var;
					Var.Type = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].Type;
					DEBUG_PRINTF("vart %d\n", Var.Type);
					switch (Var.Type)
					{
					case NLMISC::CConfigFile::CVar::T_INT: Var.Int = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].IntValues[0]; break;
					case NLMISC::CConfigFile::CVar::T_REAL: Var.Real = (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].RealValues[0]; break;
					case NLMISC::CConfigFile::CVar::T_STRING: strcpy (Var.String, (*((vector<NLMISC::CConfigFile::CVar>*)(YYPARSE_PARAM)))[i].StrValues[0].c_str()); break;
					default: DEBUG_PRINTF("*** CAN T DO THAT!!!\n"); break;
					}
					$$ = Var;
				}
				else
				{
					DEBUG_PRINTF("var existe pas\n");
				}
			}
			;
%%

/* compute the good operation with a, b and op */
cf_value cf_op (cf_value a, cf_value b, cf_operation op)
{
	DEBUG_PRINTF("[OP:%d; ", op);
	cf_print(a);
	DEBUG_PRINTF("; ");
	cf_print(b);
	DEBUG_PRINTF("; ");

	switch (op)
	{
	case OP_MULT:																//  *********************
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Int *= b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int *= (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: int*str\n"); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Real *= (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real *= b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: real*str\n"); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		DEBUG_PRINTF("ERROR: str*int\n");  break;
			case NLMISC::CConfigFile::CVar::T_REAL:	DEBUG_PRINTF("ERROR: str*real\n");  break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: str*str\n");  break;
			}
			break;
		}
		break;
	case OP_DIVIDE:																//  //////////////////////
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Int /= b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int /= (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: int/str\n"); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		a.Real /= (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real /= b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: real/str\n"); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:		DEBUG_PRINTF("ERROR: str/int\n"); break;
			case NLMISC::CConfigFile::CVar::T_REAL:	DEBUG_PRINTF("ERROR: str/real\n"); break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: str/str\n"); break;
			}
			break;
		}
		break;
	case OP_PLUS:																//  ++++++++++++++++++++++++
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Int += b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int += (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	a.Int += atoi(b.String); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Real += (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real += b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	a.Real += atof (b.String); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	{ char str2[60]; smprintf(str2, 60, "%d", b.Int); strcat(a.String, str2); break; }
			case NLMISC::CConfigFile::CVar::T_REAL:	{ char str2[60]; smprintf(str2, 60, "%f", b.Real); strcat(a.String, str2); break; }
			case NLMISC::CConfigFile::CVar::T_STRING:	strcat (a.String, b.String); break;
			}
			break;
		}
		break;
	case OP_MINUS:																//  -------------------------
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Int -= b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Int -= (int)b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: int-str\n"); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_REAL:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	a.Real -= (double)b.Int; break;
			case NLMISC::CConfigFile::CVar::T_REAL:	a.Real -= b.Real; break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: real-str\n"); break;
			}
			break;
		case NLMISC::CConfigFile::CVar::T_STRING:
			switch (b.Type)
			{
			case NLMISC::CConfigFile::CVar::T_INT:	DEBUG_PRINTF("ERROR: str-int\n"); break;
			case NLMISC::CConfigFile::CVar::T_REAL:	DEBUG_PRINTF("ERROR: str-real\n"); break;
			case NLMISC::CConfigFile::CVar::T_STRING:	DEBUG_PRINTF("ERROR: str-str\n"); break;
			}
			break;
		}
		break;
	case OP_NEG:																// neg
		switch (a.Type)
		{
		case NLMISC::CConfigFile::CVar::T_INT:		a.Int = -a.Int; break;
		case NLMISC::CConfigFile::CVar::T_REAL:		a.Real = -a.Real; break;
		case NLMISC::CConfigFile::CVar::T_STRING:		DEBUG_PRINTF("ERROR: -str\n"); break;
		}
		break;
	}
	cf_print(a);
	DEBUG_PRINTF("]\n");
	return a;
}

/* print a value, it's only for debug purpose */
void cf_print (cf_value Val)
{
	switch (Val.Type)
	{
	case NLMISC::CConfigFile::CVar::T_INT: DEBUG_PRINTF("'%d'", Val.Int); break;
	case NLMISC::CConfigFile::CVar::T_REAL: DEBUG_PRINTF("`%f`", Val.Real); break;
	case NLMISC::CConfigFile::CVar::T_STRING: DEBUG_PRINTF("\"%s\"", Val.String); break;
	}
}

/* put a value into a var */
void cf_setVar (NLMISC::CConfigFile::CVar &Var, cf_value Val)
{
	DEBUG_PRINTF("push %d ", Val.Type); cf_print(Val);
	Var.Type = Val.Type;
	switch (Val.Type)
	{
	case NLMISC::CConfigFile::CVar::T_INT: Var.IntValues.push_back (Val.Int); break;
	case NLMISC::CConfigFile::CVar::T_REAL: Var.RealValues.push_back (Val.Real); break;
	case NLMISC::CConfigFile::CVar::T_STRING: Var.StrValues.push_back(Val.String); break;
	}
}

int yyerror (const char *s)
{
	DEBUG_PRINTF("%s\n",s);
	return 1;
}


