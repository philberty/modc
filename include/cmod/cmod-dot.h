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

#ifndef _CMOD_DOT_H__
#define _CMOD_DOT_H__

extern cm_vector_t * dot_types;
extern cm_vector_t * translation_unit;
extern cm_hash_tab_t * dot_global_namespace;

typedef unsigned short opcode_t;
typedef struct cm_tree_common_dot_t {
  opcode_t type;
  union {
    int integer;
    unsigned char c;
    char * string;
  } o;
} tree_common ;

typedef struct cm_tree_dot_t {
  opcode_t type, opat, opbt;
  struct cm_tree_dot_t * field;
  union {
    struct cm_tree_dot_t * t;
    tree_common * tc;
    cm_vector_t * v;
  } opa;
  union {
    struct cm_tree_dot_t * t;
    tree_common * tc;
    cm_vector_t * v;
  } opb;
  struct cm_tree_dot_t * next;
} tree ;
#define NULL_TREE  (tree*)0

#define build_int_cst(x,y)                      \
  cm_build_int_cst (x,y)
#define build_identifier_node(x)                \
  cm_build_identifier_node (x)
#define build_function_decl(x,y)                \
  cm_build_function_decl (x->opa.t,x->opb.t,y)

#define integer_type_node                       \
  (tree*) cm_vec_index (dot_types,0)
#define boolean_type_node                       \
  (tree*) cm_vec_index (dot_types,1)
#define char_type_node                          \
  (tree*) cm_vec_index (dot_types,2)

#define INIT_TREE                               \
  (tree*) cm_malloc(sizeof(tree))

#define  VAR_DECL     0xF001
#define  FUNC_DECL    0xF002
#define  BLOCK_DECL   0xF003
#define  TYPE_DECL    0xF004
#define  MODIFY_EXPR  0xF005
#define  ADD_EXPR     0xF006
#define  SUB_EXPR     0xF007
#define  CALL_EXPR    0xF008
#define  INT_TYPE     0xF009
#define  BOL_TYPE     0xF00A
#define  CHR_TYPE     0xF00B
#define  STR_TYPE     0xF00C
#define  INTEGRAL_T   0xF00D
#define  INT_TYPE_T   0xF00E
#define  BOL_TYPE_T   0xF00F
#define  IDENT_T      0xF011
#define  LOOP_FOR     0xF012
#define  LOOP_L       0xF013
#define  LABEL        0xF014
#define  GOTO         0xF015
#define  LESS_EXPR    0xF016
#define  EXIT_STMT    0xF017
#define  ELSE_BLOCK   0xF018
#define  IF_BLOCK     0xF019
#define  CONDIT_EXPR  0xF01A
#define  TREE_COM     0xF01B
#define  TREE_T       0xF01C
#define  TYPE_ARY_DCL 0xF01D
#define  ACCESSOR     0xF01E

struct TU_context_t {
  cm_hash_tab_t * table;
  tree ** var_decl_block;
} ;

#define CM_DOT_PUSH_NEW_CTX(x)                  \
  do {                                          \
    cm_hash_tab_t * ctx;                        \
    struct TU_context_t * c;                    \
    debug ("pushing new ctx!\n");               \
    ctx = (cm_hash_tab_t*)                      \
      cm_malloc (sizeof (cm_hash_tab_t));       \
    cm_hash_init_table (ctx);			\
    c = (struct TU_context_t*)                  \
      cm_malloc (sizeof(struct TU_context_t));  \
    c->table = ctx;                             \
    c->var_decl_block = x;                      \
    cm_vec_push (context,c);                    \
  } while (0)

#define CM_DOT_POP_CTX                          \
  do {                                          \
    struct TU_context_t * ctx;                  \
    debug ("poping context!\n");                \
    ctx = (struct TU_context_t*)                \
      cm_vec_pop (context);                     \
    cm_dot_clear_table_data (ctx->table);       \
    cm_hash_free_table (ctx->table);		\
    cm_free (ctx);                              \
  } while (0)

enum label_t {
  LOOP__L,
  CONDIT__L,
  VAR__L,
  DATA_L
};

extern void cm_dot_init_types (void);
extern tree * build_decl (opcode_t, tree *, tree *, tree *);
extern tree * cm_build_int_cst (tree *,int);

extern void cm_dot_check_translation_unit (cm_vector_t * const);
extern void cm_dot_dump_translation_unit (cm_vector_t * const, const char *);
extern tree * cm_build_identifier_node (const char *);

extern void cm_dot_dump_node (FILE *, tree *);
extern void cm_dot_push_function (tree *);
extern tree * cm_build_function_decl (tree *,tree *, tree *);

extern void cm_dot_build_push_main_function (tree *);
extern void cm_dot_dump_integeral_val (FILE *, tree *);
extern void cm_dot_clear_table_data (cm_hash_tab_t * const);

extern void cm_dot_check_stmt_list (tree **);
extern tree * build_new_label (enum label_t);
extern tree * cm_dot_create_array_type (tree *, tree *, tree *);

#endif /* _CMOD_DOT_H__ */
