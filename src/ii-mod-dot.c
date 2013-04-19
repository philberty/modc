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

struct TU_labels__t {
  int loops_labels;
  int tmp_var_labels;
  int conditioanl_labels;
} ;

cm_hash_tab_t * dot_global_namespace;
cm_vector_t * dot_types;

static cm_vector_t * context;
static struct TU_labels__t dot_labels = {
  0,0,0
};

static cm_vector_t * cm_transform_bin_expr (tree *,tree*);
static cm_vector_t * cm_transform_modify_expr (tree *,tree*);
static cm_vector_t * cm_dot_transform_expression (tree **,tree*);

static tree * cm_dot_lookup_var_decl (tree *, cm_vector_t * const);
static bool cm_dot_push_var_decl (tree *, cm_vector_t * const, bool);
static inline int TU_labels_get_new_loop (void);

static inline int TU_labels_get_new_tmp_var (void);
static inline int TU_labels_get_new_condit (void);
static inline char * build_new_string (const char *, int);

static inline
int TU_labels_get_new_loop (void)
{
  int retval = dot_labels.loops_labels;
  dot_labels.loops_labels++;
  return retval;
}

static inline
int TU_labels_get_new_condit (void)
{
  int retval = dot_labels.conditioanl_labels;
  dot_labels.conditioanl_labels++;
  return retval;
}

static inline
int TU_labels_get_new_tmp_var (void)
{
  int retval = dot_labels.tmp_var_labels;
  dot_labels.tmp_var_labels++;
  return retval;
}

void cm_dot_init_types (void)
{
  tree_common *tc_i, *tc_b;
  tree * it, *ib;

  dot_types = cm_malloc(sizeof(cm_vector_t));
  cm_vec_init (dot_types);

  dot_global_namespace = (cm_hash_tab_t *)
    cm_malloc(sizeof (cm_hash_tab_t));
  cm_hash_init_table (dot_global_namespace);

  context = (cm_vector_t *)
    cm_malloc (sizeof(cm_vector_t));
  cm_vec_init (context);
  
  tc_i = (tree_common*)
    cm_malloc(sizeof(tree_common));
  tc_i->type = INT_TYPE_T;
  tc_i->o.integer = 32;

  it = (tree*) cm_malloc(sizeof(tree));
  it->type = TYPE_DECL;
  it->field = NULL_TREE;
  it->opa.tc = tc_i;
  it->opb.t = NULL_TREE;
  it->next = NULL;

  cm_vec_push (dot_types, it);

  tc_b = (tree_common*)
    cm_malloc(sizeof(tree_common));
  tc_b->type = BOL_TYPE_T;
  tc_b->o.integer = 8;
 
  ib = (tree*) malloc(sizeof(tree));
  ib->type = TYPE_DECL;
  ib->field = NULL_TREE;
  ib->opa.tc = tc_b;
  ib->opb.t = NULL_TREE;
  ib->next = NULL;

  cm_vec_push (dot_types, ib);
}

tree * cm_dot_create_array_type (tree *x, tree *y, tree *base_type)
{
  int x_i, y_i;

  x_i = x->opa.tc->o.integer;
  y_i = y->opa.tc->o.integer;

  if ((y_i - x_i) <= 0)
    error ("range arguments to array <%i->%i> are invalid!\n",
	   x_i, y_i);

  tree * type = (tree *)
    cm_malloc (sizeof (tree));

  type->type = TYPE_ARY_DCL;

  type->opat = TREE_T;
  type->opbt = TREE_T;

  type->field = base_type;
  type->opa.t = x;
  type->opb.t = y;

  type->next = NULL;

  return type;
}

static inline
char * build_new_string (const char *base, int n)
{
  char buf[50];
#ifdef _WIN32
  sprintf_s(buf,50,"%s.%i",base,n);
#else
  sprintf(buf,"%s.%i",base,n);
#endif
  debug ("new string <%s>!\n", buf);

  return cm_strdup (buf);
}

