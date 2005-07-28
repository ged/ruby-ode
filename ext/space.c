/*
 *		space.c - ODE Ruby Binding - ODE::Space class
 *		$Id$
 *		Time-stamp: <27-Jul-2005 22:39:28 ged>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2002-2005 The FaerieMUD Consortium.
 *
 *		This work is licensed under the Creative Commons Attribution License. To
 *		view a copy of this license, visit
 *		http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
 *		Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
 *
 */

#include "ode.h"


/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Macros and constants
 * -------------------------------------------------- */




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
 * ODE::Space Singleton allocator
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


/*
 * geometries
 * --
 * Returns an Array of all the geometries (ODE::Geometry objects) contained in
 * this space.
 */
static VALUE
ode_space_geometries( self )
	 VALUE self;
{
	ode_GEOMETRY *ptr = get_space( self );
	VALUE		 rary = rb_ary_new();
	int			 i, geomCount = dSpaceGetNumGeoms( (dSpaceID)ptr->id );

	for ( i = 0 ; i < geomCount ; i++ ) {
		dGeomID geom = dSpaceGetGeom( (dSpaceID)ptr->id, i );
		rb_ary_push( rary, ((ode_GEOMETRY *)dGeomGetData(geom))->object );
	}

	return rary;
}


/*
 * geometries=( geometryArray )
 * --
 * Set the geometries in the receiving space to the given Array of geometries
 * (ODE::Geometry objects).
 */
static VALUE
ode_space_geometries_eq( self, geometryArray )
	 VALUE self, geometryArray;
{
	ode_GEOMETRY	*ptr = get_space( self );
	dSpaceID		thisSpace = (dSpaceID)( ptr->id );
	int				i, geomCount = dSpaceGetNumGeoms( (dSpaceID)ptr->id );
	VALUE			removeGeoms = rb_ary_new(),
					addGeoms = rb_ary_new();

	Check_Type( geometryArray, T_ARRAY );

	/* First remove any geometries that aren't in the new array -- Build an
	   array of all geometries that are currently in the space but aren't in the
	   new array. */
	for ( i = 0 ; i < geomCount ; i++ ) {
		dGeomID			geom = dSpaceGetGeom( thisSpace, i );
		ode_GEOMETRY	*gptr = dGeomGetData( geom );

		if ( !RTEST(rb_ary_includes( geometryArray, gptr->object )) )
			rb_ary_push( removeGeoms, gptr->object );
	}

	/* If there are any geometries to remove, remove them via #removeGeometries to give
	   derivatives a chance to act on the removals. */
	if ( RARRAY(removeGeoms)->len )
		rb_funcall( self, rb_intern("removeGeometries"), 1, &removeGeoms );

	/* Now make an array of geometries which are in the new array, but aren't in
	   the space currently. */
	for ( i = 0 ; i < RARRAY(geometryArray)->len ; i++ ) {
		ode_GEOMETRY *geom = ode_get_geom( *(RARRAY(geometryArray)->ptr + i) );

		if ( !dSpaceQuery(thisSpace, geom->id) )
			rb_ary_push( addGeoms, geom->object );
	}

	/* If there are any geometries to add, add them via #addGeometries to give
	   derivatives a chance to act on the additions. */
	if ( RARRAY(addGeoms)->len )
		rb_funcall( self, rb_intern("addGeometries"), 1, &addGeoms );

	/* It doesn't really matter what we return here, as Matz decided assignment
	   methods always return what was assigned, so... */
	return Qnil;
}


/*
 * addGeometries( *geometries )
 * --
 * Add the specified geometries (ODE::Geometry objects) to the receiving
 * space and return the space itself.
 */
static VALUE
ode_space_insert( self, args )
	 VALUE self, args;
{
	ode_GEOMETRY	*ptr = get_space( self );
	int				i = 0;

	if ( TYPE(args) != T_ARRAY )
		rb_bug( "Expected array, got a %s.", rb_class2name(CLASS_OF(args)) );

	for ( i = 0; i < RARRAY(args)->len; i++ ) {
		ode_GEOMETRY	*cptr = ode_get_geom( *(RARRAY(args)->ptr + i) );
		dSpaceAdd( (dSpaceID)ptr->id, (dGeomID)cptr->id );
	}

	return self;
}


/*
 * removeGeometries( *geometries )
 * --
 * Remove the specified <tt>geometries</tt> (ODE::Geometry objects) from the
 * receiving space if they are there and return the ones that were removed.
 */
static VALUE
ode_space_remove( self, args )
	 VALUE self, args;
{
	ode_GEOMETRY	*ptr = get_space( self );
	int				i = 0;
	VALUE			rary = rb_ary_new();

	if ( TYPE(args) != T_ARRAY )
		rb_bug( "Expected array, got a %s.", rb_class2name(CLASS_OF(args)) );

	for ( i = 0; i < RARRAY(args)->len; i++ ) {
		ode_GEOMETRY	*cptr = ode_get_geom( *(RARRAY(args)->ptr + i) );
		if ( dSpaceQuery((dSpaceID)ptr->id, (dGeomID)cptr->id) ) {
			rb_ary_push( rary, *(RARRAY(args)->ptr + i) );
			dSpaceRemove( (dSpaceID)ptr->id, (dGeomID)cptr->id );
		}
	}

	return rary;
}


