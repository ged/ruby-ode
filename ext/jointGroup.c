/*
 *		jointGroup.c - ODE Ruby Binding - JointGroup Class
 *		$Id$
 *		Time-stamp: <04-Feb-2003 15:13:02 deveiant>
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

#include "ode.h"



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
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static ode_JOINTGROUP *
ode_jointGroup_alloc()
{
	ode_JOINTGROUP *ptr = ALLOC( ode_JOINTGROUP );

	ptr->id = NULL;
	ptr->jointList = NULL;

	debugMsg(( "Initialized ode_JOINTGROUP <%p>", ptr ));
	return ptr;
}


/*
 * GC Mark function
 */
static void
ode_jointGroup_gc_mark( ptr )
	 ode_JOINTGROUP *ptr;
{
	debugMsg(( "Marking an ODE::JointGroup" ));

	if ( ptr ) {
		debugMsg(( "Marking members of JointGroup <%p>", ptr ));
  
		if ( ptr->jointList )
			ode_jointList_iterate( ptr->jointList,
								   ode_jointList_mark );
	}

	else {
		debugMsg(( "Not marking uninitialized ode_JOINTGROUP" ));
	}
}


/*
 * GC Free function
 */
static void
ode_jointGroup_gc_free( ptr )
	 ode_JOINTGROUP *ptr;
{
	if ( ptr ) {
		debugMsg(( "Destroying JointGroup <%p>", ptr ));

		if ( ptr->jointList )
			ode_jointList_iterate( ptr->jointList,
								   ode_jointList_free );

		dJointGroupDestroy( ptr->id );
		ptr->id = NULL;
		ptr->jointList = NULL;

		xfree( ptr );
		ptr = NULL;
	}

	else {
		debugMsg(( "Not freeing uninitialized ode_JOINTGROUP" ));
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ode_JOINTGROUP *
check_jointGroup( self )
	 VALUE	self;
{
	debugMsg(( "Checking a JointGroup object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsJointGroup(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::JointGroup)",
				  rb_class2name(CLASS_OF( self )) );
    }

	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_JOINTGROUP *
get_jointGroup( self )
	 VALUE self;
{
	ode_JOINTGROUP *ptr = check_jointGroup( self );

	debugMsg(( "Fetching an ode_JOINTGROUP (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized jointGroup" );

	return ptr;
}


/*
 * Publicly-usable jointGroup-fetcher.
 */
ode_JOINTGROUP *
ode_get_jointGroup( self )
	 VALUE self;
{
	return get_jointGroup(self);
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
ode_jointGroup_s_alloc( klass )
{
	debugMsg(( "Wrapping an uninitialized ODE::JointGroup pointer." ));
	return Data_Wrap_Struct( klass, ode_jointGroup_gc_mark, ode_jointGroup_gc_free, 0 );
}


/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/*
 * initialize( jointType=nil, world=nil )
 * --
 * Create and return a new ODE::JointGroup. If the optional <tt>jointType</tt>
 * argument (an ODE::Joint derivative class object) is given, joints created via
 * #newJoint will be of the specified type. If it is not set, #newJoint raises
 * an exception.
 */
static VALUE
ode_jointGroup_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_JOINTGROUP	*ptr;
	VALUE			factoryClass, factoryWorld;
	
	DATA_PTR(self) = ptr = ode_jointGroup_alloc();
	
	ptr->id = dJointGroupCreate( 0 );
	ptr->jointList = NULL;

	/* Initialize instance variables */
	rb_iv_set( self, "@factoryClass", Qnil );
	rb_iv_set( self, "@factoryWorld", Qnil );

	if ( rb_scan_args(argc, argv, "02", &factoryClass, &factoryWorld) ) {
		if ( RTEST(factoryClass) )
			rb_funcall( self, rb_intern("factoryClass="), 1, factoryClass );
		if ( RTEST(factoryWorld) )
			rb_funcall( self, rb_intern("factoryWorld="), 1, factoryWorld );
	}

	return self;
}


/*
 * empty()
 * --
 * Remove all the member joints from this group, marking them as obsolete.
 */
static VALUE
ode_jointGroup_empty( self )
	 VALUE self;
{
	ode_JOINTGROUP	*ptr = get_jointGroup( self );

	/* If the joint list has joints in it (ie., isn't NULL), get the joint group
	   struct and clear the joints in it after marking them as obsolete. */
	if ( ptr->jointList ) {
		ode_jointList_iterate( ptr->jointList, ode_jointList_obsolete );
		ode_jointList_iterate( ptr->jointList, ode_jointList_free );
		ptr->jointList = NULL;

		dJointGroupEmpty( ptr->id );
	}

	return Qtrue;
}


/*
 * empty?()
 * --
 * Returns <tt>true</tt> if the joint group is empty.
 */
static VALUE
ode_jointGroup_empty_p( self )
	 VALUE self;
{
	ode_JOINTGROUP	*ptr = get_jointGroup( self );
	return ( ptr->jointList == NULL ? Qtrue : Qfalse );
}




/* -------------------------------------------------------
 * Global functions
 * ------------------------------------------------------- */

/*
 * Register a joint which has been created in a JointGroup with the Ruby part of
 * the JointGroup object.
 */
void
ode_jointGroup_register_joint( self, joint )
	 VALUE self, joint;
{
	ode_JOINTGROUP	*ptr = get_jointGroup( self );
	ode_JOINTLIST	*node;

	debugMsg(( "Registering Joint <%p> with JointGroup <%p>.", joint, self ));

	debugMsg(( "ALLOCing a new group list node." ));
	node = ALLOC( ode_JOINTLIST );
	node->joint = joint;
	node->next = NULL;

	debugMsg(( "New node is <%p>.", node ));

	if ( ptr->jointList == NULL )
		ptr->jointList = node;
	else
		ode_jointList_addNode( ptr->jointList, node );
}


/* JointGroup initializer */
void
ode_init_jointGroup( void ) {
	/* Kluge to make Rdoc see the class in this file */
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeJointGroup		= rb_define_class_under( ode_mOde, "JointGroup", rb_cObject );
#endif

	/* Allocator */
	rb_define_alloc_func( ode_cOdeJointGroup, ode_jointGroup_s_alloc );

	/* Initializer */
	rb_define_method( ode_cOdeJointGroup, "initialize", ode_jointGroup_init, -1 );

	/* Instance methods */
	rb_define_method( ode_cOdeJointGroup, "empty", ode_jointGroup_empty, 0 );
	rb_define_method( ode_cOdeJointGroup, "empty?", ode_jointGroup_empty_p, 0 );

	/* Load the Ruby half of the class */
	rb_require( "ode/jointgroup" );
}

