/*
 *		mass.c - ODE Ruby Binding - Mass object class
 *		$Id: mass.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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

#include <stdlib.h>
#include <ruby.h>
#include <ode/ode.h>

#include "ode.h"

VALUE ode_cOdeMass;
VALUE ode_cOdeRotation;



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/* new() */
VALUE
ode_mass_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
  VALUE		mass;
  dMass		*mass_struct;

  mass = Data_Make_Struct( self, dMass, 0, free, mass_struct );
  rb_obj_call_init( mass, argc, argv );
  
  return mass;
}


/* newSphere( radius, density ) */
VALUE
ode_mass_new_sphere( self, radius, density  )
	 VALUE	self, radius, density;
{
  dMass		*mass_struct;
  VALUE		newMass;
  
  newMass = ode_mass_new( 0, (VALUE *)NULL, self );

  GetMass( newMass, mass_struct );
  dMassSetSphere( mass_struct, NUM2DBL(radius), NUM2DBL(density) );

  return newMass;
}


/* newCappedCylinder( density, direction, radius, length ) */
VALUE
ode_mass_new_ccyl( self, density, direction, radius, length )
	 VALUE	self, density, direction, radius, length;
{
  dMass		*mass_struct;
  VALUE		newMass;
  int		dir;
  
  dir = NUM2INT( direction );
  if ( dir < 1 || dir > 3 )
	rb_raise( rb_eArgError, "Direction argument '%d' out of bounds. Must be between 1 and 3.", dir );

  newMass = ode_mass_new( 0, (VALUE *)NULL, self );

  GetMass( newMass, mass_struct );
  dMassSetCappedCylinder( mass_struct,
						  NUM2DBL(density),
						  dir,
						  NUM2DBL(radius),
						  NUM2DBL(length) );

  return newMass;
}


/* newBox( density, sideX, sideY, sideZ ) */
VALUE
ode_mass_new_box( self, density, x, y, z )
	 VALUE	self, density, x, y, z;
{
  dMass		*mass_struct;
  VALUE		newMass;

  newMass = ode_mass_new( 0, (VALUE *)NULL, self );

  GetMass( newMass, mass_struct );
  dMassSetBox( mass_struct,
			   NUM2DBL(density),
			   NUM2DBL(x),
			   NUM2DBL(y),
			   NUM2DBL(z) );

  return newMass;
}


/* "Native" constructor */
VALUE
ode_mass_new_from_body( mass_struct )
	 dMass	*mass_struct;
{
  return Data_Wrap_Struct( ode_cOdeMass, 0, free, mass_struct );
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/* initialize() */
VALUE
ode_mass_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
  dMass		*mass_struct;

  GetMass( self, mass_struct );

  switch( argc ) {
	
  // 0-arg: Null mass
  case 0:
	dMassSetZero( mass_struct );
	break;

  // 10-arg:  mass, cgx, cgy, cgz, I11, I22, I33, I12, I13, I23
  case 10:
	dMassSetParameters( mass_struct,
						NUM2DBL( *argv ),
						NUM2DBL( *(argv+1) ),
						NUM2DBL( *(argv+2) ),
						NUM2DBL( *(argv+3) ),
						NUM2DBL( *(argv+4) ),
						NUM2DBL( *(argv+5) ),
						NUM2DBL( *(argv+6) ),
						NUM2DBL( *(argv+7) ),
						NUM2DBL( *(argv+8) ),
						NUM2DBL( *(argv+9) ) );
	break;

  default:
	rb_raise( rb_eArgError, "Wrong # of arguments (%d for 0 or 10)", argc );
  }

  return self;
}


/* mass() */
VALUE
ode_mass_mass( self )
	 VALUE self;
{
  dMass		*mass_struct;

  GetMass( self, mass_struct );
  return rb_float_new( mass_struct->mass );
}


/* center_of_gravity() */
VALUE
ode_mass_cog( self )
	 VALUE self;
{
  dMass		*mass_struct;

  GetMass( self, mass_struct );
  return rb_ary_new3( 3,
					  rb_float_new(*( mass_struct->c )),
					  rb_float_new(*( mass_struct->c + 1 )),
					  rb_float_new(*( mass_struct->c + 2 )) );
}


/* inertia() */
VALUE
ode_mass_inertia( self )
	 VALUE self;
{
  dMass		*mass_struct;
  VALUE		I;

  GetMass( self, mass_struct );
  return ode_matrix_to_rArray( mass_struct->I );
}



/* adjust( newmass ) or mass=newmass */
VALUE
ode_mass_adjust( self, newmass )
	 VALUE self, newmass;
{
  dMass		*mass_struct;

  GetMass( self, mass_struct );
  dMassAdjust( mass_struct, NUM2DBL(newmass) );

  return Qtrue;
}


/* translate( x, y, z ) */
VALUE
ode_mass_translate( self, x, y, z )
	 VALUE self, x, y, z;
{
  dMass		*mass_struct;

  GetMass( self, mass_struct );
  dMassTranslate( mass_struct, NUM2DBL(x), NUM2DBL(y), NUM2DBL(z) );

  return Qtrue;
}


/* rotate( ) */
VALUE
ode_mass_rotate( self, rotation )
	 VALUE	self, rotation;
{
  dMass			*mass_struct;
  dMatrix3		dmatrix;

  if ( ! rb_obj_is_kind_of(rotation, ode_cOdeRotation) )
	  rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( rotation )) );

  // Get the dMatrix3 from the rotation object and use it to rotate the mass
  GetMass( self, mass_struct );
  ode_rotation_to_dMatrix3( rotation, dmatrix );
  dMassRotate( mass_struct, dmatrix );

  return Qtrue;
}



/* Mass initializer */
void
ode_init_mass(void)
{
  // Base Mass class + the RotoMass alias
  ode_cOdeMass = rb_define_class_under( ode_mOde, "Mass", rb_cObject );
  rb_define_global_const( "RotoMass", ode_cOdeMass );

  rb_define_singleton_method( ode_cOdeMass, "new", ode_mass_new, -1 );
  rb_define_method( ode_cOdeMass, "initialize", ode_mass_init, -1 );
  rb_define_method( ode_cOdeMass, "mass", ode_mass_mass, 0 );
  rb_define_method( ode_cOdeMass, "cog", ode_mass_cog, 0 );
  rb_define_alias( ode_cOdeMass, "centerOfGravity", "cog" );
  rb_define_method( ode_cOdeMass, "inertia", ode_mass_inertia, 0 );
  rb_define_method( ode_cOdeMass, "adjust", ode_mass_adjust, 1 );
  rb_define_alias( ode_cOdeMass, "mass=", "adjust" );
  rb_define_method( ode_cOdeMass, "translate", ode_mass_translate, 3 );
  rb_define_method( ode_cOdeMass, "rotate", ode_mass_rotate, 1 );

  // Shape factory methods
  rb_define_singleton_method( ode_cOdeMass, "newSphere", ode_mass_new_sphere, 2 );
  rb_define_singleton_method( ode_cOdeMass, "newCappedCylinder", ode_mass_new_ccyl, 4 );
  rb_define_singleton_method( ode_cOdeMass, "newBox", ode_mass_new_box, 4 );

  
}

