/*
 *		joints.c - ODE Ruby Binding - Joint Classes
 *		$Id: joints.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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

VALUE ode_cOdeWorld;
VALUE ode_cOdeJointGroup;
VALUE ode_cOdeJoint;

VALUE ode_cOdeBallJoint;
VALUE ode_cOdeHingeJoint;
VALUE ode_cOdeHinge2Joint;
VALUE ode_cOdeContactJoint;
VALUE ode_cOdeSliderJoint;
VALUE ode_cOdeFixedJoint;


VALUE
ode_joint_abstract_class(  argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
  rb_raise( rb_eRuntimeError, "Cannot instantiate abstract class %s.", rb_class2name(self) );
}


/* ODE::World.createJoint( [jointGroup] ) */
VALUE
ode_world_joint_create( self )
	 VALUE self;
{
  return 
}

 
/* "Native" constructor. */
VALUE
ode_joint_new( argc, argv, self, id )
	 int		argc;
	 VALUE		*argv;
	 VALUE		self;
	 dJointID	id;
{
  ode_JOINT	*jointStruct;
  VALUE		world, jointGroup, joint;

  // Check arguments
  if ( argc < 1 || argc > 2 )
	rb_raise( rb_eArgError, "Wrong # of arguments (%d for 1 or 2)", argc );
  if ( ! rb_obj_is_kind_of(*argv, ode_cOdeWorld) )
	rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( *argv )) );
  if ( argc == 2 && ! rb_obj_is_kind_of(*(argv+1), ode_cOdeJointGroup) )
	rb_raise( rb_eTypeError, "no implicit conversion from %s", rb_class2name(CLASS_OF( *(argv+1) )) );

  // Fetch the arguments
  world = *argv;
  if ( argc == 2 )
	jointGroup = *(argv+1);
  else
	jointGroup = (VALUE)NULL;

  // Allocate a joint struct and set its members
  jointStruct = ALLOC( ode_JOINT );
  jointStruct->world = world;
  jointStruct->jointGroup = jointGroup;

  // Wrap it in a Ruby object, call initialize() on it, and return it
  joint = Data_Wrap_Struct( self, ode_joint_gc_mark, ode_joint_gc_free, jointStruct );
  rb_obj_call_init( joint, 0, 0 );
  return joint;
}


/* GC mark function */
void
ode_joint_gc_mark( jointStruct )
	 ode_JOINT	*jointStruct;
{
  if (jointStruct->world) rb_gc_mark( jointStruct->world );
  if (jointStruct->jointGroup) rb_gc_mark( jointStruct->jointGroup );
}


/* GC free function */
void
ode_joint_gc_free( jointStruct )
	 ode_JOINT	*jointStruct;
{
  dJointDestroy( jointStruct->id );
  free( jointStruct );
  jointStruct = NULL;
}


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */





/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/* ODE::Joint#initialize() */
VALUE
ode_joint_init( self )
	 VALUE self;
{
  return self;
}


/* ODE::Joint#attach( bodyObj, bodyObj ) */
VALUE
ode_joint_attach( self, body1, body2 )
	 VALUE self, body1, body2;
{
  ode_JOINT	*jointStruct;
  
}


/* joint initializer */
void
ode_init_joints(void)
{
  // Abstract Base Joint Class
  ode_cOdeJoint = rb_define_class_under( ode_mOde, "Joint", rb_cObject );

  rb_define_singleton_method( ode_cOdeJoint, "new", ode_joint_abstract_class, -1 );
  rb_define_method( ode_cOdeJoint, "initialize", ode_joint_init, -1 );
  rb_define_method( ode_cOdeJoint, "attach", ode_joint_attach, 2 );

  // ODE::BallJoint class
  ode_cOdeBallJoint = rb_define_class_under( ode_mOde, "BallJoint", ode_cOdeJoint );
  rb_
  rb_define_singleton_method( ode_cOdeBallJoint, "new", ode_joint_new, -1 );

  // ODE::HingeJoint class
  ode_cOdeHingeJoint = rb_define_class_under( ode_mOde, "HingeJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeHingeJoint, "new", ode_joint_new, -1 );

  // ODE::Hinge2Joint class
  ode_cOdeHinge2Joint = rb_define_class_under( ode_mOde, "Hinge2Joint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeHinge2Joint, "new", ode_joint_new, -1 );

  // ODE::SliderJoint class
  ode_cOdeSliderJoint = rb_define_class_under( ode_mOde, "SliderJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeSliderJoint, "new", ode_joint_new, -1 );

  // ODE::FixedJoint class
  ode_cOdeFixedJoint = rb_define_class_under( ode_mOde, "FixedJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeFixedJoint, "new", ode_joint_new, -1 );

  // ODE::ContactJoint class
  ode_cOdeContactJoint = rb_define_class_under( ode_mOde, "ContactJoint", ode_cOdeJoint );
  rb_define_singleton_method( ode_cOdeContactJoint, "new", ode_contactJoint_new, -1 );


}

