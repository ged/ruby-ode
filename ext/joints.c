/*
 *		joints.c - ODE Ruby Binding - Joint Classes
 *		$Id: joints.c,v 1.3 2002/11/23 23:08:45 deveiant Exp $
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


/* Globals */
static VALUE body1Sym;
static VALUE body2Sym;
static VALUE torqueSym;
static VALUE forceSym;


// Forward declarations
static void ode_joint_gc_mark( ode_JOINT * );
static void ode_joint_gc_free( ode_JOINT * );
static VALUE ode_joint_get_feedback_hash( ode_JOINT * );
static VALUE ode_joint_make_bfhash();

/* 
 * ODE::Joint::new
 * --
 # Raises an error, as ODE::Joint is an abstract class.
*/
static VALUE
ode_joint_abstract_class( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	rb_raise( rb_eRuntimeError, "Cannot instantiate abstract class %s.", rb_class2name(self) );
}


/*
 * Constructor -- every joint class except ODE::ContactJoint uses this for its
 * constructor. It passes in its arguments and pointer to the the ODE native
 * constructor function.
 */
static VALUE
ode_joint_new( argc, argv, self, constructor )
	 int		argc;
	 VALUE		*argv;
	 VALUE		self;
	 dJointID	(*constructor)( dWorldID, dJointGroupID );
{
	dWorldID		worldId;
	dJointGroupID	jointGroupId;
	ode_JOINTGROUP	*jointGroupStruct;
	ode_JOINT		*jointStruct;
	VALUE			world, jointGroup;

	debugMsg(( "Creating a new Joint: checking args." ));

	// Check arguments
	if ( argc < 1 || argc > 2 )
		rb_raise( rb_eArgError, "Wrong # of arguments (%d for 1)", argc );
	if ( ! rb_obj_is_kind_of(argv[0], ode_cOdeWorld) )
		rb_raise( rb_eTypeError,
				  "no implicit conversion to ODE::World from %s",
				  rb_class2name(CLASS_OF( argv[0] )) );
	if ( argc == 2 && ! rb_obj_is_kind_of(argv[1], ode_cOdeJointGroup) )
		rb_raise( rb_eTypeError,
				  "no implicit conversion to ODE:JointGroup from %s",
				  rb_class2name(CLASS_OF( argv[1] )) );

	debugMsg(( "Creating a new Joint: fetching World." ));

	// Fetch the world object and its id
	world = *argv;
	GetWorld( world, worldId );

	debugMsg(( "Creating a new Joint: fetching the JointGroup." ));

	// Fetch the jointGroup and its id, if one was given
	if ( argc == 2 && RTEST(argv[1]) ) {
		jointGroup = argv[1];
		GetJointGroup( jointGroup, jointGroupStruct );
		jointGroupId = jointGroupStruct->id;
		debugMsg(( "Got a JointGroup for new Joint: <%p>", jointGroupStruct ));
	}
	else {
		jointGroup = Qnil;
		jointGroupId = 0;
		debugMsg(( "No JointGroup for new Joint" ));
	}

		// Allocate a joint struct and set its members
	debugMsg(( "Creating a new Joint: ALLOCing and setting struct members." ));
	jointStruct = ALLOC( ode_JOINT );
	jointStruct->world		= world;
	jointStruct->id			= 0;
	jointStruct->body1		= Qnil;
	jointStruct->body2		= Qnil;
	jointStruct->jointGroup = jointGroup;
	jointStruct->fbhash		= Qnil;
	jointStruct->feedback	= NULL;
	jointStruct->contact	= Qnil;

	// Create the actual joint by calling the class-specific constructor and set
	// it in the struct
	jointStruct->id = (*constructor)( worldId, jointGroupId );
	debugMsg(( "Created Joint <%p> (id = <%p>)", jointStruct, jointStruct->id ));

	// Wrap it in a Ruby object, call initialize() on it, and return it
	debugMsg(( "Creating a new Joint: Wrapping the struct." ));
	jointStruct->joint = Data_Wrap_Struct( self, ode_joint_gc_mark, ode_joint_gc_free, jointStruct );
	dJointSetData( jointStruct->id, jointStruct );

	// Register with the Ruby part of the JointGroup, if the joint is a member
	if ( jointGroupId ) {
		debugMsg(( "Creating a new Joint: Registering joint with jointGroup." ));
		ode_jointGroup_register_joint( jointGroup, jointStruct->joint );
	}

	debugMsg(( "Creating a new Joint: Calling initialize." ));
	rb_iv_set( jointStruct->joint, "@obsolete", Qfalse );
	rb_obj_call_init( jointStruct->joint, 0, 0 );

	return jointStruct->joint;
}