tree * build_new_label (enum label_t type)
{
  const char * name = NULL;
  tree * retval = NULL_TREE;

  switch (type)
    {
    case LOOP__L:
      name = build_new_string (".L", TU_labels_get_new_loop ());
      retval = build_decl (LABEL, build_identifier_node (name),
                           NULL_TREE,NULL_TREE);
      break;

    case CONDIT__L:
      name = build_new_string (".C",  TU_labels_get_new_condit ());
      retval = build_decl (LABEL, build_identifier_node (name),
                           NULL_TREE,NULL_TREE);
      break;

    default:
      error ("unhandled label type!\n");
      break;
    }

  return retval;
}

tree * create_new_tmp_var (tree * type)
{
  tree *retval, *id, *decl;
  char * name = build_new_string ("T", TU_labels_get_new_tmp_var ());
  retval = build_identifier_node (name);

  id = build_identifier_node (name);
  decl = build_decl (VAR_DECL,type,id,NULL_TREE);

  /* push the new var_decl in the current_context */
  if (!cm_dot_push_var_decl (decl,context,false))
    fatal ("error pushing tmp_var_decl!\n");
  
  return retval;
}

/**
 * @param t the enum type code VAR_DECL, MODIFY_EXPR etc
 * @param tt the target type INT_TYPE etc
 * @param x the first operand
 * @param y the 2nd operand
 *
 * @returns a decl representing a decl like operation or assignment etc
 **/
inline
tree * build_decl (opcode_t t, tree *tt, tree *x, tree *y)
{
  tree * retval = (tree*) cm_malloc(sizeof(tree));
  retval->type = t;
  retval->field = tt;

  if (x == NULL_TREE)
    retval->opat = 0;
  else
    retval->opat = TREE_T;
  
  retval->opa.t = x;

  if (y == NULL_TREE)
    retval->opbt = 0;
  else
    retval->opbt = TREE_T;

  retval->opb.t = y;

  retval->next = NULL;
  return retval;
}

tree * cm_build_int_cst (tree * tt, int v)
{
  tree * retval;

  tree_common * val = (tree_common*) cm_malloc
    (sizeof(tree_common));
  val->type = INT_TYPE;
  val->o.integer = v;

  retval = INIT_TREE;
  retval->type = INTEGRAL_T;
  retval->field = tt;
  retval->opat = TREE_COM;
  retval->opa.tc = val;
  retval->opbt = 0;
  retval->opb.t = NULL_TREE;
  retval->next = NULL;

  return retval;
}

tree * cm_build_identifier_node (const char * s)
{
  tree * retval;

  tree_common * val = (tree_common*) cm_malloc(sizeof(tree_common));
  val->type = STR_TYPE;
  val->o.string = cm_strdup(s);

  retval = (tree*) cm_malloc(sizeof(tree));
  retval->type = IDENT_T;
  retval->field = NULL_TREE;

  retval->opat = TREE_COM;
  retval->opa.tc = val;

  retval->opbt = 0;
  retval->opb.t = NULL_TREE;

  retval->next = NULL;
  return retval;
}

/**
 * Tranform something like:
 *   for ( i=0; i<10; ++i ) { do_some stuff }
 *
 * into 3-address code instructions using a label something like:
 *
                      - i := 0;         ;; the initilizer
                      - goto label_for_loop__checks;
 * label_for_loop__start:
                      - do_some_stuff
                      - i := i + 1;     ;; the incrementor
 * label_for_loop__checks:
                      - if (i<10) { goto label_for_loop__start; }  ;; the loop_conditional
                      - { continue on with rest of program }
 **/

void cm_dot_transform_loop_for (tree ** decl)
{
  cm_assert ((*decl)->type == LOOP_FOR);
  return;
}

cm_vector_t * cm_transform_bin_expr (tree *d,tree *type)
{
  cm_vector_t * t1 = NULL, * t2 = NULL;

  tree *op = NULL;
  tree *opa, *opb, *t1op, *t2op, *tmp_var;

  opa= d->opa.t;
  opb= d->opb.t;

  t1 = cm_dot_transform_expression (&opa,type);
  t2 = cm_dot_transform_expression (&opb,type);
  
  t1op = (tree*) cm_vec_pop (t1);
  t2op = (tree*) cm_vec_pop (t2);
  tmp_var = create_new_tmp_var (type);

  switch( d->type )
    {
    default:
      {
        tree *o = build_decl (d->type,type,t1op,t2op);
        op = build_decl (MODIFY_EXPR,type,tmp_var,o);
      }
      break;
    }

  cm_vec_push (t1,op);
  cm_vec_push (t1,tmp_var);

  return t1;
}

