/*
 *		rotation.c - ODE Ruby Binding - Rotation Class
 *		$Id: rotation.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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


VALUE ode_cOdeRotation;


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/* new() */
VALUE
ode_rotation_new( argc, argv, self )
	 int argc;
	 VALUE *argv;
	 VALUE self;
{
  dMatrix3	*rotMatrix;
  VALUE		rotation;

  rotation = Data_Make_Struct( self, dMatrix3, 0, free, rotMatrix );
  rb_obj_call_init( rotation, argc, argv );

  return rotation;
}

/* Native Constructor for body data */
VALUE
ode_rotation_new_from_body( matrix )
	 dMatrix3	matrix;
{
  dMatrix3	*rotMatrix;

  rotMatrix = ALLOC( dMatrix3 );
  ode_copy_array( matrix, *rotMatrix, 12 );

  return Data_Wrap_Struct( ode_cOdeRotation, 0, free, rotMatrix );
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

VALUE
ode_rotation_init( argc, argv, self )
	 int argc;
	 VALUE *argv;
	 VALUE self;
{
  dMatrix3	*rotMatrix;

  GetRotationMatrix( self, rotMatrix );
  
  // Figure out how we're being invoked, and build a rotation matrix from the
  // arguments specified.
  switch( argc ) {

  // 0-arg: Identity matrix
  case 0:
	dRSetIdentity( *rotMatrix );
	break;

  // 3-arg: Euler angles
  case 3:
	dRFromEulerAngles( *rotMatrix,
					   (dReal)NUM2DBL(argv[0]),
					   (dReal)NUM2DBL(argv[1]),
					   (dReal)NUM2DBL(argv[2]) );
	break;

  // 4-arg: Axis and angle
  case 4:
	dRFromAxisAndAngle( *rotMatrix,
						(dReal)NUM2DBL(argv[0]), (dReal)NUM2DBL(argv[1]), (dReal)NUM2DBL(argv[2]),
						(dReal)NUM2DBL(argv[3]) );
	break;

  // 6-arg: Two axes
  case 6:
	dRFrom2Axes( *rotMatrix,
				 (dReal)NUM2DBL(argv[0]), (dReal)NUM2DBL(argv[1]), (dReal)NUM2DBL(argv[2]),
				 (dReal)NUM2DBL(argv[3]), (dReal)NUM2DBL(argv[4]), (dReal)NUM2DBL(argv[5]) );
	break;

  // Anything else is an error
  default:
	rb_raise( rb_eArgError, "Wrong # of arguments (%d for 0, 3, 4, or 6)", argc );
  }

  return self;
}


/* to_quaternion() */
VALUE
ode_rotation_to_quaternion( self, args )
	 VALUE self, args;
{
  dMatrix3		*rotMatrix;
  dQuaternion	quat;
  VALUE			quatArray;

  GetRotationMatrix( self, rotMatrix );
  dRtoQ( *rotMatrix, quat );

  quatArray = rb_ary_new3( 4,
						   rb_float_new(*quat),
						   rb_float_new(*(quat + 1)),
						   rb_float_new(*(quat + 2)),
						   rb_float_new(*(quat + 3)) );

  return quatArray;
}


/* to_matrix() */
VALUE
ode_rotation_to_matrix( self, args )
	 VALUE self, args;
{
  dMatrix3		*rotMatrix;

  GetRotationMatrix( self, rotMatrix );
  return (VALUE)ode_matrix3_to_rArray( *rotMatrix );
}


/* --------------------------------------------------
  * "Native" functions
  * -------------------------------------------------- */

void
ode_rotation_to_dMatrix3( self, dmatrix )
	 VALUE		self;
	 dMatrix3	dmatrix;
{
  dMatrix3	*rotMatrix;

  GetRotationMatrix( self, rotMatrix );
  ode_copy_array( *rotMatrix, dmatrix, 12 );
}


/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void
ode_init_rotation(void)
{
  ode_cOdeRotation = rb_define_class_under( ode_mOde, "Rotation", rb_cObject );

  rb_define_singleton_method( ode_cOdeRotation, "new", ode_rotation_new, -1 );
  rb_define_method( ode_cOdeRotation, "initialize", ode_rotation_init, -1 );
  rb_define_method( ode_cOdeRotation, "to_quaternion", ode_rotation_to_quaternion, 0 );
  rb_define_method( ode_cOdeRotation, "to_matrix", ode_rotation_to_matrix, 0 );
}


