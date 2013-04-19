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

#ifndef _VECTORS_H__
#define _VECTORS_H__

typedef struct cm_vector_t {
  void ** vector;
  signed long size, length;
} cm_vector_t;

typedef unsigned int cm_hashval_t;
typedef struct cm_hash_entry {
  cm_hashval_t hash;
  void * data;
} cm_hash_entry_t ;

typedef struct cm_hash_table_t {
  unsigned int size, length;
  cm_hash_entry_t * array;
} cm_hash_tab_t ;

extern cm_hashval_t cm_hash_string (const char *);
extern cm_hash_entry_t * cm_hash_lookup_table (cm_hash_tab_t *, cm_hashval_t);

extern void ** cm_hash_insert (cm_hashval_t, void *, cm_hash_tab_t *);
extern void cm_hash_grow_table (cm_hash_tab_t *);

extern void cm_hash_init_table (cm_hash_tab_t *);
extern void cm_hash_free_table (cm_hash_tab_t *);

extern void cm_vec_init (cm_vector_t * const);

extern void cm_vec_push (cm_vector_t * const, void *);
extern void * cm_vec_pop (cm_vector_t * const);

#define VEC_INIT                                        \
  (cm_vector_t*) cm_malloc (sizeof(cm_vector_t))

#define NULL_VEC (cm_vector_t*)0
#define VEC_length(x)  x->length

#define VEC_index(T,V,I)                                        \
  (T)cm_vec_index_diag (V,(int)I,__FILE__,__LINE__,__func__)

#define tree_vec_index(x,y)  VEC_index(tree*,x,y)

extern void * cm_vec_index (cm_vector_t * const, int);
extern void * cm_vec_index_diag (cm_vector_t * const, int,
                                 const char *, unsigned int,
                                 const char *);
extern void cm_vec_free (cm_vector_t *);

#endif /* _VECTORS_H__ */