cm_vector_t * cm_transform_modify_expr (tree *d,tree *type)
{
  tree * acc = d->opa.t;
  tree * rhs = d->opb.t;
  tree *var_decl, *rhs_tree;
  cm_vector_t * rhs_tree_vec;

  cm_assert (acc->type == IDENT_T);

  var_decl = cm_dot_lookup_var_decl (acc,context);
  if (!var_decl)
    {
      error ("un-declared variable <%s>!\n", acc->opa.tc->o.string);
    }

  rhs_tree_vec = cm_dot_transform_expression (&rhs,type);
  //tree *ast_type = cm_dot_analyse_expr_type (rhs);

  rhs_tree = (tree*) cm_vec_pop (rhs_tree_vec);

  d->opb.t = rhs_tree;
  cm_vec_push (rhs_tree_vec,d);

  return rhs_tree_vec;
}

/* Lets assume for now all expressions are x = 2 + 2...
   and not x = y = z...
 */
cm_vector_t * cm_dot_transform_expression (tree ** decl, tree * type)
{
  cm_vector_t * retval = NULL_VEC;
  tree * d = *decl;

  debug ("expr type is <0x%x>!\n", d->type);

  switch (d->type)
    {
    case IDENT_T:
      {
        tree * var_decl;
        cm_assert (d->opat == TREE_COM);
        var_decl = cm_dot_lookup_var_decl (d, context);
        if (!var_decl)
          error ("un-declared variable <%s>!\n", d->opa.tc->o.string);

        retval = VEC_INIT;
        cm_vec_init (retval);
        cm_vec_push (retval,d);
      }
      break;

    case VAR_DECL:
      {
        retval = VEC_INIT;
        cm_vec_init (retval);
        cm_vec_push (retval,d);
      }
      break;
    
    case INTEGRAL_T:
      {
        retval = VEC_INIT;
        cm_vec_init (retval);
        cm_vec_push (retval,d);
      }
      break;

    default:
      {
        switch (d->type)
          {
          case MODIFY_EXPR:
            retval = cm_transform_modify_expr (d,type);
            break;

          default:
            retval = cm_transform_bin_expr (d,type);
            break;
          }
      }
      break;
    }

  return retval;
}

tree * cm_dot_analyse_type (tree * d)
{
  tree * retval = NULL_TREE;

  switch (d->type)
    {
    case VAR_DECL:
      {
        tree * v = cm_dot_lookup_var_decl (d->opa.t,context);
        retval = v->field;
      }
      break;

    case IDENT_T:
      {
        tree * v = cm_dot_lookup_var_decl (d,context);
        if (!v)
          {
            error ("undeclared variable <%s>!\n",
                   d->opa.tc->o.string);
            return NULL_TREE;
          }
        retval = v->field;
      } 
      break;

    case INTEGRAL_T:
      retval = d->field;
      break;

    default:
      error ("unknown tree to resolve type!\n");
      retval = NULL;
      break;
    }

  return retval;
}

/**
 * Fairly Confusing Function to read.
 *
 * example:
 *    >>> x = y = z = 2 + 2 + 2;
 *
 *    --- Currently Yacc parses that expression into this Tree:

                      +
                     / \
                    +   2
                   / \
                  =   2
                 / \
                x   =
                   / \
                  y   =
                     / \
                    z   2

  -- Is converted into the procedure:

  1. z = 2 + 2 + 2;
  2. y = z;
  3. x = y;

  -- Tree structure as so:

                 =
                / \
               x   =
                  / \
                 y   =
                    / \
                   z   +
                      / \
                     +   2
                    / \
                   2   2
 **/