/*
 * Utility recursive containment tester function for ode_space_contains_p().
 */
static int
ode_spaceId_contains( space, geom )
	 dSpaceID space;
	 dGeomID  geom;
{
	int i, numGeoms = dSpaceGetNumGeoms( space );

	if ( dSpaceQuery(space, geom) )
		return 1;

	for ( i = 0 ; i < numGeoms ; i++ ) {
		dGeomID containedGeom = dSpaceGetGeom( space, i );

		if ( dGeomIsSpace(containedGeom) && ode_spaceId_contains((dSpaceID)containedGeom, geom) )
			return 1;
	}

	return 0;
}


/*
 * contains?( geom )
 * --
 * Returns true if the receiving space, or any of the spaces it contains
 * (recursively), contains the specified <tt>geom</tt> (an ODE::Geometry
 * object).
 */
static VALUE
ode_space_contains_p( self, geom )
	 VALUE self, geom;
{
	ode_GEOMETRY	*ptr = get_space( self );
	ode_GEOMETRY	*targetPtr = ode_get_geom( geom );

	if ( ode_spaceId_contains((dSpaceID)ptr->id, targetPtr->id) )
		return Qtrue;
	else
		return Qfalse;
}




/*
 * directlyContains?( geom )
 * --
 * Returns true if the receiving space contains the geom (an ODE::Geometry
 * object) specified. This, unlike contains?, does not recurse into contained
 * subspaces.
 */
