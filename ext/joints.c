/*
 *		joints.c - ODE Ruby Binding - Joint Classes
 *		$Id: joints.c,v 1.5 2003/02/08 08:25:46 deveiant Exp $
 *		Time-stamp: <04-Feb-2003 15:31:57 deveiant>
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
 * Macros and constants
 * -------------------------------------------------- */
static VALUE body1Sym;
static VALUE body2Sym;
static VALUE torqueSym;
static VALUE forceSym;

/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */
static VALUE ode_joint_get_feedback_hash( ode_JOINT * );
static VALUE ode_joint_make_bfhash();


/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static ode_JOINT *
ode_joint_alloc()
{
	ode_JOINT *ptr = ALLOC( ode_JOINT );

	ptr->object		= Qnil;
	ptr->world		= Qnil;
	ptr->body1		= Qnil;
	ptr->body2		= Qnil;
	ptr->jointGroup = Qnil;
	ptr->fbhash		= Qnil;
	ptr->contact	= Qnil;
	ptr->obsolete	= Qnil;
	ptr->id			= NULL;
	ptr->feedback	= NULL;

	debugMsg(( "Initialized ode_JOINT <%p>", ptr ));
	return ptr;
}


/*
 * GC Mark function
 */
static void
ode_joint_gc_mark( ptr )
	 ode_JOINT *ptr;
{
	debugMsg(( "Marking an ODE::Joint" ));
	if ( ptr ) {
		debugMsg(( "Marking Joint <%p>", ptr ));

		/* Mark the world the joint belongs to */
		rb_gc_mark( ptr->world );
		rb_gc_mark( ptr->body1 );
		rb_gc_mark( ptr->body2 );
		rb_gc_mark( ptr->jointGroup );
		rb_gc_mark( ptr->fbhash );
		rb_gc_mark( ptr->contact );
	}

	else {
		debugMsg(( "Not marking uninitialized ode_JOINT" ));
	}
}


/*
 * GC Free function
 */