tree * cm_dot_process_expression_AST (tree **sym)
{
  tree *retval = NULL_TREE;
  tree *nn = NULL;
  if( (*sym)->next )
    {
      nn = (*sym)->next;
      (*sym)->next = NULL;
    }
  retval = (*sym);
  
  debug ("Processing Expression AST!\n");

  if( retval->type != MODIFY_EXPR )
    {
      tree *o = retval;
      tree *h = NULL;
      while( o != NULL )
        {
          if ((o->type != IDENT_T) ||
              (o->type != INTEGRAL_T) ||
              (o->type != VAR_DECL))
            {
              if (o->opat == TREE_T)
                {
                  if (o->opa.t->type == MODIFY_EXPR)
                    {
                      h = o; break;
                    }
                  else
                    {
                      o = o->opa.t;
                    }
                }
              else break;
            }
          else break;
        }
      if (h)
        {
          tree  *head = h->opa.t;
          if( head->opb.t->type == MODIFY_EXPR )
            {
              tree  *t = head, *m = NULL_TREE;
              while( t )
                {
                  if ((t->type != IDENT_T) ||
                      (t->type != INTEGRAL_T) ||
                      (t->type != VAR_DECL))
                    {
                      if( t->opb.t->type != MODIFY_EXPR )
                        {
                          m = t;
                          break;
                        }
                      else
                        {
                          t = t->opb.t;
                        }
                    }
                  else break;
                }
              
              if( m )
                {
                  h->opa.t = m->opb.t;
                  m->opb.t = retval;
                }
              else
                fatal ("error processing the expression AST!\n");
            }
          else
            {
              h->opa.t = head->opb.t;
              head->opb.t = retval;
            }
          retval = head;
        }
    }
  
  if( nn )
    retval->next = nn;

  (*sym) = retval;
  return retval;
}

/**
 * Lots of folding and checking to do:
 * example: int x = 1 + 2 + 3
 *
 * This needs split into several operations
 *
        - int x;
        - t1 := 1 + 2
        - t2 := t1 + 3
        - x := t2;
 **/
void cm_dot_check_expression (tree ** decl)
{
  cm_vector_t * v = NULL_VEC;
  tree *md = cm_dot_process_expression_AST (decl);

  tree * opa = md->opa.t;
  tree * type = cm_dot_analyse_type (opa);

  cm_assert (type);

  switch (md->type)
    {
    default:
      v = cm_dot_transform_expression (&md, type);
      break;
    }

  if (v)
    {
      long idx;
      tree * zero = VEC_index (tree*, v, 0);
      (*decl) = zero;
      (*decl)->field = type;
      cm_assert ((*decl)->type == MODIFY_EXPR);

      idx = 1;
      for ( ; idx<VEC_length(v); ++idx )
        {
          tree * i = VEC_index (tree*,v,idx);
          i->field = type;
          zero->next = i;
          zero = zero->next;
        }
    }
}

/**
 * Conditionals such as:

   if (x<5) { do_if_stuff } else { do_else_stuff }

   - Needs to get transformed into something like:

                   - if (x<5) goto label_if_block_start;
                   - else goto_else_block_start
 * label_if_block_start:
                   - do_if_stuff;
                   - goto condit_block_exit;
 * label_else_block_start:
                   - do_else_block;
 * label_codit_exit:
                   - continue_with_prog            
 **/
