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

%{
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cmod/cmod.h>
#include <cmod/vectors.h>
#include <cmod/cmod-dot.h>

/* y.tab.h ylwrap handles things here and renames */
#include "ss-parser.h"

extern int yyparse( void );
%}

%x comment

%pointer
%option yylineno

DIGIT    [0-9]
ID       [_a-zA-Z][a-zA_Z0-9_$]*
qstring  \"[^\"\n]*[\"\n]

%%

\(                      { return '('; }
\)                      { return ')'; }
"["                     { return '['; }
"]"                     { return ']'; }

";"                     { return ';'; }
":"                     { return ':'; }
","                     { return ','; }
"."                     { return '.'; }
".."                    { return RANGE; }

":="                    { return '='; }
"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"<"                     { return '<'; }
">"                     { return '>'; }

<comment>"[*"           { BEGIN(comment); }
<comment>[^*\n]*        /* eat anything that's not a '*' */
<comment>"*"+[^*/\n]*   /* eat up '*'s not followed by '/'s */
<comment>"\n"
<comment>"*]"           { BEGIN(INITIAL); }

"PROCEDURE"             { return K_PRO; }
"MODULE"                { return K_MOD; }
"VAR"                   { return K_VAR; }

"BEGIN"                 { return '{'; }
"END"                   { return '}'; }

"INTEGER"               { return K_INT; }
"CHAR"                  { return K_CHR; }
"BOOLEAN"               { return K_BOL; }
"ARRAY"                 { return K_ARY; }

"AND"                   { return AND; }
"OR"                    { return OR; }
"NOT"                   { return NOT; }

"IF"                    { return IF; }
"THEN"                  { return THEN; }
"ELSE"                  { return ELSE; }
"FOR"                   { return FOR; }
"TO"                    { return TO; }
"DO"                    { return '{'; }
"LOOP"                  { return LOOP; }
"EXIT"                  { return EXIT; }
"OF"                    { return OF; }

"TRUE"                  { yylval.node = build_int_cst (boolean_type_node, 1);
                          return s_TRUE;
                        }
"FALSE"                 { yylval.node = build_int_cst (boolean_type_node, 0);
                          return s_FALSE;
                        }

{ID}                    {
                          yylval.node = build_identifier_node (cm_strdup(yytext));
                          return IDENTIFIER;
                        }

{DIGIT}+                {
                          long val = strtol (yytext, (char**)NULL, 10);
                          yylval.node = build_int_cst (integer_type_node, val);
                          return INTEGER;
                        }

[\n]                    ;
.                       ;

%%

int yywrap( void )
{
  return 1;
}

int cm_parse_file( const char * in )
{
  int retval = 1;
  FILE * fd = fopen (in, "rb");
  debug("trying to open <%s>!\n", in);
  if( fd )
    {
      yyin = fd;
      retval = yyparse( );
      fclose( fd );
      // yylex_destroy( );
    }
  else
    {
      fprintf(stderr, "error opening <%s>!\n", in );
      retval = 0;
    }
  return retval;
}