static VALUE
ode_space_directly_contains_p( self, geom )
	 VALUE self, geom;
{
	ode_GEOMETRY	*ptr = get_space( self );
	ode_GEOMETRY	*otherptr = ode_get_geom( geom );

	if ( dSpaceQuery((dSpaceID)ptr->id, otherptr->id) )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * each( &block )
 * --
 * Iterate over the geometries in the space, passing each to the given block.
 */
static VALUE
ode_space_each( self )
	 VALUE self;
{
	ode_GEOMETRY *ptr = get_space( self );
	VALUE		 rary = rb_ary_new();
	int			 i, geomCount = dSpaceGetNumGeoms( (dSpaceID)ptr->id );

	if ( !rb_block_given_p() )
		rb_raise( ruby_eLocalJumpError, "no block given" );

	for ( i = 0 ; i < geomCount ; i++ ) {
		dGeomID subgeom = dSpaceGetGeom( (dSpaceID)ptr->id, i );
		ode_GEOMETRY *subgeomPtr = dGeomGetData(subgeom);

		rb_ary_push( rary, rb_yield(subgeomPtr->object) );
	}

	return rary;
}


/*
 * eachAdjacentPair( *data ) {|geom1, geom2, *data| block }
 * --
 * Call the specified <tt>block</tt> once for each adjacent pair of
 * ODE::Geometry objects in the receiving space. The block must accept three
 * arguments: the two geometries and a <tt>data</tt> Array.
 */
static VALUE
ode_space_each_adjacent_pair( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_GEOMETRY	*ptr = get_space( self );
	ode_CALLBACK	*callback;
	VALUE			data, block;

	rb_scan_args( argc, argv, "0*&", &data, &block );

	ode_check_arity( block, 3 );

	callback = ALLOCA_N( ode_CALLBACK, 1 );
	callback->callback = block;
	callback->args = data;

	dSpaceCollide( (dSpaceID)ptr->id, callback,
				   (dNearCallback *)(ode_near_callback) );

	return Qtrue;
}





/* --- ODE::HashSpace ------------------------------ */

/*
 * setLevels( minlevel, maxlevel )
 * --
 * Set some parameters for a multi-resolution hash table space. The smallest and
 * largest cell sizes used in the hash table will be 2^minlevel and 2^maxlevel
 * respectively. The value of minlevel must be less than or equal to the value
 * of maxlevel.
 */
static VALUE
ode_hashspace_set_levels( self, minlevel, maxlevel )
	 VALUE self, minlevel, maxlevel;
{
	ode_GEOMETRY *ptr = get_space( self );
	int min = NUM2INT(minlevel), max = NUM2INT(maxlevel);

	if ( min > max )
		rb_raise( rb_eRangeError, "Min may not be greater than max." );

	dHashSpaceSetLevels( (dSpaceID)ptr->id, min, max );

	return rb_ary_new3( 2, INT2FIX(min), INT2FIX(max) );
}


/* --- ODE::GeometryTransform ------------------------------ */

/*
 * ODE::GeometryTransform Singleton allocator
 */
static VALUE
ode_geom_transform_s_alloc( klass )
	 VALUE klass;
{
	debugMsg(( "Wrapping an uninitialized ODE::GeometryTransform ptr." ));
	return Data_Wrap_Struct( klass, ode_space_gc_mark, ode_space_gc_free, 0 );
}


/*
 * ODE::GeometryTransform#geometries
 * --
 * Return the Array of geometries in the GeometryTransform.
 */
static VALUE
ode_geom_transform_geometries( self )
	 VALUE self;
{
	rb_notimplement();
	return rb_ary_new();
}


/*
 * ODE::GeometryTransform#geometries=( *geometries )
 * --
 * Set the Array of geometries currently in the GeometryTransform.
 */
static VALUE
ode_geom_transform_geometries_eq( self, geoms )
	 VALUE self, geoms;
{
	rb_notimplement();
	return rb_ary_new();
}




/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_space()
{
	/* Kluge to make Rdoc see the class in this file */
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeSpace			= rb_define_class_under( ode_mOde, "Space", ode_cOdeGeometry );
	ode_cOdeHashSpace		= rb_define_class_under( ode_mOde, "HashSpace", ode_cOdeSpace );
	ode_cOdeGeometryTransform = rb_define_class_under( ode_cOdeGeometry, "Transform", ode_cOdeSpace );
	ode_cOdeGeometryTransformGroup = rb_define_class_under( ode_cOdeGeometry, "TransformGroup", ode_cOdeSpace );
#endif


	/* --- ODE::Space ------------------------------ */
	/* Constants */
	rb_define_const( ode_cOdeSpace, "Version", vstr );
	rb_define_const( ode_cOdeSpace, "Rcsid", rb_str_new2(rcsid) );

	/* Constructor */
#ifdef NEW_ALLOC
	rb_define_alloc_func( ode_cOdeSpace, ode_space_s_alloc );
#else
	rb_define_singleton_method( ode_cOdeSpace, "allocate", ode_space_s_alloc, 0 );
#endif

	/* Instance methods */
	rb_define_method( ode_cOdeSpace, "initialize", ode_space_init, -1 );
	rb_enable_super( ode_cOdeSpace, "initialize" );

	rb_define_method( ode_cOdeSpace, "geometries", ode_space_geometries, 0 );
	rb_define_method( ode_cOdeSpace, "geometries=", ode_space_geometries_eq, 1 );

	rb_define_method( ode_cOdeSpace, "addGeometries", ode_space_insert, -2 );
	rb_define_alias ( ode_cOdeSpace, "add_geometries", "addGeometries" );
	rb_define_alias ( ode_cOdeSpace, "<<", "addGeometries" );

	rb_define_method( ode_cOdeSpace, "removeGeometries", ode_space_remove, -2 );
	rb_define_alias ( ode_cOdeSpace, "remove_geometries", "removeGeometries" );

	rb_define_method( ode_cOdeSpace, "contains?", ode_space_contains_p, 1 );
	rb_define_method( ode_cOdeSpace, "directlyContains?", ode_space_directly_contains_p, 1 );

	rb_define_method( ode_cOdeSpace, "each", ode_space_each, 0 );
	rb_define_alias ( ode_cOdeSpace, "eachGeometry", "each" );
	rb_define_alias ( ode_cOdeSpace, "each_geometry", "each" );

	rb_define_method( ode_cOdeSpace, "eachAdjacentPair", ode_space_each_adjacent_pair, -1 );
	rb_define_alias ( ode_cOdeSpace, "each_adjacent_pair", "eachAdjacentPair" );
	rb_define_alias ( ode_cOdeSpace, "eachNearPair", "eachAdjacentPair" );
	rb_define_alias ( ode_cOdeSpace, "each_near_pair", "eachAdjacentPair" );


	/* --- ODE::HashSpace ------------------------------ */
#ifdef NEW_ALLOC
	rb_define_alloc_func( ode_cOdeHashSpace, ode_space_s_alloc );
#else
	rb_define_singleton_method( ode_cOdeHashSpace, "allocate", ode_space_s_alloc, 0 );
#endif

	rb_define_method( ode_cOdeHashSpace, "setLevels", ode_hashspace_set_levels, 2 );
	rb_define_alias ( ode_cOdeHashSpace, "set_levels", "setLevels" );


	/* --- ODE::GeometryTransform ------------------------------ */
#ifdef NEW_ALLOC
	rb_define_alloc_func( ode_cOdeGeometryTransform, ode_geom_transform_s_alloc );
#else
	rb_define_singleton_method( ode_cOdeGeometryTransform, "allocate", ode_geom_transform_s_alloc, 0 );
#endif

	rb_define_method( ode_cOdeGeometryTransform, "geometry", ode_geom_transform_geometries, 0 );
	rb_define_method( ode_cOdeGeometryTransform, "geometry=", ode_geom_transform_geometries_eq, 1 );

	rb_require( "ode/Space" );
}

