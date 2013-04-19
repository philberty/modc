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
#include <cmod/cmod-backend.h>

#define ERROR_CHECK                             \
  if (error_count > 0)                          \
    return 1;

cm_vector_t * translation_unit;

/**
 * @param id the identifier_node for the name
 * @param var_decls the functions var_decls
 * @param block the function block
 **/
tree *
cm_build_function_decl (tree * id, tree *var_decls,
                        tree * block)
{
  tree * retval;

  cm_assert (id->type == IDENT_T);
  debug("building function <%s>!\n", id->opa.tc->o.string);

  retval = INIT_TREE;
  retval->type = FUNC_DECL;
  retval->field = id;
  retval->opa.t = var_decls;
  retval->opb.t = block;
  retval->next = NULL;

  return retval;
}

/* create main function to call into the program id! */
void cm_dot_build_push_main_function (tree * id)
{
  tree * stmt_list;
  char * main_ident;

  tree * block, * fnc;

  cm_assert (id->type == IDENT_T);
  stmt_list = build_decl (CALL_EXPR, NULL_TREE, id, NULL_TREE);

#ifdef CRAPLE
  main_ident = "_main"; /* darwin crto-begin.o expects _main not main */
#else
  main_ident = "main";
#endif

  block = build_decl (BLOCK_DECL,
                             build_identifier_node (main_ident),
                             stmt_list,NULL_TREE);

  fnc = cm_build_function_decl (build_identifier_node (main_ident),
                                       NULL_TREE,block);
  cm_dot_push_function (fnc);
}

void cm_dot_push_function (tree * decl)
{
  /* toplevl decls must be var_decls or functions */
  cm_assert ((decl->type == VAR_DECL) ||
             (decl->type == FUNC_DECL));

  if (!translation_unit)
    {
      translation_unit = cm_malloc (sizeof(cm_vector_t));
      cm_vec_init (translation_unit);
    }

  cm_vec_push (translation_unit, decl);
}

static char *
cm_get_new_string__ (const char * in, const char * ext)
{
  char * t_out = cm_malloc (strlen(in)+strlen(ext)+1);
  size_t idx = 0, count = 0;
  for ( ; idx<strlen(in); ++idx )
    {
      t_out[idx] = in[idx];
    }
  count = idx;
  for ( idx=0; idx<strlen(ext); ++idx )
    {
      t_out[count] = ext[idx];
      ++count;
    }
  t_out[count] = '\0';
  return t_out;
}

/* handle one file at a time ... */
int cm_do_compile (const char * in)
{
  int retval = 0;
  char * t_out;

  cm_dot_init_types ();

  /* call parser which fills up the translation unit for us... */
  retval = cm_parse_file (in);

  if (f_syntax_only)
    return error_count;
  
  ERROR_CHECK;

  /* Do checks and some transformations on the IR */
  cm_dot_check_translation_unit (translation_unit);
  ERROR_CHECK;

  t_out = cm_get_new_string__ (in,".mod_dot");
  /* Dump out the IR to file ... */
  cm_dot_dump_translation_unit (translation_unit,t_out);
  cm_free(t_out);
  ERROR_CHECK;

  /*
   * lower to RTL then to target ASM
   *  - some livness processing etc
   **/

  t_out = cm_get_new_string__ (in,".S");
  cm_target_gen_ASM__ (translation_unit,t_out,in);
  cm_free(t_out);

  return retval;
}
