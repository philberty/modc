%{
/**
 * Cmod is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmod/cmod.h>
#include <cmod/vectors.h>
#include <cmod/cmod-dot.h>

extern int yylineno;
static cm_vector_t * pv = NULL;

extern int yylex( void );
extern void yyerror( const char * );
%}

%error-verbose
%start declarations
%debug

%union {
  tree * node;
  cm_vector_t * vec;
}

%token K_PRO "PROCEDURE"
%token K_MOD "MODULE"
%token K_VAR "VAR"

%token K_INT "INTEGER"
%token K_CHR "CHAR"
%token K_BOL "BOOLEAN"
%token K_ARY "ARRAY"

%token AND "AND (&&)"
%token OR "OR (||)"
%token NOT "NOT (!)"

%token IF "IF"
%token THEN "THEN"
%token ELSE "ELSE"
%token FOR "FOR"
%token TO "TO"
%token LOOP "LOOP"
%token EXIT "EXIT"
%token OF "OF"
%token RANGE ".."

%token <node> IDENTIFIER "identifier"
%token <node> INTEGER "integer value"
%token <node> s_TRUE "true value"
%token <node> s_FALSE "false value"

%type<node> module_decl
%type<node> head_var_decl
%type<node> var_decls_list
%type<node> var_decls
%type<node> var_decl
%type<node> ident_list

%type<node> procedure_decl
%type<node> block_decl
%type<node> mod_block_decl
%type<node> stmt_list
%type<node> stmts
%type<node> stmt

%type<node> expression
%type<node> expr
%type<node> accessor
%type<node> loop_for
%type<node> conditional
%type<node> else_block
%type<node> loop
%type<node> exit_stmt

%type<node> arguments_list
%type<node> arguments
%type<node> call_expr

%type<node> type
%type<node> integral_type
%type<node> primary

%left '-' '+'
%left '*' '/'
%left '<' '>'
%right '='
%nonassoc UMINUS

%%

declarations:
            | module_decl head_var_decl pdecls mod_block_decl
            {
	      tree * fnc = cm_build_function_decl ($1, $2, $4);
	      cm_dot_push_function (fnc);
	      cm_dot_build_push_main_function ($1);
	    }
            ;

module_decl: K_MOD IDENTIFIER ';'
           { $$ = $2; }
           ;

head_var_decl:
             { $$ = NULL_TREE; }
             | var_decls
	     ;

var_decls: K_VAR var_decls_list
         {
	   tree * x = cm_vec_pop (pv);
	   $$ = x;
	 }
         ;

var_decls_list: var_decls_list var_decl
              {
		$1->next = $2;
		$$ = $2;
	      }
              | var_decl
              {
		if (!pv)
		  {
		    pv = (cm_vector_t*) cm_malloc (sizeof(cm_vector_t));
		    cm_vec_init (pv);
		  }
		cm_vec_push (pv, $1);
		$$ = $1;
	      }
              ;

var_decl: ident_list ':' type ';'
        {
	  tree * x = cm_vec_pop (pv);
	  $$ = build_decl (VAR_DECL, $3, x, NULL_TREE);
	}
        ;

ident_list: ident_list ',' IDENTIFIER
          {
	    $1->next = $3;
	    $$ = $3;
	  }
          | IDENTIFIER
          {
	    if (!pv)
	      {
		pv = (cm_vector_t*) cm_malloc(sizeof(cm_vector_t));
		cm_vec_init (pv);
	      }
	    cm_vec_push (pv,$1);
	    $$ = $1;
	  }
          ;

pdecls:
      | procedure_decl pdecls block_decl
      {
	tree * fnc = build_function_decl ($1,$3);
	cm_dot_push_function (fnc);
      }
      ;

procedure_decl: K_PRO IDENTIFIER ';' var_decls
              {	$$ = build_decl (FUNC_DECL,NULL_TREE,$2,$4); }
              ;

block_decl: '{' stmt_list '}' IDENTIFIER ';'
          { $$ = build_decl (BLOCK_DECL,$4,$2,NULL_TREE); }
          ;

mod_block_decl: '{' stmt_list '}' IDENTIFIER '.'
              { $$ = build_decl (BLOCK_DECL,$4,$2,NULL_TREE); }
              ;

stmt_list: stmts
         { $$ = cm_vec_pop (pv); }
	 ;

stmts: stmts ';' stmt
     {
       $1->next = $3;
       $$ = $3;
     }
     | stmt
     {
       if (!pv)
	 {
	   pv = (cm_vector_t*) 
	     cm_malloc (sizeof(cm_vector_t));
	   cm_vec_init (pv);
	 }
       cm_vec_push (pv,$1);
       $$ = $1;
     }
     ;

stmt: expression
    | loop_for
    | conditional
    | loop
    | exit_stmt
    ;

exit_stmt: EXIT
         {
	   $$ = build_decl (EXIT_STMT, NULL_TREE,
			    NULL_TREE, NULL_TREE);
	 }
         ;

loop: LOOP stmt_list '}'
    {
      $$ = build_decl (LOOP_L, NULL_TREE, $2, NULL_TREE);
    }
    ;

else_block:
          { $$ = NULL_TREE; }
          | ELSE stmt_list
	  {
	    $$ = build_decl (ELSE_BLOCK, NULL_TREE, NULL_TREE, $2);
	  }
	  ;

conditional: IF expression THEN stmt_list else_block '}'
           {
	     tree * ifb = build_decl (IF_BLOCK, NULL_TREE, $2, $4);
	     $$ = build_decl (CONDIT_EXPR, NULL_TREE, ifb, $5);
	   }
           ;

loop_for: FOR expression TO primary '{' stmt_list '}'
        { $$ = build_decl (LOOP_FOR,$4,$2,$6); }
        ;

expression: expr
          {
	    if( $1->type == IDENT_T )
	      {
		$$ = build_decl (CALL_EXPR,NULL_TREE,$1,NULL_TREE);
	      }
	    else
	      $$ = $1;
	  }
          ;

accessor: IDENTIFIER
        { $$ = $1; }
        | IDENTIFIER '[' INTEGER ']'
	{ $$ = build_decl (ACCESSOR, NULL_TREE, $1, $3); }
        ;

expr: accessor '=' expr
    { $$ = build_decl (MODIFY_EXPR,NULL_TREE,$1,$3); }
    | expr '+' expr
    { $$ = build_decl (ADD_EXPR,NULL_TREE,$1,$3); }
    | expr '-' expr
    { $$ = build_decl (SUB_EXPR,NULL_TREE,$1,$3); }
    | expr '<' expr
    { $$ = build_decl (LESS_EXPR,NULL_TREE,$1,$3); }
    | '(' expr ')'
    { $$ = $2; }
    | primary
    ;

integral_type: K_INT
               { $$ = integer_type_node; }
               | K_BOL
               { $$ = boolean_type_node; }
               | K_CHR
               { $$ = char_type_node; }

type: integral_type
    | K_ARY '[' INTEGER RANGE INTEGER ']' OF integral_type
    {
      $$ = cm_dot_create_array_type ($3, $5, $8);
    }
    ;

arguments_list: arguments_list ',' expression
              {
		$1->next = $3;
		$$ = $3;
	      }
              | expression
              {
		if (!pv)
		  {
		    pv = (cm_vector_t*) cm_malloc(sizeof(cm_vector_t));
		    cm_vec_init (pv);
		  }
		cm_vec_push (pv,$1);
		$$ = $1;
	      }
              ;

arguments:
         { $$ = NULL_TREE; }
         | arguments_list
         { $$ = cm_vec_pop (pv); }
	 ;

call_expr: IDENTIFIER '(' arguments ')'
         { $$ = build_decl (CALL_EXPR,NULL_TREE,$1,$3); }
         ;

primary: INTEGER
       | accessor
       | call_expr
       | s_TRUE
       | s_FALSE
       ;
%%

void yyerror( const char *msg )
{
  error ("%s at line %i\n", msg, yylineno);
}

