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

#include <stdbool.h>
#include <time.h>

#include <cmod/cmod.h>
#include <cmod/vectors.h>
#include <cmod/cmod-dot.h>
#include <cmod/cmod-backend.h>

static cm_hash_tab_t symbol_offsets;
static cm_vector_t offsets;

static int last_aquired_reg_idx = -1;
static struct TG_register__ cpu_state[] = {
  { 0, false, "a" },
  { 0, false, "b" },
  { 0, false, "c" },
  { 0, false, "d" },
  { 0, false, NULL }
} ;

void cm_target_dump_header (FILE *fd)
{
  fprintf (fd, ".code32\n");

  // fprintf (fd, ".intel_syntax\n");
  /* if we want to use intel syntax in the assembler or not */

  // fprintf (fd, ".section .data\n\n");
  /* Add string literals here.... */

  fprintf (fd, ".text\n\n");
  /* the text section starting "actual code" */
}

char * cm_target_lookup_symbol_offset (tree * decl)
{
  char * retval = NULL;
  cm_assert (decl->type == IDENT_T);

  cm_hashval_t h = cm_hash_string (decl->opa.tc->o.string);
  cm_hash_entry_t * e = cm_hash_lookup_table (&symbol_offsets,
                                              h);
  if (!e)
    fatal ("undefined symbol <%s>!\n", decl->opa.tc->o.string);

  struct TU_symbol__ * sym = (struct TU_symbol__ *) e->data;
  debug ("offset for <%s> is <%i>!\n", sym->ident, sym->offset);

  if (sym->offset > 0)
    {
      unsigned l = 50;
      char buffer[l];
      snprintf (buffer, l, "-%i(%%ebp)", sym->offset);
      retval = strdup (buffer);
    }
  else
    retval = strdup ("(%ebp)");

  return retval;
}

char * cm_target_lookup_symbol_offset_array (tree * decl)
{
  char * retval = NULL;
  cm_assert (decl->type == ACCESSOR);

  cm_hashval_t h = cm_hash_string (decl->opa.tc->o.string);
  cm_hash_entry_t * e = cm_hash_lookup_table (&symbol_offsets,
                                              h);
  if (!e)
    fatal ("undefined symbol <%s>!\n", decl->opa.tc->o.string);

  struct TU_symbol__ * sym = (struct TU_symbol__ *) e->data;
  debug ("offset for <%s> is <%i>!\n", sym->ident, sym->offset);

  if (sym->offset > 0)
    {
      unsigned l = 50;
      char buffer[l];
      snprintf (buffer, l, "-%i(%%ebp)", sym->offset);
      retval = strdup (buffer);
    }
  else
    retval = strdup ("(%ebp)");

  return retval;
}

void cm_target_dump_stack_defined_offsets__ (void)
{
  int idx;
  debug ("Curr block Stack Offsets are:\n");

  cm_vector_t * offsets_p = &offsets;
  for (idx = 0; idx < VEC_length (offsets_p); ++idx)
    {
      struct TU_symbol__ * sym = VEC_index (struct TU_symbol__ *,
                                            &offsets, idx);
      debug ("\t<%s> -> offset %i size %i\n", sym->ident,
             sym->offset, sym->size);
    }
}

char * cm_target_get_reg_from_base (const char * base, enum REG_T T)
{
  debug ("aquiring register <%s>!\n", base);

  char * retval = NULL;
  switch (T)
    {
    case REG:
      {
        char buf[50];
        sprintf (buf, "%%e%sx", base);
        retval = strdup (buf);
      }
      break;

    case HIGH:
      {
        char buf[50];
        sprintf (buf, "%%%sx", base);
        retval = strdup (buf);
      }
      break;

    case LOW:
      {
        char buf[50];
        sprintf (buf, "%%%sl", base);
        retval = strdup (buf);
      }
      break;

    default:
      fatal ("Something bonkers went wrong here...!\n");
      break;
    }

  return retval;
}