static void
ode_joint_gc_free( ptr )
	 ode_JOINT *ptr;
{
	if ( ptr ) {
		if ( ! ptr->jointGroup )
			dJointDestroy( ptr->id );

		ptr->id			= NULL;
		ptr->object		= Qnil;
		ptr->world		= Qnil;
		ptr->body1		= Qnil;
		ptr->body2		= Qnil;
		ptr->jointGroup = Qnil;
		ptr->fbhash		= Qnil;
		ptr->contact	= Qnil;
		ptr->obsolete	= Qnil;

		if ( ptr->feedback ) xfree( ptr->feedback );
		ptr->feedback	= NULL;

		debugMsg(( "Freeing ode_JOINT <%p>", ptr ));
		xfree( ptr );
		ptr = NULL;
	}

	else {
		debugMsg(( "Not freeing uninitialized ode_JOINT" ));
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ode_JOINT *
check_joint( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Joint object (%d).", self ));
	Check_Type( self, T_DATA );
	debugMsg(( "...is a T_DATA object" ));

    if ( !IsJoint(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Joint)",
				  rb_class2name(CLASS_OF( self )) );
    }

	debugMsg(( "...is a ode_cOdeJoint derivative." ));
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_JOINT *
get_joint( self )
	 VALUE self;
{
	ode_JOINT *ptr = check_joint( self );

	debugMsg(( "Fetching an ode_JOINT (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized joint" );
	if ( RTEST(ptr->obsolete) )
		rb_raise( ode_eOdeObsoleteJointError,
				  "Cannot use a joint which has been marked obsolete." );

	return ptr;
}


/*
 * Publicly-usable joint-fetcher.
 */
ode_JOINT *
ode_get_joint( self )
	 VALUE self;
{
	return get_joint(self);
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * allocate()
 * --
 * Allocate a new ODE::Joint object.
 */
static VALUE
ode_joint_s_alloc( klass )
{
	if ( klass == ode_cOdeJoint || klass == ode_cOdeParamJoint )
		rb_raise( rb_eScriptError, "Instantiation attempted of abstract class." );

	debugMsg(( "Wrapping an uninitialized ODE::Joint pointer." ));
	return Data_Wrap_Struct( klass, ode_joint_gc_mark, ode_joint_gc_free, 0 );
}



/* --------------------------------------------------
 * Constructor functions
 * -------------------------------------------------- */

/*
 * Common constructor -- all joint classes (except ContactJoint, which has an
 * additional parameter, and so can't use this constructor) use this to build
 * the actual object, passing a pointer to the specific joint constructor used
 * to create their particular flavor of joint.
 */
static VALUE
ode_joint_construct( argc, argv, self, constructor )
	 int			argc;
	 VALUE			*argv, self;
	 dJointID		(*constructor)( dWorldID, dJointGroupID );
{
	debugMsg(( "ODE::Joint init." ));

	if ( !check_joint(self) ) {
		ode_JOINT		*ptr;
		VALUE			world, jointGroup = Qnil;
		dWorldID		worldId;
		dJointGroupID	jointGroupId = 0;

		debugMsg(( "Creating a new Joint: checking args." ));

		/* Scan arguments */
		rb_scan_args( argc, argv, "11", &world, &jointGroup );

		/* Unwrap the underlying ruby objects */
		worldId = ode_get_world( world );
		if ( RTEST(jointGroup) ) {
			ode_JOINTGROUP	*jointGroupPtr = ode_get_jointGroup( jointGroup );

			jointGroupId = jointGroupPtr->id;
			debugMsg(( "Got a JointGroup for new Joint: <%p>", jointGroupId ));
		}

		else {
			debugMsg(( "No JointGroup for new Joint" ));
			jointGroupId = 0;
		}

		/* Allocate a joint struct and set its members */
		debugMsg(( "Creating a new Joint: ALLOCing and setting struct members." ));
		DATA_PTR( self )	= ptr = ode_joint_alloc();

		ptr->object			= self;
		ptr->world			= world;
		ptr->jointGroup		= jointGroup;
		ptr->obsolete		= Qfalse;

		ptr->id = (constructor)( worldId, jointGroupId );

		/* Set the Ruby  */
		dJointSetData( ptr->id, ptr );

		if ( RTEST(jointGroup) )
			ode_jointGroup_register_joint( jointGroup, self );
	}

	/* Can't initialize twice, as a joint cannot be removed from a world. */
	else {
		rb_raise( rb_eRuntimeError,
				  "Cannot re-initialize a joint once it's been set in a world." );
	}

	return self;
}


/* --------------------------------------------------
 * Utility Methods
 * -------------------------------------------------- */

/*
 * Set a 3-dReal parameter on a joint given the dJointID, a ruby value that will
 * be transformed into three doubles via to_ary, the name of the parameter (for
 * building error messages), a class to instantiate as the return value (an
 * ODE::Vector or derivative), and a function pointer to the actual set
 * function.
 */
static VALUE
ode_set_joint_param3( id, ary, name, fptr, rklass )
	 dJointID	id;
	 VALUE		ary, rklass;
	 const char	name[];
	 void		(*fptr)( dJointID, dReal x, dReal y, dReal z );
{
	VALUE posArray;

	debugMsg(( "In ode_set_joint_param3: ary.length = %d", RARRAY(ary)->len ));

	if ( RARRAY(ary)->len == 1 )
		posArray = ode_obj_to_ary3( *(RARRAY(ary)->ptr), name );
	else
		posArray = ode_obj_to_ary3( ary, name );

	debugMsg(( "posArray is a %s",
			   rb_class2name(CLASS_OF(posArray)) ));

	(fptr)( id,
			(dReal)NUM2DBL(*(RARRAY(posArray)->ptr    )),
			(dReal)NUM2DBL(*(RARRAY(posArray)->ptr + 1)),
			(dReal)NUM2DBL(*(RARRAY(posArray)->ptr + 2)) );

	/* When/if Math3d supports the new allocation framework, this should be: */
	/* return rb_class_new_instance( 3, RARRAY(posArray)->ptr, rklass ); */
	return rb_class_new_instance( 3, RARRAY(posArray)->ptr, rklass );
}


/*
 * Get a parameter of a joint using the specified id and function pointer and
 * return it as an instance of the specified rklass.
 */
static VALUE
ode_get_joint_param3( id, fptr, rklass )
	 dJointID	id;
	 void		(*fptr)( dJointID, dVector3 );
	 VALUE		rklass;
{
	dVector3	pos;
	VALUE		axes[3];
	int			i;

	debugMsg(( "In ode_get_joint_param3." ));

	(fptr)( id, pos );
	for ( i = 0; i <= 2; i++ ) {
		debugMsg(( "   Setting axis %d to %f",
				   i, *((pos)+i) ));
		axes[i] = rb_float_new( *((pos) + i) );
	}

	return rb_class_new_instance( 3, axes, rklass );
}



/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/*
 * ODE::Joint#attach( body1, body2 )
 * --
 * Attach the specified bodies (ODE::Body objects) with the receiving joint. If
 * the joint is already attached, it will be detached from the old bodies
 * first. To attach this joint to only one body, set body1 or body2 to
 * <tt>nil</tt> - a <tt>nil</tt> body refers to the static environment. Setting
 * both bodies to <tt>nil</tt> puts the joint into "limbo", i.e. it will have no
 * effect on the simulation.
 */
static VALUE
ode_joint_attach( self, body1, body2 )
	 VALUE self, body1, body2;
{
	ode_JOINT	*ptr = get_joint( self );
	ode_BODY	*body1Ptr, *body2Ptr;
	dBodyID		body1Id, body2Id;

	if ( body1 == Qnil ) {
		body1Id = 0;
	} else {
		body1Ptr = ode_get_body( body1 );
		body1Id	 = body1Ptr->id;
	}

	if ( body2 == Qnil ) {
		body2Id = 0;
	} else {
		body2Ptr = ode_get_body( body2 );
		body2Id	 = body2Ptr->id;
	}

	/* :TODO: Perhaps add callbacks into any bodies being detached? */

	dJointAttach( ptr->id, body1Id, body2Id );
	ptr->body1 = body1;
	ptr->body2 = body2;

	return Qtrue;
}


/*
 * ODE::Joint#attachedBodies()
 * --
 * Return an Array containing the two bodies which the recieving joint attaches,
 * if any.
 */
static VALUE
ode_joint_attached_bodies( self )
	 VALUE self;
{
	ode_JOINT	*ptr = get_joint( self );

	if ( ptr->body1 )
		return rb_ary_new3( 2, ptr->body1, ptr->body2 );
	else
		return rb_ary_new2( 0 );
}


/*
 * ODE::Joint#obsolete?() 
 * --
 * Returns +true+ if the joint was a member of a ODE::JointGroup that has been
 * emptied. A joint cannot be used if it is marked as obsolete.
 */
static VALUE
ode_joint_obsolete_p( self )
	 VALUE self;
{
	ode_JOINT *ptr = check_joint( self );
	if ( ptr )
		return ptr->obsolete;
	else
		return Qnil;
}


/*
 * ODE::Joint#makeObsolete()
 * --
 * Mark the specified ODE::Joint as obsolete (ie., it was a member of an
 * ODE::JointGroup that has been emptied). The object will be useless after this
 * is done.
 */
static VALUE
ode_joint_make_obsolete( self )
	 VALUE	self;
{
	ode_JOINT *ptr = get_joint( self );

	ptr->obsolete = Qtrue;
	return Qtrue;
}


/*
 * ODE::Joint#feedback
 * --
 * Get the feedback hash for the receiving joint. This hash will be updated
 * every time step with the torque and force applied to the attached bodies if
 * feedback is enabled for the receiving joint. If feedback is not enabled, the
 * hash will be empty.
 */
static VALUE
ode_joint_get_feedback( self )
	 VALUE	self;
{
	VALUE		fbhash;
	ode_JOINT	*ptr = get_joint( self );

	if ( ! ptr->feedback ) return Qnil;

	fbhash = ode_joint_get_feedback_hash( ptr );

	/* If the feedback struct is being populated, update the hash with its data. */
	if ( ptr->feedback ) {
		VALUE b1hash, b2hash;
		VALUE b1Force, b2Force, b1Torque, b2Torque;

		b1hash = rb_hash_aref( fbhash, body1Sym );
		b2hash = rb_hash_aref( fbhash, body2Sym );

		b1Force  = rb_hash_aref( b1hash, forceSym );
		b1Torque = rb_hash_aref( b1hash, torqueSym );
		b2Force  = rb_hash_aref( b2hash, forceSym );
		b2Torque = rb_hash_aref( b2hash, torqueSym );

		SetOdeVectorFromVec3( ptr->feedback->f1, b1Force );
		SetOdeVectorFromVec3( ptr->feedback->t1, b1Torque );
		SetOdeVectorFromVec3( ptr->feedback->f2, b2Force );
		SetOdeVectorFromVec3( ptr->feedback->t2, b2Torque );
	}

	return fbhash;
}


/*
 * ODE::Joint#feedbackEnabled?
 * --
 * Returns <tt>true</tt> if feedback is turned on for this joint.
 */
static VALUE
ode_joint_feedback_enabled_p( self )
	 VALUE self;
{
	ode_JOINT	*ptr = get_joint( self );

	if ( ptr->feedback )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * ODE::Joint#feedbackEnabled=( value )
 * --
 * Enable/disable feedback for this joint. If <tt>value</tt> is true, feedback
 * will be enabled for the next world step. Returns the status of feedback flag
 * before the call.
 */
static VALUE
ode_joint_feedback_enabled( self, value )
	 VALUE self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	VALUE		rval = Qfalse;

	/* If feedback is being turned on, allocate a new feedback struct and set it 
	   in the joint as well as our own struct. */
	if ( RTEST(value) ) {
		if ( ! ptr->feedback ) {
			ptr->feedback = ALLOC( dJointFeedback );
			dJointSetFeedback( ptr->id, ptr->feedback );
		} else {
			rval = Qtrue;
		}
	}

	/* Otherwise, unset the feedback struct and free it */
	else if ( ptr->feedback ) {
		dJointSetFeedback( ptr->id, 0 );
		xfree( ptr->feedback );
		ptr->feedback = 0;

		rval = Qtrue;
	}

	return rval;
}


/*
 *	Fetch the cached feedback hash from the joint, or create and cache a new
 *	one.
 */
static VALUE
ode_joint_get_feedback_hash( ptr )
	 ode_JOINT	*ptr;
{
	VALUE		fbhash;

	/* Make the toplevel hash if it's net yet defined, or just get it if it is 
	   defined. */
	if ( ptr->fbhash == Qnil ) {

		fbhash = rb_hash_new();
		ptr->fbhash = fbhash;

		rb_hash_aset( fbhash, body1Sym, ode_joint_make_bfhash() );
		rb_hash_aset( fbhash, body2Sym, ode_joint_make_bfhash() );
	} else {
		fbhash = ptr->fbhash;
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


/* --- ODE::BallJoint ------------------------------ */

/*
 * ODE::BallJoint#initialize()
 * --
 * Create and return a new ODE::BallJoint.
 */
static VALUE
ode_ballJoint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateBall, 0 );
}


/*
 * ODE::BallJoint#anchor
 * --
 * Return the joint anchor point as an ODE::Position.
 */
static VALUE
ode_ballJoint_anchor( self )
	 VALUE self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetBallAnchor, ode_cOdePosition );
}


/*
 * ODE::BallJoint#anchor=( position )
 * --
 * Set the joint's anchor point, which can be any object which returns an array
 * with 3 numeric values when <tt>to_ary</tt> is called on it, such as an
 * ODE::Position object, a Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_ballJoint_anchor_eq( self, position )
	 VALUE	self, position;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, position, "anchor",
								 dJointSetBallAnchor, ode_cOdePosition );
}



/* --- ODE::FixedJoint ------------------------------ */

/*
 * ODE::FixedJoint#initialize( world, jointGroup )
 * --
 * Create and return a new ODE::FixedJoint.
 */
static VALUE
ode_fixedJoint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateFixed, 0 );
}


/*
 * ODE::FixedJoint#fix()
 * --
 * Call this on the fixed joint after it has been attached to remember the
 * current desired relative offset between the bodies.
 */
static VALUE
ode_fixedJoint_fix( self )
	 VALUE self;
{
	ode_JOINT *ptr = get_joint( self );
	dJointSetFixed( ptr->id );

	return Qtrue;
}



/* --- ODE::UniversalJoint ------------------------------ */

/*
 * ODE::UniversalJoint#initialize( world, jointGroup )
 * --
 * Create and return a new ODE::UniversalJoint.
 */
static VALUE
ode_universalJoint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateUniversal, 0 );
}


/*
 * ODE::UniversalJoint#anchor
 * --
 * Get the position of the joint's anchor as an ODE::Position object.
 */
static VALUE
ode_universalJoint_anchor( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetUniversalAnchor, ode_cOdePosition );
}


/*
 * ODE::UniversalJoint#anchor=( position )
 * --
 * Set the position of the joint's anchor. The <tt>position</tt> argument can be
 * any object which returns an array of three numeric values when
 * <tt>to_ary</tt> is called on it, such as a Math3d::Vector3, an ODE::Position,
 * or an Array with three numeric elements.
 */
