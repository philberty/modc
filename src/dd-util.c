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

unsigned int error_count = 0;

inline
void * cm_malloc (size_t s)
{
  register void * retval = (void*) malloc (s);
  if (!retval)
    fatal("Exhausted Virtual Memory!\n");
  return retval;
}

inline
void * cm_realloc (void * v, size_t s)
{
  register void * retval = (void*) realloc (v,s);
  if (!retval)
    fatal("Exhausted Virtual Memory!\n");
  return retval;
}

inline
void * cm_calloc (size_t i, size_t l)
{
  register void * retval = (void*) calloc (i,l);
  if (!retval)
    fatal ("Exhausted Virtual Memory!\n");
  return retval;
}

inline
char * cm_strdup (const char * str)
{
#ifdef HAVE_STRDUP
  return (strdup (str));
#else
  register size_t len= (strlen( str )+1);
  register char *ret= cm_alloc (char*,len);
  memcpy (ret, str, len);
  return ret;
#endif
}

inline
void __cm_debug__( const char* file, unsigned int line,
                   const char* functor, const char* fmt, ... )
{
#ifdef DEBUG
  fprintf (stderr, "debug: <%s:%s:%i> -> ",
           file, functor, line);
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
#else
  return;
#endif
}

inline
void __cm_error__( const char* file, unsigned int line,
                   const char* functor, const char* fmt, ... )
{
  va_list v_args;
#ifdef DEBUG
  fprintf (stderr, "error: <%s:%s:%i> -> ",
           file, functor, line);
#else
  fprintf (stderr, "error: -> ");
#endif
  va_start (v_args, fmt);
  vfprintf (stderr, fmt, v_args);
  va_end (v_args);

  error_count++;
}

inline
void __cm_fatal__( const char* file, unsigned int line,
                   const char* functor, const char* fmt, ... )
{
  va_list args;
#ifdef DEBUG
  fprintf (stderr, "fatal: <%s:%s:%i> -> ",
           file, functor, line);
#else
  fprintf (stderr, "error: -> ");
#endif
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);

  exit (EXIT_FAILURE);
}

/**
 * a way of overiding assert.h to give a more
 * specific assertion method..
 **/
inline
void cm_assertion_failed( const char * assertion, unsigned int line,
                          const char * file, const char * functor )
{
  fprintf (stdout, "fatal: <%s:%s:%i> -> assertion '%s' failed exiting...\n",
           file, functor, line, assertion);
  exit (EXIT_FAILURE);
}