/* Lets try to keep %eax fairly clear for transactions */
char * cm_target_fold_Integral (FILE * fd, tree * decl, tree * type)
{
  char * retval = NULL;

  int idx;
  TG_register__ *ret = NULL;
  AQUIRE_REGISTER(ret, idx, 1);

  if (ret->ident)
    {
      /* this isnt safe but it will do for now... */
      int size = type->opa.tc->o.integer/8;
      char * ret_reg = NULL;

      switch (size)
        {
        case 1:
          ret_reg = cm_target_get_reg_from_base ((*ret).ident, LOW);
          break;

        case 2:
          ret_reg = cm_target_get_reg_from_base ((*ret).ident, HIGH);
          break;

        case 4:
          ret_reg = cm_target_get_reg_from_base ((*ret).ident, REG);
          break;

        default:
          fatal ("unhandled literal fold!\n");
          break;
        }

      cpu_state[idx].val = decl->opa.tc->o.integer;
      cpu_state[idx].usage = true;

      fprintf (fd, "\tmov $%i, %s\n", 
               decl->opa.tc->o.integer, ret_reg);
      retval = ret_reg;
    }
  else
    fatal ("unable to find usable register for integral fold!\n");

  return retval;
}

char * cm_target_fold_add_expr (FILE * fd, tree * decl, tree * type)
{
  tree * opa = decl->opa.t;
  tree * opb = decl->opb.t;

  char * retval_reg_a = NULL;
  char * retval_reg_b = NULL;

  int x,y;

  switch (opa->type)
    {
    case INTEGRAL_T:
      retval_reg_a = cm_target_fold_Integral (fd, opa, type);
      break;
      
    case IDENT_T:
      retval_reg_a = cm_target_lookup_symbol_offset (opa);
      break;

    default:
      fatal ("unhandled expression operand type <0x%x>!\n",
             opa->type);
      break;
    }
  x = last_aquired_reg_idx;
  
  switch (opb->type)
    {
    case INTEGRAL_T:
      retval_reg_b = cm_target_fold_Integral (fd, opb, type);
      break;
      
    case IDENT_T:
      retval_reg_b = cm_target_lookup_symbol_offset (opb);
      break;
      
    default:
      fatal ("unhandled expression operand type <0x%x>!\n",
             opb->type);
      break;
    }
  y = last_aquired_reg_idx;
  
  CLEAR_LAST_REG (x);
  CLEAR_LAST_REG (y);
  
  fprintf (fd,"\taddl %s, %s\n", retval_reg_a, retval_reg_b);
  fprintf (fd,"\tmov %s, %%eax\n", retval_reg_b);
  
  return strdup ("%eax");
}

char * cm_target_fold_less_than_expr (FILE * fd, tree * decl, tree * type)
{
  tree * opa = decl->opa.t;
  tree * opb = decl->opb.t;

  char * retval_reg_a = NULL;
  char * retval_reg_b = NULL;

  int x,y;

  switch (opa->type)
    {
    case INTEGRAL_T:
      retval_reg_a = cm_target_fold_Integral (fd, opa, type);
      break;
      
    case IDENT_T:
      retval_reg_a = cm_target_lookup_symbol_offset (opa);
      break;

    default:
      fatal ("unhandled expression operand type <0x%x>!\n",
             opa->type);
      break;
    }
  x = last_aquired_reg_idx;
  
  switch (opb->type)
    {
    case INTEGRAL_T:
      retval_reg_b = cm_target_fold_Integral (fd, opb, type);
      break;
      
    case IDENT_T:
      retval_reg_b = cm_target_lookup_symbol_offset (opb);
      break;
      
    default:
      fatal ("unhandled expression operand type <0x%x>!\n",
             opb->type);
      break;
    }
  y = last_aquired_reg_idx;
  
  CLEAR_LAST_REG (x);
  CLEAR_LAST_REG (y);
  
  tree * label_ift = build_new_label (CONDIT__L);
  tree * label_iftc = build_new_label (CONDIT__L);

  tree * ift_f = label_ift->field;
  tree * iftc_f = label_iftc->field;

  char * ift_label_c = ift_f->opa.tc->o.string;
  char * iftc_label_c = iftc_f->opa.tc->o.string;

  fprintf (fd, "\tcmp %s, %s\n", retval_reg_b, retval_reg_a);
  fprintf (fd, "\tjl %s\n", ift_label_c);
  fprintf (fd, "\tmov $0, %%eax\n");
  fprintf (fd, "\tjmp %s\n", iftc_label_c);
  fprintf (fd, "%s:\n", ift_label_c);
  fprintf (fd, "\tmov $1, %%eax\n");
  fprintf (fd, "%s:\n", iftc_label_c);
  
  return strdup ("%eax");
}

