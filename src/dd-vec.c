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
#include <stdarg.h>

#include <cmod/cmod.h>
#include <cmod/vectors.h>
#include <cmod/cmod-dot.h>

#define threshold_alloc(x) (((x)+16)*3/2)

/* Hash P as a null-terminated string.

   Copied from gcc/hashtable.c.  Zack had the following to say with respect
   to applicability, though note that unlike hashtable.c, this hash table
   implementation re-hashes rather than chain buckets.

   http://gcc.gnu.org/ml/gcc-patches/2001-08/msg01021.html
   From: Zack Weinberg <zackw@panix.com>
   Date: Fri, 17 Aug 2001 02:15:56 -0400

   I got it by extracting all the identifiers from all the source code
   I had lying around in mid-1999, and testing many recurrences of
   the form "H_n = H_{n-1} * K + c_n * L + M" where K, L, M were either
   prime numbers or the appropriate identity.  This was the best one.
   I don't remember exactly what constituted "best", except I was
   looking at bucket-length distributions mostly.
   
   So it should be very good at hashing identifiers, but might not be
   as good at arbitrary strings.
   
   I'll add that it thoroughly trounces the hash functions recommended
   for this use at http://burtleburtle.net/bob/hash/index.html, both
   on speed and bucket distribution.  I haven't tried it against the
   function they just started using for Perl's hashes.  */

cm_hashval_t
cm_hash_string (const char * s)
{
  const unsigned char *str = (const unsigned char *) s;
  cm_hashval_t r = 0;
  unsigned char c;

  while ((c = *str++) != 0)
    r = r * 67 + c - 113;

  return r;
}

cm_hash_entry_t *
cm_hash_lookup_table (cm_hash_tab_t * tbl, cm_hashval_t h)
{
  cm_hash_entry_t* retval = NULL;
  if (tbl->array)
    {
      cm_hashval_t size= tbl->size, idx= (h % size);
      cm_hash_entry_t *array= tbl->array;

      while (array[idx].data)
        {
          if (array[idx].hash == h)
            break;

          idx++;
          if (idx >= size)
            idx= 0;
        }
      retval = (array+idx);
    }
  else
    retval= NULL;

  return retval;
}

void ** cm_hash_insert (cm_hashval_t h, void * obj,
                        cm_hash_tab_t * tbl )
{
  void **retval = NULL;
  cm_hash_entry_t *entry = NULL;
  if (tbl->length >= tbl->size)
    cm_hash_grow_table (tbl);

  entry = cm_hash_lookup_table (tbl, h);
  if (entry->data)
    retval = &(entry->data);
  else
    {
      entry->data = obj;
      entry->hash = h;
      tbl->length++;
    }
  return retval;
}

void cm_hash_grow_table (cm_hash_tab_t * tbl)
{
  unsigned int prev_size = tbl->size, size = 0, i;
  cm_hash_entry_t *prev_array = tbl->array, *array;

  size = threshold_alloc (prev_size);
  array = (cm_hash_entry_t*)
    cm_calloc (size, sizeof(cm_hash_entry_t));

  tbl->length = 0;
  tbl->size= size;
  tbl->array= array;

  for (i = 0; i<prev_size; ++i)
    {
      cm_hashval_t h = prev_array[i].hash;
      void *s = prev_array[i].data;

      if (s)
        cm_hash_insert (h, s, tbl);
    }
  if (prev_array)
    cm_free (prev_array);
}

inline
void cm_hash_init_table (cm_hash_tab_t * tbl)
{
  tbl->size = 0;
  tbl->length = 0;
  tbl->array = NULL;
}

void cm_dot_free_t_var_decl (tree *decl)
{
  /*
  if (decl)
    {
      tree_common *tc = decl->opa.tc;
      cm_assert (tc->type == STR_TYPE);
      cm_free (tc->o.string);
      cm_free (tc);
      cm_free (decl);
      }*/
  return;
}

void cm_dot_clear_table_data (cm_hash_tab_t * const tbl )
{
  cm_hash_entry_t * array = tbl->array;
  size_t idx = 0;
  for ( ; idx<tbl->size; ++idx )
    {
      cm_hash_entry_t e = array[idx];
      tree * t = (tree*) e.data;
      if (t)
        cm_dot_free_t_var_decl (t);
    }
}

inline
void cm_hash_free_table (cm_hash_tab_t * tbl)
{
  cm_hash_tab_t *tb = tbl;

  if (tb->array)
    cm_free (tb->array);

  cm_free (tb);
}

void cm_vec_init (cm_vector_t * const v)
{
  v->size = threshold_alloc (0);
  v->vector = (void**) cm_calloc (v->size, sizeof (void *));
  v->length = 0;
}

void cm_vec_push( cm_vector_t * const v,  void * s )
{
  if (s)
    {
      if (v->length >= v->size-1)
        {
          signed long size = threshold_alloc( v->size );
          v->vector = (void**) cm_realloc( v->vector, size*sizeof(void*) );
          v->size = size;
        }
      v->vector [v->length] = s;
      v->length++;
    }
}

void * cm_vec_pop( cm_vector_t * const v )
{
  void * retval = NULL;
  int idx = v->length-1;
  if (idx>=0)
    {
      retval = v->vector[ v->length-1 ];
      v->length--;
    }
  return retval;
}

void * cm_vec_index (cm_vector_t * const v, int i)
{
  void * retval = NULL;
  if ((i>=0) || (i<v->length))
    {
      retval = v->vector[i];
    }
  else
    fatal("index <%i> out of bounds on vector <%p>!\n", i, (void*)v );
  return retval;
}

void * cm_vec_index_diag (cm_vector_t * const v, int i,
                          const char* file, unsigned int line,
                          const char* functor)
{
  void * retval = NULL;
  if ((i>=0) || (i<(v->length)))
    {
      retval = v->vector[i];
    }
  else
    {
      fprintf (stderr, "fatal: <%s:%s:%i> -> index <%i> out of bounds on vector <%p>!\n",
               file, functor, line, i, (void*)v);
      exit( EXIT_FAILURE );
    }

  return retval;
}

void cm_vec_free (cm_vector_t * v)
{
  if (v->vector)
    cm_free (v->vector);

  cm_free (v);
}