static VALUE
ode_universalJoint_anchor_eq( self, position )
	 VALUE	self, position;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, position, "anchor", dJointSetUniversalAnchor,
								 ode_cOdePosition );
}



/*
 * ODE::UniversalJoint#axis1()
 * --
 * Get the first axis of the joint as an ODE::Vector;
 */
static VALUE
ode_universalJoint_axis1( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetUniversalAxis1, ode_cOdeVector );
}


/*
 * ODE::UniversalJoint#axis1=( vector )
 * --
 * Set the first axis of the joint, which must be parallel to the second
 * axis. The <tt>vector</tt> argument can be any object which returns an array
 * of three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_universalJoint_axis1_eq( self, axis )
	 VALUE	self, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, axis, "axis", dJointSetUniversalAxis1,
								 ode_cOdeVector );
}


/*
 * ODE::UniversalJoint#axis2()
 * --
 * Get the second axis of the joint as an ODE::Vector.
 */
static VALUE
ode_universalJoint_axis2( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetUniversalAxis2, ode_cOdeVector );
}


/*
 * ODE::UniversalJoint#axis2=( vector )
 * --
 * Set the second axis of the joint, which must be parallel to the first
 * axis. The <tt>vector</tt> argument can be any object which returns an array
 * of three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_universalJoint_axis2_eq( self, axis )
	 VALUE	self, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, axis, "axis", dJointSetUniversalAxis2,
								 ode_cOdeVector );
}



/* --- ODE::ContactJoint ------------------------------ */

/*
 * ODE::ContactJoint::initialize( contact, world, jointGroup )
 * --
 * Create and return a new contact joint for the given contact.
 */
static VALUE
ode_contactJoint_init( argc, argv, self )
	 int		argc;
	 VALUE		*argv, self;
{
	debugMsg(( "ODE::Joint init." ));

	if ( !check_joint(self) ) {
		ode_JOINT		*ptr;
		VALUE			world, contact, jointGroup = Qnil;
		dWorldID		worldId;
		ode_CONTACT		*contactPtr;
		dJointGroupID	jointGroupId = 0;

		/* Scan arguments */
		debugMsg(( "Creating a new Joint: checking args." ));
		rb_scan_args( argc, argv, "21", &world, &contact, &jointGroup );

		/* Unwrap the underlying values of the world, the contact object, and
		   the joint group if it was specified. */
		worldId = ode_get_world( world );
		contactPtr = ode_get_contact( contact );
		if ( jointGroup ) {
			ode_JOINTGROUP	*jointGroupPtr = ode_get_jointGroup( jointGroup );
			jointGroupId = jointGroupPtr->id;
			debugMsg(( "Got a JointGroup for new Joint: <%p>", jointGroupId ));
		}

		else {
			debugMsg(( "No JointGroup for new Joint" ));
		}

		/* Allocate a joint struct and set its members */
		debugMsg(( "Creating a new Joint: ALLOCing and setting struct members." ));
		DATA_PTR( self )	= ptr = ode_joint_alloc();

		ptr->object			= self;
		ptr->world			= world;
		ptr->contact		= contact;
		ptr->jointGroup		= jointGroup;
		ptr->obsolete		= Qfalse;

		ptr->id = dJointCreateContact( worldId, jointGroupId, contactPtr->contact );

		/* Set the Ruby struct as the data pointer of the ODE object */
		dJointSetData( ptr->id, ptr );

		/* Register the new joint with the joint group specified, if there was
		   one. */
		if ( jointGroup )
			ode_jointGroup_register_joint( jointGroup, self );
	}

	return self;
}


/*
 * ODE::ContactJoint#contact
 * --
 * Returns the ODE::Contact object associated with the joint.
 */
static VALUE
ode_contactJoint_contact( self )
	 VALUE self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ptr->contact;
}



/* --- ODE::ParameterizedJoint ------------------------------ */

/*
 * Lame-ass macro to look up the correct get function-pointer given a subclass
 * of ParamJoint.
 */
#define JointParamGetFunction( fptr, obj ) {\
	if ( rb_obj_is_kind_of((obj), ode_cOdeHingeJoint) ) {\
		(fptr) = dJointGetHingeParam;\
	} \
	else if ( rb_obj_is_kind_of((obj), ode_cOdeHinge2Joint) ) {\
		(fptr) = dJointGetHinge2Param;\
	} \
	else if ( rb_obj_is_kind_of((obj), ode_cOdeSliderJoint) ) {\
		(fptr) = dJointGetSliderParam;\
	} \
	else if ( rb_obj_is_kind_of((obj), ode_cOdeAMotorJoint) ) {\
		(fptr) = dJointGetAMotorParam; \
	} \
	else \
		rb_bug( "No available function for '%s'.", \
				rb_class2name(CLASS_OF( obj )) ); \
}


/*
 * Lame-ass macro to look up the correct set function-pointer given a subclass
 * of ParamJoint.
 */
#define JointParamSetFunction( fptr, obj ) {\
	if ( rb_obj_is_kind_of((obj), ode_cOdeHingeJoint) ) {\
		(fptr) = dJointSetHingeParam; \
	} \
	else if ( rb_obj_is_kind_of((obj), ode_cOdeHinge2Joint) ) {\
		(fptr) = dJointSetHinge2Param; \
	} \
	else if ( rb_obj_is_kind_of((obj), ode_cOdeSliderJoint) ) {\
		(fptr) = dJointSetSliderParam; \
	} \
	else if ( rb_obj_is_kind_of((obj), ode_cOdeAMotorJoint) ) {\
		(fptr) = dJointSetAMotorParam; \
	} \
	else \
		rb_bug( "No available function for '%s'.", \
				rb_class2name(CLASS_OF( obj )) ); \
}


/*
 * Get the offset to the dParamX parameter number for ParameterizedJoint get/set
 * functions, as determined by the number appended to the name of the current
 * Ruby method (eg., vel2, CFM3, etc.).
 */
static inline int
ode_axis_offset( self )
	 VALUE self;
{
	const char *methodName = rb_id2name( rb_frame_last_func() );
	int len = strlen(methodName);
	long i;

	if ( methodName[len -1] == '=' )
		i = strtol( methodName + (strlen(methodName) - 2), NULL, 10 );
	else
		i = strtol( methodName + (strlen(methodName) - 1), NULL, 10 );

	if ( i != 2 && i != 3 ) i = 1;
	if ( i > FIX2INT(rb_const_get(CLASS_OF(self), rb_intern("Axes"))) )
		rb_raise( rb_eIndexError, "No such axis %d for %s",
				  i, rb_class2name(CLASS_OF(self)) );

	debugMsg(( "Axis index for '%s' of a %s is '%li'",
			   methodName, rb_class2name(CLASS_OF(self)), i ));
	return dParamGroup * (i-1);
}


/*
 * ODE::ParamJoint#loStop()
 * --
 * Get the low stop angle or position.
 */
static VALUE
ode_paramJoint_LoStop( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id,
									dParamLoStop + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#loStop=( value )
 * --
 * Set the low stop angle or position. Setting this to -dInfinity (the default
 * value) turns off the low stop. For rotational joints, this stop must be
 * greater than -Pi to be effective.
 */
static VALUE
ode_paramJoint_LoStop_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	(setParam)( ptr->id,
				dParamLoStop + ode_axis_offset(self),
				(dReal)NUM2DBL(value) );
	return ode_paramJoint_LoStop( self );
}


/*
 * ODE::ParamJoint#hiStop()
 * --
 * Get the high stop angle or position.
 */