void cm_target_dump_expression_ (FILE * fd, tree * decl)
{
  cm_assert (decl->type == MODIFY_EXPR);

  tree * type = decl->field;
  cm_assert (type);

  tree * acc = decl->opa.t;
  tree * expr = decl->opb.t;

  char * acc_reg = cm_target_lookup_symbol_offset (acc);
  char * retval_reg = NULL;

  switch (expr->type)
    {
    case INTEGRAL_T:
      retval_reg = cm_target_fold_Integral (fd, expr, type);
      break;

    case IDENT_T:
      {
        retval_reg = cm_target_lookup_symbol_offset (expr);
        fprintf (fd, "\tmov %s, %%eax\n", retval_reg );
        retval_reg = "%eax";
      }
      break;

    case ACCESSOR:
      {
	retval_reg = cm_target_lookup_symbol_offset_array (expr);
        fprintf (fd, "\tmov %s, %%eax\n", retval_reg);
        retval_reg = "%eax";
      }
      break;

    case ADD_EXPR:
      retval_reg = cm_target_fold_add_expr (fd, expr, type);
      break;

    case LESS_EXPR:
      retval_reg = cm_target_fold_less_than_expr (fd, expr, type);
      break;

    default:
      fatal ("unhandled expression operand type <0x%x>!\n",
             expr->type);
      break;
    }

  CLEAR_LAST_REG (last_aquired_reg_idx);
  fprintf (fd, "\tmov %s, %s\n", retval_reg, acc_reg);
}

void cm_target_dump_conditional_ (FILE *fd, tree * decl)
{
  cm_assert (decl->type == CONDIT_EXPR);

  tree * ifb = decl->opa.t;
  tree * efb = decl->opb.t;

  tree * eval = ifb->field;
  cm_assert (eval->type == IDENT_T);

  cm_assert (ifb->type == IF_BLOCK);
  tree * start_goto = ifb->opa.t;

  cm_assert (start_goto->type == GOTO);
  tree * start_label = start_goto->field;
  char * start_label_c = start_label->field->opa.tc->o.string;

  cm_assert (efb->type == ELSE_BLOCK);
  tree * else_goto = efb->opa.t;

  cm_assert (else_goto->type == GOTO);
  tree * else_label = else_goto->field;
  char * else_label_c = else_label->field->opa.tc->o.string;

  char * retval_reg = cm_target_lookup_symbol_offset (eval);
  fprintf (fd, "\tmov $1, %%eax\n");
  fprintf (fd, "\tcmp %%eax, %s\n", retval_reg);
  fprintf (fd, "\tjge %s\n", start_label_c);
  fprintf (fd, "\tjmp %s\n", else_label_c);
}

/**
 * http://sourceware.org/binutils/docs-2.20/as/Global.html
 **/