/*
 * GC mark function
 */
static void
ode_joint_gc_mark( jointStruct )
	 ode_JOINT	*jointStruct;
{
	debugMsg(( "Marking Joint <%p>", jointStruct ));

	// Mark the objects behind any of the struct members which are set
	if ( jointStruct->jointGroup )	rb_gc_mark( jointStruct->jointGroup );
	if ( jointStruct->world )		rb_gc_mark( jointStruct->world );
	if ( jointStruct->body1 )		rb_gc_mark( jointStruct->body1 );
	if ( jointStruct->body2 )		rb_gc_mark( jointStruct->body2 );
	if ( jointStruct->contact )		rb_gc_mark( jointStruct->contact );
	if ( jointStruct->fbhash )		rb_gc_mark( jointStruct->fbhash );
}


/*
 * GC free function
 */
static void
ode_joint_gc_free( jointStruct )
	 ode_JOINT	*jointStruct;
{
	debugMsg(( "Destroying Joint <%p> (id = <%p>)", jointStruct, jointStruct->id ));

	// If this joint wasn't in a jointGroup, we have to destroy it ourselves.
	if ( ! jointStruct->jointGroup )
		dJointDestroy( jointStruct->id );

	debugMsg(( "Clearing Joint struct <%p>", jointStruct ));

	// Clear out the struct members
	jointStruct->id			= NULL;
	jointStruct->joint		= 0;
	jointStruct->jointGroup	= 0;
	jointStruct->world		= 0;
	jointStruct->body1		= 0;
	jointStruct->body2		= 0;
	jointStruct->contact	= 0;
	jointStruct->fbhash		= 0;

	if ( jointStruct->feedback )
		free( jointStruct->feedback );
	jointStruct->feedback	= 0;

	debugMsg(( "Freeing Joint struct <%p>", jointStruct ));

	// Free the struct and reset the reference
	free( jointStruct );
	jointStruct = NULL;
}



/*
 * ODE::ContactJoint::new( world, contact, jointGroup=DEFAULT_JOINT_GROUP )
 * --
 * Create and return a new ODE::ContactJoint in the specified world (an
 * ODE::World object) with the specified contact (an ODE::Contact object).
 */
