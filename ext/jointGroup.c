/*
 *		jointGroup.c - ODE Ruby Binding - JointGroup Class
 *		$Id: jointGroup.c,v 1.2 2002/03/20 14:15:48 deveiant Exp $
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

// Forward declarations
static void ode_jointGroup_gc_mark( ode_JOINTGROUP * );
static void ode_jointGroup_gc_free( ode_JOINTGROUP * );


/* --------------------------------------------------
 * Joint group linked list iterator functions
 * -------------------------------------------------- */

/*
 * Obsolete flag setter function for joint member linked list node. Sets the
 * obsolete flag in the object in the specified node (in preparation for
 * clearing it from the JointGroup, for example).
 */
static void
ode_jointList_obsolete( node )
	 ode_JOINTLIST	*node;
{
  debugMsg(( "Obsoleting node <%p>.", node ));
  rb_funcall( node->joint, rb_intern("makeObsolete"), 0 );
}


/*
 * Free function for joint member linked list node.
 */
static void
ode_jointList_free( node )
	 ode_JOINTLIST	*node;
{
  debugMsg(( "Prepping node <%p> for free (joint = <%p>, next = <%p>.",
			 node, node->joint, node->next ));
  node->joint	= 0;
  debugMsg(( "Setting next to NULL." ));
  node->next	= NULL;
  debugMsg(( "Freeing node <%p>", node ));
  free( node );
}


/*
 * Mark function for joint member linked list node.
 */
static void
ode_jointList_mark( node )
	 ode_JOINTLIST	*node;
{
  debugMsg(( "Marking node <%p>.", node ));
  rb_gc_mark( node->joint );
}


/*
 * Iterator function for joint member linked list.
 */
static void
ode_jointList_iterate( jointList, func )
	 ode_JOINTLIST	*jointList;
	 void			(*func)( ode_JOINTLIST * );
{
  ode_JOINTLIST	*node, *nextNode;

  if ( jointList == NULL )
	rb_fatal( "Got NULL jointList" );

  debugMsg(( "Iterating over jointList <%p>.", jointList ));
  nextNode = jointList;
  while ( nextNode != NULL ) {
	node = nextNode;
	nextNode = node->next;

	debugMsg(( "Calling iterator for node <%p>.", node ));
	(*func)( node );
  }
}


/*
 * Add joint member linked list node.
 */
static void
ode_jointList_addNode( jointList, newNode )
	 ode_JOINTLIST	*jointList, *newNode;
{
  ode_JOINTLIST *node;

  debugMsg(( "Adding a new node <%p> to jointList <%p>.", newNode, jointList ));

  node = jointList;
  while ( node->next != NULL ) {
	node = node->next;
  }

  newNode->next = NULL;
  node->next = newNode;
}


/*
 * Remove joint member linked list node.
 */
/* static */ void
ode_jointList_deleteNode( jointList, targetNode )
	 ode_JOINTLIST	*jointList, *targetNode;
{
  ode_JOINTLIST *node, *lastNode;

  debugMsg(( "Deleting node <%p> from jointList <%p>.", targetNode, jointList ));

  node = lastNode = jointList;
  while ( node != NULL && node != targetNode ) {
	lastNode = node;
	node = lastNode->next;
  }

  if ( node != NULL )
	lastNode->next = node->next;
}


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */


/**
 * new( jointType=nil )
 * --
 * Create and return a new ODE::JointGroup. If the optional <tt>jointType</tt>
 * argument (an ODE::Joint derivative class object) is given, joints created via
 * #newJoint will be of the specified type. If it is not set, #newJoint raises
 * an exception.
 */
VALUE
ode_jointGroup_new( argc, argv, self )
	 int	argc;
	 VALUE	*argv;
	 VALUE	self;
{
  dJointGroupID		id;
  ode_JOINTGROUP	*jointGroupStruct;
  VALUE				jointGroup;

  // Create a new jointGroup
  id = dJointGroupCreate( 0 );

  jointGroupStruct = ALLOC( ode_JOINTGROUP );
  debugMsg(( "Created JointGroup <%p>", jointGroupStruct ));
  jointGroupStruct->id = id;
  jointGroupStruct->jointList = NULL;

  // Wrap the struct in a Ruby object
  jointGroup = Data_Wrap_Struct( self,
								 ode_jointGroup_gc_mark,
								 ode_jointGroup_gc_free,
								 jointGroupStruct );

  // Call #initialize()
  rb_obj_call_init( jointGroup, argc, argv );
  return jointGroup;
}