void cm_target_dump_functor (FILE *fd, tree * decl)
{
  cm_assert (decl->type == FUNC_DECL);
  char * decl_name = decl->field->opa.tc->o.string;
  fprintf (fd, ".globl %s\n\n", decl_name);
  fprintf (fd, "%s:\n", decl_name);

  tree * var_decls_head = decl->opa.t;
  tree * var_decls = var_decls_head;

  int locals = 0;
  int curr_offset_size = 0;

  while (var_decls)
    {
      tree * type = var_decls->field;
      tree * idents = var_decls->opa.t;
      int size = -1;

      if (type->type == TYPE_DECL)
	size = (type->opa.tc->o.integer)/8;
      else if (type->type == TYPE_ARY_DCL)
	{
	  int x, y, range, base_type_size;
	  base_type_size = type->field->opa.tc->o.integer / 8;
	  
	  x = type->opa.t->opa.tc->o.integer;
          y = type->opb.t->opa.tc->o.integer;
	  
	  range = y - x;
	  cm_assert (range > 0);

	  size = range * base_type_size;
	}
      else 
	fatal ("unhandled type!\n");

      cm_vector_t * offsets_p = &offsets;
      int index = VEC_length (offsets_p);

      while (idents)
        {
          struct TU_symbol__ * sym = (struct TU_symbol__ *)
            cm_malloc (sizeof (struct TU_symbol__));
          sym->offset = curr_offset_size + size;
          sym->size = size;
          sym->ident = strdup (idents->opa.tc->o.string);
          sym->index = index;

          cm_hashval_t h = cm_hash_string (idents->opa.tc->o.string);
          void ** e = cm_hash_insert (h, sym, &symbol_offsets);
          
          if (e)
            {
              fatal ("already defined symbol offset!\n");
            }
          cm_vec_push (&offsets, sym);

          ++locals;
          curr_offset_size += size;
          idents = idents->next;
        }
      var_decls = var_decls->next;
    }
  
  fprintf (fd, "\tpushl %%ebp\n");
  fprintf (fd, "\tmovl %%esp, %%ebp\n");
  
  if (curr_offset_size > 0)
    {
      /* Lets keep the stack dword aligned */
      int rem = curr_offset_size % 4;
      if (rem > 0)
        curr_offset_size += (4-rem);
      fprintf (fd, "\tsubl $%i, %%esp\n", curr_offset_size);
    }

  cm_target_dump_stack_defined_offsets__ ();

  tree *block = decl->opb.t;
  cm_assert (block->type == BLOCK_DECL);

  tree * idx_block = block->opa.t;
  while (idx_block)
    {
      switch (idx_block->type)
        {
        case MODIFY_EXPR:
          cm_target_dump_expression_ (fd, idx_block);
          break;

        case CALL_EXPR:
          {
            /* Dont handle arguments yet... and follow c-decl style */
            cm_assert (idx_block->opa.t->type == IDENT_T);
            fprintf (fd, "\tcall %s\n", idx_block->opa.t->opa.tc->o.string);
          }
          break;

        case LABEL:
          {
            tree * ident = idx_block->field;
            char * ident_c = ident->opa.tc->o.string;
            fprintf (fd, "%s:\n", ident_c);
          }
          break;

        case GOTO:
          {
            tree * label = idx_block->field;
            tree * lf = label->field;
            char * label_c = lf->opa.tc->o.string;
            fprintf (fd, "\tjmp %s\n", label_c);
          }
          break;

        case CONDIT_EXPR:
          cm_target_dump_conditional_ (fd, idx_block);
          break;

        default:
          fatal ("unhandled mod dot <0x%x>!\n", idx_block->type);
          break;
        }

      idx_block = idx_block->next;
    }

  var_decls = var_decls_head;
  while (var_decls)
    {
      tree * idents = var_decls->opa.t;

      while (idents)
        {
          cm_hashval_t h = cm_hash_string (idents->opa.tc->o.string);
          cm_hash_entry_t * e = cm_hash_lookup_table (&symbol_offsets, h);

          if (e)
            {
              e->hash = 0;
              e->data = NULL;
            }
          idents = idents->next;
        }
      var_decls = var_decls->next;
    }

  int curr_offset = 0;
  int idx; struct TU_symbol__ * i = NULL;
  for (idx = 0; idx < locals; ++idx)
    {
      i = (struct TU_symbol__ *) cm_vec_pop (&offsets);
      curr_offset += i->size;

      cm_free (i->ident);
      cm_free (i);
    }

  /* can be reduced to the leave instruction for 8086... */
  if (curr_offset_size > 0)
    fprintf (fd, "\tmovl %%ebp, %%esp\n");

  fprintf (fd, "\tpopl %%ebp\n");
  fprintf (fd, "\tret\n");
}

void cm_target_gen_ASM__ (cm_vector_t * const TU__,
                          const char * out, const char * in)
{
  debug ("trying to dump target code to <%s>!\n", out);

  cm_hash_init_table (&symbol_offsets);
  cm_vec_init (&offsets);

  FILE * fd = fopen (out, "w");
  if (!fd)
    {
      error ("unable to open <%s> for write!\n", out);
      return;
    }

  if (TU__)
    {
      int idx;
      cm_target_dump_header (fd);
      
      for (idx = 0; idx < VEC_length (TU__); ++idx)
        {
          tree * i = VEC_index (tree*, TU__, idx);
          cm_assert (i->type == FUNC_DECL);
          cm_target_dump_functor (fd, i);
          fprintf (fd, "\n");
        }
    }

  fprintf (fd, "/* C-Mod compiler target code for:\n\t%s <%s:%s>\n*/\n",
           in,MACHINE_TYPE,SYSTEM_TYPE);
  time_t rawtime;
  time (&rawtime);
  fprintf (fd, "/*\n\tCompiled on %s\tCompiler Version %s\n*/\n\n",
           ctime (&rawtime), PACKAGE_STRING);
  fclose (fd);
}