VALUE
ode_contactJoint_new( argc, argv, self )
	 int		argc;
	 VALUE		*argv;
	 VALUE		self;
{
	dWorldID		worldId;
	dJointGroupID	jointGroupId;
	ode_JOINT		*jointStruct;
	VALUE			world, jointGroup, contact;
	dContact		*contactStruct;


	// Read in the arguments
	debugMsg(( "Creating a new ContactJoint: checking args." ));
	if ( rb_scan_args(argc, argv, "21", &world, &contact, &jointGroup ) == 3 )
		CheckKindOf( jointGroup, ode_cOdeJointGroup );

	CheckKindOf( world, ode_cOdeWorld );
	CheckKindOf( contact, ode_cOdeContact );

	// Fetch the world object's dWorldId
	debugMsg(( "Creating a new ContactJoint: fetching dWorldId." ));
	GetWorld( world, worldId );

	// Fetch the dContact from the contact object
	debugMsg(( "Creating a new ContactJoint: fetching dContact" ));
	GetContact( contact, contactStruct );

	// Fetch the jointGroup and its id, if one was given
	debugMsg(( "Creating a new ContactJoint: fetching the JointGroup." ));
	if ( RTEST(jointGroup) ) {
		ode_JOINTGROUP	*jointGroupStruct;

		GetJointGroup( jointGroup, jointGroupStruct );
		jointGroupId = jointGroupStruct->id;
		debugMsg(( "Got a JointGroup for new ContactJoint: <%p>", jointGroupStruct ));
	}
	else {
		jointGroupId = 0;
		debugMsg(( "No JointGroup for new ContactJoint" ));
	}

	// Allocate a joint struct and set its members
	debugMsg(( "Creating a new ContactJoint: ALLOCing and setting struct members." ));
	jointStruct = ALLOC( ode_JOINT );
	jointStruct->world		= world;
	jointStruct->body1		= Qnil;
	jointStruct->body2		= Qnil;
	jointStruct->jointGroup = jointGroup;
	jointStruct->fbhash		= Qnil;
	jointStruct->feedback	= NULL;
	jointStruct->contact	= contact;

	// Create the actual joint by calling the class-specific constructor and set
	// it in the struct
	jointStruct->id = dJointCreateContact( worldId, jointGroupId, contactStruct );
	debugMsg(( "Created ContactJoint <%p> (id = <%p>)", jointStruct, jointStruct->id ));

	// Wrap it in a Ruby object, call initialize() on it, and return it
	debugMsg(( "Creating a new ContactJoint: Wrapping the struct." ));
	jointStruct->joint = Data_Wrap_Struct( self, ode_joint_gc_mark, ode_joint_gc_free, jointStruct );
	dJointSetData( jointStruct->id, jointStruct );

	// Register with the Ruby part of the JointGroup, if the joint is a member
	if ( jointGroupId ) {
		debugMsg(( "Creating a new ContactJoint: Registering joint with jointGroup." ));
		ode_jointGroup_register_joint( jointGroup, jointStruct->joint );
	}

	debugMsg(( "Creating a new ContactJoint: Calling initialize." ));
	rb_iv_set( jointStruct->joint, "@obsolete", Qfalse );
	rb_obj_call_init( jointStruct->joint, 0, 0 );

	return jointStruct->joint;
}


/* BallJoint constructor */
VALUE
ode_ballJoint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateBall );
}

/* HingeJoint constructor */
VALUE
ode_hingeJoint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateHinge );
}

/* Hinge2Joint constructor */
VALUE
ode_hinge2Joint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateHinge2 );
}

/* SliderJoint constructor */
VALUE
ode_sliderJoint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateSlider );
}

/* FixedJoint constructor */
VALUE
ode_fixedJoint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateFixed );
}


/* UniversalJoint constructor */
VALUE
ode_universalJoint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateUniversal );
}


/* AngularMotorJoint constructor */
VALUE
ode_aMotorJoint_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
	return ode_joint_new( argc, argv, self, dJointCreateAMotor );
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/*
 * ODE::Joint#attach( body1, body2 )
 * --
 * Attach the specified bodies (ODE::Body objects) with the receiving joint.
 */
VALUE
ode_joint_attach( self, body1, body2 )
	 VALUE self, body1, body2;
{
	ode_JOINT	*jointStruct;
	ode_BODY	*body1Struct, *body2Struct;

	CheckForObsoleteJoint( self );
	GetJoint( self, jointStruct );
	GetBody( body1, body1Struct );
	GetBody( body2, body2Struct );

	// :TODO: Perhaps add callbacks into any bodies being detached?

	dJointAttach( jointStruct->id, body1Struct->id, body2Struct->id );
	jointStruct->body1 = body1;
	jointStruct->body2 = body2;

	return Qtrue;
}


/*
 * ODE::Joint#attachedBodies()
 * --
 * Return an Array containing the two bodies which the recieving joint attaches,
 * if any.
 */
VALUE
ode_joint_attached_bodies( self )
	 VALUE self;
{
	ode_JOINT	*jointStruct;

	CheckForObsoleteJoint( self );
	GetJoint( self, jointStruct );

	if ( jointStruct->body1 )
		return rb_ary_new3( 2, jointStruct->body1, jointStruct->body2 );
	else
		return rb_ary_new2( 0 );
}