void cm_dot_check_conditional (tree ** decl)
{
  tree * ifb, *efb, *if_block, *else_block, *c_expr;
  tree * start, *exit, *c_next, *eval, *ifbn, *elbn, *elabel;

  cm_assert ((*decl)->type == CONDIT_EXPR);
  ifb = (*decl)->opa.t;
  efb = (*decl)->opb.t;

  cm_assert (ifb->opbt == TREE_T);
  if_block = ifb->opb.t;
  else_block = NULL_TREE;

  cm_assert (ifb->type == IF_BLOCK);
  c_expr = ifb->opa.t;
  cm_dot_check_expression (&c_expr);

  (*decl) = c_expr;

  start = build_new_label (CONDIT__L);
  exit = build_new_label (CONDIT__L);
  elabel = NULL_TREE;
  if (efb)
    {
      elabel = build_new_label (CONDIT__L);
      cm_assert (efb->opbt == TREE_T);
      else_block = efb->opb.t;
    }

  c_next = c_expr;
  while (c_next->next != NULL_TREE)
    {
      if (c_next->next->type == IDENT_T)
        break;
      c_next = c_next->next;
    }
  eval = c_next->next;

  ifbn = build_decl (IF_BLOCK, eval,
                            build_decl (GOTO,start,NULL_TREE,NULL_TREE),
                            NULL_TREE);
  elbn = NULL_TREE;
  if (elabel)
    {
      elbn = build_decl (ELSE_BLOCK, NULL_TREE,
                         build_decl (GOTO, elabel, NULL_TREE, NULL_TREE),
                         NULL_TREE);
    }
  else
    {
      elbn = build_decl (ELSE_BLOCK, NULL_TREE,
                         build_decl (GOTO, exit, NULL_TREE, NULL_TREE),
                         NULL_TREE);
    }

  c_next->next = build_decl (CONDIT_EXPR,NULL_TREE,ifbn, elbn);
  c_next = c_next->next;

  c_next->next = start;
  c_next = c_next->next;

  cm_dot_check_stmt_list (&if_block);
  c_next->next = if_block;
  c_next = c_next->next;

  while (c_next->next != NULL_TREE)
    c_next = c_next->next;

  c_next->next = build_decl (GOTO,exit, NULL_TREE,NULL_TREE);
  c_next = c_next->next;

  if (elabel)
    {
      c_next->next = elabel;
      c_next = c_next->next;

      cm_dot_check_stmt_list (&else_block);
      c_next->next = else_block;
      while (c_next->next != NULL_TREE)
        c_next = c_next->next;
    }

  c_next->next = exit;
}

bool cm_dot_push_var_decl (tree *decl, cm_vector_t * const v, bool c)
{
  bool retval = true;
  cm_hashval_t h;
  void ** e;

  struct TU_context_t * curr_ctx = VEC_index (struct TU_context_t *,
                                              v,v->length-1);

  debug ("pushing var decl <%s>!\n", decl->opa.t->opa.tc->o.string);
  h = cm_hash_string (decl->opa.t->opa.tc->o.string);
  e = cm_hash_insert (h,decl,curr_ctx->table);

  if (e)
    {
      error ("variable declaration already defined: <%s>!\n",
             decl->opa.tc->o.string);
      retval = false;
    }

  if (!c)
    {
      /* create clone of decl and append to the decl_list */
          tree * t;
      tree * clone = INIT_TREE;
      clone->type = VAR_DECL;
      clone->field = decl->field;
      clone->opa.t = build_identifier_node (decl->opa.t->opa.tc->o.string);
      clone->opb.t = NULL_TREE;
      clone->next = NULL;

      t = (*curr_ctx->var_decl_block);
      while (t->next != NULL)
        t = t->next;

      t->next = clone;
    }

  return retval;
}

tree * cm_dot_lookup_var_decl (tree * decl, cm_vector_t * const v)
{
  tree * retval;
  struct TU_context_t * curr_ctx;
  cm_hashval_t h;
  cm_hash_entry_t * e;

  cm_assert (decl->type == IDENT_T);
  retval = NULL_TREE;
  curr_ctx = VEC_index (struct TU_context_t*,v,v->length-1);

  h = cm_hash_string (decl->opa.tc->o.string);
  e = cm_hash_lookup_table (curr_ctx->table,h);
  if (e)
    {
      if (e->data)
        retval = (tree*) e->data;
    }
  else
    error ("var decl <%s> is undefined!\n",
           decl->opa.tc->o.string);

  return retval;
}

void cm_dot_check_call (tree * decl)
{
  tree * id;
  char *n;
  cm_hashval_t h;
  cm_hash_entry_t * e;

  cm_assert (decl->type == CALL_EXPR);
  cm_assert (decl->opa.t->type == IDENT_T);

  id = decl->opa.t;
  n = id->opa.tc->o.string;
 
  h = cm_hash_string (n);
  debug ("looking up function <%s> hash <0x%x>!\n", n, h);
  e = cm_hash_lookup_table (dot_global_namespace, h);

  if (!e)
    {
      error ("undefined function <%s>!\n", n);
    }
  else
    {
      if (!e->data)
        error ("undefined function <%s>!\n", n);
    }
}

