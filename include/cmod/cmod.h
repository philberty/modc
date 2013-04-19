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

#ifndef _CMOD_H__
#define _CMOD_H__

#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

#ifdef DEBUG
# define cm_assert( expr )                                              \
  ((expr) ? (void) 0 : cm_assertion_failed( #expr,                      \
                                            __LINE__,                   \
                                            __FILE__,                   \
                                            __func__ ));
#else
# define cm_assert( expr )
#endif

#ifdef GNUC
# define __cm_check_fmt(x,y)                    \
  __attribute__ ((format (printf,x,y)))
#else
# define __cm_check_fmt(x,y)
#endif

#ifndef HAVE_STDBOOL_H
typedef unsigned char bool;
#define true 1
#define false 0
#else
# include <stdbool.h>
#endif

#ifdef _WIN32
# define inline __inline
# include <windows.h>
#endif

extern unsigned int error_count;
extern int f_syntax_only;

extern void * cm_malloc (size_t);
extern void * cm_realloc (void *, size_t);
extern void * cm_calloc (size_t, size_t);

#define cm_alloc(T,S)     (T) cm_malloc (S)
#define cm_rlloc(T,X,S)   (T) cm_realloc ((void*) X,S)
#define cm_clloc(T,S1,S2) (T) cm_calloc (S1,S2)

#ifdef DEBUG
# define cm_free( __xp )                             \
  cm_assert (__xp);                                  \
  free (__xp);                                       \
  __xp = NULL;
#else
#define cm_free( __xp )                         \
  free (__xp);                                  \
  __xp = NULL;
#endif

/* 2 of the driver functions ...*/

/* ii-toplev.c */
extern int cm_do_compile (const char *);
/* ss-lexer.l */
extern int cm_parse_file (const char *);

/* dd-util.c */
extern char * cm_strdup( const char * );
extern void cm_assertion_failed( const char *, unsigned int,
                                  const char *, const char * );

extern void
__cm_debug__( const char *, unsigned int,
               const char *, const char *, ... )
 __cm_check_fmt(4,5) ;

extern void
__cm_error__( const char *, unsigned int,
               const char *, const char *, ... )
 __cm_check_fmt(4,5) ;

extern void __cm_fatal__( const char *, unsigned int,
               const char *, const char *, ... )
 __cm_check_fmt(4,5) ;



#ifdef DEBUG
# define debug( ... )                                           \
  __cm_debug__( __FILE__, __LINE__, __func__, __VA_ARGS__ );
#else
# define debug( ... ) ;
#endif

#define error( ... )                                            \
  __cm_error__( __FILE__, __LINE__, __func__, __VA_ARGS__ );

#define fatal( ... )                                            \
  __cm_fatal__( __FILE__, __LINE__, __func__, __VA_ARGS__ );

#endif /* _CMOD_H__ */
