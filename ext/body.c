/*
 *		body.c - ODE Ruby Binding - Body Class
 *		$Id: body.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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

VALUE ode_mOde;
VALUE ode_cOdeWorld;
VALUE ode_cOdeBody;
VALUE ode_cOdeMass;
VALUE ode_cOdeRotation;



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/* ODE::World.createBody() */
VALUE
ode_world_body_create( self )
	 VALUE self;
{
  return ode_body_new( ode_cOdeBody, self );
}


/* ODE::Body.new( worldObj ) */
VALUE
ode_body_new( self, world )
	 VALUE self, world;
{
  dWorldID	worldId;
  dBodyID	bodyId;
  ode_BODY	*bodyStruct;
  VALUE		body;

  if ( ! rb_obj_is_kind_of(world, ode_cOdeWorld) )
	  rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( world )) );

  // Get the world and create a body in it
  GetWorld( world, worldId );
  bodyId = dBodyCreate( worldId );

  // Create the body struct and set its members
  bodyStruct = ALLOC( ode_BODY );
  bodyStruct->id = bodyId;
  bodyStruct->world = self;

  // Now wrap the body in a ruby object and call initialize() on it
  body = Data_Wrap_Struct( ode_cOdeBody, ode_body_gc_mark, ode_body_gc_free, bodyStruct );
  rb_obj_call_init( body, 0, 0 );

  return body;
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/* initialize() */
VALUE
ode_body_init( self )
	 VALUE self;
{
  return self;
}


/* GC Mark function */
void
ode_body_gc_mark( bodyStruct )
	 ode_BODY *bodyStruct;
{
  // Mark the world object that we belong to
  if ( bodyStruct->world ) rb_gc_mark( bodyStruct->world );
}


/* GC Free function */
void
ode_body_gc_free( bodyStruct )
	 ode_BODY *bodyStruct;
{
  // Tell the world we belong to that we're going away and free the body struct
  dBodyDestroy( bodyStruct->id );
  free( bodyStruct );
  bodyStruct = NULL;
}


/* position() */
VALUE
ode_body_position( self, args )
	 VALUE self, args;
{
  ode_BODY	*bodyStruct;
  dReal		*position;

  // Get the body struct and fetch its current position
  GetBody( self, bodyStruct );
  position = (dReal *)dBodyGetPosition( bodyStruct->id );

  // Create a 1x3 ruby array and fill it with our x,y, and z
  return rb_ary_new3( 3,
					  rb_float_new(*(position)),
					  rb_float_new(*(position + 1)),
					  rb_float_new(*(position + 2)) );
}


/* position=() */
VALUE
ode_body_position_eq( self, position )
	 VALUE self, position;
{
  ode_BODY	*body;

  // Make sure we got an array argument
  Check_Type( position, T_ARRAY );

  // Fetch our body struct and set the new position from the argument array
  GetBody( self, body );
  dBodySetPosition( body->id,
					(dReal)NUM2DBL( rb_ary_entry(position, 0) ),
					(dReal)NUM2DBL( rb_ary_entry(position, 1) ),
					(dReal)NUM2DBL( rb_ary_entry(position, 2) ));

  return Qtrue;
}


/* rotation() */
VALUE
ode_body_rotation( self, args )
	 VALUE self, args;
{
  ode_BODY	*bodyStruct;
  dReal		*plainMatrix;
  dMatrix3	rotMatrix;

  // Fetch the body struct and the 4x3 rotation matrix, then copy it to a
  // dMatrix3
  GetBody( self, bodyStruct );
  plainMatrix = (dReal *)dBodyGetRotation( bodyStruct->id );
  ode_copy_array( plainMatrix, rotMatrix, 12 );

  // Create a new ODE::Rotation object from the matrix and return it
  return ode_rotation_new_from_body( rotMatrix );
}


/* rotation=() */
VALUE
ode_body_rotation_eq( self, rotation )
	 VALUE self, rotation;
{
  ode_BODY	*bodyStruct;
  dMatrix3	*rotMatrix;

  // Make sure the argument is a ODE::Rotation object
  if ( ! rb_obj_is_kind_of(rotation, ode_cOdeRotation) )
	rb_raise( rb_eTypeError, "Expected ODE::Rotation object, not a '%s'.",
			  STR2CSTR(rb_funcall( rb_funcall(rotation,rb_intern("type"),0), rb_intern("name"), 0 )) );

  // Get the body and rotation structs from the objects involved
  GetBody( self, bodyStruct );
  GetRotationMatrix( rotation, rotMatrix );

  // Set the body's rotation to the new rotation value
  dBodySetRotation( bodyStruct->id, *rotMatrix );

  return Qtrue;
}


/* const dReal * dBodyGetLinearVel  (dBodyID); */
/* void dBodySetLinearVel  (dBodyID, dReal x, dReal y, dReal z); */