void cm_dot_check_stmt_list (tree ** decl)
{
  tree * head = (*decl);
  tree * block = head;
  tree * pointer = NULL_TREE;

  bool first = true;
  while (block)
    {
      tree * nn;
      debug ("checking stmt <%p> of type <0x%x>!\n",
             (void*)block, block->type);

      nn = block->next;
      block->next = NULL;

      switch (block->type)
        {
        case CALL_EXPR:
          cm_dot_check_call (block);
          break;

        case CONDIT_EXPR:
          cm_dot_check_conditional (&block);
          break;

        default:
          cm_dot_check_expression (&block);
          break;
        }

      if (first)
        {
          (*decl) = block;
          first = false;
        }
      
      if (pointer)
        pointer->next = block;
      
      while (block->next)
        block = block->next;
      block->next = nn;
      
      pointer = block;
      block = block->next;
    }
}

void cm_dot_check_function (tree *decl)
{
  const char * n;
  cm_hashval_t h;
  cm_hash_entry_t * e;

  tree * var_decls_list_head, * var_decls;
  tree * block, *pointer;

  cm_assert (decl->type == FUNC_DECL);

  n = decl->field->opa.tc->o.string;
  h = cm_hash_string (n);
  /* Look for previous definitions ... */

  debug ("processing function <%s>:<0x%x>!\n", n,h);
  e = cm_hash_lookup_table (dot_global_namespace, h);
  if (e)
    {
      if (e->data)
        {
          error ("function <%s> already defined!\n", n);
          return;
        }
      else
        {
          void ** e = cm_hash_insert (h, decl, dot_global_namespace);
          if (e)
            fatal("error pushing function defintion into translation unit!\n");
        }
    }
  else
    {
      void ** e = cm_hash_insert (h,decl,dot_global_namespace);
      if (e)
        fatal("error pushing function defintion into translation unit!\n");
    }

  CM_DOT_PUSH_NEW_CTX (&(decl->opa.t));

  var_decls_list_head = decl->opa.t;
  var_decls = var_decls_list_head;

  while (var_decls)
    {
      tree * type, * ident_list_head, * ident_list;

      cm_assert (var_decls->type == VAR_DECL);
      debug ("trying to process VAR_DECL <%p>!\n", (void*)var_decls);

      type = var_decls->field;
      ident_list_head = var_decls->opa.t;
      ident_list = ident_list_head;

      while (ident_list)
        {
          char * vn = ident_list->opa.tc->o.string;
          tree * v;
          debug ("ident <%s>!\n", vn);

          v = build_decl (VAR_DECL, type,
			  build_identifier_node (vn),
			  NULL_TREE);
          if (!cm_dot_push_var_decl (v,context,true))
            {
              CM_DOT_POP_CTX;
              return;
            }

          ident_list = ident_list->next;
        }

      var_decls = var_decls->next;
    }

  block = decl->opb.t;

  pointer = NULL_TREE;
  cm_assert (block->type == BLOCK_DECL);
  block = block->opa.t;
  tree * n_head = NULL_TREE;

  while (block)
    {
      tree * nn;
      debug ("checking stmt <%p> of type <0x%x>!\n",
             (void*)block, block->type);

      nn = block->next;
      block->next = NULL;

      switch (block->type)
        {
        case CALL_EXPR:
          cm_dot_check_call (block);
          break;

        case CONDIT_EXPR:
          cm_dot_check_conditional (&block);
          break;

        default:
          cm_dot_check_expression (&block);
          break;
        }

      if (pointer)
        pointer->next = block;
      if (!n_head)
	n_head = block;
      
      while (block->next)
        block = block->next;
      block->next = nn;
      
      pointer = block;
      block = block->next;
    }
  decl->opb.t->opa.t = n_head;

  CM_DOT_POP_CTX;
}

void cm_dot_check_decl_node (tree *decl)
{
  switch (decl->type)
    {
    case FUNC_DECL:
      cm_dot_check_function (decl);
      break;

    default:
      break;
    }
}

