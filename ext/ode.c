/*
 *		ode.c - ODE Ruby Binding
 *		$Id: ode.c,v 1.2 2002/03/20 14:20:03 deveiant Exp $
 *
 *		Author: Michael Granger <ged@FaerieMUD.org>
 *		Copyright (c) 2001 The FaerieMUD Consortium. All rights reserved.
 *
 *		This library is free software; you can redistribute it and/or modify it
 *		under the terms of the GNU Lesser General Public License as published by
 *		the Free Software Foundation; either version 2.1 of the License, or (at
 *		your option) any later version.
 *
 *		This library is distributed in the hope that it will be useful, but
 *		WITHOUT ANY WARRANTY; without even the implied warranty of
 *		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
 *		General Public License for more details.
 *
 *		You should have received a copy of the GNU Lesser General Public License
 *		along with this library (see the file LICENSE.TXT); if not, write to the
 *		Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *		02111-1307 USA.
 *
 */

#include <ruby.h>
#include <stdio.h>
#include <ode/ode.h>

#include "ode.h"


/* -------------------------------------------------------
 * Globals
 * ------------------------------------------------------- */
VALUE ode_mOde;

VALUE ode_cOdeObsoleteJointError;



/* --------------------------------------------------
 * Utility functions
 * -------------------------------------------------- */

void
#ifdef HAVE_STDARG_PROTOTYPES
ode_debug(const char *fmt, ...)
#else
ode_debug(fmt, va_alist)
    const char *fmt;
    va_dcl
#endif
{
  char		buf[BUFSIZ], buf2[BUFSIZ];
  va_list	args;

  if (!RTEST(ruby_verbose)) return;

  snprintf( buf, BUFSIZ, "ODE Debug>>> %s", fmt );

  va_init_list( args, fmt );
  vsnprintf( buf2, BUFSIZ, buf, args );
  fputs( buf2, stderr );
  fputs( "\n", stderr );
  fflush( stderr );
  va_end( args );
}


VALUE
ode_matrix3_to_rArray( matrix )
	 dMatrix3 matrix;
{
  VALUE rArray = rb_ary_new();
  int	index;

  for ( index = 0; index <= 8; index += 4 )
	rb_ary_push( rArray,
				 rb_ary_new3(4,
							 rb_float_new( *(matrix + (index+0)) ),
							 rb_float_new( *(matrix + (index+1)) ),
							 rb_float_new( *(matrix + (index+2)) )) );

  return rArray;
}


void
ode_copy_array( original, copy, depth )
	 dReal	*original, *copy;
	 int	depth;
{
  int	index;
  for ( index = 0; index < depth; index++ )
	*(copy + index) = *(original + index);
}


/* --------------------------------------------------
 * Initialization function
 * -------------------------------------------------- */
void
Init_ode()
{

  //VALUE version;
  
  // Modules
  ode_mOde = rb_define_module( "ODE" );
  rb_define_const( ode_mOde, "PI", rb_float_new(M_PI) );

  // Define exception classes
  ode_cOdeObsoleteJointError = rb_define_class_under( ode_mOde,
													  "ObsoleteJointError",
													  rb_eRuntimeError );

  // Init all the other classes
  ode_init_world();
  //ode_init_space();
  ode_init_body();
  ode_init_rotation();
  ode_init_mass();
  ode_init_joints();
  ode_init_jointGroup();
  
}
