/*
 *		world.c - ODE Ruby Binding - World Class
 *		$Id: world.c,v 1.3 2002/11/23 23:08:45 deveiant Exp $
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2001, 2002 The FaerieMUD Consortium. All rights reserved.
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


/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Macros and constants
 * -------------------------------------------------- */

#define IsWorld( obj ) rb_obj_is_kind_of( (obj), ode_cOdeWorld )


/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * GC mark function 
 */
static void
ode_world_gc_mark( id )
	 dWorldID id;
{
	debugMsg(( "Marking World <%p>", id ));
}


/*
 * GC free function
 */
static void
ode_world_gc_free( id )
	 dWorldID id;
{
	debugMsg(( "Destroying World <%p>", id ));

	// Destroy the world =:)
	dWorldDestroy( id );
	id = NULL;
}


/*
 * Object validity checker. Returns the data pointer.
 */
static dWorldID
check_world( self )
	 VALUE	self;
{
	debugMsg(( "Checking a World object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsWorld(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::World)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static dWorldID
get_world( self )
	 VALUE self;
{
	dWorldID ptr = check_world( self );

	debugMsg(( "Fetching a dWorldID (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized world" );

	return ptr;
}


/*
 * Publicly-usable world-fetcher.
 */
dWorldID
ode_get_world( self )
	 VALUE self;
{
	return get_world(self);
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

static VALUE
ode_world_s_alloc( klass )
	 VALUE klass;
{
	debugMsg(( "Wrapping an uninitialized ODE::World pointer." ));
	return Data_Wrap_Struct( klass, ode_world_gc_mark, ode_world_gc_free, 0 );
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/*
 * initialize()
 * --
 # Create and return a new ODE::World object.
*/
static VALUE
ode_world_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	debugMsg(( "ODE::World init" ));

	if ( !check_world(self) ) {
		dWorldID	id;

		DATA_PTR(self) = id = dWorldCreate();
		debugMsg(( "Created world <%p>", id ));
	}

	rb_call_super( argc, argv );

	return self;
}


/*
 * gravity()
 * --
 * Get the world's global gravity vector in meters/second^2.
 */
static VALUE
ode_world_gravity( self, args )
	 VALUE self, args;
{
	dVector3	gravity;
	dWorldID	world;
	VALUE		rvec;

	// Get the world struct and its gravity
	GetWorld( self, world );
	dWorldGetGravity( world, gravity );

	// Make a new ODE::Vector with the gravity value and return it
	Vec3ToOdeVector( gravity, rvec );
	return rvec;
}


/*
 * gravity=( gravityVector )
 * --
 * Set the world's global gravity vector in meters/second^2. The
 * <tt>gravityVector</tt> can be any object which returns an array of three
 * numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_world_gravity_eq( self, gravity )
	 VALUE self, gravity;
{
	dWorldID	world;
	VALUE		gravArray;

	// Make sure we got an array argument
	// Normalize the gravity into an array
	if ( RARRAY(gravity)->len == 1 ) 
		gravArray = ode_obj_to_ary3( *(RARRAY(gravity)->ptr), "gravity" );
	else
		gravArray = ode_obj_to_ary3( gravity, "gravity" );

	// Get the world struct and set its gravity vector from the values in the
	// array
	GetWorld( self, world );
	dWorldSetGravity( world,
					  NUM2DBL(rb_ary_entry( gravArray, 0 )),
					  NUM2DBL(rb_ary_entry( gravArray, 1 )),
					  NUM2DBL(rb_ary_entry( gravArray, 2 )) );

	return Qtrue;
}


/*
 * erp()
 * --
 * Get the world's global ERP value. ERP controls how much error correction is
 * performed in each time step. Typical values are in the range 0.1 .. 0.8. The
 * default is 0.2.
 */
static VALUE
ode_world_erp( self, args )
	 VALUE self, args;
{
	dWorldID	world;

	// Get the world struct and return its ERP as a Float
	GetWorld( self, world );
	return rb_float_new( dWorldGetERP(world) );
}


/*
 * erp=( newERP )
 * --
 * Set the world's global ERP value. ERP controls how much error correction is
 * performed in each time step. Typical values are in the range (0.1
 * .. 0.8). The default is 0.2.
 */
static VALUE
ode_world_erp_eq( self, erp )
	 VALUE self, erp;
{
	dWorldID	world;

	// Get the world struct and set the ERP from the argument value
	GetWorld( self, world );
	dWorldSetERP( world, NUM2DBL(erp) );

	return Qtrue;
}

/*
 * cfm()
 * --
 * Get the world's global CFM (constraint force mixing) value. Typical values
 * are in the range (1e-9 .. 1). The default is 1e-5 if single precision is
 * being used, or 1e-10 if double precision is being used.
 */
static VALUE
ode_world_cfm( self, args )
	 VALUE self, args;
{
	dWorldID	world;

	// Get the world struct and return the CFM as a Float
	GetWorld( self, world );
	return rb_float_new( dWorldGetCFM(world) );
}


/*
 * cfm=( newCFM )
 * --
 * Set the world's global CFM (constraint force mixing) value. Typical values
 * are in the range (1e-9 .. 1). The default is 1e-5 if single precision is
 * being used, or 1e-10 if double precision is being used.
 */
static VALUE
ode_world_cfm_eq( self, cfm )
	 VALUE self, cfm;
{
	dWorldID	world;

	// Get the world struct and set the CFM from the argument value
	GetWorld( self, world );
	dWorldSetCFM( world, NUM2DBL(cfm) );

	return Qtrue;
}




/*
 * step( stepsize )
 * --
 * Step the world.
 */
static VALUE
ode_world_step( self, stepsize )
	 VALUE self, stepsize;
{
	dWorldID	world;

	// Get the world struct and step it with the argument value
	GetWorld( self, world );
	dWorldStep( world, NUM2DBL(stepsize) );

	return Qtrue;
}


/*
 * createBody()
 * --
 * Factory method: Create a new body object in the World object it is called on.
 */
static VALUE
ode_world_body_create( self )
	 VALUE self;
{
	debugMsg(( "createBody: Calling Body constructor." ));
	return rb_class_new_instance( 1, &self, ode_cOdeBody );
}


/*
 * impulseToForce( stepsize, ix, iy, iz )
 * --
 * If you want to apply a linear or angular impulse to a rigid body, instead of
 * a force or a torque, then you can use this method to convert the desired
 * impulse into a force/torque vector before calling the
 * ODE::Body#add... method.
 *
 * This method is given the desired impulse as
 * (<tt>ix</tt>,<tt>iy</tt>,<tt>iz</tt>) and returns an ODE::Force, scaled by
 * 1/<tt>stepsize</tt>, where <tt>stepsize</tt> is the step size for the next
 * world step that will be taken.
 *
 * This is a method of ODE::World because, in the future, the force computation
 * may depend on integrator parameters that are set as properties of the world.
 */
static VALUE
ode_world_imp2force( self, stepsize, ix, iy, iz )
	 VALUE self, stepsize, ix, iy, iz;
{
	dWorldID	world;
	dVector3	fvec;
	VALUE		force;

	GetWorld( self, world );
	dWorldImpulseToForce( world,
						  (dReal) NUM2DBL(stepsize),
						  (dReal) NUM2DBL(ix),
						  (dReal) NUM2DBL(iy),
						  (dReal) NUM2DBL(iz),
						  fvec );
	Vec3ToOdeForce( fvec, force );

	return force;
}




/* World initializer */
void
ode_init_world()
{
	/* Constructor */
	rb_define_singleton_method( ode_cOdeWorld, "allocate", ode_world_s_alloc, 0 );

	/* Initializer */
	rb_define_method( ode_cOdeWorld, "initialize", ode_world_init, -1 );
	rb_enable_super( ode_cOdeWorld, "initialize" );

	/* Attribute methods */
	rb_define_method( ode_cOdeWorld, "gravity", ode_world_gravity, 0 );
	rb_define_method( ode_cOdeWorld, "gravity=", ode_world_gravity_eq, -2 );
	rb_define_method( ode_cOdeWorld, "erp", ode_world_erp, 0 );
	rb_define_method( ode_cOdeWorld, "erp=", ode_world_erp_eq, 1 );
	rb_define_method( ode_cOdeWorld, "cfm", ode_world_cfm, 0 );
	rb_define_method( ode_cOdeWorld, "cfm=", ode_world_cfm_eq, 1 );

	/* Utility methods */
	rb_define_method( ode_cOdeWorld, "createBody", ode_world_body_create, 0 );
	rb_define_method( ode_cOdeWorld, "impulseToForce", ode_world_imp2force, 4 );

	/* Operations */
	rb_define_method( ode_cOdeWorld, "step", ode_world_step, 1 );
}


