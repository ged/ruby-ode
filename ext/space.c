/*
 *		space.c - ODE Ruby Binding - ODE::Space class
 *		$Id: space.c,v 1.1 2002/11/23 23:08:10 deveiant Exp $
 *		Time-stamp: <21-Nov-2002 05:37:51 deveiant>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2002 The FaerieMUD Consortium. All rights reserved.
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

#define IsSpace( obj ) rb_obj_is_kind_of( (obj), ode_cOdeSpace )



/* --------------------------------------------------
 * Memory-management functions
 * -------------------------------------------------- */

/*
 * GC mark function
 */
static void
ode_space_gc_mark( ptr )
	 ode_GEOMETRY	*ptr;
{
	debugMsg(( "Marking an ODE::Space." ));

	if ( ptr ) {
		int			geomCount;

		debugMsg(( "Marking Space <%p>.", ptr ));

		/* Mark the space's container, if any */
		if ( ptr->container ) {
			debugMsg(( "Marking container." ));
			rb_gc_mark( ptr->container );
		}

		/* Mark the body object associated with the space, if any */
		if ( ptr->body ) {
			debugMsg(( "Marking body object." ));
			rb_gc_mark( ptr->body );
		}

		/* Mark any contained geometries/spaces */
		if (( geomCount = dSpaceGetNumGeoms((dSpaceID)ptr->id) )) {
			int				i = 0;
			dGeomID			geom;
			ode_GEOMETRY	*subptr;

			debugMsg(( "Marking %d contained geoms.", geomCount ));

			for ( i = 0; i < geomCount; i++ ) {
				geom = dSpaceGetGeom( (dSpaceID)ptr->id, i );
				subptr = (ode_GEOMETRY *)dGeomGetData( (dGeomID)geom );

				debugMsg(( "Marking contained geom <%p> (%p).", geom, subptr ));
				rb_gc_mark( subptr->object );
			}
		} else {
			debugMsg(( "No contained geometries to mark." ));
		}
	} else {
		debugMsg(( "No need to mark an uninitialized Space." ));
	}
}
	
/*
 * GC free function
 */
static void
ode_space_gc_free( ptr )
	 ode_GEOMETRY *ptr;
{
	debugMsg(( "Destroying an ODE::Space." ));

	if ( ptr ) {
		dSpaceID space = (dSpaceID)ptr->id;

		debugMsg(( "Freeing Space <%p>.", ptr ));
		dGeomSetData( (dGeomID)space, 0 );
		dSpaceDestroy( space );
		
		ptr->id			= NULL;
		ptr->container	= Qnil;
		ptr->object		= Qnil;

		xfree( ptr );
		ptr = NULL;
	} else {
		debugMsg(( "Not freeing NULL pointer." ));
	}
}

/*
 * Alloc a new Space.
*/
static ode_GEOMETRY *
ode_space_alloc()
{
	ode_GEOMETRY *ptr = ALLOC( ode_GEOMETRY );

	/* Fill in the struct with reasonable defaults */
	ptr->id			= 0;
	ptr->object		= Qnil;
	ptr->container	= Qnil;
	ptr->body		= Qnil;

	debugMsg(( "Initialized ode_GEOMETRY <%p> for an ODE::Space.", ptr ));
	return ptr;
}




/*
 * Object validity checker. Returns the data pointer.
 */
static ode_GEOMETRY *
check_space( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Space object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsSpace(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Space)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_GEOMETRY *
get_space( self )
	 VALUE self;
{
	ode_GEOMETRY *ptr = check_space( self );

	debugMsg(( "Fetching a Space's ode_GEOMETRY  (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized space" );

	return ptr;
}


/* 
 * Fetch method accessable to the other ODE classes.
 */
ode_GEOMETRY *
ode_get_space( obj )
	 VALUE obj;
{
	return get_space( obj );
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * Singleton allocator
 */
static VALUE
ode_space_s_alloc( klass )
	 VALUE klass;
{
	debugMsg(( "Wrapping an uninitialized ODE::Space ptr." ));
	return Data_Wrap_Struct( klass, ode_space_gc_mark, ode_space_gc_free, 0 );
}


/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */

/* 
 * Base initializer.
 */
static VALUE
ode_space_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	debugMsg(( "ODE::Space init." ));

	/* Create the underlying dSpaceID object if it hasn't been already */
	if ( !check_space(self) ) {
		ode_GEOMETRY	*ptr;
		dSpaceID		containerSpace = 0;
		VALUE			container = Qnil;

		debugMsg(( "Space::initialize: Fetching new data object." ));

		/* If they gave a container space, fetch it */
		if ( rb_scan_args(argc, argv, "01", &container) ) {
			ode_GEOMETRY	*containerPtr;

			containerPtr = get_space( container );
			debugMsg(( "Got container space <%p>", containerPtr ));
			containerSpace = (dSpaceID)containerPtr->id;
		}

		/* Allocate the ode_GEOMETRY struct for this space */
		DATA_PTR(self) = ptr = ode_space_alloc();
		ptr->object		= self;
		ptr->container	= container;
		debugMsg(( "New space = <%p>", ptr ));

		/* Create the ODE space object according to which class is being initialized */
		if ( CLASS_OF(self) == ode_cOdeSpace )
			ptr->id = (dGeomID)dSimpleSpaceCreate( containerSpace );
		else if ( CLASS_OF(self) == ode_cOdeHashSpace )
			ptr->id = (dGeomID)dHashSpaceCreate( containerSpace );
		else
			rb_raise( rb_eTypeError, "No allocator defined for a %s.",
					  rb_class2name(CLASS_OF( self )) );

		/* Tell ODE not to clean up its spaces itself to prevent running around
		   pointing to freed memory */
		debugMsg(( "Turning off cleanup flag." ));
		dSpaceSetCleanup( (dSpaceID)ptr->id, 0 );

		/* Set the ode_GEOMETRY struct as the space's data pointer so we can get
		   the object from the dSpaceID */
		dGeomSetData( ptr->id, ptr );
	}

	/* Call our parent's initializer */
	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	return self;
}



/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_space()
{
	static char
		rcsid[]		= "$Id: space.c,v 1.1 2002/11/23 23:08:10 deveiant Exp $",
		revision[]	= "$Revision: 1.1 $";

	VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );

	/* --- ODE::Space ------------------------------ */
	/* Constants */
	rb_define_const( ode_cOdeSpace, "Version", vstr );
	rb_define_const( ode_cOdeSpace, "Rcsid", rb_str_new2(rcsid) );

	/* Constructor */
	rb_define_singleton_method( ode_cOdeSpace, "allocate", ode_space_s_alloc, 0 );

	/* Instance methods */
	rb_define_method( ode_cOdeSpace, "initialize", ode_space_init, -1 );
	rb_enable_super( ode_cOdeSpace, "initialize" );


	/* --- ODE::HashSpace ------------------------------ */
	rb_define_singleton_method( ode_cOdeHashSpace, "allocate", ode_space_s_alloc, 0 );

	
}

