/*
 *		body.c - ODE Ruby Binding - Body Class
 *		$Id: body.c,v 1.3 2002/11/23 23:08:45 deveiant Exp $
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

#include "ode.h"


/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */


/* --------------------------------------------------
 * Macros and constants
 * -------------------------------------------------- */

#define IsBody( obj ) rb_obj_is_kind_of( (obj), ode_cOdeBody )



/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static ode_BODY *
ode_body_alloc()
{
	ode_BODY *ptr = ALLOC( ode_BODY );

	ptr->id = NULL;
	ptr->world = Qnil;

	debugMsg(( "Initialized ode_BODY <%p>", ptr ));
	return ptr;
}


/*
 * GC Mark function
 */
static void
ode_body_gc_mark( ptr )
	 ode_BODY *ptr;
{

	debugMsg(( "Marking an ODE::Body" ));
	if ( ptr ) {
		int jointCount;

		debugMsg(( "Marking Body <%p>", ptr ));

		/* Mark the world the body belongs to */
		rb_gc_mark( ptr->world );

		/* If this body has any attached joints, mark those as well */
		if (( jointCount = dBodyGetNumJoints(ptr->id) )) {
			int			i;
			dJointID	jointId;
			ode_JOINT	*jointStruct;

			debugMsg(( "Going to mark %d attached Joints", jointCount ));

			/* Rip the Ruby object out of each joint and mark it. */
			for ( i = 0 ; i < jointCount ; i++ ) {
				debugMsg(( "Getting joint %d from the body", i ));
				jointId = dBodyGetJoint( ptr->id, i );
				debugMsg(( "Getting joint struct." ));
				jointStruct = (ode_JOINT *)dJointGetData( jointId );

				debugMsg(( "Marking joint <%p>", jointStruct->joint ));
				rb_gc_mark( jointStruct->joint );
			}
		} else {
			debugMsg(( "No attached Joints." ));
		}
	}

	else {
		debugMsg(( "Not marking uninitialized ode_BODY" ));
	}
}


/*
 * GC Free function
 */