/*
 * GC mark function
 */
static void
ode_jointGroup_gc_mark( jointGroupStruct )
	 ode_JOINTGROUP	*jointGroupStruct;
{
  debugMsg(( "Marking members of JointGroup <%p>", jointGroupStruct ));

  if ( jointGroupStruct->jointList )
	ode_jointList_iterate( jointGroupStruct->jointList,
						   ode_jointList_mark );
}


/*
 * GC free function
 */
static void
ode_jointGroup_gc_free( jointGroupStruct )
	 ode_JOINTGROUP	*jointGroupStruct;
{
  debugMsg(( "Destroying JointGroup <%p>", jointGroupStruct ));

  if ( jointGroupStruct->jointList )
	ode_jointList_iterate( jointGroupStruct->jointList,
						   ode_jointList_free );

  dJointGroupDestroy( jointGroupStruct->id );
  jointGroupStruct->id = NULL;
  jointGroupStruct->jointList = NULL;

  free( jointGroupStruct );
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */


/*
 * empty()
 * --
 * Remove all the member joints from this group, marking them as obsolete.
 */
VALUE
ode_jointGroup_empty( self )
	 VALUE self;
{
  ode_JOINTGROUP	*jointGroupStruct;

  // Get the joint group struct and clear the joints in it, after marking them
  // as obsolete.
  GetJointGroup( self, jointGroupStruct );
  ode_jointList_iterate( jointGroupStruct->jointList,
						 ode_jointList_obsolete );
  ode_jointList_iterate( jointGroupStruct->jointList,
						 ode_jointList_free );
  jointGroupStruct->jointList = NULL;

  dJointGroupEmpty( jointGroupStruct->id );
  return Qtrue;
}


/*
 * empty?()
 * --
 * Returns <tt>true</tt> if the joint group is empty.
 */
VALUE
ode_jointGroup_empty_p( self )
	 VALUE self;
{
  ode_JOINTGROUP	*jointGroupStruct;

  GetJointGroup( self, jointGroupStruct );
  return (jointGroupStruct->jointList == NULL ? Qtrue : Qfalse);
}




/* -------------------------------------------------------
 * Global functions
 * ------------------------------------------------------- */

/*
 * Register a joint which has been created in a JointGroup with the Ruby part of
 * the JointGroup object.
 */
void
ode_jointGroup_register_joint( jointGroup, joint )
	 VALUE jointGroup, joint;
{
  ode_JOINTGROUP	*jointGroupStruct;
  ode_JOINTLIST		*node;

  debugMsg(( "Registering Joint <%p> with JointGroup <%p>.", joint, jointGroup ));
  GetJointGroup( jointGroup, jointGroupStruct );

  debugMsg(( "ALLOCing a new group list node." ));
  node = ALLOC( ode_JOINTLIST );
  node->joint = joint;
  node->next = NULL;

  debugMsg(( "New node is <%p>.", node ));

  if ( jointGroupStruct->jointList == NULL )
	jointGroupStruct->jointList = node;
  else
	ode_jointList_addNode( jointGroupStruct->jointList, node );
}


/* JointGroup initializer */
void
ode_init_jointGroup(void)
{
  ode_cOdeJointGroup = rb_define_class_under( ode_mOde, "JointGroup", rb_cObject );

  rb_define_singleton_method( ode_cOdeJointGroup, "new", ode_jointGroup_new, -1 );
  rb_define_method( ode_cOdeJointGroup, "empty", ode_jointGroup_empty, 0 );
  rb_define_method( ode_cOdeJointGroup, "empty?", ode_jointGroup_empty_p, 0 );

  rb_require( "ode/JointGroup" );
}