static VALUE
ode_paramJoint_HiStop( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamHiStop + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#hiStop=( value )
 * --
 * Set the high stop angle or position. Setting this to dInfinity (the default
 * value) turns off the high stop. For rotational joints, this stop must be less
 * than Pi to be effective. If the high stop is less than the low stop then both
 * stops will be ineffective.
 */
static VALUE
ode_paramJoint_HiStop_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	(setParam)( ptr->id, dParamHiStop + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_HiStop( self );
}


/*
 * ODE::ParamJoint#vel()
 * --
 * Get motor velocity (this will be an angular or linear velocity).
 */
static VALUE
ode_paramJoint_Vel( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamVel + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#vel=( velocity )
 * --
 * Set desired motor velocity (this will be an angular or linear velocity).
 */
static VALUE
ode_paramJoint_Vel_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	(setParam)( ptr->id, dParamVel + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_Vel( self );
}


/*
 * ODE::ParamJoint#fMax()
 * --
 * Get the maximum force or torque that the motor will use to achieve the
 * desired velocity.
 */
static VALUE
ode_paramJoint_FMax( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamFMax + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#fMax=( value )
 * --
 * Set the maximum force or torque that the motor will use to achieve the
 * desired velocity. This must always be greater than or equal to zero. Setting
 * this to zero (the default value) turns off the motor.
 */
static VALUE
ode_paramJoint_FMax_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckPositiveNumber( NUM2DBL(value), "value" );

	(setParam)( ptr->id, dParamFMax + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_FMax( self );
}


/*
 * ODE::ParamJoint#fudgeFactor()
 * --
 * Get the current joint stop/motor fudge factor. See the ODE docs for more on
 * what this is.
 */
static VALUE
ode_paramJoint_FudgeFactor( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamFudgeFactor + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#fudgeFactor=( factor )
 * --
 * Set the joint/motor fudge factor.
 */
static VALUE
ode_paramJoint_FudgeFactor_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0, 1 );

	(setParam)( ptr->id, dParamFudgeFactor + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_FudgeFactor( self );
}


/*
 * ODE::ParamJoint#bounce()
 * --
 * Get the elasticity of the stops as the coefficient of restitution, which will
 * be a value between 0 and 1, inclusive. 0 means the stops are not elastic at
 * all; 1 is perfect elasticity.
 */
static VALUE
ode_paramJoint_Bounce( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamBounce + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#bounce=( value )
 * --
 * Set the elasticity of the stops as the coefficient of restitution, which will
 * be a value between 0 and 1, inclusive. 0 means the stops are not elastic at
 * all; 1 is perfect elasticity.
 */
static VALUE
ode_paramJoint_Bounce_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0, 1 );

	(setParam)( ptr->id, dParamBounce + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_Bounce( self );
}


/*
 * ODE::ParamJoint#CFM()
 * --
 * Get the constraint force mixing (CFM) value used when not at a stop.
 */
static VALUE
ode_paramJoint_CFM( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamCFM + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#CFM=( value )
 * --
 * Set the constraint force mixing (CFM) value used when not at a stop.
 */
static VALUE
ode_paramJoint_CFM_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0.0, 1.0 );

	(setParam)( ptr->id, dParamCFM + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_CFM( self );
}


/*
 * ODE::ParamJoint#stopERP()
 * --
 * Get the error reduction parameter (ERP) used by the stops.
 */
static VALUE
ode_paramJoint_StopERP( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamStopERP + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#stopERP=( value )
 * --
 * Set the error reduction parameter (ERP) used by the stops.
 */
static VALUE
ode_paramJoint_StopERP_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0, 1 );

	(setParam)( ptr->id, dParamStopERP + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_StopERP( self );
}


/*
 * ODE::ParamJoint#stopCFM()
 * --
 * Get constraint force mixing (CFM) value used by the stops.
 */
static VALUE
ode_paramJoint_StopCFM( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamStopCFM + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#stopCFM=( value )
 * --
 * Set the constraint force mixing (CFM) value used by the stops. Together with
 * the ERP value this can be used to get spongy or soft stops. Note that this is
 * intended for unpowered joints, it does not really work as expected when a
 * powered joint reaches its limit.
 */
static VALUE
ode_paramJoint_StopCFM_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0, 1 );

	(setParam)( ptr->id, dParamStopCFM + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_StopCFM( self );
}


/*
 * ODE::ParamJoint#suspensionERP()
 * --
 * Get the suspension error reduction parameter (ERP). Currently this is only
 * implemented for ODE::Hinge2Joint.
 */
static VALUE
ode_paramJoint_SuspensionERP( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamSuspensionERP + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#suspensionERP=( value )
 * --
 * Set the suspension error reduction parameter (ERP). Currently this is only
 * implemented for ODE::Hinge2Joint.
 */
static VALUE
ode_paramJoint_SuspensionERP_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0, 1 );

	(setParam)( ptr->id, dParamSuspensionERP + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_SuspensionERP( self );
}


/*
 * ODE::ParamJoint#suspensionCFM()
 * --
 * Get the suspension constraint force mixing (CFM) value. Currently this is
 * only implemented for ODE::Hinge2Joint.
 */
static VALUE
ode_paramJoint_SuspensionCFM( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	dReal		(* getParam)( dJointID, int );

	JointParamGetFunction( getParam, self );
	return rb_float_new( (getParam)(ptr->id, dParamSuspensionCFM + ode_axis_offset(self)) );
}


/*
 * ODE::ParamJoint#suspensionCFM=( value )
 * --
 * Set the suspension constraint force mixing (CFM) value. Currently this is
 * only implemented for ODE::Hinge2Joint.
 */
static VALUE
ode_paramJoint_SuspensionCFM_eq( self, value )
	 VALUE	self, value;
{
	ode_JOINT	*ptr = get_joint( self );
	void		(* setParam)( dJointID, int, dReal );

	JointParamSetFunction( setParam, self );
	CheckNumberBetween( NUM2DBL(value), "value", 0.0, 1.0 );

	(setParam)( ptr->id, dParamSuspensionCFM + ode_axis_offset(self), (dReal)NUM2DBL(value) );
	return ode_paramJoint_SuspensionCFM( self );
}



/* --- ODE::HingeJoint ------------------------------ */

/*
 * ODE::HingeJoint#initialize( world, jointGroup )
 * --
 * Create and return a new ODE::HingeJoint.
 */
static VALUE
ode_hingeJoint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateHinge, 0 );
}


/*
 * ODE::HingeJoint#anchor()
 * --
 * Get the hinge's anchor point as an ODE::Position object.
 */
static VALUE
ode_hingeJoint_anchor( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetHingeAnchor, ode_cOdePosition );
}


/*
 * ODE::HingeJoint#anchor=( position )
 * --
 * Set the joint's anchor point, which can be any object which returns an array
 * with 3 numeric values when <tt>to_ary</tt> is called on it, such as an
 * ODE::Position object, a Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_hingeJoint_anchor_eq( self, position )
	 VALUE	self, position;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, position, "anchor",
								 dJointSetHingeAnchor, ode_cOdePosition );
}


/*
 * ODE::HingeJoint#axis()
 * --
 * 
 */
static VALUE
ode_hingeJoint_axis( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetHingeAxis, ode_cOdeVector );
}


/*
 * ODE::HingeJoint#axis=( vector )
 * --
 * Set the first axis of the joint, which must be parallel to the second
 * axis. The <tt>vector</tt> argument can be any object which returns an array
 * of three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_hingeJoint_axis_eq( self, axis )
	 VALUE	self, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, axis, "axis",
								 dJointSetHingeAxis, ode_cOdeVector );
}


/*
 * ODE::HingeJoint#angle()
 * --
 * 
 */
static VALUE
ode_hingeJoint_angle( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetHingeAngle(ptr->id) );
}


/*
 * ODE::HingeJoint#angleRate()
 * --
 * 
 */
static VALUE
ode_hingeJoint_angle_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetHingeAngleRate(ptr->id) );
}



/* --- ODE::Hinge2Joint ------------------------------ */

/*
 * ODE::Hinge2Joint#initialize( world, jointGroup )
 * --
 * Create and return a new ODE::Hinge2Joint.
 */
static VALUE
ode_hinge2Joint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateHinge2, 0 );
}


/*
 * ODE::Hinge2Joint#anchor()
 * --
 * 
 */
static VALUE
ode_hinge2Joint_anchor( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetHinge2Anchor, ode_cOdePosition );
}



/*
 * ODE::Hinge2Joint#anchor=( position )
 * --
 * 
 */
static VALUE
ode_hinge2Joint_anchor_eq( self, position )
	 VALUE	self, position;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, position, "anchor",
								 dJointSetHinge2Anchor, ode_cOdePosition );
}


/*
 * ODE::Hinge2Joint#axis1()
 * --
 * 
 */
static VALUE
ode_hinge2Joint_axis1( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetHinge2Axis1, ode_cOdeVector );
}


/*
 * ODE::Hinge2Joint#axis1=( vector )
 * --
 * Set the first axis of the joint, which must be parallel to the second
 * axis. The <tt>vector</tt> argument can be any object which returns an array
 * of three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_hinge2Joint_axis1_eq( self, axis )
	 VALUE	self, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, axis, "axis",
								 dJointSetHinge2Axis1, ode_cOdeVector );
}


/*
 * ODE::Hinge2Joint#axis2()
 * --
 */
static VALUE
ode_hinge2Joint_axis2( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetHinge2Axis2, ode_cOdeVector );
}


