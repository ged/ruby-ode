/*
 *		geomTransformGroup.c - ODE Ruby Binding - ODE::Geometry::TransformGroup class
 *		$Id$
 *		Time-stamp: <27-Jul-2005 22:37:39 ged>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2003-2005 The FaerieMUD Consortium. All rights reserved.
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
 * Memory-management functions
 * -------------------------------------------------- */

/*
 * GC mark function
 */
static void
ode_geomtg_gc_mark( ptr )
	 ode_GEOMETRY	*ptr;
{
	debugMsg(( "Marking an ODE::GeometryTransformGroup." ));

	if ( ptr ) {
		int			geomCount;

		debugMsg(( "Marking GeometryTransformGroup <%p>.", ptr ));

		/* Mark the GeometryTransformGroup's container, if any */
		if ( ptr->container ) {
			debugMsg(( "Marking container." ));
			rb_gc_mark( ptr->container );
		}

		/* Mark the body object associated with the GeometryTransformGroup, if any */
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
ode_geomtg_gc_free( ptr )
	 ode_GEOMETRY *ptr;
{
	debugMsg(( "Destroying an ODE::GeometryTransformGroup." ));

	if ( ptr ) {
		dSpaceID space = (dSpaceID)ptr->id;

		debugMsg(( "Freeing GeometryTransformGroup <%p>.", ptr ));
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
 * Alloc a new GeometryTransformGroup.
*/
static ode_GEOMETRY *
ode_geomtg_alloc() {
	ode_GEOMETRY *ptr = ALLOC( ode_GEOMETRY );

	/* Fill in the struct with reasonable defaults */
	ptr->id			= 0;
	ptr->object		= Qnil;
	ptr->container	= Qnil;
	ptr->body		= Qnil;

	debugMsg(( "Initialized ode_GEOMETRY <%p> for an ODE::GeometryTransformGroup.", ptr ));
	return ptr;
}




/*
 * Object validity checker. Returns the data pointer.
 */
static ode_GEOMETRY *
check_geomtg( self )
	 VALUE	self;
{
	debugMsg(( "Checking a ODE::GeometryTransformGroup object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsGeomTg(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::GeometryTransformGroup)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_GEOMETRY *
get_geomtg( self )
	 VALUE self;
{
	ode_GEOMETRY *ptr = check_geomtg( self );

	debugMsg(( "Fetching a GeometryTransformGroup's ode_GEOMETRY  (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized geomtg" );

	return ptr;
}


/* 
 * Fetch method accessable to the other ODE classes.
 */
ode_GEOMETRY *
ode_get_geomtg( obj )
	 VALUE obj;
{
	return get_geomtg( obj );
}


/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * Singleton allocator
 */
static VALUE
ode_geomtg_s_alloc( klass )
	 VALUE klass;
{
	debugMsg(( "Wrapping an uninitialized ODE::GeometryTransformGroup ptr." ));
	return Data_Wrap_Struct( klass, ode_geomtg_gc_mark, ode_geomtg_gc_free, 0 );
}




/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */

/*
 * ODE::GeometryTransformGroup#initialize
 * --
 * Initialize a new ODE::GeometryTransformGroup object.
 */
static VALUE
ode_geomtg_init( klass )
	 VALUE klass;
{
	rb_notimplement();
	return Qnil;
}


/* dGeomID dCreateGeomTransformGroup (dSpaceID space); */
/*   - create a GeomTransformGroup  */
    
/* void dGeomTransformGroupAddGeom    (dGeomID tg, dGeomID obj); */
/*   - Comparable to dGeomTransformSetGeom or dGeomGroupAdd */
/*   - add objects to this group */
   
/* void dGeomTransformGroupRemoveGeom (dGeomID tg, dGeomID obj); */
/*   - remove objects from this group */

/* void dGeomTransformGroupSetRelativePosition */
/*             (dGeomID g, dReal x, dReal y, dReal z); */
/* void dGeomTransformGroupSetRelativeRotation */
/*             (dGeomID g, const dMatrix3 R); */
/*   - Comparable to setting the position and rotation of all the */
/*     dGeomTransform encapsulated geometry. The difference  */
/*     is that it is global with respect to this group and therefore */
/*     affects all geoms in this group. */
/*   - The relative position and rotation are attributes of the  */
/*     transformgroup, so the position and rotation of the individual */
/*     geoms are not changed  */

/* const dReal * dGeomTransformGroupGetRelativePosition (dGeomID g); */
/* const dReal * dGeomTransformGroupGetRelativeRotation (dGeomID g); */
/*   - get the relative position and rotation */
  
/* dGeomID dGeomTransformGroupGetGeom (dGeomID tg, int i); */
/*   - Comparable to dGeomGroupGetGeom */
/*   - get a specific geom of the group */
  
/* int dGeomTransformGroupGetNumGeoms (dGeomID tg); */
/*   - Comparable to dGeomGroupGetNumGeoms */
/*   - get the number of geoms in the group */
  


/*
 * ODE::GeometryTransformGroup#geometries
 * --
 * Return the Array of geometries in the GeometryTransformGroup.
 */
static VALUE
ode_geomtg_geometries( self )
	 VALUE self;
{
	rb_notimplement();
	return rb_ary_new();
}


/*
 * ODE::GeometryTransformGroup#geometries=( *geometries )
 * --
 * Set the Array of geometries currently in the GeometryTransformGroup.
 */
static VALUE
ode_geomtg_geometries_eq( self, geoms )
	 VALUE self, geoms;
{
	rb_notimplement();
	return rb_ary_new();
}






/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_geometry_transform_group()
{
	/* Constructor */
	rb_define_alloc_func( ode_cOdeGeometryTransformGroup, ode_geomtg_s_alloc );

	/* Initializer */
	rb_define_method( ode_cOdeGeometryTransformGroup, "initialize", ode_geomtg_init, -1 );
	rb_enable_super ( ode_cOdeGeometryTransformGroup, "initialize" );

	/* Instance methods */
	rb_define_method( ode_cOdeGeometryTransformGroup, "geometries", ode_geomtg_geometries, 0 );
	rb_define_method( ode_cOdeGeometryTransformGroup, "geometries=", ode_geomtg_geometries_eq, 1 );
	
}