/*
 * ODE::Joint#obsolete?() 
 * --
 * Returns +true+ if the joint was a member of a ODE::JointGroup that has been
 * emptied. A joint cannot be used if it is marked as obsolete.
 */
VALUE
ode_joint_obsolete_p( self )
	 VALUE self;
{
	return RTEST(rb_iv_get( self, "@obsolete" )) ? Qtrue : Qfalse;
}


/*
 * ODE::Joint#makeObsolete()
 * --
 * Mark the specified ODE::Joint as obsolete (ie., it was a member of an
 * ODE::JointGroup that has been emptied). The object will be useless after this
 * is done.
 */
VALUE
ode_joint_make_obsolete( self )
	 VALUE	self;
{
	VALUE rv;
	rv = rb_iv_set( self, "@obsolete", Qtrue );
	OBJ_FREEZE( self );

	return rv;
}


/*
 * ODE::Joint#feedback
 * --
 * Get the feedback hash for the receiving joint. This hash will be updated
 * every time step with the torque and force applied to the attached bodies if
 * feedback is enabled for the receiving joint. If feedback is not enabled, the
 * hash will be empty.
 */
VALUE
ode_joint_get_feedback( self )
	 VALUE	self;
{
	VALUE		fbhash;
	ode_JOINT	*jointStruct;

	CheckForObsoleteJoint( self );
	GetJoint( self, jointStruct );
	if ( ! jointStruct->feedback ) return Qnil;

	fbhash = ode_joint_get_feedback_hash( (ode_JOINT *)jointStruct );

	// If the feedback struct is being populated, update the hash with its data.
	if ( jointStruct->feedback ) {
		VALUE b1hash, b2hash;
		VALUE b1Force, b2Force, b1Torque, b2Torque;

		b1hash = rb_hash_aref( fbhash, body1Sym );
		b2hash = rb_hash_aref( fbhash, body2Sym );

		b1Force  = rb_hash_aref( b1hash, forceSym );
		b1Torque = rb_hash_aref( b1hash, torqueSym );
		b2Force  = rb_hash_aref( b2hash, forceSym );
		b2Torque = rb_hash_aref( b2hash, torqueSym );

		SetOdeVectorFromVec3( jointStruct->feedback->f1, b1Force );
		SetOdeVectorFromVec3( jointStruct->feedback->t1, b1Torque );
		SetOdeVectorFromVec3( jointStruct->feedback->f2, b2Force );
		SetOdeVectorFromVec3( jointStruct->feedback->t2, b2Torque );
	}

	return fbhash;
}


/*
 * ODE::Joint#feedbackEnabled?
 * --
 * Returns <tt>true</tt> if feedback is turned on for this joint.
 */
