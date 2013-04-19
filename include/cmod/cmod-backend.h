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

#ifndef _CMOD_BACKEND_H__
#define _CMOD_BACKEND_H__

/**
 * Lets keep track of whats going on in our cpu reg's
 * so we can choose an apropriate reg to select for usage.
 *
 * EAX, EBX, ECX, EDX, EBP, ESP, ESI, and EDI.
 **/
typedef struct TG_register__ {
  int val;
  bool usage;
  const char * ident;
} TG_register__ ;

struct TU_symbol__ {
  int offset;
  int index;
  int size;
  char * ident;
} ;

enum REG_T { REG, HIGH, LOW };

#define AQUIRE_REGISTER(X, I, S)					\
  for (I = S; I < 4; ++I)						\
    {									\
      TG_register__ i_ = cpu_state[I];					\
      if (!(i_.usage))							\
	{								\
          X = &i_;							\
	  break;							\
	}								\
    }									\
  cm_assert (X); last_aquired_reg_idx = I;

#define CLEAR_LAST_REG(I)						\
  do {									\
    if ((I>=0) || (I<4))						\
      {									\
	cpu_state[I].usage = false;					\
	cpu_state[I].val = 0;						\
      }									\
    else								\
      fatal ("index <%i> is out of bounds on cpu state!\n", I);		\
  } while (0)

extern void cm_target_gen_ASM__ (cm_vector_t * const, const char *,
				 const char * in);
extern char * cm_target_fold_Integral (FILE *, tree *, tree *);

#endif /* _CMOD_BACKEND_H__ */