/* linearVelocity() */
VALUE
ode_body_linearVelocity( self, args )
	 VALUE self, args;
{
  ode_BODY		*bodyStruct;
  dReal			*vvec;

  // Get the body struct and the velocity vector
  GetBody( self, bodyStruct );
  vvec = (dReal *)dBodyGetLinearVel( bodyStruct->id );

  // Return a new 3-element Ruby array with the vector values
  return rb_ary_new3( 3,
					  rb_float_new(*(vvec)),
					  rb_float_new(*(vvec + 1)),
					  rb_float_new(*(vvec + 2)) );
}


/* linearVelocity=() */
VALUE
ode_body_linearVelocity_eq( self, linearVelocity )
	 VALUE self, linearVelocity;
{
  ode_BODY	*bodyStruct;

  // Make sure we got an array argument
  Check_Type( linearVelocity, T_ARRAY );

  // Get the body struct and set the vector with the values from the array
  GetBody( self, bodyStruct );
  dBodySetLinearVel( bodyStruct->id,
					 NUM2DBL(rb_ary_entry(linearVelocity,0)), 
					 NUM2DBL(rb_ary_entry(linearVelocity,1)), 
					 NUM2DBL(rb_ary_entry(linearVelocity,2)) );

  return Qtrue;
}


/* const dReal * dBodyGetAngularVel (dBodyID); */
/* void dBodySetAngularVel (dBodyID, dReal x, dReal y, dReal z); */

/* angularVelocity() */
VALUE
ode_body_angularVelocity( self, args )
	 VALUE self, args;
{
  ode_BODY		*bodyStruct;
  dReal			*avec;

  // Get the body struct and fetch the velocity vector
  GetBody( self, bodyStruct );
  avec = (dReal *)dBodyGetAngularVel( bodyStruct->id );

  // Construct and return a new Ruby array with the vector values
  return rb_ary_new3( 3,
					  rb_float_new(*(avec)),
					  rb_float_new(*(avec + 1)),
					  rb_float_new(*(avec + 2)) );
}

/* angularVelocity=() */
VALUE
ode_body_angularVelocity_eq( self, angularVelocity )
	 VALUE self, angularVelocity;
{
  ode_BODY	*bodyStruct;

  // Make sure we got an array argument
  Check_Type( angularVelocity, T_ARRAY );

  // Fetch the body struct and set the vector with the values from the array
  GetBody( self, bodyStruct );
  dBodySetAngularVel( bodyStruct->id,
					 NUM2DBL(rb_ary_entry(angularVelocity,0)), 
					 NUM2DBL(rb_ary_entry(angularVelocity,1)), 
					 NUM2DBL(rb_ary_entry(angularVelocity,2)) );

  return Qtrue;
}


/* mass() */
VALUE
ode_body_mass( self, massObj )
	 VALUE self, massObj;
{
  ode_BODY	*bodyStruct;
  dMass		*mass;
  dMass		*newMass;

  // Get the body and mass structs from the object
  GetBody( self, bodyStruct );
  dBodyGetMass( bodyStruct->id, mass );

  // Make a new mass struct and copy the values from our current one to avoid
  // mass containing a pointer into freed data if the body object goes out of
  // scope. (Is this necessary, or does C copy on assignment?)
  newMass = ALLOC( dMass );
  newMass->mass = mass->mass;
  ode_copy_array( newMass->c, mass->c, 3 );
  ode_copy_array( newMass->I, mass->I, 12 );

  return ode_mass_new_from_body( newMass );
}


/* mass=() */
VALUE
ode_body_mass_eq( self, massObj )
	 VALUE self, massObj;
{
  ode_BODY	*bodyStruct;
  dMass		*mass;

  // Make sure the argument is an ODE::Mass object or subclass, raising an error
  // if it isn't
  if ( ! rb_obj_is_kind_of(massObj, ode_cOdeMass) )
	rb_raise( rb_eTypeError, "Expected ODE::Mass object, not a '%s'.",
			  STR2CSTR(rb_funcall( rb_funcall(massObj,rb_intern("type"),0), rb_intern("name"), 0 )) );

  // Get the mass struct from the object and the body struct from the body
  // object
  GetMass( massObj, mass );
  GetBody( self, bodyStruct );

  // Set the mass of the body
  dBodySetMass( bodyStruct->id, mass );

  return Qtrue;
}


/* addForce( fx, fy, fz )*/
VALUE
ode_body_add_force( self, fx, fy, fz )
	 VALUE self, fx, fy, fz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector
  GetBody( self, bodyStruct );
  dBodyAddForce( bodyStruct->id,
				 NUM2DBL(fx),
				 NUM2DBL(fy),
				 NUM2DBL(fz) );

  return Qtrue;
}


/* addTorque( fx, fy, fz )*/
VALUE
ode_body_add_torque( self, fx, fy, fz )
	 VALUE self, fx, fy, fz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector
  GetBody( self, bodyStruct );
  dBodyAddTorque( bodyStruct->id,
				  NUM2DBL(fx),
				  NUM2DBL(fy),
				  NUM2DBL(fz) );

  return Qtrue;
}