/*
 * ODE::Hinge2Joint#axis2=( vector )
 * --
 * Set the second axis of the joint, which must be parallel to the first
 * axis. The <tt>vector</tt> argument can be any object which returns an array
 * of three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_hinge2Joint_axis2_eq( self, axis )
	 VALUE	self, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, axis, "axis",
								 dJointSetHinge2Axis2, ode_cOdeVector );
}


/*
 * ODE::Hinge2Joint#angle1()
 * --
 * 
 */
static VALUE
ode_hinge2Joint_angle1( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetHinge2Angle1(ptr->id) );
}


/*
 * ODE::Hinge2Joint#angle1Rate()
 * --
 * 
 */
static VALUE
ode_hinge2Joint_angle1_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetHinge2Angle1Rate(ptr->id) );
}


/*
 * ODE::Hinge2Joint#angle2Rate()
 * --
 * 
 */
static VALUE
ode_hinge2Joint_angle2_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetHinge2Angle2Rate(ptr->id) );
}



/* --- ODE::SliderJoint ------------------------------ */

/*
 * ODE::SliderJoint#initialize( world, jointGroup )
 * --
 * Create and return a new ODE::SliderJoint.
 */
static VALUE
ode_sliderJoint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateSlider, 0 );
}


/*
 * ODE::SliderJoint#axis()
 * --
 * 
 */
static VALUE
ode_sliderJoint_axis( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_joint_param3( ptr->id, dJointGetSliderAxis, ode_cOdeVector );
}


/*
 * ODE::SliderJoint#axis=( vector )
 * --
 * Set the axis of the joint. The <tt>vector</tt> argument can be any object
 * which returns an array of three numeric values when <tt>to_ary</tt> is called
 * on it, such as a Math3d::Vector3, an ODE::Vector, or an Array with three
 * numeric elements.
 */
static VALUE
ode_sliderJoint_axis_eq( self, axis )
	 VALUE	self, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_set_joint_param3( ptr->id, axis, "axis",
								 dJointSetSliderAxis, ode_cOdeVector );
}


/*
 * ODE::SliderJoint#position()
 * --
 * 
 */
static VALUE
ode_sliderJoint_position( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetSliderPosition(ptr->id) );
}


/*
 * ODE::SliderJoint#positionRate()
 * --
 * 
 */
static VALUE
ode_sliderJoint_position_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( (dReal)dJointGetSliderPositionRate(ptr->id) );
}



/* --- ODE::AngularMotorJoint ------------------------------ */

/*
 * ODE::AngularMotorJoint#initialize( world, jointGroup )
 * --
 * Create and return a new ODE::AngularMotorJoint.
 */
static VALUE
ode_aMotorJoint_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	return ode_joint_construct( argc, argv, self, dJointCreateAMotor, 0 );
}


/*
 * ODE::AngularMotorJoint#eulerMode=( boolean )
 * --
 * If set to <tt>true</tt>, the angular motor mode is set to automatically
 * compute Euler angles. The first axis is also automatically computed. The
 * AMotor axes must be set correctly when in this mode, as described below. When
 * this mode is initially set the current relative orientations of the bodies
 * will correspond to all euler angles at zero. If set to <tt>false</tt> (the
 * default), the angular motor axes and joint angle settings are entirely
 * controlled by the user.
 */
static VALUE
ode_aMotorJoint_euler_mode_eq( self, mode )
	 VALUE	self, mode;
{
	ode_JOINT	*ptr = get_joint( self );
	if ( RTEST(mode) )
		dJointSetAMotorMode( ptr->id, dAMotorEuler );
	else
		dJointSetAMotorMode( ptr->id, dAMotorUser );

	return (RTEST(mode) ? Qtrue : Qfalse);
}


/*
 * ODE::AngularMotorJoint#eulerMode?
 * --
 * Returns true if the joint has been set to Euler mode.
 */
static VALUE
ode_aMotorJoint_euler_mode_p( self )
	 VALUE self;
{
	ode_JOINT	*ptr = get_joint( self );

	if ( dJointGetAMotorMode(ptr->id) == dAMotorEuler )
		return Qtrue;
	else
		return Qfalse;
}



/*
 * ODE::AngularMotorJoint#numAxes
 * --
 * Get the number of angular axes that will be controlled by the AMotor.
 */
static VALUE
ode_aMotorJoint_num_axes( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return INT2FIX( dJointGetAMotorNumAxes(ptr->id) );
}


/*
 * ODE::AngularMotorJoint#numAxes=( num )
 * --
 * Set the number of angular axes that will be controlled by the AMotor. The num
 * argument can range from 0 (which effectively deactivates the joint) to
 * 3. This is automatically set to 3 in eulerMode.
 */
static VALUE
ode_aMotorJoint_num_axes_eq( self, arg )
	 VALUE	self, arg;
{
	ode_JOINT	*ptr = get_joint( self );
	int			num = NUM2INT( arg );
	
	if ( num < 0 || num > 3 )
		rb_raise( rb_eRangeError, "Argument out of bounds: must be between 0 and 3." );

	dJointSetAMotorNumAxes( ptr->id, num );
	return INT2FIX( dJointGetAMotorNumAxes(ptr->id) );
}


/*
 * Get a parameter of a joint using the specified id and function pointer and
 * return it as an instance of the specified rklass.
 */
static VALUE
ode_get_amotor_joint_param3( id, anum, fptr, rklass )
	 dJointID	id;
	 const int	anum;
	 void		(*fptr)( dJointID, int, dVector3 );
	 VALUE		rklass;
{
	dVector3	pos;
	VALUE		axes[3];
	int			i;

	(fptr)( id, anum, pos );
	for ( i = 0; i <= 2; i++ )
		axes[i] = rb_float_new( *((pos) + i) );

	/* When/if Math3d supports the new allocation framework, this should be: */
	/* return rb_class_new_instance( 3, axes, rklass ); */
	return rb_funcall( rklass, rb_intern("new"), 3, axes );
}


/*
 * ODE::AngularMotorJoint#axis1()
 * --
 * Set the first axis of the joint, which must be parallel to the second
 * axis. The <tt>vector</tt> argument can be any object which returns an array
 * of three numeric values when <tt>to_ary</tt> is called on it, such as a
 * Math3d::Vector3, an ODE::Vector, or an Array with three numeric elements.
 */
static VALUE
ode_aMotorJoint_axis1( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_amotor_joint_param3( ptr->id, dJointGetAMotorAxis,
										ode_cOdeVector );
}


/*
 * ODE::AngularMotorJoint#axis1Rel()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_axis1_rel( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return INT2FIX( dJointGetAMotorAxisRel(ptr->id, 1) );
}


/*
 * ODE::AngularMotorJoint#setAxis1( mode, axis )
 * --
 * Set the 1st axis of the AngularMotorJoint to the given mode and axis. Each
 * axis can have one of three ``relative orientation'' modes, selected by <tt>mode</tt>:
 *
 * <tt>ODE::Joint::GlobalFrameOrientation</tt>:: The axis is anchored to the global frame.
 * <tt>ODE::Joint::Body1Orientation</tt>:: The axis is anchored to the first body.
 * <tt>ODE::Joint::Body2Orientation</tt>:: The axis is anchored to the second body. 
 *
 * The axis vector, specified in global coordinates regardless of the
 * <tt>mode</tt>, can be any object which returns an array with 3 numeric values
 * when <tt>to_ary</tt> is called on it, such as an ODE::Position object, a
 * Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_aMotorJoint_set_axis1( self, mode, axis )
	 VALUE self, mode, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	VALUE		axisArray;
	int			rel = NUM2INT(mode);

	/* Bounds-check the relmode */
	if ( rel < 0 || rel > 2 )
		rb_raise( rb_eArgError, "Invalid mode '%d': Must be 0, 1, or 2.", rel );

	/* Convert the axis argument to a 3rd order vector */
	if ( RARRAY(axis)->len == 1 )
		axisArray = ode_obj_to_ary3( *(RARRAY(axis)->ptr), "axis vector" );
	else
		axisArray = ode_obj_to_ary3( axis, "axis vector" );

	/* Set the axis */
	dJointSetAMotorAxis( ptr->id, 1, rel,
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr    )),
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr + 1)),
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr + 2)) );

	return Qtrue;
}

