/*
 *		position.c - ODE Ruby Binding - Position Class
 *		$Id: position.c,v 1.2 2002/03/20 14:22:44 deveiant Exp $
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


VALUE ode_cOdePosition;


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/* new() */
VALUE
ode_position_new( argc, argv, self )
	 int argc;
	 VALUE *argv;
	 VALUE self;
{
  dReal		coords[3] = {0,0,0};
  VALUE		position;

  if ( argc == 3 ) {
	*(coords) = *(argv);
	*(coords+1) = *(argv+1);
	*(coords+2) = *(argv+2);
  } else {
	rb_raise( rb_eArgError, "Wrong # of arguments (%d for 3)", argc );
  }

  position = Data_Wrap_Struct( ode_cOdePosition, 0, 0, coords );
  rb_obj_call_init( position, 0, 0 );

  return position;
}


