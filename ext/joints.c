/*
 *		joints.c - ODE Ruby Binding - Joint Classes
 *		$Id: joints.c,v 1.2 2002/03/20 14:18:04 deveiant Exp $
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

VALUE ode_cOdeJoint;

VALUE ode_cOdeBallJoint;
VALUE ode_cOdeHingeJoint;
VALUE ode_cOdeHinge2Joint;
VALUE ode_cOdeSliderJoint;
VALUE ode_cOdeFixedJoint;

VALUE ode_cOdeContactJoint;


// Forward declarations
static void ode_joint_gc_mark( ode_JOINT * );
static void ode_joint_gc_free( ode_JOINT * );


/* 
 * ODE::Joint::new
 * --
 # Raises an error, as ODE::Joint is an abstract class.
 */
VALUE
ode_joint_abstract_class( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
  rb_raise( rb_eRuntimeError, "Cannot instantiate abstract class %s.", rb_class2name(self) );
}


/* ODE::World.createJoint( [jointGroup] ) */

/* VALUE */
/* ode_world_joint_create( self ) */
/* 	 VALUE self; */
/* { */
/*   return  */
/* } */


/*
 * Constructor -- every joint class except ODE::ContactJoint uses this for its
 * constructor. It passes in its arguments and pointer to the the ODE native
 * constructor function.
 */
VALUE
ode_joint_new( argc, argv, self, constructor )
	 int		argc;
	 VALUE		*argv;
	 VALUE		self;
	 dJointID	(*constructor)( dWorldID, dJointGroupID );
{
  dJointID			id;
  dWorldID			worldId;
  dJointGroupID		jointGroupId;
  ode_JOINTGROUP	*jointGroupStruct;
  ode_JOINT			*jointStruct;
  VALUE				world, jointGroup, joint;

  debugMsg(( "Creating a new Joint: checking args." ));

  // Check arguments
  if ( argc < 1 || argc > 2 )
	rb_raise( rb_eArgError, "Wrong # of arguments (%d for 1 or 2)", argc );
  if ( ! rb_obj_is_kind_of(*argv, ode_cOdeWorld) )
	rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( *argv )) );
  if ( argc == 2 && ! rb_obj_is_kind_of(*(argv+1), ode_cOdeJointGroup) )
	rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( *(argv+1) )) );

  debugMsg(( "Creating a new Joint: fetching World." ));

  // Fetch the world object and its id
  world = *argv;
  GetWorld( world, worldId );

  debugMsg(( "Creating a new Joint: fetching the JointGroup." ));

  // Fetch the jointGroup and its id, if one was given
  if ( argc == 2 ) {
	jointGroup = *(argv+1);
	GetJointGroup( jointGroup, jointGroupStruct );
	jointGroupId = jointGroupStruct->id;
	debugMsg(( "Got a JointGroup for new Joint: <%p>", jointGroupStruct ));
  }
  else {
	jointGroup = 0;
	jointGroupId = 0;
	debugMsg(( "No JointGroup for new Joint" ));
  }

  // Create the actual joint by calling the real constructor
  id = (*constructor)( worldId, jointGroupId );

  // Allocate a joint struct and set its members
  debugMsg(( "Creating a new Joint: ALLOCing and setting struct members." ));
  jointStruct = ALLOC( ode_JOINT );
  jointStruct->world = world;
  jointStruct->id = id;
  jointStruct->body1 = 0;
  jointStruct->body2 = 0;
  jointStruct->surface = 0;
  jointStruct->jointGroup = jointGroup;

  debugMsg(( "Created Joint <%p> (id = <%p>)", jointStruct, id ));

  // Wrap it in a Ruby object, call initialize() on it, and return it
  debugMsg(( "Creating a new Joint: Wrapping the struct." ));
  joint = Data_Wrap_Struct( self, ode_joint_gc_mark, ode_joint_gc_free, jointStruct );

  // Register with the Ruby part of the JointGroup, if the joint is a member
  debugMsg(( "Creating a new Joint: Registering joint with jointGroup." ));
  if ( jointGroup )
	ode_jointGroup_register_joint( jointGroup, joint );

  debugMsg(( "Creating a new Joint: Calling initialize." ));
  rb_obj_call_init( joint, 0, 0 );
  return joint;
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
  if ( jointStruct->jointGroup ) rb_gc_mark( jointStruct->jointGroup );
  if ( jointStruct->world ) rb_gc_mark( jointStruct->world );
  if ( jointStruct->body1 ) rb_gc_mark( jointStruct->body1 );
  if ( jointStruct->body2 ) rb_gc_mark( jointStruct->body2 );
  if ( jointStruct->surface ) rb_gc_mark( jointStruct->surface );
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
  jointStruct->jointGroup = 0;
  jointStruct->id = NULL;
  jointStruct->world = 0;
  jointStruct->body1 = 0;
  jointStruct->body2 = 0;
  jointStruct->surface = 0;

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
  dJointID			id;
  dWorldID			worldId;
  dJointGroupID		jointGroupId;
  dContact			contact;
  ode_JOINTGROUP	*jointGroupStruct;
  ode_JOINT			*jointStruct;
  VALUE				world, jointGroup, joint;

  debugMsg(( "Creating a new ContactJoint: checking args." ));

  // Check arguments
  if ( argc < 1 || argc > 2 )
	rb_raise( rb_eArgError, "Wrong # of arguments (%d for 1 or 2)", argc );
  if ( ! rb_obj_is_kind_of(*argv, ode_cOdeWorld) )
	rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( *argv )) );
  if ( argc == 2 && ! rb_obj_is_kind_of(*(argv+1), ode_cOdeJointGroup) )
	rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( *(argv+1) )) );

  debugMsg(( "Creating a new ContactJoint: fetching World." ));

  // Fetch the world object and its id
  world = *argv;
  GetWorld( world, worldId );

  debugMsg(( "Creating a new ContactJoint: fetching the JointGroup." ));

  // Fetch the jointGroup and its id, if one was given
  if ( argc == 2 ) {
	jointGroup = *(argv+1);
	GetJointGroup( jointGroup, jointGroupStruct );
	jointGroupId = jointGroupStruct->id;
	debugMsg(( "Got a JointGroup for new ContactJoint: <%p>", jointGroupStruct ));
  }
  else {
	jointGroup = 0;
	jointGroupId = 0;
	debugMsg(( "No JointGroup for new ContactJoint" ));
  }

  // For now, just create a default surface. Later this will need to grab
  // parameters from somewhere...
  contact.surface.mode = 0;
  contact.surface.mu = dInfinity;

  // Allocate a joint struct and set its members
  id = dJointCreateContact( worldId, jointGroupId, &contact );

  // Allocate a joint struct and set its members
  debugMsg(( "Creating a new ContactJoint: ALLOCing and setting struct members." ));
  jointStruct = ALLOC( ode_JOINT );
  jointStruct->world = world;
  jointStruct->id = id;
  jointStruct->body1 = 0;
  jointStruct->body2 = 0;
  jointStruct->jointGroup = jointGroup;
  jointStruct->surface = 0; // Eventually, set this to the ODE::Surface object

  debugMsg(( "Created ContactJoint <%p> (id = <%p>)", jointStruct, id ));

  // Wrap it in a Ruby object, call initialize() on it, and return it
  debugMsg(( "Creating a new ContactJoint: Wrapping the struct." ));
  joint = Data_Wrap_Struct( self, ode_joint_gc_mark, ode_joint_gc_free, jointStruct );

  // Register with the Ruby part of the JointGroup, if the joint is a member
  debugMsg(( "Creating a new ContactJoint: Registering joint with jointGroup." ));
  if ( jointGroup )
	ode_jointGroup_register_joint( jointGroup, joint );

  debugMsg(( "Creating a new ContactJoint: Calling initialize." ));
  rb_obj_call_init( joint, 0, 0 );
  return joint;
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