void cm_dot_check_translation_unit (cm_vector_t * const tv)
{
  /* Loop through do some optimizations and some static analysis */
  debug ("checking translation unit...!\n");

  /* setup global context */
  CM_DOT_PUSH_NEW_CTX (NULL);

  if (tv)
    {
      size_t length = tv->length;
      int idx;
      for (idx = 0; idx < length; ++idx)
        {
          cm_dot_check_decl_node (tree_vec_index (tv,idx));
        }
    }
}

void cm_dot_dump_translation_unit (cm_vector_t * const tv,
                                   const char * out )
{
  FILE * fd;
  debug ("trying to output translation unit to <%s>!\n", out);

#ifndef WIN32
  fd = fopen (out,"w");
  if (!fd)
    {
      error("unable to  open <%s> for write!\n", out);
      return;
    }
#else
  if(fopen_s (&fd, out, "w"))
    {
      error("unable to  open <%s> for write!\n", out);
      return;
    }
#endif

  if (tv)
    {
      long length = tv->length;
      long idx = 0;
      
      for( ; idx<length; ++idx )
        {
          cm_dot_dump_node (fd, VEC_index(tree*,tv,idx));
          fprintf (fd,"\n");
        }
    }
  fprintf (fd, ";; Compiled on <%s:%s>!\n", SYSTEM_TYPE, MACHINE_TYPE);
  fprintf (fd, ";; Mod-dot Representation!\n\n");
  fclose (fd);
}

void cm_dot_dump_integral_val (FILE *fd, tree *decl)
{
  tree_common * val = decl->opa.tc;

  switch (val->type)
    {
    case INT_TYPE:
      if (decl->field == integer_type_node)
        fprintf (fd, "%i", val->o.integer );
      else if (decl->field == boolean_type_node)
        {
          int v = val->o.integer;
          if (v)
            fprintf (fd, "True");
          else
            fprintf (fd, "False");
        }
      else
        fatal ("unhandled type <%p>!\n", (void*)decl->field)
      break;

    case STR_TYPE:
      fprintf (fd, "%s", val->o.string );
      break;

    default:
      fatal ("unhandled integral type <0x%x>!\n", val->type);
      break;
    }
}

void cm_dot_dump_expr_node (FILE *fd, tree *decl)
{
  switch (decl->type)
    {
    case INTEGRAL_T:
      cm_dot_dump_integral_val (fd, decl);
      break;

    case IDENT_T:
      cm_dot_dump_integral_val (fd, decl);
      break;

    case ACCESSOR:
      cm_dot_dump_integral_val (fd, decl->opa.t);
      fprintf (fd, "[");
      cm_dot_dump_integral_val (fd, decl->opb.t);
      fprintf (fd, "] ");
      break;

    case ADD_EXPR:
      cm_dot_dump_expr_node (fd, decl->opa.t);
      fprintf (fd, " + ");
      cm_dot_dump_expr_node (fd, decl->opb.t);
      break;

    case LESS_EXPR:
      cm_dot_dump_expr_node (fd, decl->opa.t);
      fprintf (fd, " < ");
      cm_dot_dump_expr_node (fd, decl->opb.t);
      break;

    default:
      fatal ("unhandled decl type <0x%x>!\n", decl->type);
      break;
    }
}

void cm_dot_dump_arguments (FILE *fd, tree *decl)
{
  if (decl)
    {
      fprintf (fd, "<arguments>");
    }
}