/* addRelForce( fx, fy, fz )*/
VALUE
ode_body_add_rel_force( self, fx, fy, fz )
	 VALUE self, fx, fy, fz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector
  GetBody( self, bodyStruct );
  dBodyAddRelForce( bodyStruct->id,
					NUM2DBL(fx),
					NUM2DBL(fy),
					NUM2DBL(fz) );

  return Qtrue;
}


/* addRelTorque( fx, fy, fz )*/
VALUE
ode_body_add_rel_torque( self, fx, fy, fz )
	 VALUE self, fx, fy, fz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector
  GetBody( self, bodyStruct );
  dBodyAddRelTorque( bodyStruct->id,
					 NUM2DBL(fx),
					 NUM2DBL(fy),
					 NUM2DBL(fz) );

  return Qtrue;
}


/* addForceAtPosition( fx, fy, fz, px, py, pz )*/
VALUE
ode_body_add_force_at_pos( self, fx, fy, fz, px, py, pz )
	 VALUE self, fx, fy, fz, px, py, pz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector and a position
  // vector
  GetBody( self, bodyStruct );
  dBodyAddForceAtPos( bodyStruct->id,
					  NUM2DBL(fx),
					  NUM2DBL(fy),
					  NUM2DBL(fz),
					  NUM2DBL(px),
					  NUM2DBL(py),
					  NUM2DBL(pz) );

  return Qtrue;
}


/* addRelForceAtPosition( fx, fy, fz, px, py, pz )*/
VALUE
ode_body_add_rel_force_at_pos( self, fx, fy, fz, px, py, pz )
	 VALUE self, fx, fy, fz, px, py, pz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector and a position
  // vector
  GetBody( self, bodyStruct );
  dBodyAddRelForceAtPos( bodyStruct->id,
						 NUM2DBL(fx),
						 NUM2DBL(fy),
						 NUM2DBL(fz),
						 NUM2DBL(px),
						 NUM2DBL(py),
						 NUM2DBL(pz) );

  return Qtrue;
}


/* addRelForceAtRelPosition( fx, fy, fz, px, py, pz )*/
VALUE
ode_body_add_rel_force_at_rel_pos( self, fx, fy, fz, px, py, pz )
	 VALUE self, fx, fy, fz, px, py, pz;
{
  ode_BODY	*bodyStruct;

  // Get the body struct and add the arguments as a force vector and a position
  // vector
  GetBody( self, bodyStruct );
  dBodyAddRelForceAtRelPos( bodyStruct->id,
							NUM2DBL(fx),
							NUM2DBL(fy),
							NUM2DBL(fz),
							NUM2DBL(px),
							NUM2DBL(py),
							NUM2DBL(pz) );

  return Qtrue;
}





/* Body initializer */
void
ode_init_body(void)
{
  // Body class
  ode_cOdeBody = rb_define_class_under( ode_mOde, "Body", rb_cObject );

  // Constructors: Add a createBody() method to the world class, and define our
  // own new() method. All bodies must be created in the context of a World
  // object.
  rb_define_method( ode_cOdeWorld, "createBody", ode_world_body_create, 0 );
  rb_define_singleton_method( ode_cOdeBody, "new", ode_body_new, 1 );

  // Methods
  rb_define_method( ode_cOdeBody, "initialize", ode_body_init, 0 );

  rb_define_method( ode_cOdeBody, "position", ode_body_position, 0 );
  rb_define_method( ode_cOdeBody, "position=", ode_body_position_eq, 1 );
  rb_define_method( ode_cOdeBody, "rotation", ode_body_rotation, 0 );
  rb_define_method( ode_cOdeBody, "rotation=", ode_body_rotation_eq, 1 );

  rb_define_method( ode_cOdeBody, "linearVelocity", ode_body_linearVelocity, 0 );
  rb_define_method( ode_cOdeBody, "linearVelocity=", ode_body_linearVelocity_eq, 1 );
  rb_define_method( ode_cOdeBody, "angularVelocity", ode_body_angularVelocity, 0 );
  rb_define_method( ode_cOdeBody, "angularVelocity=", ode_body_angularVelocity_eq, 1 );

  rb_define_method( ode_cOdeBody, "mass", ode_body_mass, 0 );
  rb_define_method( ode_cOdeBody, "mass=", ode_body_mass_eq, 1 );

  rb_define_method( ode_cOdeBody, "addForce", ode_body_add_force, 3 );
  rb_define_method( ode_cOdeBody, "addTorque", ode_body_add_torque, 3 );
  rb_define_method( ode_cOdeBody, "addRelForce", ode_body_add_rel_force, 3 );
  rb_define_method( ode_cOdeBody, "addRelTorque", ode_body_add_rel_torque, 3 );
  rb_define_method( ode_cOdeBody, "addForceAtPosition", ode_body_add_force_at_pos, 6 );
  rb_define_method( ode_cOdeBody, "addRelForceAtPosition", ode_body_add_rel_force_at_pos, 6 );
  rb_define_method( ode_cOdeBody, "addRelForceAtRelPosition", ode_body_add_rel_force_at_rel_pos, 6 );

}




