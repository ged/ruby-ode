/*
 *		world.c - ODE Ruby Binding - World Class
 *		$Id$
 *		Time-stamp: <04-Feb-2003 15:33:52 deveiant>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2001, 2002, 2003 The FaerieMUD Consortium.
 *
 *		This work is licensed under the Creative Commons Attribution License. To
 *		view a copy of this license, visit
 *		http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
 *		Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
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
	dWorldID	world = get_world( self );
	dVector3	gravity;
	VALUE		rvec;

	// Get the world's gravity, make a new ODE::Vector with it, and return it
	dWorldGetGravity( world, gravity );
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
	dWorldID	world = get_world( self );
	VALUE		gravArray;

	// Make sure we got an array argument
	// Normalize the gravity into an array
	if ( RARRAY(gravity)->len == 1 ) 
		gravArray = ode_obj_to_ary3( *(RARRAY(gravity)->ptr), "gravity" );
	else
		gravArray = ode_obj_to_ary3( gravity, "gravity" );

	// Set the world's gravity vector from the values in the array
	dWorldSetGravity( world,
					  NUM2DBL(rb_ary_entry( gravArray, 0 )),
					  NUM2DBL(rb_ary_entry( gravArray, 1 )),
					  NUM2DBL(rb_ary_entry( gravArray, 2 )) );

	return gravArray;
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
	dWorldID	world = get_world( self );
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
	dWorldID	world = get_world( self );

	dWorldSetERP( world, NUM2DBL(erp) );
	return erp;
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
	dWorldID	world = get_world( self );
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
	dWorldID	world = get_world( self );

	dWorldSetCFM( world, NUM2DBL(cfm) );
	return cfm;
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
	dWorldID	world = get_world( self );

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
	dWorldID	world = get_world( self );
	dVector3	fvec;
	VALUE		force;

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

	/* Kluge to make Rdoc see the class in this file */
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeWorld = rb_define_class_under( ode_mOde, "World", rb_cObject );
#endif


	/* Allocator */
#ifdef NEW_ALLOC
	rb_define_alloc_func( ode_cOdeWorld, ode_world_s_alloc );
#else
	rb_define_singleton_method( ode_cOdeWorld, "allocate", ode_world_s_alloc, 0 );
#endif

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