void cm_dot_dump_stmt (FILE *fd, tree * decl, int indents)
{
  bool semi = true;

  int idx = 0;
  for ( ; idx<indents; ++idx)
    fprintf (fd, "\t");

  switch (decl->type)
    {
    case MODIFY_EXPR:
      cm_assert (decl->opa.t->type == IDENT_T);
      cm_dot_dump_integral_val (fd, decl->opa.t);
      fprintf(fd, " = ");
      cm_dot_dump_expr_node (fd, decl->opb.t);
      break;

    case CALL_EXPR:
      cm_assert (decl->opa.t->type == IDENT_T);
      cm_dot_dump_integral_val (fd, decl->opa.t);
      fprintf (fd, " (");
      cm_dot_dump_arguments (fd, decl->opb.t);
      fprintf (fd, ")");
      break;

    case GOTO:
      {
        tree * label = decl->field;
        tree * ident;
        cm_assert (label->type == LABEL);
        ident = label->field;
        cm_assert (ident->type == IDENT_T);
        fprintf (fd, "goto ");
        cm_dot_dump_integral_val (fd, ident);
      }
      break;

    case LABEL:
      {
	tree * ident;
        fprintf (fd, "\n");
        ident = decl->field;
        cm_assert (ident->type == IDENT_T);
        cm_dot_dump_integral_val (fd, ident);
        fprintf (fd, ":\n");
        semi = false;
      }
      break;

    case CONDIT_EXPR:
      {
        tree * ifb = decl->opa.t;
        tree * elb = decl->opb.t;

        fprintf (fd, "if (");
        cm_dot_dump_integral_val (fd, ifb->field);
        fprintf (fd, ")\n\t  {\n");
        cm_dot_dump_stmt (fd, ifb->opa.t, indents+1);
        fprintf (fd, "\t  }");

        if (elb)
          {
            fprintf (fd, "\n");
            
            for ( idx=0; idx<indents; ++idx)
              fprintf (fd, "\t");

            fprintf (fd, "else");
            fprintf (fd, " \n\t  {\n");
            cm_dot_dump_stmt (fd, elb->opa.t, indents+1);
            fprintf (fd, "\t  }");
          }
        fprintf (fd, "\n");
        semi = false;
      }
      break;

    default:
      fatal ("unhandled decl type <0x%x>!\n", decl->type);
      break;
    }

  if (semi)
    fprintf(fd,";\n");
}

void cm_dot_dump_func_block (FILE *fd, tree * block, int indents)
{
  cm_assert (block->type == BLOCK_DECL);
  block = block->opa.t;
  while (block)
    {
      cm_dot_dump_stmt (fd,block,indents);
      block = block->next;
    }
}

void cm_dot_dump_func_decl (FILE *fd, tree *decl)
{
  tree * var_decls, * block;
  fprintf (fd,"void %s ( void )\n", decl->field->opa.tc->o.string);
  fprintf (fd,"{\n");
  
  /* Var_decls first */
  var_decls = decl->opa.t;
  while (var_decls)
    {
      tree * ident_list;
      fprintf (fd,"\t");

      if (var_decls->field->type == TYPE_ARY_DCL)
	{
	  tree * base_type = var_decls->field->field;
	  tree * x_r = base_type->opa.t;
	  tree * y_r = base_type->opb.t;

	  cm_assert (x_r->opat == TREE_COM);
	  int x = x_r->opa.tc->o.integer;
	  cm_assert (y_r->opat == TREE_COM);
	  int y = y_r->opa.tc->o.integer;

	  if (base_type == integer_type_node)
	    fprintf (fd, "int [%i...%i] ", x, y);
	  else if (base_type == boolean_type_node)
	    fprintf (fd, "bool [%i...%i] ", x, y);
	  else
	    fatal ("unhandled type!\n");
	}

      if (var_decls->field == integer_type_node)
        fprintf (fd,"int ");
      else if (var_decls->field == boolean_type_node)
        fprintf (fd,"bool ");
      else
        fatal ("unhandled type!\n");

      ident_list = var_decls->opa.t;
      while (ident_list)
        {
          fprintf (fd,"%s, ", ident_list->opa.tc->o.string);
          ident_list = ident_list->next;
        }
      fseek (fd, -2, SEEK_CUR);
      fprintf (fd,"; \n");

      var_decls = var_decls->next;
    }
  fprintf (fd, "\n");

  block = decl->opb.t;
  cm_assert (block->type == BLOCK_DECL);

  cm_dot_dump_func_block (fd, block, 1);

  fprintf (fd,"}\n");
}

void cm_dot_dump_node (FILE *fd, tree *node)
{
  switch (node->type)
    {
    case FUNC_DECL:
      cm_dot_dump_func_decl (fd,node);
      break;

    default:
      error ("unhandled type <0x%x>!\n", node->type);
      break;
    }
}
