/*
 *		ode.c - ODE Ruby Binding
 *		$Id: ode.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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
#include <ode/ode.h>

#include "ode.h"


VALUE ode_mOde;


/* --------------------------------------------------
 * Utility functions
 * -------------------------------------------------- */

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

  // Init all the other classes
  ode_init_world();
  //ode_init_space();
  ode_init_body();
  ode_init_rotation();
  ode_init_mass();
  ode_init_joints();
  ode_init_jointGroup();
  
}
