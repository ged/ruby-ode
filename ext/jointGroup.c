/*
 *		jointGroup.c - ODE Ruby Binding - JointGroup Class
 *		$Id: jointGroup.c,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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

VALUE ode_cOdeJointGroup;


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/* new() */
VALUE
ode_jointGroup_new( self, maxSize )
	 VALUE self, maxSize;
{
  dJointGroupID		id;
  VALUE				jointGroup;

  id = dJointGroupCreate( NUM2INT(maxSize) );
  jointGroup = Data_Wrap_Struct( self, 0, ode_jointGroup_gc_free, id );
  rb_obj_call_init( jointGroup, 0, 0 );
  return jointGroup;
}


/* GC free function */
void
ode_jointGroup_gc_free( id )
	 dJointGroupID id;
{
  dJointGroupDestroy( id );
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/* initialize() */
VALUE
ode_jointGroup_init( self )
	 VALUE self;
{
  return self;
}


/* empty() */
VALUE
ode_jointGroup_empty( self )
	 VALUE self;
{
  dJointGroupID	id;

  // Get the joint group struct and clear it
  // :FIXME: This doesn't remove the group from the joint's idea of what group
  // object they need to mark during GC.
  GetJointGroup( self, id );
  dJointGroupEmpty( id );

  return Qtrue;
}



/* jointGroup initializer */
void
ode_init_jointGroup(void)
{
  ode_cOdeJointGroup = rb_define_class_under( ode_mOde, "JointGroup", rb_cObject );

  rb_define_singleton_method( ode_cOdeJointGroup, "new", ode_jointGroup_new, 1 );
  rb_define_method( ode_cOdeJointGroup, "initialize", ode_jointGroup_init, 0 );
  rb_define_method( ode_cOdeJointGroup, "empty", ode_jointGroup_empty, 0 );
}