/*
 * ODE::AngularMotorJoint#axis2()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_axis2( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_amotor_joint_param3( ptr->id, 2, dJointGetAMotorAxis,
										ode_cOdeVector );
}


/*
 * ODE::AngularMotorJoint#axis2Rel()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_axis2_rel( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return INT2FIX( dJointGetAMotorAxisRel(ptr->id, 2) );
}


/*
 * ODE::AngularMotorJoint#setAxis2( mode, axis )
 * --
 * Set the 2nd axis of the AngularMotorJoint to the given mode and axis. Each
 * axis can have one of three ``relative orientation'' modes, selected by
 * <tt>mode</tt>:
 *
 * <tt>ODE::Joint::GlobalFrameOrientation</tt>:: The axis is anchored to the global frame.
 * <tt>ODE::Joint::Body1Orientation</tt>:: The axis is anchored to the first body.
 * <tt>ODE::Joint::Body2Orientation</tt>:: The axis is anchored to the second body. 
 *
 * The axis vector, specified in global coordinates regardless of the
 * <tt>mode</tt>, can be any object which returns an array with 3 numeric values
 * when <tt>to_ary</tt> is called on it, such as an ODE::Position object, a
 * Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_aMotorJoint_set_axis2( self, mode, axis )
	 VALUE	self, mode, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	VALUE		axisArray;
	int			rel = NUM2INT(mode);

	/* Bounds-check the relmode */
	if ( rel < 0 || rel > 2 )
		rb_raise( rb_eArgError, "Invalid mode '%d': Must be 0, 1, or 2.", rel );

	/* Convert the axis argument to a 3rd order vector */
	if ( RARRAY(axis)->len == 1 )
		axisArray = ode_obj_to_ary3( *(RARRAY(axis)->ptr), "axis vector" );
	else
		axisArray = ode_obj_to_ary3( axis, "axis vector" );

	/* Set the axis */
	dJointSetAMotorAxis( ptr->id, 2, rel,
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr    )),
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr + 1)),
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr + 2)) );

	return Qtrue;
}


/*
 * ODE::AngularMotorJoint#axis3()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_axis3( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return ode_get_amotor_joint_param3( ptr->id, 3, dJointGetAMotorAxis,
										ode_cOdeVector );
}


/*
 * ODE::AngularMotorJoint#axis3Rel()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_axis3_rel( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return INT2FIX( dJointGetAMotorAxisRel(ptr->id, 3) );
}


/*
 * ODE::AngularMotorJoint#setAxis3( mode, axis )
 * --
 * Set the 3rd axis of the AngularMotorJoint to the given mode and axis. Each
 * axis can have one of three ``relative orientation'' modes, selected by
 * <tt>mode</tt>:
 *
 * <tt>ODE::Joint::GlobalFrameOrientation</tt>:: The axis is anchored to the global frame.
 * <tt>ODE::Joint::Body1Orientation</tt>:: The axis is anchored to the first body.
 * <tt>ODE::Joint::Body2Orientation</tt>:: The axis is anchored to the second body. 
 *
 * The axis vector, specified in global coordinates regardless of the
 * <tt>mode</tt>, can be any object which returns an array with 3 numeric values
 * when <tt>to_ary</tt> is called on it, such as an ODE::Position object, a
 * Math3d::Vector3, or an Array with 3 numeric values.
 */
static VALUE
ode_aMotorJoint_set_axis3( self, mode, axis )
	 VALUE	self, mode, axis;
{
	ode_JOINT	*ptr = get_joint( self );
	VALUE		axisArray;
	int			rel = NUM2INT(mode);

	/* Bounds-check the relmode */
	if ( rel < 0 || rel > 2 )
		rb_raise( rb_eArgError, "Invalid mode '%d': Must be 0, 1, or 2.", rel );

	/* Convert the axis argument to a 3rd order vector */
	if ( RARRAY(axis)->len == 1 )
		axisArray = ode_obj_to_ary3( *(RARRAY(axis)->ptr), "axis vector" );
	else
		axisArray = ode_obj_to_ary3( axis, "axis vector" );

	/* Set the axis */
	dJointSetAMotorAxis( ptr->id, 3, rel,
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr    )),
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr + 1)),
						 (dReal)NUM2DBL(*(RARRAY(axisArray)->ptr + 2)) );

	return Qtrue;
}


/*
 * ODE::AngularMotorJoint#motor1Angle()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor1_angle( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( dJointGetAMotorAngle(ptr->id, 1) );
}


/*
 * ODE::AngularMotorJoint#motor1AngleRate()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor1_angle_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( dJointGetAMotorAngleRate(ptr->id, 1) );
}


/*
 * ODE::AngularMotorJoint#motor1Angle=( angle )
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor1_angle_eq( self, angle )
	 VALUE	self, angle;
{
	ode_JOINT	*ptr = get_joint( self );
	dJointSetAMotorAngle( ptr->id, 1, (dReal)NUM2DBL(angle) );
	return angle;
}


/*
 * ODE::AngularMotorJoint#motor2Angle()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor2_angle( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( dJointGetAMotorAngle(ptr->id, 2) );
}


/*
 * ODE::AngularMotorJoint#motor2AngleRate()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor2_angle_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( dJointGetAMotorAngleRate(ptr->id, 2) );
}


/*
 * ODE::AngularMotorJoint#motor2Angle=( angle )
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor2_angle_eq( self, angle )
	 VALUE	self, angle;
{
	ode_JOINT	*ptr = get_joint( self );
	dJointSetAMotorAngle( ptr->id, 2, (dReal)NUM2DBL(angle) );
	return angle;
}


/*
 * ODE::AngularMotorJoint#motor3Angle()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor3_angle( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( dJointGetAMotorAngle(ptr->id, 3) );
}


/*
 * ODE::AngularMotorJoint#motor3AngleRate()
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor3_angle_rate( self )
	 VALUE	self;
{
	ode_JOINT	*ptr = get_joint( self );
	return rb_float_new( dJointGetAMotorAngleRate(ptr->id, 3) );
}


/*
 * ODE::AngularMotorJoint#motor3Angle=( angle )
 * --
 * 
 */
static VALUE
ode_aMotorJoint_motor3_angle_eq( self, angle )
	 VALUE	self, angle;
{
	ode_JOINT	*ptr = get_joint( self );
	dJointSetAMotorAngle( ptr->id, 3, (dReal)NUM2DBL(angle) );
	return angle;
}





/*
 * Initializer for ODE::Joint and subclasses 
*/
void
ode_init_joints( void )
{
	static char
		rcsid[]		= "$Id: joints.c,v 1.5 2003/02/08 08:25:46 deveiant Exp $",
		revision[]	= "$Revision: 1.5 $";

	VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );

	/* Kluge to make Rdoc see the class in this file */
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeJoint			= rb_define_class_under( ode_mOde, "Joint", rb_cObject );
	ode_cOdeBallJoint		= rb_define_class_under( ode_mOde, "BallJoint", ode_cOdeJoint );
	ode_cOdeFixedJoint		= rb_define_class_under( ode_mOde, "FixedJoint", ode_cOdeJoint );
	ode_cOdeUniversalJoint	= rb_define_class_under( ode_mOde, "UniversalJoint", ode_cOdeJoint );
	ode_cOdeContactJoint	= rb_define_class_under( ode_mOde, "ContactJoint", ode_cOdeJoint );
	ode_cOdeParamJoint		= rb_define_class_under( ode_mOde, "ParameterizedJoint", ode_cOdeJoint );
	ode_cOdeHingeJoint		= rb_define_class_under( ode_mOde, "HingeJoint", ode_cOdeParamJoint );
	ode_cOdeHinge2Joint		= rb_define_class_under( ode_mOde, "Hinge2Joint", ode_cOdeParamJoint );
	ode_cOdeSliderJoint		= rb_define_class_under( ode_mOde, "SliderJoint", ode_cOdeParamJoint );
	ode_cOdeAMotorJoint		= rb_define_class_under( ode_mOde, "AngularMotorJoint", ode_cOdeParamJoint );
#endif

	/* Constants */
	rb_obj_freeze( vstr );
	rb_define_const( ode_cOdeJoint, "Version", vstr );
	vstr = rb_str_new2( rcsid );
	rb_obj_freeze( vstr );
	rb_define_const( ode_cOdeJoint, "Rcsid", vstr );

	/* Define the symbol constants for the keys of the feedback hash */
	body1Sym	= ID2SYM(rb_intern("body1"));
	body2Sym	= ID2SYM(rb_intern("body2"));
	torqueSym	= ID2SYM(rb_intern("torque"));
	forceSym	= ID2SYM(rb_intern("force"));

	/* Allocator - ODE::Joint class (abstract) */
#ifdef NEW_ALLOC
	rb_define_alloc_func( ode_cOdeJoint, ode_joint_s_alloc );
#else
	rb_define_singleton_method( ode_cOdeJoint, "allocate", ode_joint_s_alloc, 0 );