static void
ode_body_gc_free( ptr )
	 ode_BODY *ptr;
{
	debugMsg(( "Freeing an ODE::Body." ));

	if ( ptr ) {

		/* Avoid double-freeing when ruby shuts down by testing to see if the
		   world this body belongs to is still a data object. If it's not, it
		   must be assumed that this is happening as Ruby is shutting down. */
		if ( TYPE(ptr->world) == T_DATA ) {
			dWorldID	worldId = (dWorldID)DATA_PTR( ptr->world );

			debugMsg(( "Destroying body <%p> (worldID = <%p>)", ptr, worldId ));
			if ( worldId ) dBodyDestroy( ptr->id );
		}

		ptr->object = Qnil;
		ptr->world  = Qnil;
		ptr->id		= NULL;

		debugMsg(( "Freeing ode_BODY <%p>", ptr ));
		xfree( ptr );
		ptr = NULL;
	}

	else {
		debugMsg(( "Not freeing uninitialized ode_BODY" ));
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ode_BODY *
check_body( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Body object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsBody(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Body)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_BODY *
get_body( self )
	 VALUE self;
{
	ode_BODY *ptr = check_body( self );

	debugMsg(( "Fetching an ode_BODY (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized body" );

	return ptr;
}


/*
 * Publicly-usable body-fetcher.
 */
ode_BODY *
ode_get_body( self )
	 VALUE self;
{
	return get_body(self);
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * allocate()
 * --
 * Allocate a new ODE::Body object.
 */
static VALUE
ode_body_s_alloc( klass )
{
	debugMsg(( "Wrapping an uninitialized ODE::Body pointer." ));
	return Data_Wrap_Struct( klass, ode_body_gc_mark, ode_body_gc_free, 0 );
}



/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/*
 * initialize( worldObj )
 * --
 * Create a body in the given world with default mass parameters at position
 * (0,0,0).
 */
static VALUE
ode_body_init( self, world )
	 VALUE self, world;
{
	debugMsg(( "ODE::Body init." ));

	if ( !check_body(self) ) {
		dWorldID	worldId = ode_get_world( world );
		ode_BODY	*ptr;

		DATA_PTR(self) = ptr = ode_body_alloc();

		ptr->object = self;
		ptr->id		= dBodyCreate( worldId );
		ptr->world	= world;

		debugMsg(( "Created Body <%p> in World <%p>.", ptr, worldId ));

		/* Set the data pointer to this */
		dBodySetData( ptr->id, ptr );
	}

	/* Can't initialize twice, as a body cannot be removed from a world. */
	else {
		rb_raise( rb_eRuntimeError,
				  "Cannot re-initialize a body once it's been set in a world." );
	}

	return self;
}


/*
 * position()
 * --
 * Get the position of the body as an ODE::Position object.
 */
static VALUE
ode_body_position( self, args )
	 VALUE self, args;
{
	ode_BODY	*bodyStruct;
	dReal		*position;

	/* Get the body struct and fetch its current position */
	GetBody( self, bodyStruct );
	position = (dReal *)dBodyGetPosition( bodyStruct->id );

	/* Create a new Position object with our x, y, and z */
	return rb_funcall( ode_cOdePosition, rb_intern("new"), 3,
					   rb_float_new(*(position)),
					   rb_float_new(*(position + 1)),
					   rb_float_new(*(position + 2)) );
}


/*
 * position=( *args )
 * --
 * Set the body's current position, which can be any object which returns an
 * array with 3 numeric values when <tt>to_ary</tt> is called on it, such as an
 * ODE::Position object, a Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_body_position_eq( self, position )
	 VALUE self, position;
{
	ode_BODY	*body;
	VALUE		posArray;

	/* Normalize the position into an array */
	if ( RARRAY(position)->len == 1 ) 
		posArray = ode_obj_to_ary3( *(RARRAY(position)->ptr), "position" );
	else
		posArray = ode_obj_to_ary3( position, "position" );
		

	/* Fetch the body struct */
	GetBody( self, body );
	dBodySetPosition( body->id,
					  (dReal)NUM2DBL(rb_ary_entry( posArray, 0 )),
					  (dReal)NUM2DBL(rb_ary_entry( posArray, 1 )),
					  (dReal)NUM2DBL(rb_ary_entry( posArray, 2 )) );

	return Qtrue;
}


/*
 * rotation()
 * --
 * Return the body's rotation as an ODE::Rotation object.
 */
static VALUE
ode_body_rotation( self, args )
	 VALUE self, args;
{
	ode_BODY	*bodyStruct;
	dReal		*quat;
	VALUE		x, y, z, angle;

	/* Fetch the body struct and the quaternion for it. */
	GetBody( self, bodyStruct );
	quat = (dReal *)dBodyGetQuaternion( bodyStruct->id );

	x = rb_float_new( *(quat  ) );
	y = rb_float_new( *(quat+1) );
	z = rb_float_new( *(quat+2) );
	angle = rb_float_new( *(quat+3) );
	
	/* Create a new ODE::Rotation object from the quaternion and return it */
	return rb_funcall( ode_cOdeRotation, rb_intern("new"), 4, x, y, z, angle );
}


/*
 * rotation=( rotation )
 * --
 * Set the body's orientation to the specified <tt>rotation</tt>, which can be
 * any object which returns an Array of four numeric values when <tt>to_ary</tt>
 * is called on it such as an ODE::Rotation object, a Math3d::Vector4, or an
 * Array with four numeric values.
 */
static VALUE
ode_body_rotation_eq( self, rotation )
	 VALUE self, rotation;
{
	VALUE		ary;
	ode_BODY	*bodyStruct;
	dQuaternion	quat;
	int			i;

	/* Call to_ary on whatever we got, and make sure it's an Array with four elements. */
	if ( RARRAY(rotation)->len == 1 ) 
		ary = ode_obj_to_ary4( *(RARRAY(rotation)->ptr), "rotation" );
	else
		ary = ode_obj_to_ary4( rotation, "rotation" );

	/* Copy the values in the array into the quaternion */
	for ( i = 0 ; i <= 3 ; i++ )
		quat[i] = NUM2DBL( *(RARRAY(ary)->ptr + i) );
  
	/* Get the body and set its rotation */
	GetBody( self, bodyStruct );
	dBodySetRotation( bodyStruct->id, quat );

	return Qtrue;
}


/*
 * quaternion()
 * --
 * Get the quaternion of the body as an ODE::Rotation object.
 */
static VALUE
ode_body_quaternion( self, args )
	 VALUE self, args;
{
	ode_BODY	*bodyStruct;
	dReal		*quaternion;

	/* Get the body struct and fetch its current quaternion */
	GetBody( self, bodyStruct );
	quaternion = (dReal *)dBodyGetQuaternion( bodyStruct->id );

	/* Create a new Quaternion object with our x, y, and z */
	return rb_funcall( ode_cOdeRotation, rb_intern("new"), 4,
					   rb_float_new(*(quaternion)),
					   rb_float_new(*(quaternion + 1)),
					   rb_float_new(*(quaternion + 2)),
					   rb_float_new(*(quaternion + 3)) );
}


/*
 * quaternion=( *args )
 * --
 * Set the body's current quaternion, which can be any object which returns an
 * array with 3 numeric values when <tt>to_ary</tt> is called on it, such as an
 * ODE::Quaternion object, a Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_body_quaternion_eq( self, quaternion )
	 VALUE self, quaternion;
{
	ode_BODY	*body;
	dQuaternion	q;
	VALUE		ary;
	int			i;

	/* Normalize the quaternion into an array */
	if ( RARRAY(quaternion)->len == 1 ) 
		ary = ode_obj_to_ary4( *(RARRAY(quaternion)->ptr), "quaternion" );
	else
		ary = ode_obj_to_ary4( quaternion, "quaternion" );

	for ( i = 0 ; i <= 3 ; i++ )
		q[i] = (dReal)NUM2DBL(rb_ary_entry( ary, i ));

	/* Fetch the body struct */
	GetBody( self, body );
	dBodySetQuaternion( body->id, q );

	return Qtrue;
}


/*
 * linearVelocity()
 * --
 * Get the body's linear velocity as an ODE::LinearVelocity object (3rd order
 * vector).
 */
static VALUE
ode_body_linearVelocity( self, args )
	 VALUE self, args;
{
	ode_BODY	*bodyStruct;
	dReal		*velocityVector;

	/* Get the body struct and the velocity vector */
	GetBody( self, bodyStruct );
	velocityVector = (dReal *)dBodyGetLinearVel( bodyStruct->id );

	/* Return a new 3-element Ruby array with the vector values */
	return rb_funcall( ode_cOdeLinearVelocity, rb_intern("new"), 3,
					   rb_float_new(*(velocityVector)),
					   rb_float_new(*(velocityVector + 1)),
					   rb_float_new(*(velocityVector + 2)) );
}


/*
 * linearVelocity=( linearVelocity ) 
 * --
 * Set the linear velocity of the body. The <tt>linearVelocity</tt> argument can
 * be any object that returns a three-element array when <tt>to_ary</tt> is called
 * on it, such as an ODE::LinearVelocity or a Math3d::Vector3.
 */
static VALUE
ode_body_linearVelocity_eq( self, linearVelocity )
	 VALUE self, linearVelocity;
{
	VALUE		ary;
	ode_BODY	*bodyStruct;

 	/* Normalize the velocity into an array */
	if ( RARRAY(linearVelocity)->len == 1 ) 
		ary = ode_obj_to_ary3( *(RARRAY(linearVelocity)->ptr), "linear velocity" );
	else
		ary = ode_obj_to_ary3( linearVelocity, "linear velocity" );

	/* Get the body struct and set the vector with the values from the array */
	GetBody( self, bodyStruct );
	dBodySetLinearVel( bodyStruct->id,
					   NUM2DBL(*(RARRAY(ary)->ptr    )), 
					   NUM2DBL(*(RARRAY(ary)->ptr + 1)), 
					   NUM2DBL(*(RARRAY(ary)->ptr + 2)) );

	return Qtrue;
}


/*
 * angularVelocity()
 * --
 * Get the body's angular velocity as a 3-element Array vector.
 */
static VALUE
ode_body_angularVelocity( self, args )
	 VALUE self, args;
{
	ode_BODY	*bodyStruct;
	dReal		*velocityVector;

	/* Get the body struct and the velocity vector */
	GetBody( self, bodyStruct );
	velocityVector = (dReal *)dBodyGetAngularVel( bodyStruct->id );

	/* Return a new 3-element Ruby array with the vector values */
	return rb_funcall( ode_cOdeAngularVelocity, rb_intern("new"), 3,
					   rb_float_new(*(velocityVector)),
					   rb_float_new(*(velocityVector + 1)),
					   rb_float_new(*(velocityVector + 2)) );
}


/*
 * angularVelocity=( vector ) 
 * --
 * Set the angular velocity of the body. The <tt>angularVelocity</tt> argument
 * can be any object that returns a three-element array when <tt>to_ary</tt> is
 * called on it, such as an ODE::AngularVelocity or a Math3d::Vector3.
 */
static VALUE
ode_body_angularVelocity_eq( self, angularVelocity )
	 VALUE self, angularVelocity;
{
	VALUE		ary;
	ode_BODY	*bodyStruct;

	/* Normalize the position into an array */
	if ( RARRAY(angularVelocity)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(angularVelocity)->ptr), "angular velocity" );
	else
		ary = ode_obj_to_ary3( angularVelocity, "angular velocity" );

	/* Get the body struct and set the vector with the values from the array */
	GetBody( self, bodyStruct );
	dBodySetAngularVel( bodyStruct->id,
						(dReal) NUM2DBL(*(RARRAY(ary)->ptr  )), 
						(dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)), 
						(dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}



/*
 * mass()
 * --
 * Get the mass of the body object (an ODE::Mass object).
 */
static VALUE
ode_body_mass( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	dMass		*mass = 0;

	/* Get the body and mass structs from the object */
	GetBody( self, bodyStruct );
	dBodyGetMass( bodyStruct->id, mass );

	return ode_mass_new_from_body( mass );
}


/*
 * mass=( massObj )
 * --
 * Set the body's mass (an ODE::Mass object).
 */
static VALUE
ode_body_mass_eq( self, massObj )
	 VALUE self, massObj;
{
	ode_BODY	*bodyStruct;
	dMass		*mass;

	/* Make sure the argument is an ODE::Mass object or subclass, raising an 
	   error if it isn't */
	if ( ! rb_obj_is_kind_of(massObj, ode_cOdeMass) )
		rb_raise( rb_eTypeError, "Expected ODE::Mass object, not a '%s'.",
				  STR2CSTR(rb_funcall( rb_funcall(massObj,rb_intern("type"),0), rb_intern("name"), 0 )) );

	/* Get the mass struct from the object and the body struct from the body 
	   object */
	GetMass( massObj, mass );
	GetBody( self, bodyStruct );

	/* Set the mass of the body */
	dBodySetMass( bodyStruct->id, mass );

	return Qtrue;
}


/*
 * enable
 * --
 * Mark the body as enabled. See #disable for more information about
 * enabling/disabling bodies.
 */
static VALUE
ode_body_enable( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;

	GetBody( self, bodyStruct );
	dBodyEnable( bodyStruct->id );

	return Qtrue;
}

/*
 * disable
 * --
 * Mark the body as disabled. Disabled bodies are effectively ``turned off'' and
 * are not updated during a simulation step. However, if a disabled body is
 * connected to an island containing one or more enabled bodies then it will be
 * re-enabled at the next simulation step.
 */
static VALUE
ode_body_disable( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;

	GetBody( self, bodyStruct );
	dBodyDisable( bodyStruct->id );

	return Qtrue;
}


/*
 * enabled?
 * --
 * Returns <tt>true</tt> if the body is enabled. See #disable for more
 * information about enabling/disabling bodies.
 */
static VALUE
ode_body_enabled_p( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;

	GetBody( self, bodyStruct );
	if ( dBodyIsEnabled(bodyStruct->id) )
		return Qtrue;
	else
		return Qfalse;
}

/*
 * finiteRotationMode
 * --
 * Return the finite rotation mode, which controls the way a body's orientation
 * is updated at each time step. See the docs for #finiteRotationMode= for
 * possible modes.
 */
static VALUE
ode_body_finite_rotation_mode( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	int			mode;

	GetBody( self, bodyStruct );
	mode = dBodyGetFiniteRotationMode( bodyStruct->id );

	return INT2FIX( mode );
}


/*
 * finiteRotationMode=( mode )
 * --
 * Set the body's finite rotation mode, which controls the way a body's
 * orientation is updated at each time step. The mode argument can be:
 *
 *  [ODE::Rotation::INFINITESIMAL]
 *    An ``infitesimal'' orientation update is used. This is fast to compute,
 *    but it can occasionally cause inaccuracies for bodies that are rotating at
 *    high speed, especially when those bodies are joined to other bodies. This
 *    is the default for every new body that is created.
 *
 *  [ODE::Rotation::FINITE]
 *    A ``finite'' orientation update is used. This is more costly to compute,
 *    but will be more accurate for high speed rotations. Note however that high
 *    speed rotations can result in many types of error in a simulation, and
 *    this mode will only fix one of those sources of error.
 *
 */
static VALUE
ode_body_finite_rotation_mode_eq( self, modeArg )
	 VALUE self, modeArg;
{
	ode_BODY	*bodyStruct;
	int			mode;

	mode = NUM2INT( modeArg );
	if ( mode < 0 || mode > 1 )
		rb_raise( rb_eArgError, "Illegal mode." );

	GetBody( self, bodyStruct );
	dBodySetFiniteRotationMode( bodyStruct->id, mode );

	return modeArg;
}


/*
 * finiteRotationAxis()
 * --
 * Returns the body's finite rotation axis as an ODE::Vector.
 */
static VALUE
ode_body_finite_rotation_axis( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	dVector3	axis;

	GetBody( self, bodyStruct );
	dBodyGetFiniteRotationAxis( bodyStruct->id, (dReal *)axis );

	return ode_vector3_to_rArray( axis );
}


/*
 * finiteRotationAxis=( axis )
 * --
 * Set the finite rotation <tt>axis</tt> for a body, where <tt>axis</tt> is any
 * object which returns an Array of three numeric elements when <tt>to_ary</tt> is
 * called on it. This is axis only has meaning when the finite rotation mode is
 * set (see #finiteRotationMode=).
 *
 * If this axis is zero (0,0,0), full finite rotations are performed on the
 * body.
 *
 * If this axis is nonzero, the body is rotated by performing a partial finite
 * rotation along the axis direction followed by an infitesimal rotation along
 * an orthogonal direction.
 *
 * This can be useful to alleviate certain sources of error caused by quickly
 * spinning bodies. For example, if a car wheel is rotating at high speed you
 * can call this function with the wheel's hinge axis as the argument to try and
 * improve its behavior.
 */
static VALUE
ode_body_finite_rotation_axis_eq( self, axis )
	 VALUE self, axis;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

 	/* Normalize the axis into an array */
	if ( RARRAY(axis)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(axis)->ptr), "finite rotation axis" );
	else
		ary = ode_obj_to_ary3( axis, "finite rotation axis" );

	GetBody( self, bodyStruct );
	dBodySetFiniteRotationAxis( bodyStruct->id,
								(dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
								(dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
								(dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}


/*
 * addForce( forceVector )
 * --
 * Add a force vector to the body. The <tt>forceVector</tt> can be anything that
 * returns an array of three numeric values when <tt>to_ary</tt> is called on it,
 * such as a Math3d::Vector3, an ODE::ForceVector, or an Array with three
 * numeric elements.
 */
static VALUE
ode_body_add_force( self, forceVector )
	 VALUE self, forceVector;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

 	/* Normalize the velocity into an array */
	if ( RARRAY(forceVector)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(forceVector)->ptr), "force vector" );
	else
		ary = ode_obj_to_ary3( forceVector, "force vector" );

	GetBody( self, bodyStruct );
	dBodyAddForce( bodyStruct->id,
				   (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
				   (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
				   (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}


/*
 * force()
 * --
 * Get the current accumulated linear forces applied to the body as an
 * ODE::Force object.
 */
static VALUE
ode_body_get_force( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	const dReal	*fvec;

	GetBody( self, bodyStruct );
	fvec = dBodyGetForce( bodyStruct->id );

	/* Create and return a new ODE::Force object with the value of the force 
	   vector. */
	return rb_funcall( ode_cOdeForce, rb_intern("new"), 3,
					   rb_float_new(*(fvec  )),
					   rb_float_new(*(fvec+1)),
					   rb_float_new(*(fvec+2)) );
}


/*
 * force=( forceVector )
 * ---
 * Set the accumulated force vector. The <tt>forceVector</tt> can be anything
 * that returns an array of three numeric values when <tt>to_ary</tt> is called on
 * it, such as a Math3d::Vector3, and ODE::ForceVector, or an Array with three
 * numeric elements.
 */
static VALUE
ode_body_set_force( self, forceVector )
	 VALUE self, forceVector;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

 	/* Normalize the velocity into an array */
	if ( RARRAY(forceVector)->len ==  1 )
		ary = ode_obj_to_ary3( *(RARRAY(forceVector)->ptr), "force vector" );
	else
		ary = ode_obj_to_ary3( forceVector, "force vector" );

	GetBody( self, bodyStruct );
	dBodySetForce( bodyStruct->id,
				   (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
				   (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
				   (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}


/*
 * addTorque( torqueVector )
 * --
 * Add a toque vector to the body. The <tt>torqueVector</tt> can be any object
 * which returns an array of three numeric values when <tt>to_ary</tt> is called
 * on it, such as a Math3d::Vector, an ODE::TorqueVector, or an Array with three
 * numeric elements.
 */
static VALUE
ode_body_add_torque( self, torqueVector )
	 VALUE self, torqueVector;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

 	/* Normalize the velocity into an array */
	ary = ode_obj_to_ary3( rb_funcall(torqueVector, rb_intern("flatten"), 0, 0), "torque vector" );

	GetBody( self, bodyStruct );
	dBodyAddTorque( bodyStruct->id,
					(dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
					(dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
					(dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}


/*
 * getTorque()
 * --
 * Get the current accumulated torque applied to the body as an ODE::Torque
 * object.
 */
static VALUE
ode_body_get_torque( self )
	VALUE self;
{
	ode_BODY	*bodyStruct;
	const dReal	*tvec;

	GetBody( self, bodyStruct );
	tvec = dBodyGetTorque( bodyStruct->id );

	/* Create and return a new ODE::Torque object with the values of the torque 
	   vector. */
	return rb_funcall( ode_cOdeTorque, rb_intern("new"), 3,
					   rb_float_new(*(tvec  )),
					   rb_float_new(*(tvec+1)),
					   rb_float_new(*(tvec+2)) );
}


/*
 * torque=( torqueVector )
 * ---
 * Set the accumulated torque vector. The <tt>torqueVector</tt> can be anything
 * that returns an array of three numeric values when <tt>to_ary</tt> is called on
 * it, such as a Math3d::Vector3, and ODE::TorqueVector, or an Array with three
 * numeric elements.
 */
static VALUE
ode_body_set_torque( self, torqueVector )
	 VALUE self, torqueVector;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

 	/* Normalize the velocity into an array */
	if ( RARRAY(torqueVector)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(torqueVector)->ptr), "torque vector" );
	else
		ary = ode_obj_to_ary3( torqueVector, "torque vector" );

	GetBody( self, bodyStruct );
	dBodySetTorque( bodyStruct->id,
					(dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
					(dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
					(dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}


/*
 * getRelPointPosition( point )
 * --
 * Given the specified <tt>point</tt> on the body (in body-relative
 * coordinates), return that point's position (an ODE::Position object) in
 * global (World) coordinates. The <tt>point</tt> argument can be any object
 * which returns an array of three numeric values when <tt>to_ary</tt> is called
 * on it, such as a Math3d::Vector3, an ODE::Position, or an Array with three
 * numeric elements.
 */
static VALUE
ode_body_get_rel_point_pos( self, point )
{
	ode_BODY	*bodyStruct;
	VALUE		ary;
	dVector3	position;

	if ( RARRAY(point)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(point)->ptr), "relative point" );
	else
		ary = ode_obj_to_ary3( point, "relative point" );

	GetBody( self, bodyStruct );
	dBodyGetRelPointPos( bodyStruct->id,
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)),
						 (dReal *)position );

	return rb_funcall( ode_cOdePosition, rb_intern("new"),
					   rb_float_new(*(position  )),
					   rb_float_new(*(position+1)),
					   rb_float_new(*(position+2)) );
}



/*
 * getPositionRelPoint( point )
 * --
 * Given the specified <tt>point</tt> in global coordinates, return that point's
 * position (an ODE::Position object) in body-relative coordinates. The
 * <tt>point</tt> argument can be any object which returns an array of three
 * numeric values when <tt>to_ary</tt> is called on it, such as a Math3d::Vector3,
 * an ODE::Position, or an Array with three numeric elements.
 */
static VALUE
ode_body_get_pos_rel_point( self, point )
{
	ode_BODY	*bodyStruct;
	VALUE		ary;
	dVector3	position;

	if ( RARRAY(point)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(point)->ptr), "relative point" );
	else
		ary = ode_obj_to_ary3( point, "relative point" );

	GetBody( self, bodyStruct );
	dBodyGetPosRelPoint( bodyStruct->id,
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)),
						 (dReal *)position );

	return rb_funcall( ode_cOdePosition, rb_intern("new"),
					   rb_float_new(*(position  )),
					   rb_float_new(*(position+1)),
					   rb_float_new(*(position+2)) );
}


/*
 * getRelPointVelocity( point )
 * --
 * Given the specified <tt>point</tt> on the body (in body-relative
 * coordinates), return that point's velocity (an ODE::LinearVelocity object) in
 * global (World) coordinates. The <tt>point</tt> argument can be any object
 * which returns an array of three numeric values when <tt>to_ary</tt> is called
 * on it, such as a Math3d::Vector3, an ODE::Position, or an Array with three
 * numeric elements.
 */
static VALUE
ode_body_get_rel_point_vel( self, point )
{
	ode_BODY	*bodyStruct;
	VALUE		ary;
	dVector3	velocity;

	ary = ode_obj_to_ary3( rb_funcall(point, rb_intern("flatten"), 0, 0), "relative point" );

	GetBody( self, bodyStruct );
	dBodyGetRelPointVel( bodyStruct->id,
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
						 (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)),
						 (dReal *)velocity );

	return rb_funcall( ode_cOdeLinearVelocity, rb_intern("new"),
					   rb_float_new(*(velocity  )),
					   rb_float_new(*(velocity+1)),
					   rb_float_new(*(velocity+2)) );
}



/*
 * getPointVelocity( point )
 * --
 * Given the specified <tt>point</tt> on the body (in global coordinates),
 * return that point's velocity (an ODE::LinearVelocity object) in global
 * (World) coordinates. The <tt>point</tt> argument can be any object which
 * returns an array of three numeric values when <tt>to_ary</tt> is called on it,
 * such as a Math3d::Vector3, an ODE::Position, or an Array with three numeric
 * elements.
 */
static VALUE
ode_body_get_point_vel( self, point )
{
	ode_BODY	*bodyStruct;
	VALUE		ary;
	dVector3	velocity;

	ary = ode_obj_to_ary3( rb_funcall(point, rb_intern("flatten"), 0, 0), "point" );

	GetBody( self, bodyStruct );
	dBodyGetPointVel( bodyStruct->id,
					  (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
					  (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
					  (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)),
					  (dReal *)velocity );

	return rb_funcall( ode_cOdeLinearVelocity, rb_intern("new"),
					   rb_float_new(*(velocity  )),
					   rb_float_new(*(velocity+1)),
					   rb_float_new(*(velocity+2)) );
}



/*
 * addRelForce( forceVector )
 * --
 * Add a force vector to the body relative to the body's own frame of
 * reference. The <tt>forceVector</tt> can be anything that returns an array of
 * three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::ForceVector, or an Array with three numeric
 * elements.
 */
static VALUE
ode_body_add_rel_force( self, forceVector )
	 VALUE self, forceVector;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

 	/* Normalize the velocity into an array */
	if ( RARRAY(forceVector)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(forceVector)->ptr), "force vector" );
	else
		ary = ode_obj_to_ary3( forceVector, "force vector" );

	GetBody( self, bodyStruct );
	dBodyAddRelForce( bodyStruct->id,
					  (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
					  (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
					  (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}


/*
 * addRelTorque( fx, fy, fz )
 * --
 * 
 */
static VALUE
ode_body_add_rel_torque( self, forceVector )
	 VALUE self, forceVector;
{
	ode_BODY	*bodyStruct;
	VALUE		ary;

	/* Normalize the force vector */
	if ( RARRAY(forceVector)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(forceVector)->ptr), "force vector" );
	else
		ary = ode_obj_to_ary3( forceVector, "force vector" );

	/* Get the body struct and add the arguments as a force vector */
	GetBody( self, bodyStruct );
	dBodyAddRelTorque( bodyStruct->id,
					   (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
					   (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
					   (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)) );

	return Qtrue;
}



/*
 * addForceAtPosition( forceVector, position )
 * --
 * Add a force vector to the body originating at the specified position relative
 * to the world. Both the <tt>forceVector</tt> and the <tt>position</tt> can be
 * any object that returns an array of three numeric values when <tt>to_ary</tt>
 * is called on it, such as a Math3d::Vector3, an ODE::ForceVector, an
 * ODE::Position, or an Array with three numeric elements.
 */
static VALUE
ode_body_add_force_at_pos( self, forceVector, position )
	 VALUE self, forceVector, position;
{
	ode_BODY	*bodyStruct;
	VALUE		fary, pary;

 	/* Normalize the velocity and position into arrays */
	fary = ode_obj_to_ary3( forceVector, "force vector" );
	pary = ode_obj_to_ary3( position, "position" );

	/* Get the body struct and add the arguments as a force vector and a position 
	   vector */
	GetBody( self, bodyStruct );
	dBodyAddForceAtPos( bodyStruct->id,
						(dReal) NUM2DBL(*(RARRAY(fary)->ptr  )),
						(dReal) NUM2DBL(*(RARRAY(fary)->ptr+1)),
						(dReal) NUM2DBL(*(RARRAY(fary)->ptr+2)),
						(dReal) NUM2DBL(*(RARRAY(pary)->ptr  )),
						(dReal) NUM2DBL(*(RARRAY(pary)->ptr+1)),
						(dReal) NUM2DBL(*(RARRAY(pary)->ptr+2)) );

	return Qtrue;
}



/*
 * addForceAtRelPosition( forceVector, position )
 * --
 * Add a force vector to the body originating at the specified position relative
 * to the body. Both the <tt>forceVector</tt> and the <tt>position</tt> can be
 * any object that returns an array of three numeric values when <tt>to_ary</tt>
 * is called on it, such as a Math3d::Vector3, an ODE::ForceVector, an
 * ODE::Position, or an Array with three numeric elements.
 */
static VALUE
ode_body_add_force_at_rel_pos( self, forceVector, position )
	 VALUE self, forceVector, position;
{
	ode_BODY	*bodyStruct;
	VALUE		fary, pary;

 	/* Normalize the velocity and position into arrays */
	fary = ode_obj_to_ary3( forceVector, "force vector" );
	pary = ode_obj_to_ary3( position, "position" );

	/* Get the body struct and add the arguments as a force vector and a
	   position vector */
	GetBody( self, bodyStruct );
	dBodyAddForceAtRelPos( bodyStruct->id,
						   (dReal) NUM2DBL(*(RARRAY(fary)->ptr  )),
						   (dReal) NUM2DBL(*(RARRAY(fary)->ptr+1)),
						   (dReal) NUM2DBL(*(RARRAY(fary)->ptr+2)),
						   (dReal) NUM2DBL(*(RARRAY(pary)->ptr  )),
						   (dReal) NUM2DBL(*(RARRAY(pary)->ptr+1)),
						   (dReal) NUM2DBL(*(RARRAY(pary)->ptr+2)) );

	return Qtrue;
}


/*
 * addRelForceAtPosition( forceVector, position )
 * --
 * Add a force vector to the body relative to the body's own frame of reference,
 * originating at the specified position relative to the world. Both the
 * <tt>forceVector</tt> and the <tt>position</tt> can be any object that returns
 * an array of three numeric values when <tt>to_ary</tt> is called on it, such as
 * a Math3d::Vector3, an ODE::ForceVector, an ODE::Position, or an Array with
 * three numeric elements.
 */
static VALUE
ode_body_add_rel_force_at_pos( self, forceVector, position )
	 VALUE self, forceVector, position;
{
	ode_BODY	*bodyStruct;
	VALUE		fary, pary;

 	/* Normalize the velocity and position into arrays */
	fary = ode_obj_to_ary3( forceVector, "force vector" );
	pary = ode_obj_to_ary3( position, "position" );

	/* Get the body struct and add the arguments as a force vector and a position 
	   vector */
	GetBody( self, bodyStruct );
	dBodyAddRelForceAtPos( bodyStruct->id,
						   (dReal) NUM2DBL(*(RARRAY(fary)->ptr  )),
						   (dReal) NUM2DBL(*(RARRAY(fary)->ptr+1)),
						   (dReal) NUM2DBL(*(RARRAY(fary)->ptr+2)),
						   (dReal) NUM2DBL(*(RARRAY(pary)->ptr  )),
						   (dReal) NUM2DBL(*(RARRAY(pary)->ptr+1)),
						   (dReal) NUM2DBL(*(RARRAY(pary)->ptr+2)) );

	return Qtrue;
}


/*
 * addRelForceAtRelPosition( forceVector, position )
 * --
 * Add a force vector to the body originating at the specified position, both
 * of which are relative to the body's own frame of reference. Both the
 * <tt>forceVector</tt> and the <tt>position</tt> can be any object that returns
 * an array of three numeric values when <tt>to_ary</tt> is called on it, such as
 * a Math3d::Vector3, an ODE::ForceVector, an ODE::Position, or an Array with
 * three numeric elements.
 */
static VALUE
ode_body_add_rel_force_at_rel_pos( self, forceVector, position )
	 VALUE self, forceVector, position;
{
	ode_BODY	*bodyStruct;
	VALUE		fary, pary;

 	/* Normalize the velocity and position into arrays */
	fary = ode_obj_to_ary3( forceVector, "force vector" );
	pary = ode_obj_to_ary3( position, "position" );

	/* Get the body struct and add the arguments as a force vector and a
	   position vector */
	GetBody( self, bodyStruct );
	dBodyAddRelForceAtRelPos( bodyStruct->id,
							  (dReal) NUM2DBL(*(RARRAY(fary)->ptr  )),
							  (dReal) NUM2DBL(*(RARRAY(fary)->ptr+1)),
							  (dReal) NUM2DBL(*(RARRAY(fary)->ptr+2)),
							  (dReal) NUM2DBL(*(RARRAY(pary)->ptr  )),
							  (dReal) NUM2DBL(*(RARRAY(pary)->ptr+1)),
							  (dReal) NUM2DBL(*(RARRAY(pary)->ptr+2)) );

	return Qtrue;
}

/*
 * vectorToWorld( vector )
 * --
 * Given the specified <tt>vector</tt> in body-relative coordinates, return the
 * vector rotated to global coordinates. The <tt>vector</tt> argument can be any
 * object which returns an array of three numeric values when <tt>to_ary</tt> is
 * called on it, such as a Math3d::Vector3, an ODE::LinearVelocity, or an Array
 * with three numeric elements.
 */
static VALUE
ode_body_vec_to_world( self, vector )
{
	ode_BODY	*bodyStruct;
	VALUE		ary;
	dVector3	newVector;

	if ( RARRAY(vector)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(vector)->ptr), "body-relative vector" );
	else
		ary = ode_obj_to_ary3( vector, "body-relative vector" );
	
	GetBody( self, bodyStruct );
	dBodyVectorToWorld( bodyStruct->id,
						(dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
						(dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
						(dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)),
						(dReal *)newVector );

	return rb_funcall( ode_cOdeVector, rb_intern("new"),
					   rb_float_new(*(newVector  )),
					   rb_float_new(*(newVector+1)),
					   rb_float_new(*(newVector+2)) );
}

/*
 * vectorFromWorld( vector )
 * --
 * Given the specified <tt>vector</tt> in global coordinates, return the vector
 * rotated to body-relative coordinates. The <tt>vector</tt> argument can be any
 * object which returns an array of three numeric values when <tt>to_ary</tt> is
 * called on it, such as a Math3d::Vector3, an ODE::LinearVelocity, or an Array
 * with three numeric elements.
 */
static VALUE
ode_body_vec_from_world( self, vector )
{
	ode_BODY	*bodyStruct;
	VALUE		ary;
	dVector3	newVector;

	if ( RARRAY(vector)->len == 1 )
		ary = ode_obj_to_ary3( *(RARRAY(vector)->ptr), "body-relative vector" );
	else
		ary = ode_obj_to_ary3( vector, "body-relative vector" );

	GetBody( self, bodyStruct );
	dBodyVectorFromWorld( bodyStruct->id,
						  (dReal) NUM2DBL(*(RARRAY(ary)->ptr  )),
						  (dReal) NUM2DBL(*(RARRAY(ary)->ptr+1)),
						  (dReal) NUM2DBL(*(RARRAY(ary)->ptr+2)),
						  (dReal *)newVector );

	return rb_funcall( ode_cOdeVector, rb_intern("new"),
					   rb_float_new(*(newVector  )),
					   rb_float_new(*(newVector+1)),
					   rb_float_new(*(newVector+2)) );
}




/*
 * connectedTo?( otherBody )
 * --
 * Returns <tt>true</tt> if the specified <tt>otherBody</tt> is connected to the
 * receiving one.
 */
static VALUE
ode_body_connected_to_p( self, otherBody )
	 VALUE self, otherBody;
{
	ode_BODY	*thisBodyStruct, *otherBodyStruct;
	int			res;

	if ( !rb_obj_is_kind_of(otherBody, ode_cOdeBody) )
		rb_raise( rb_eTypeError, "no implicit conversion to ODE::Body for %s",
				  rb_class2name(CLASS_OF(otherBody)) );

	GetBody( self, thisBodyStruct );
	GetBody( otherBody, otherBodyStruct );
	res = dAreConnected( thisBodyStruct->id, otherBodyStruct->id );

	return res ? Qtrue : Qfalse;
}

/*
 * getNumberOfJoints()
 * --
 * Returns the number of joints attached to this body.
 */
static VALUE
ode_body_get_num_joints( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	int			jointCount;

	GetBody( self, bodyStruct );
	jointCount = dBodyGetNumJoints( bodyStruct->id );

	return INT2NUM( jointCount );
}


/*
 * getJoint( index )
 * --
 * Return the ODE::Joint object associated with the <tt>index</tt>th joint on
 * the body.
 */
static VALUE
ode_body_get_joint( self, index )
	 VALUE self, index;
{
	ode_BODY	*bodyStruct;
	dJointID	joint;
	ode_JOINT	*jointStruct;
	int			i, jointCount;

	i = NUM2INT( index );
	GetBody( self, bodyStruct );
	jointCount = dBodyGetNumJoints( bodyStruct->id );

	if ( i > jointCount - 1 )
		rb_raise( rb_eIndexError, "body has no joint %d", i );

	joint = dBodyGetJoint( bodyStruct->id, i );
	jointStruct = (ode_JOINT *)dJointGetData( (dJointID)joint );

	return jointStruct->joint;
}


/*
 * joints()
 * --
 * Returns an Array of ODE::Joint objects that are attached to the receiving
 * body.
 */
static VALUE
ode_body_joints( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	int			i, jointCount;
	VALUE		jointAry;
	ode_JOINT	*jointStruct;
	dJointID	joint;
	
	GetBody( self, bodyStruct );
	jointCount = dBodyGetNumJoints( bodyStruct->id );

	/* Create an array of the joints attached to this body. */
	jointAry = rb_ary_new2( (long)jointCount );
	for ( i = 0 ; i < jointCount ; i++ ) {
		joint = dBodyGetJoint( bodyStruct->id, i );
		jointStruct = (ode_JOINT *)dJointGetData( (dJointID)joint );
		rb_ary_store( jointAry, i, jointStruct->joint );
	}

	return jointAry;
}


/*
 * gravityMode?()
 * --
 * Returns <tt>true</tt> if the body is influenced by the world's gravity.
 */
static VALUE
ode_body_gravity_mode_p( self )
	 VALUE self;
{
	ode_BODY	*bodyStruct;
	int			mode;
	
	GetBody( self, bodyStruct );
	mode = dBodyGetGravityMode( bodyStruct->id );

	return mode ? Qtrue : Qfalse;
}


/*
 * gravityMode=( trueOrFalse )
 * --
 * Turn the influence of the world's gravity on the body on or off
 * (<tt>true</tt> = influenced by gravity).
 */
static VALUE
ode_body_gravity_mode_eq( self, flag )
	 VALUE self, flag;
{
	ode_BODY	*bodyStruct;
	
	GetBody( self, bodyStruct );
	dBodySetGravityMode( bodyStruct->id, RTEST(flag) ? 1 : 0 );

	return RTEST(flag) ? Qtrue : Qfalse;
}




/* Body initializer */
void
ode_init_body(void)
{
	rb_define_singleton_method( ode_cOdeBody, "allocate", ode_body_s_alloc, 0 );

	/* Initializer */
	rb_define_method( ode_cOdeBody, "initialize", ode_body_init, 1 );
	rb_enable_super ( ode_cOdeBody, "initialize" );

	/* Position and orientation */
	rb_define_method( ode_cOdeBody, "position=", ode_body_position_eq, -2 );
	rb_define_method( ode_cOdeBody, "rotation=", ode_body_rotation_eq, -2 );
	rb_define_method( ode_cOdeBody, "quaternion=", ode_body_quaternion_eq, -2 );
	rb_define_method( ode_cOdeBody, "linearVelocity=", ode_body_linearVelocity_eq, -2 );
	rb_define_alias ( ode_cOdeBody, "linearVel=", "linearVelocity=" );
	rb_define_method( ode_cOdeBody, "angularVelocity=", ode_body_angularVelocity_eq, -2 );
	rb_define_alias ( ode_cOdeBody, "angularVel=", "angularVelocity=" );

	rb_define_method( ode_cOdeBody, "position", ode_body_position, 0 );
	rb_define_method( ode_cOdeBody, "rotation", ode_body_rotation, 0 );
	rb_define_method( ode_cOdeBody, "quaternion", ode_body_quaternion, 0 );
	rb_define_method( ode_cOdeBody, "linearVelocity", ode_body_linearVelocity, 0 );
	rb_define_alias ( ode_cOdeBody, "linearVel", "linearVelocity" );
	rb_define_method( ode_cOdeBody, "angularVelocity", ode_body_angularVelocity, 0 );
	rb_define_alias ( ode_cOdeBody, "angularVel", "angularVelocity" );

	/* Mass and force */
	rb_define_method( ode_cOdeBody, "mass", ode_body_mass, 0 );
	rb_define_method( ode_cOdeBody, "mass=", ode_body_mass_eq, -2 );

	rb_define_method( ode_cOdeBody, "addForce", ode_body_add_force, -2 );
	rb_define_method( ode_cOdeBody, "addTorque", ode_body_add_torque, -2 );
	rb_define_method( ode_cOdeBody, "addRelForce", ode_body_add_rel_force, -2 );
	rb_define_method( ode_cOdeBody, "addRelTorque", ode_body_add_rel_torque, -2 );
	rb_define_method( ode_cOdeBody, "addForceAtPosition", ode_body_add_force_at_pos, 2 );
	rb_define_alias ( ode_cOdeBody, "addForceAtPos", "addForceAtPosition" );
	rb_define_method( ode_cOdeBody, "addForceAtRelPosition", ode_body_add_force_at_rel_pos, 2 );
	rb_define_alias ( ode_cOdeBody, "addForceAtRelPos", "addForceAtRelPosition" );
	rb_define_method( ode_cOdeBody, "addRelForceAtPosition", ode_body_add_rel_force_at_pos, 2 );
	rb_define_alias ( ode_cOdeBody, "addRelForceAtPos", "addRelForceAtPosition" );
	rb_define_method( ode_cOdeBody, "addRelForceAtRelPosition", ode_body_add_rel_force_at_rel_pos, 2 );
	rb_define_alias ( ode_cOdeBody, "addRelForceAtRelPos", "addRelForceAtRelPosition" );

	rb_define_method( ode_cOdeBody, "force", ode_body_get_force, 0 );
	rb_define_alias ( ode_cOdeBody, "getForce", "force" );
	rb_define_method( ode_cOdeBody, "torque", ode_body_get_torque, 0 );
	rb_define_alias ( ode_cOdeBody, "getTorque", "torque" );

	rb_define_method( ode_cOdeBody, "force=", ode_body_set_force, 1 );
	rb_define_alias ( ode_cOdeBody, "setForce", "force=" );
	rb_define_method( ode_cOdeBody, "torque=", ode_body_set_torque, 1 );
	rb_define_alias ( ode_cOdeBody, "setTorque", "torque=" );

	/* Utilities */
	rb_define_method( ode_cOdeBody, "getRelPointPosition", ode_body_get_rel_point_pos, -2 );
	rb_define_alias ( ode_cOdeBody, "getRelPointPos", "getRelPointPosition" );
	rb_define_method( ode_cOdeBody, "getRelPointVelocity", ode_body_get_rel_point_vel, -2 );
	rb_define_alias ( ode_cOdeBody, "getRelPointVel", "getRelPointVelocity" );
	rb_define_method( ode_cOdeBody, "getPointVelocity", ode_body_get_point_vel, -2 );
	rb_define_alias ( ode_cOdeBody, "getPointVel", "getPointVelocity" );

	rb_define_method( ode_cOdeBody, "getPositionRelPoint", ode_body_get_pos_rel_point, -2 );
	rb_define_alias ( ode_cOdeBody, "getPosRelPoint", "getPositionRelPoint" );

	rb_define_method( ode_cOdeBody, "vectorToWorld", ode_body_vec_to_world, -2 );
	rb_define_method( ode_cOdeBody, "vectorFromWorld", ode_body_vec_from_world, -2 );

	/* Miscellaneous */
	rb_define_method( ode_cOdeBody, "enable", ode_body_enable, 0 );
	rb_define_method( ode_cOdeBody, "disable", ode_body_disable, 0 );
	rb_define_method( ode_cOdeBody, "enabled?", ode_body_enabled_p, 0 );

	rb_define_method( ode_cOdeBody, "finiteRotationMode", ode_body_finite_rotation_mode, 0 );
	rb_define_method( ode_cOdeBody, "finiteRotationMode=", ode_body_finite_rotation_mode_eq, 1 );

	rb_define_method( ode_cOdeBody, "finiteRotationAxis", ode_body_finite_rotation_axis, 0 );
	rb_define_method( ode_cOdeBody, "finiteRotationAxis=", ode_body_finite_rotation_axis_eq, -2 );

	rb_define_method( ode_cOdeBody, "getNumberOfJoints", ode_body_get_num_joints, 0 );
	rb_define_method( ode_cOdeBody, "getJoint", ode_body_get_joint, 1 );
	rb_define_method( ode_cOdeBody, "joints", ode_body_joints, 0 );

	rb_define_method( ode_cOdeBody, "gravityMode?", ode_body_gravity_mode_p, 0 );
	rb_define_method( ode_cOdeBody, "gravityMode=", ode_body_gravity_mode_eq, 1 );

	/* Others */
	rb_define_method( ode_cOdeBody, "connectedTo?", ode_body_connected_to_p, 1 );
}