/* joint initializer */
void
ode_init_joints( void )
{
  // ODE::Joint class (abstract)
  ode_cOdeJoint = rb_define_class_under( ode_mOde, "Joint", rb_cObject );
  rb_define_singleton_method( ode_cOdeJoint, "new", ode_joint_abstract_class, -1 );
  rb_define_method( ode_cOdeJoint, "attach", ode_joint_attach, 2 );
  rb_define_method( ode_cOdeJoint, "attachedBodies", ode_joint_attached_bodies, 0 );
  rb_define_method( ode_cOdeJoint, "obsolete?", ode_joint_obsolete_p, 0 );
  rb_define_method( ode_cOdeJoint, "makeObsolete", ode_joint_make_obsolete, 0 );

  // ODE::BallJoint class
  ode_cOdeBallJoint = rb_define_class_under( ode_mOde, "BallJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeBallJoint, "new", ode_ballJoint_new, -1 );

  // ODE::HingeJoint class
  ode_cOdeHingeJoint = rb_define_class_under( ode_mOde, "HingeJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeHingeJoint, "new", ode_hingeJoint_new, -1 );

  // ODE::Hinge2Joint class
  ode_cOdeHinge2Joint = rb_define_class_under( ode_mOde, "Hinge2Joint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeHinge2Joint, "new", ode_hinge2Joint_new, -1 );

  // ODE::SliderJoint class
  ode_cOdeSliderJoint = rb_define_class_under( ode_mOde, "SliderJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeSliderJoint, "new", ode_sliderJoint_new, -1 );

  // ODE::FixedJoint class
  ode_cOdeFixedJoint = rb_define_class_under( ode_mOde, "FixedJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeFixedJoint, "new", ode_fixedJoint_new, -1 );


  // ODE::Surface
  

  // ODE::ContactJoint class
  //ode_cOdeContactJoint = rb_define_class_under( ode_mOde, "ContactJoint", ode_cOdeJoint );
  //rb_define_singleton_method( ode_cOdeContactJoint, "new", ode_contactJoint_new, -1 );

}

