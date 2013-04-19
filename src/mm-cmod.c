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

int f_syntax_only = 0;

static void cm_print_version (void);
static void cm_print_help (const char *);
static int cm_cmod (const char *);

static
int cm_cmod( const char * in )
{
  return cm_do_compile (in);
}

static
void cm_print_version( void )
{
  fprintf (stdout, "Cmod Modula Compiler-i386 version: %s\n", VERSION);
  fprintf (stdout, "Copyright (c) Philip Herron 2010\n");

#ifndef CMAKE
  fprintf (stdout, "Compiled for: %s %s\n", SYSTEM_TYPE, MACHINE_TYPE);
  fprintf (stdout, "Report suggestions and bugs to: %s\n", PACKAGE_BUGREPORT);
#endif
  
  fprintf (stdout, "\nThis is free software; see the source for copying conditions.  There is NO\n");
  fprintf (stdout, "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n");
}

static
void cm_print_help (const char * arg)
{
  fprintf (stdout, "Cmod usage: %s [OPTION]... [FILE]...\n", arg);
  fprintf (stdout, "  -h, --help    \t prints this help\n");
  fprintf (stdout, "  -v, --version \t prints the version\n");
  fprintf (stdout, "  <file.mod>    \t compiles file\n\n");
}

int main( int argc, char *argv[] )
{
  unsigned exit= EXIT_SUCCESS;
  if( argc == 1 )
    {
      fprintf (stderr, "no specified input file!\n");
      cm_print_help (argv[0]);
    }
  else
    {
      int i= 1;
      for( ; i<argc; ++i )
        {
          if( strcmp("--version", argv[i]) == 0 )
            {
              cm_print_version ();
              break;
            }
          else if( strcmp("--help", argv[i]) == 0 )
            {
              cm_print_help (argv[0]);
              break;
            }
          /* assume file for now just super basic
	     arg parsing for now...
	  */
          else
            {
              exit = (cm_cmod (argv[i]));
              break;
            }
        }
    }
  return exit;
}