#endif

	/* Instance methods */
	rb_define_method( ode_cOdeJoint, "attach", ode_joint_attach, 2 );
	rb_define_method( ode_cOdeJoint, "attachedBodies", ode_joint_attached_bodies, 0 );
	rb_define_method( ode_cOdeJoint, "obsolete?", ode_joint_obsolete_p, 0 );
	rb_define_method( ode_cOdeJoint, "makeObsolete", ode_joint_make_obsolete, 0 );
	rb_define_method( ode_cOdeJoint, "feedback", ode_joint_get_feedback, 0 );
	rb_define_method( ode_cOdeJoint, "feedback=", ode_joint_feedback_enabled, 1 );
	rb_define_alias ( ode_cOdeJoint, "feedback_enabled=", "feedback=" );
	rb_define_method( ode_cOdeJoint, "feedback?", ode_joint_feedback_enabled_p, 0 );
	rb_define_alias ( ode_cOdeJoint, "feedback_enabled?", "feedback?" );

	/* ODE::BallJoint class */
	rb_define_method( ode_cOdeBallJoint, "initialize", ode_ballJoint_init, -1 );
	rb_define_method( ode_cOdeBallJoint, "anchor", ode_ballJoint_anchor, 0 );
	rb_define_method( ode_cOdeBallJoint, "anchor=", ode_ballJoint_anchor_eq, -2 );

	/* ODE::FixedJoint class */
	rb_define_method( ode_cOdeFixedJoint, "initialize", ode_fixedJoint_init, -1 );
	rb_define_method( ode_cOdeFixedJoint, "fix", ode_fixedJoint_fix, 0 );

	/* ODE::UniversalJoint class */
	rb_define_method( ode_cOdeUniversalJoint, "initialize", ode_universalJoint_init, -1 );
	rb_define_method( ode_cOdeUniversalJoint, "anchor", ode_universalJoint_anchor, 0 );
	rb_define_method( ode_cOdeUniversalJoint, "anchor=", ode_universalJoint_anchor_eq, -2 );
	rb_define_method( ode_cOdeUniversalJoint, "axis1", ode_universalJoint_axis1, 0 );
	rb_define_method( ode_cOdeUniversalJoint, "axis1=", ode_universalJoint_axis1_eq, -2 );
	rb_define_method( ode_cOdeUniversalJoint, "axis2", ode_universalJoint_axis2, 0 );
	rb_define_method( ode_cOdeUniversalJoint, "axis2=", ode_universalJoint_axis2_eq, -2 );

	/* ODE::ContactJoint class */
	rb_define_method( ode_cOdeContactJoint, "initialize", ode_contactJoint_init, -1 );
	rb_define_method( ode_cOdeContactJoint, "contact", ode_contactJoint_contact, 0 );

	/* ODE::ParameterizedJoint - Superclass of joints with motor and stop parameters (abstract) */
	rb_define_method( ode_cOdeParamJoint, "loStop", ode_paramJoint_LoStop, 0 );
	rb_define_method( ode_cOdeParamJoint, "loStop2", ode_paramJoint_LoStop, 0 );
	rb_define_method( ode_cOdeParamJoint, "loStop3", ode_paramJoint_LoStop, 0 );

	rb_define_method( ode_cOdeParamJoint, "loStop=", ode_paramJoint_LoStop_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "loStop2=", ode_paramJoint_LoStop_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "loStop3=", ode_paramJoint_LoStop_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "hiStop", ode_paramJoint_HiStop, 0 );
	rb_define_method( ode_cOdeParamJoint, "hiStop2", ode_paramJoint_HiStop, 0 );
	rb_define_method( ode_cOdeParamJoint, "hiStop3", ode_paramJoint_HiStop, 0 );

	rb_define_method( ode_cOdeParamJoint, "hiStop=", ode_paramJoint_HiStop_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "hiStop2=", ode_paramJoint_HiStop_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "hiStop3=", ode_paramJoint_HiStop_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "vel", ode_paramJoint_Vel, 0 );
	rb_define_alias ( ode_cOdeParamJoint, "velocity", "vel" );
	rb_define_method( ode_cOdeParamJoint, "vel2", ode_paramJoint_Vel, 0 );
	rb_define_alias ( ode_cOdeParamJoint, "velocity2", "vel2" );
	rb_define_method( ode_cOdeParamJoint, "vel3", ode_paramJoint_Vel, 0 );
	rb_define_alias ( ode_cOdeParamJoint, "velocity3", "vel3" );

	rb_define_method( ode_cOdeParamJoint, "vel=", ode_paramJoint_Vel_eq, 1 );
	rb_define_alias ( ode_cOdeParamJoint, "velocity=", "vel=" );
	rb_define_method( ode_cOdeParamJoint, "vel2=", ode_paramJoint_Vel_eq, 1 );
	rb_define_alias ( ode_cOdeParamJoint, "velocity2=", "vel2=" );
	rb_define_method( ode_cOdeParamJoint, "vel3=", ode_paramJoint_Vel_eq, 1 );
	rb_define_alias ( ode_cOdeParamJoint, "velocity3=", "vel3=" );

	rb_define_method( ode_cOdeParamJoint, "fMax", ode_paramJoint_FMax, 0 );
	rb_define_method( ode_cOdeParamJoint, "fMax2", ode_paramJoint_FMax, 0 );
	rb_define_method( ode_cOdeParamJoint, "fMax3", ode_paramJoint_FMax, 0 );

	rb_define_method( ode_cOdeParamJoint, "fMax=", ode_paramJoint_FMax_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "fMax2=", ode_paramJoint_FMax_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "fMax3=", ode_paramJoint_FMax_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "fudgeFactor", ode_paramJoint_FudgeFactor, 0 );
	rb_define_method( ode_cOdeParamJoint, "fudgeFactor2", ode_paramJoint_FudgeFactor, 0 );
	rb_define_method( ode_cOdeParamJoint, "fudgeFactor3", ode_paramJoint_FudgeFactor, 0 );

	rb_define_method( ode_cOdeParamJoint, "fudgeFactor=", ode_paramJoint_FudgeFactor_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "fudgeFactor2=", ode_paramJoint_FudgeFactor_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "fudgeFactor3=", ode_paramJoint_FudgeFactor_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "bounce", ode_paramJoint_Bounce, 0 );
	rb_define_method( ode_cOdeParamJoint, "bounce2", ode_paramJoint_Bounce, 0 );
	rb_define_method( ode_cOdeParamJoint, "bounce3", ode_paramJoint_Bounce, 0 );

	rb_define_method( ode_cOdeParamJoint, "bounce=", ode_paramJoint_Bounce_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "bounce2=", ode_paramJoint_Bounce_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "bounce3=", ode_paramJoint_Bounce_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "CFM", ode_paramJoint_CFM, 0 );
	rb_define_method( ode_cOdeParamJoint, "CFM2", ode_paramJoint_CFM, 0 );
	rb_define_method( ode_cOdeParamJoint, "CFM3", ode_paramJoint_CFM, 0 );

	rb_define_method( ode_cOdeParamJoint, "CFM=", ode_paramJoint_CFM_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "CFM2=", ode_paramJoint_CFM_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "CFM3=", ode_paramJoint_CFM_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "stopERP", ode_paramJoint_StopERP, 0 );
	rb_define_method( ode_cOdeParamJoint, "stopERP2", ode_paramJoint_StopERP, 0 );
	rb_define_method( ode_cOdeParamJoint, "stopERP3", ode_paramJoint_StopERP, 0 );

	rb_define_method( ode_cOdeParamJoint, "stopERP=", ode_paramJoint_StopERP_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "stopERP2=", ode_paramJoint_StopERP_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "stopERP3=", ode_paramJoint_StopERP_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "stopCFM", ode_paramJoint_StopCFM, 0 );
	rb_define_method( ode_cOdeParamJoint, "stopCFM2", ode_paramJoint_StopCFM, 0 );
	rb_define_method( ode_cOdeParamJoint, "stopCFM3", ode_paramJoint_StopCFM, 0 );

	rb_define_method( ode_cOdeParamJoint, "stopCFM=", ode_paramJoint_StopCFM_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "stopCFM2=", ode_paramJoint_StopCFM_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "stopCFM3=", ode_paramJoint_StopCFM_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "suspensionERP", ode_paramJoint_SuspensionERP, 0 );
	rb_define_method( ode_cOdeParamJoint, "suspensionERP2", ode_paramJoint_SuspensionERP, 0 );
	rb_define_method( ode_cOdeParamJoint, "suspensionERP3", ode_paramJoint_SuspensionERP, 0 );

	rb_define_method( ode_cOdeParamJoint, "suspensionERP=", ode_paramJoint_SuspensionERP_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "suspensionERP2=", ode_paramJoint_SuspensionERP_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "suspensionERP3=", ode_paramJoint_SuspensionERP_eq, 1 );

	rb_define_method( ode_cOdeParamJoint, "suspensionCFM", ode_paramJoint_SuspensionCFM, 0 );
	rb_define_method( ode_cOdeParamJoint, "suspensionCFM2", ode_paramJoint_SuspensionCFM, 0 );
	rb_define_method( ode_cOdeParamJoint, "suspensionCFM3", ode_paramJoint_SuspensionCFM, 0 );

	rb_define_method( ode_cOdeParamJoint, "suspensionCFM=", ode_paramJoint_SuspensionCFM_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "suspensionCFM2=", ode_paramJoint_SuspensionCFM_eq, 1 );
	rb_define_method( ode_cOdeParamJoint, "suspensionCFM3=", ode_paramJoint_SuspensionCFM_eq, 1 );


	/* ODE::HingeJoint class */
	rb_define_const( ode_cOdeHingeJoint, "Axes", INT2FIX(1) );

	rb_define_method( ode_cOdeHingeJoint, "initialize", ode_hingeJoint_init, -1 );
	rb_define_method( ode_cOdeHingeJoint, "anchor", ode_hingeJoint_anchor, 0 );
	rb_define_method( ode_cOdeHingeJoint, "anchor=", ode_hingeJoint_anchor_eq, -2 );
	rb_define_method( ode_cOdeHingeJoint, "axis", ode_hingeJoint_axis, 0 );
	rb_define_method( ode_cOdeHingeJoint, "axis=", ode_hingeJoint_axis_eq, -2 );
	rb_define_method( ode_cOdeHingeJoint, "angle", ode_hingeJoint_angle, 0 );
	rb_define_method( ode_cOdeHingeJoint, "angleRate", ode_hingeJoint_angle_rate, 0 );
	rb_define_alias ( ode_cOdeHingeJoint, "angle_rate", "angleRate" );

	/* ODE::Hinge2Joint class */
	rb_define_const( ode_cOdeHinge2Joint, "Axes", INT2FIX(2) );

	rb_define_method( ode_cOdeHinge2Joint, "initialize", ode_hinge2Joint_init, -1 );
	rb_define_method( ode_cOdeHinge2Joint, "anchor", ode_hinge2Joint_anchor, 0 );
	rb_define_method( ode_cOdeHinge2Joint, "anchor=", ode_hinge2Joint_anchor_eq, -2 );
	rb_define_method( ode_cOdeHinge2Joint, "axis1", ode_hinge2Joint_axis1, 0 );
	rb_define_method( ode_cOdeHinge2Joint, "axis1=", ode_hinge2Joint_axis1_eq, -2 );
	rb_define_method( ode_cOdeHinge2Joint, "axis2", ode_hinge2Joint_axis2, 0 );
	rb_define_method( ode_cOdeHinge2Joint, "axis2=", ode_hinge2Joint_axis2_eq, -2 );
	rb_define_method( ode_cOdeHinge2Joint, "angle1", ode_hinge2Joint_angle1, 0 );
	rb_define_method( ode_cOdeHinge2Joint, "angle1Rate", ode_hinge2Joint_angle1_rate, 0 );
	rb_define_alias ( ode_cOdeHinge2Joint, "angle1_rate", "angle1Rate" );
	rb_define_method( ode_cOdeHinge2Joint, "angle2Rate", ode_hinge2Joint_angle2_rate, 0 );
	rb_define_alias ( ode_cOdeHinge2Joint, "angle2_rate", "angle2Rate" );

	/* ODE::SliderJoint class */
	rb_define_const( ode_cOdeSliderJoint, "Axes", INT2FIX(1) );

	rb_define_method( ode_cOdeSliderJoint, "initialize", ode_sliderJoint_init, -1 );
	rb_define_method( ode_cOdeSliderJoint, "axis", ode_sliderJoint_axis, 0 );
	rb_define_method( ode_cOdeSliderJoint, "axis=", ode_sliderJoint_axis_eq, -2 );
	rb_define_method( ode_cOdeSliderJoint, "position", ode_sliderJoint_position, 0 );
	rb_define_method( ode_cOdeSliderJoint, "positionRate", ode_sliderJoint_position_rate, 0 );
	rb_define_alias ( ode_cOdeSliderJoint, "position_rate", "positionRate" );

	/* ODE::AngularMotorJoint class */
	rb_define_const( ode_cOdeAMotorJoint, "Axes", INT2FIX(3) );

	rb_define_const( ode_cOdeJoint, "GlobalFrameOrientation", INT2FIX(0) );
	rb_define_const( ode_cOdeJoint, "Body1Orientation", INT2FIX(1) );
	rb_define_const( ode_cOdeJoint, "Body2Orientation", INT2FIX(2) );

	rb_define_method( ode_cOdeAMotorJoint, "initialize", ode_aMotorJoint_init, -1 );
	rb_define_method( ode_cOdeAMotorJoint, "eulerMode=", ode_aMotorJoint_euler_mode_eq, 1 );
	rb_define_alias ( ode_cOdeAMotorJoint, "euler_mode=", "eulerMode=" );
	rb_define_method( ode_cOdeAMotorJoint, "eulerMode?", ode_aMotorJoint_euler_mode_p, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "euler_mode?", "eulerMode?" );

	rb_define_method( ode_cOdeAMotorJoint, "numAxes", ode_aMotorJoint_num_axes, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "num_axes", "numAxes" );
	rb_define_method( ode_cOdeAMotorJoint, "numAxes=", ode_aMotorJoint_num_axes_eq, 1 );
	rb_define_alias ( ode_cOdeAMotorJoint, "num_axes=", "numAxes=" );

	rb_define_method( ode_cOdeAMotorJoint, "axis1", ode_aMotorJoint_axis1, 0 );
	rb_define_method( ode_cOdeAMotorJoint, "axis1Rel", ode_aMotorJoint_axis1_rel, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "axis1_rel", "axis1Rel" );
	rb_define_method( ode_cOdeAMotorJoint, "setAxis1", ode_aMotorJoint_set_axis1, 2 );
	rb_define_method( ode_cOdeAMotorJoint, "axis2", ode_aMotorJoint_axis2, 0 );
	rb_define_method( ode_cOdeAMotorJoint, "axis2Rel", ode_aMotorJoint_axis2_rel, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "axis2_rel", "axis2Rel" );
	rb_define_method( ode_cOdeAMotorJoint, "setAxis2", ode_aMotorJoint_set_axis2, 2 );
	rb_define_method( ode_cOdeAMotorJoint, "axis3", ode_aMotorJoint_axis3, 0 );
	rb_define_method( ode_cOdeAMotorJoint, "axis3Rel", ode_aMotorJoint_axis3_rel, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "axis3_rel", "axis3Rel" );
	rb_define_method( ode_cOdeAMotorJoint, "setAxis3", ode_aMotorJoint_set_axis3, 2 );

	rb_define_method( ode_cOdeAMotorJoint, "motor1Angle", ode_aMotorJoint_motor1_angle, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor1_angle", "motor1Angle" );
	rb_define_method( ode_cOdeAMotorJoint, "motor1AngleRate", ode_aMotorJoint_motor1_angle_rate, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor1_angle_rate", "motor1AngleRate" );
	rb_define_method( ode_cOdeAMotorJoint, "motor1Angle=", ode_aMotorJoint_motor1_angle_eq, 1 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor1_angle=", "motor1Angle=" );
	rb_define_method( ode_cOdeAMotorJoint, "motor2Angle", ode_aMotorJoint_motor2_angle, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor2_angle", "motor2Angle" );
	rb_define_method( ode_cOdeAMotorJoint, "motor2AngleRate", ode_aMotorJoint_motor2_angle_rate, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor2_angle_rate", "motor2AngleRate" );
	rb_define_method( ode_cOdeAMotorJoint, "motor2Angle=", ode_aMotorJoint_motor2_angle_eq, 1 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor2_angle=", "motor2Angle=" );
	rb_define_method( ode_cOdeAMotorJoint, "motor3Angle", ode_aMotorJoint_motor3_angle, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor3_angle", "motor3Angle" );
	rb_define_method( ode_cOdeAMotorJoint, "motor3AngleRate", ode_aMotorJoint_motor3_angle_rate, 0 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor3_angle_rate", "motor3AngleRate" );
	rb_define_method( ode_cOdeAMotorJoint, "motor3Angle=", ode_aMotorJoint_motor3_angle_eq, 1 );
	rb_define_alias ( ode_cOdeAMotorJoint, "motor3_angle=", "motor3Angle=" );
}