VALUE
ode_joint_feedback_enabled_p( self )
	 VALUE self;
{
	ode_JOINT	*jointStruct;

	CheckForObsoleteJoint( self );
	GetJoint( self, jointStruct );
	
	if ( jointStruct->feedback )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * ODE::Joint#feedbackEnabled=( value )
 * --
 * Enable/disable feedback for this joint. If <tt>value</tt> is true, feedback
 * will be enabled for the next world step. Returns the previous 
 */
VALUE
ode_joint_feedback_enabled( self, value )
	 VALUE self, value;
{
	ode_JOINT	*jointStruct;
	VALUE		rval;

	CheckForObsoleteJoint( self );
	GetJoint( self, jointStruct );
	
	// If feedback is being turned on, allocate a new feedback struct and set it
	// in the joint as well as our own struct.
	if ( RTEST(value) ) {
		if ( ! jointStruct->feedback ) {
			jointStruct->feedback = ALLOC( dJointFeedback );
			dJointSetFeedback( jointStruct->id, jointStruct->feedback );
		}

		rval = Qfalse;
	}

	// Otherwise, unset the feedback struct and free it
	else {
		if ( jointStruct->feedback ) {
			dJointSetFeedback( jointStruct->id, 0 );
			free( jointStruct->feedback );
			jointStruct->feedback = 0;
		}

		rval = Qtrue;
	}

	return rval;
}


/*
 *	Fetch the cached feedback hash from the joint, or create and cache a new
 *	one.
 */
static VALUE
ode_joint_get_feedback_hash( jointStruct )
	 ode_JOINT	*jointStruct;
{
	VALUE		fbhash;

	// Make the toplevel hash if it's net yet defined, or just get it if it is
	// defined.
	if ( jointStruct->fbhash == Qnil ) {

		fbhash = rb_hash_new();
		jointStruct->fbhash = fbhash;

		rb_hash_aset( fbhash, body1Sym, ode_joint_make_bfhash() );
		rb_hash_aset( fbhash, body2Sym, ode_joint_make_bfhash() );
	} else {
		fbhash = jointStruct->fbhash;
	}

	return fbhash;
}


/*
 * Create and return a new Hash with :force => ODE::Force, and :torque =>
 * ODE::Torque pairs.
 */
static VALUE
ode_joint_make_bfhash()
{
	VALUE bfhash;

	bfhash = rb_hash_new();
	rb_hash_aset( bfhash, forceSym,  rb_class_new_instance(0, 0, ode_cOdeForce) );
	rb_hash_aset( bfhash, torqueSym, rb_class_new_instance(0, 0, ode_cOdeTorque) );

	return bfhash;
}



/*
 * Initializer for ODE::Joint and subclasses 
*/
void
ode_init_joints( void )
{
	// Define the symbol constants for the keys of the feedback hash
	body1Sym	= ID2SYM(rb_intern("body1"));
	body2Sym	= ID2SYM(rb_intern("body2"));
	torqueSym	= ID2SYM(rb_intern("torque"));
	forceSym	= ID2SYM(rb_intern("force"));

	// ODE::Joint class (abstract)
	rb_define_singleton_method( ode_cOdeJoint, "new", ode_joint_abstract_class, -1 );
	rb_define_method( ode_cOdeJoint, "attach", ode_joint_attach, 2 );
	rb_define_method( ode_cOdeJoint, "attachedBodies", ode_joint_attached_bodies, 0 );
	rb_define_method( ode_cOdeJoint, "obsolete?", ode_joint_obsolete_p, 0 );
	rb_define_method( ode_cOdeJoint, "makeObsolete", ode_joint_make_obsolete, 0 );
	rb_define_method( ode_cOdeJoint, "feedback", ode_joint_get_feedback, 0 );
	rb_define_method( ode_cOdeJoint, "feedback=", ode_joint_feedback_enabled, 1 );
	rb_define_alias ( ode_cOdeJoint, "feedback_enabled=", "feedback=" );
	rb_define_method( ode_cOdeJoint, "feedback?", ode_joint_feedback_enabled_p, 0 );
	rb_define_alias ( ode_cOdeJoint, "feedback_enabled?", "feedback?" );

	// ODE::BallJoint class
	rb_define_singleton_method( ode_cOdeBallJoint, "new", ode_ballJoint_new, -1 );

	// ODE::HingeJoint class
	rb_define_singleton_method( ode_cOdeHingeJoint, "new", ode_hingeJoint_new, -1 );

	// ODE::Hinge2Joint class
	rb_define_singleton_method( ode_cOdeHinge2Joint, "new", ode_hinge2Joint_new, -1 );

	// ODE::SliderJoint class
	rb_define_singleton_method( ode_cOdeSliderJoint, "new", ode_sliderJoint_new, -1 );

	// ODE::FixedJoint class
	rb_define_singleton_method( ode_cOdeFixedJoint, "new", ode_fixedJoint_new, -1 );

	// ODE::UniversalJoint class
	rb_define_singleton_method( ode_cOdeUniversalJoint, "new", ode_universalJoint_new, -1 );

	// ODE::AngularMotorJoint class
	rb_define_singleton_method( ode_cOdeAMotorJoint, "new", ode_aMotorJoint_new, -1 );

	// ODE::ContactJoint class
	rb_define_singleton_method( ode_cOdeContactJoint, "new", ode_contactJoint_new, -1 );
}

