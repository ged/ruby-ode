/*
 *		world.c - ODE Ruby Binding - World Class
 *		$Id: world.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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
VALUE ode_cOdeWorld;


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/* new() */
VALUE
ode_world_new( self, args )
	 VALUE self, args;
{
  dWorldID	id;
  VALUE		world;

  // Create a new world and wrap it in a Ruby object
  id = dWorldCreate();
  world = Data_Wrap_Struct( ode_cOdeWorld, 0, ode_world_gc_free, id );

  // Call initialize()
  rb_obj_call_init( world, 0, 0 );

  return world;
}

/* free function */
void
ode_world_gc_free( id )
	 dWorldID id;
{
  // Destroy the world =:)
  dWorldDestroy( id );
  id = NULL;
}


/* initialize() */
VALUE
ode_world_init( self, args )
	 VALUE self, args;
{
  // No-op. This is here mainly for subclasses
  return self;
}


/* gravity() */
VALUE
ode_world_gravity( self, args )
	 VALUE self, args;
{
  dVector3	gravity;
  dWorldID	world;

  // Get the world struct and its gravity
  GetWorld( self, world );
  dWorldGetGravity( world, gravity );

  // Make a new hash, populate it with the gravity values, and return it
  return rb_ary_new3( 3,
					  rb_float_new(*(gravity)),
					  rb_float_new(*(gravity + 1)),
					  rb_float_new(*(gravity + 2)) );
}

/* gravity=() */
VALUE
ode_world_gravity_eq( self, grav )
	 VALUE self, grav;
{
  dWorldID	world;

  // Make sure we got an array argument
  Check_Type( grav, T_ARRAY );

  // Get the world struct and set its gravity vector from the values in the
  // array
  GetWorld( self, world );
  dWorldSetGravity( world,
					NUM2DBL(rb_ary_entry( grav, 0 )),
					NUM2DBL(rb_ary_entry( grav, 1 )),
					NUM2DBL(rb_ary_entry( grav, 2 )) );

  return Qtrue;
}


/* erp() */
VALUE
ode_world_erp( self, args )
	 VALUE self, args;
{
  dWorldID	world;

  // Get the world struct and return its ERP as a Float
  GetWorld( self, world );
  return rb_float_new( dWorldGetERP(world) );
}


/* erp=() */
VALUE
ode_world_erp_eq( self, erp )
	 VALUE self, erp;
{
  dWorldID	world;

  // Get the world struct and set the ERP from the argument value
  GetWorld( self, world );
  dWorldSetERP( world, NUM2DBL(erp) );

  return Qtrue;
}

/* cfm() */
VALUE
ode_world_cfm( self, args )
	 VALUE self, args;
{
  dWorldID	world;

  // Get the world struct and return the CFM as a Float
  GetWorld( self, world );
  return rb_float_new( dWorldGetCFM(world) );
}


/* cfm=() */
VALUE
ode_world_cfm_eq( self, cfm )
	 VALUE self, cfm;
{
  dWorldID	world;

  // Get the world struct and set the CFM from the argument value
  GetWorld( self, world );
  dWorldSetCFM( world, NUM2DBL(cfm) );

  return Qtrue;
}

/* step( stepsize ) */
VALUE
ode_world_step( self, stepsize )
	 VALUE self, stepsize;
{
  dWorldID	world;

  // Get the world struct and step it with the argument value
  GetWorld( self, world );
  dWorldStep( world, NUM2DBL(stepsize) );

  return Qtrue;
}


/* World initializer */
void
ode_init_world(void)
{
  // World class
  ode_cOdeWorld = rb_define_class_under( ode_mOde, "World", rb_cObject );

  // Constructor
  rb_define_singleton_method( ode_cOdeWorld, "new", ode_world_new, 0 );

  // Methods
  rb_define_method( ode_cOdeWorld, "initialize", ode_world_init, 0 );
  rb_define_method( ode_cOdeWorld, "gravity", ode_world_gravity, 0 );
  rb_define_method( ode_cOdeWorld, "gravity=", ode_world_gravity_eq, 1 );
  rb_define_method( ode_cOdeWorld, "erp", ode_world_erp, 0 );
  rb_define_method( ode_cOdeWorld, "erp=", ode_world_erp_eq, 1 );
  rb_define_method( ode_cOdeWorld, "cfm", ode_world_cfm, 0 );
  rb_define_method( ode_cOdeWorld, "cfm=", ode_world_cfm_eq, 1 );
  rb_define_method( ode_cOdeWorld, "step", ode_world_step, 1 );
}


