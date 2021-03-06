/*
 *		geometry.c - ODE Ruby Binding - ODE::Geometry class
 *		$Id$
 *		Time-stamp: <27-Jul-2005 22:38:14 ged>
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
#ifdef HAVE_ODE_DCYLINDER_H
#include <ode/dCylinder.h>
#endif

/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Macros and constants
 * -------------------------------------------------- */

static unsigned int X = 0;
static unsigned int Y = 1;
static unsigned int Z = 2;

#define SetContainer( obj, sptr, gs ) {\
	(gs)->container = obj; \
	if ( RTEST(obj) ) { \
		(sptr) = (dSpaceID)(ode_get_space(obj)->id); \
		debugMsg(( "Setting container space to <%p>.", (sptr) )); \
	} else { \
		(sptr) = 0; \
		debugMsg(( "Unsetting container space." )); \
	} \
}

#define IsGeom( obj ) rb_obj_is_kind_of( (obj), ode_cOdeGeometry )

/* --------------------------------------------------
 * Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static ode_GEOMETRY *
ode_geometry_alloc()
{
	ode_GEOMETRY *ptr = ALLOC( ode_GEOMETRY );

	/* Fill in the struct */
	ptr->id			= 0;
	ptr->object		= Qnil;
	ptr->container	= Qnil;
	ptr->body		= Qnil;
	ptr->surface	= Qnil;
	
	debugMsg(( "Initialized ode_GEOMETRY <%p>", ptr ));
	return ptr;
}


/*
 * GC mark function
 */
static void 
ode_geometry_gc_mark( ptr )
	 ode_GEOMETRY *ptr;
{
	debugMsg(( "Marking an ODE::Geometry." ));
	if ( ptr ) {
		debugMsg(( "Marking Geometry <%p>.", ptr ));

		rb_gc_mark( ptr->container );
		rb_gc_mark( ptr->body );
		rb_gc_mark( ptr->surface );
	}

	else {
		debugMsg(( "Not marking NULL pointer." ));
	}
}


/*
 * GC free function
 */
static void 
ode_geometry_gc_free( ptr )
	 ode_GEOMETRY *ptr;
{
	debugMsg(( "Freeing an ODE::Geometry." ));

	if ( ptr ) {
		debugMsg(( "Freeing Geometry <%p>.", ptr ));

		if ( ptr->id )
			dGeomDestroy( ptr->id );

		ptr->id			= NULL;
		ptr->object		= Qnil;
		ptr->container	= Qnil;
		ptr->body		= Qnil;
		ptr->surface	= Qnil;

		xfree( ptr );
		ptr = NULL;
	}

	else {
		debugMsg(( "Not freeing NULL pointer." ));
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ode_GEOMETRY *
check_geom( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Geometry object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsGeom(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Geometry)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_GEOMETRY *
get_geom( self )
	 VALUE self;
{
	ode_GEOMETRY *ptr = check_geom( self );

	debugMsg(( "Fetching an ode_GEOMETRY (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized geometry" );

	return ptr;
}


/*
 * Publicly-usable geometry-fetcher.
 */
ode_GEOMETRY *
ode_get_geom( self )
	 VALUE self;
{
	return get_geom(self);
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */


/*
 * Generalized constructor
 */
static VALUE
ode_geometry_s_alloc( klass )
	 VALUE klass;
{
	debugMsg(( "Wrapping an uninitialized ODE::Geometry pointer." ));
	return Data_Wrap_Struct( klass, ode_geometry_gc_mark, ode_geometry_gc_free, 0 );
}



/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */

/* --- ODE::Geometry ------------------------------ */

/* 
 * Base initializer.
 */
static VALUE
ode_geometry_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	debugMsg(( "ODE::Geometry init." ));
	if ( !check_geom(self) ) {
		ode_GEOMETRY *geom;

		debugMsg(( "Fetching new data object." ));
		DATA_PTR(self) = geom = ode_geometry_alloc();

		geom->object = self;

		debugMsg(( "New ode_GEOMETRY = <%p>", geom ));
	}

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	return self;
}


/*
 * ODE::Geometry#container
 * --
 * Return the ODE::Space which contains this object, if it is contained, or
 * <tt>nil</tt> if not.
 */
static VALUE
ode_geometry_container( self )
	 VALUE self;
{
	ode_GEOMETRY *ptr = get_geom( self );
	return ptr->container;
}


/*
 * ODE::Geometry#aabb
 * --
 * Return an axis aligned bounding box that surrounds the given geom as a
 * six-element Array. The aabb array has elements [minx, maxx, miny, maxy, minz,
 * maxz]. If the geom is a space, a bounding box that surrounds all contained
 * geoms is returned.
 *
 * This function may return a pre-computed cached bounding box, if it can
 * determine that the geom has not moved since the last time the bounding box
 * was computed.
 */
static VALUE
ode_geometry_aabb( self )
	 VALUE self;
{
	VALUE			aabbArray;
	dReal			aabb[6];
	ode_GEOMETRY	*geometry = get_geom( self );
	int				i;
	
	dGeomGetAABB( geometry->id, aabb );
	
	aabbArray = rb_ary_new2( 6 );
	for ( i = 0; i < 6; i++ )
		rb_ary_store( aabbArray, i, rb_float_new(aabb[i]) );
	
	return aabbArray;
}


/*
 * ODE::Geometry#isSpace?
 * --
 * Returns <tt>true</tt> if the receiving Geometry is an ODE::Space or one of
 * its derivatives. This should be functionally (but not implementationally) the
 * same as #kind_of?( ODE::Space ), but is provided for completeness. Alias:
 * <tt>is_space?</tt>.
 */
static VALUE
ode_geometry_space_p( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom(self);

	if ( dGeomIsSpace(geometry->id) )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * ODE::Geometry#categoryMask
 * --
 * Get the 'category' bitfield for the receiving Geometry. This bitfield is
 * used by spaces to govern which geoms will interact with each other; each bit
 * position in the bitfield represents a different category of object. The
 * actual meaning of these categories (if any) is user defined. The category
 * bitfield indicates which categories a geom is a member of. The bit fields are
 * guaranteed to be at least 32 bits wide. The default category mask for newly
 * created geoms has all bits set.
 */
static VALUE
ode_geometry_category_mask( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	unsigned long	mask;

	mask = dGeomGetCategoryBits( geometry->id );

	return INT2NUM( mask );
}


/*
 * ODE::Geometry#categoryMask=( mask )
 * --
 * Set the 'category' bitfield for the receiving Geometry. This bitfield is
 * used by spaces to govern which geoms will interact with each other; each bit
 * position in the bitfield represents a different category of object. The
 * actual meaning of these categories (if any) is user defined. The category
 * bitfield indicates which categories a geom is a member of. The bit fields are
 * guaranteed to be at least 32 bits wide. The default category mask for newly
 * created geoms has all bits set.
 */
static VALUE
ode_geometry_category_mask_eq( self, newMask )
	 VALUE self, newMask;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	unsigned long	mask;

	mask = NUM2ULONG( newMask );
	CheckPositiveNumber( mask, "mask" );

	dGeomSetCategoryBits( geometry->id, mask );

	return INT2NUM( dGeomGetCategoryBits(geometry->id) );
}


/*
 * ODE::Geometry#collideMask
 * --
 * Get the 'collide' bitfield for the receiving Geometry. This bitfield is used
 * by spaces to govern which geoms will interact with each other; each bit
 * position in the bitfield represents a different category of object. A
 * Geometry will only collide with an object which belongs to a category that it
 * has a collide bit set for. The bit fields are guaranteed to be at least 32
 * bits wide. The default category mask for newly created geoms has all bits
 * set.
 */
static VALUE
ode_geometry_collide_mask( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	unsigned long	mask;

	mask = dGeomGetCollideBits( geometry->id );

	return INT2NUM( mask );
}


/*
 * ODE::Geometry#collideMask=( mask )
 * --
 * Set the 'collide' bitfield for the receiving Geometry. This bitfield is used
 * by spaces to govern which geoms will interact with each other; each bit
 * position in the bitfield represents a different category of object. A
 * Geometry will only collide with an object which belongs to a category that is
 * has a collide bit set for. The bit fields are guaranteed to be at least 32
 * bits wide. The default category mask for newly created geoms has all bits
 * set.
 */
static VALUE
ode_geometry_collide_mask_eq( self, newMask )
	 VALUE self, newMask;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	unsigned long	mask;

	mask = NUM2ULONG( newMask );
	CheckPositiveNumber( mask, "mask" );

	dGeomSetCollideBits( geometry->id, mask );

	return INT2NUM( dGeomGetCollideBits(geometry->id) );
}


/*
 * ODE::Geometry#enable
 * --
 * Enable the receiving Geometry; Disabled geoms are completely ignored by their
 * containing space's collision methods, although they can still be members of a
 * one.
 */
static VALUE
ode_geometry_enable( self )
	 VALUE self;
{
#ifdef HAVE_DGEOMENABLE
	ode_GEOMETRY	*geometry = get_geom(self);
	dGeomEnable( geometry->id );
	return Qtrue;
#else
	rb_notimplement();
#endif /* HAVE_DGEOMENABLE */
}


/*
 * ODE::Geometry#disable
 * --
 * Diaable the receiving Geometry; Disabled geoms are completely ignored by
 * their containing space's collision methods, although they can still be
 * members of a one.
 */
static VALUE
ode_geometry_disable( self )
	 VALUE self;
{
#ifdef HAVE_DGEOMENABLE
	ode_GEOMETRY	*geometry = get_geom(self);
	dGeomDisable( geometry->id );
	return Qfalse;
#else
	rb_notimplement();
#endif /* HAVE_DGEOMENABLE */
}


/*
 * ODE::Geometry#enabled?
 * --
 * Returns true if the receiving Geometry is enabled.
 */
static VALUE
ode_geometry_enabled_p( self )
	 VALUE self;
{
#ifdef HAVE_DGEOMENABLE
	ode_GEOMETRY	*geometry = get_geom(self);
	if ( dGeomIsEnabled(geometry->id) )
		return Qtrue;
	else
		return Qfalse;
#else
	rb_notimplement();
#endif /* HAVE_DGEOMENABLE */
}


/*
 * ODE::Geometry#surface
 * --
 * Get the geometry's surface (an ODE::Surface object).
 */
static VALUE
ode_geometry_surface( self )
	 VALUE self;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	return ptr->surface;
}


/*
 * ODE::Geometry#surface=( surface )
 * --
 * Set the geometry's surface (an ODE::Surface object) to the one specified.
 */
static VALUE
ode_geometry_surface_eq( self, surface )
	 VALUE self, surface;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	
	CheckKindOf( surface, ode_cOdeSurface );
	ptr->surface = surface;

	return surface;
}


/*
 * ODE::Geometry#body
 * --
 * Returns <tt>nil</tt>; overridden in ODE::Geometry::Placeable.
 */
static VALUE
ode_geometry_body( self )
	 VALUE self;
{
	return Qnil;
}


/*
 * ODE::Geometry#body=
 * --
 * Raises an exception; overridden in ODE::Geometry::Placeable.
 */
static VALUE
ode_geometry_body_eq( self )
	 VALUE self;
{
	rb_raise( ode_eOdeGeometryError, "Cannot set a body for a non-placeable geometry." );
}


/*
 * ODE::Geometry#collideWith( otherGeometry, maxContacts=5, &contactHandler )
 * --
 * Generate contact information for the receiving Geometry and
 * <tt>otherGeometry</tt> in the form of at most <tt>maxContacts</tt>
 * ODE::Contact objects, yielding each in turn to the given
 * <tt>contactHandler</tt>. This corresponds to (and is really just a wrapper
 * around) the dCollide() function in the C API.
 */
static VALUE
ode_geometry_collide( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_GEOMETRY	*geom1, *geom2;
	VALUE			otherGeom, maxContacts, contact;
	dContactGeom	*cgeoms;
	int				flags, contactCount, i;

	rb_scan_args( argc, argv, "11", &otherGeom, &maxContacts );

	CheckKindOf( otherGeom, ode_cOdeGeometry );
	if ( !rb_block_given_p() )
		rb_raise( ruby_eLocalJumpError, "no block given" );
	
	/* Fetch or default the contact count */
	if ( RTEST(maxContacts) ) {
		flags = NUM2INT( maxContacts );
		CheckPositiveNonZeroNumber( flags, "maxContacts" );
	}
	else {
		flags = 5;
	}
	
	/* Unwrap the geometry structs */
	geom1 = get_geom( self );
	geom2 = get_geom( otherGeom );
	
	/* Echo some of ODE's own optimizations here to save pointless memory
	   allocation.
	   :TODO: ODE currently doesn't do much more than this, but if it ever does
	   provide some use for colliding an object with itself, this will have to
	   be removed. */
	if ( geom1 == geom2 ) return INT2FIX( 0 );
	if ( geom1->body == geom2->body && RTEST(geom1->body) )
		return INT2FIX( 0 );
	
	/* Allocate contacts and generate contact information. */
	cgeoms = ALLOCA_N( dContactGeom, (flags & 0xffff) );
	contactCount = dCollide( geom1->id, geom2->id, flags, cgeoms, sizeof(dContactGeom) );

	/* Yield to the block for each contact object */
	for ( i = 0; i < contactCount; i++ ) {
		contact = rb_class_new_instance( 0, 0, ode_cOdeContact );

		/* Set the internal contact geometry of the contact object to this
		   contact geom and call the collision callback. */
		ode_contact_set_cgeom( contact, cgeoms + i );
		rb_yield( contact );
	}

	return INT2FIX( contactCount );
}


/*
 * ODE::Geometry#intersectWith( otherGeom, *data, &nearCallback )
 * --
 * Call the <tt>nearCallback</tt> for potentially intersecting pairs that
 * contain one geom from the receiving geometry, and one from the
 * <tt>otherGeom</tt>. Any arguments passed in the <tt>data</tt> Array will be
 * passed to the callback as arguments after the two potentially-colliding
 * geometry objects.
 *
 * The exact behavior depends on the types of the receiver and the
 * <tt>otherGeom</tt>:
 *
 * * If one of the geoms is a non-space geom and the other is a space, the
 *   callback is called with all potential intersections between the geom and
 *   the objects in the space.
 *
 * * If both the receiver and <tt>otherGeom</tt> are spaces, then this calls the
 *   callback for all potentially intersecting pairs that contain one geom from
 *   the receiver and one geom from <tt>otherGeom</tt>. The algorithm that is
 *   used depends on what kinds of spaces are being collided. If no optimized
 *   algorithm can be selected then this function will resort to one of the
 *   following two strategies:
 *
 *   1. All the geoms in the receiver are tested one-by-one against
 *      <tt>otherGeom</tt>.
 *   2. All the geoms in <tt>otherGeom</tt> are tested one-by-one against the
 *      receiver.
 *
 * The strategy used may depends on a number of rules, but in general the space
 * with less objects has its geoms examined one-by-one.
 *
 * * If both geoms are the same space, this is equivalent to calling
 *   #eachAdjacentPair() on that space.
 *
 * * If both the receiver and <tt>otherGeom</tt> are non-space geoms, this
 *   simply calls the callback once with them.
 *
 * This method is the equivalent of the dSpaceCollide2() function from the C
 * API.
 */
static VALUE
ode_geometry_isect( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_GEOMETRY	*geometry, *geometry2;
	ode_CALLBACK	*callback;
	VALUE			otherGeom, data, block;

	rb_scan_args( argc, argv, "1*&", &otherGeom, &data, &block );

	geometry  = get_geom( self );
	geometry2 = get_geom( otherGeom );

	ode_check_arity( block, 3 );

	callback = ALLOCA_N( ode_CALLBACK, 1 );
	callback->callback = block;
	callback->args = data;

	dSpaceCollide2( geometry->id, geometry2->id, callback,
					(dNearCallback *)(ode_near_callback) );

	return Qtrue;
}





/* --- ODE::Geometry::Placeable ------------------------------ */

/*
 * ODE::Geometry::Placeable#body
 * --
 * Fetch the body associated with the receiving geometry, if any. If no body is
 * associated, returns <tt>nil</tt>.
 */
static VALUE
ode_geometry_placeable_body( self )
	 VALUE self;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	dBodyID			body = dGeomGetBody( ptr->id );

	if ( body ) {
		ode_BODY *bodyPtr = (ode_BODY *)dBodyGetData( body );

		if ( ! bodyPtr ) rb_bug( "dBodyID with null data pointer in placeable geom." );
		return bodyPtr->object;
	}

	else {
		return Qnil;
	}
}


/*
 * ODE::Geometry::Placeable#body=( ODE::Body )
 * --
 * Set the body associated with the receiving geometry. Setting a body on a
 * geometry automatically combines the position vector and rotation matrix of
 * the body and geom, so that setting the position or orientation of one will
 * set the value for both objects.
 *
 * Setting a geometry's body to <tt>nil</tt> gives it its own position and
 * rotation, independent from any body. If the geometry was previously connected
 * to a body then its new independent position/rotation is set to the last
 * position/rotation of the body.
 */
static VALUE
ode_geometry_placeable_body_eq( self, body )
	 VALUE self, body;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	ode_BODY		*bodyPtr = ode_get_body( body );

	dGeomSetBody( ptr->id, bodyPtr->id );

	return body;
}


/*
 * ODE::Geometry::Placeable#position
 * --
 * 
 */
static VALUE
ode_geometry_placeable_position( self )
	 VALUE self;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	const dReal		*vec = dGeomGetPosition( ptr->id );
	VALUE			position;
	
	Vec3ToOdePosition( vec, position );
	return position;
}


/*
 * ODE::Geometry::Placeable#position=
 * --
 * 
 */
static VALUE
ode_geometry_placeable_position_eq( self, position )
	 VALUE self, position;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	VALUE			posArray;

	/* Normalize the argument/s into a 3rd-order vector */
	if ( RARRAY(position)->len == 1 ) 
		posArray = ode_obj_to_ary3( *(RARRAY(position)->ptr), "position" );
	else
		posArray = ode_obj_to_ary3( position, "position" );

	/* Set the position from the normalized array */
	dGeomSetPosition( ptr->id,
					  (dReal)NUM2DBL(*(RARRAY(posArray)->ptr    )),
					  (dReal)NUM2DBL(*(RARRAY(posArray)->ptr + 1)),
					  (dReal)NUM2DBL(*(RARRAY(posArray)->ptr + 2)) );
	
	return Qtrue;
}


/*
 * ODE::Geometry::Placeable#rotation
 * --
 * 
 */
static VALUE
ode_geometry_placeable_rotation( self )
	 VALUE self;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	const dReal		*quat;
	VALUE			args[4];

	/* Fetch the rotation quaternion for the geom. */
	quat = dGeomGetRotation( ptr->id );

	args[0] = rb_float_new( *(quat  ) );
	args[1] = rb_float_new( *(quat+1) );
	args[2] = rb_float_new( *(quat+2) );
	args[3] = rb_float_new( *(quat+3) );
	
	/* Create a new ODE::Quaternion object from the quaternion and return it */
	return rb_class_new_instance( 4, args, ode_cOdeQuaternion );
	
	
}


/*
 * ODE::Geometry::Placeable#rotation=
 * --
 * 
 */
static VALUE
ode_geometry_placeable_rotation_eq( self, rotation )
	 VALUE self, rotation;
{
	ode_GEOMETRY	*ptr = get_geom( self );
	VALUE			ary;
	dQuaternion		quat;
	dMatrix3		R;
	int				i;

	/* Call to_ary on whatever we got, and make sure it's an Array with four elements. */
	if ( RARRAY(rotation)->len == 1 ) 
		ary = ode_obj_to_ary4( *(RARRAY(rotation)->ptr), "rotation" );
	else
		ary = ode_obj_to_ary4( rotation, "rotation" );

	/* Copy the values in the array into the quaternion */
	for ( i = 0 ; i <= 3 ; i++ )
		quat[i] = NUM2DBL( *(RARRAY(ary)->ptr + i) );
	dQtoR( quat, R );
  
	/* Get the body and set its rotation */
	dGeomSetRotation( ptr->id, R );

	return Qtrue;
	
	
}



/* --- ODE::Geometry::Sphere -------------------- */

/*
 * ODE::Geometry::Sphere::new( radius, space=nil )
 * --
 * Create a new spherical collision geometry with the specified radius,
 * inserting it into the specified space, if given.
 */
static VALUE
ode_geometry_sphere_init( argc, argv, self )
	 int		argc;
	 VALUE		*argv, self;
{
	VALUE			radius, spaceObj;
	dSpaceID		space = 0;
	ode_GEOMETRY	*geometry = 0;

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	/* Fetch the ode_GEOMETRY pointer */
	geometry = get_geom( self );
	if ( !geometry ) rb_bug( "Superclass's initialize didn't return a valid Geometry." );
	debugMsg(( "Sphere::initialize: Geometry is <%p>", geometry ));
	
	debugMsg(( "Sphere::initialize: Scanning arguments." ));
	if ( rb_scan_args(argc, argv, "11", &radius, &spaceObj) == 2 ) {
		SetContainer( spaceObj, space, geometry );
	}

	CheckPositiveNonZeroNumber( NUM2DBL(radius), "radius" );

	debugMsg(( "Creating new Sphere geometry." ));
	geometry->id = (dGeomID)dCreateSphere( space, (dReal)NUM2DBL(radius) );

	/* Set the ode_GEOMETRY pointer as the data pointer of the dGeomID */
	dGeomSetData( geometry->id, geometry );

	return self;
}


/*
 * ODE::Geometry::Sphere#radius
 * --
 * Get the sphere's radius.
 */
static VALUE
ode_geometry_sphere_radius( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom(self);
	return rb_float_new( (dReal)dGeomSphereGetRadius(geometry->id) );
}


/*
 * ODE::Geometry::Sphere#radius=( newValue )
 * --
 * Set the sphere's radius.
 */
static VALUE
ode_geometry_sphere_radius_eq( self, newRadius )
	 VALUE self, newRadius;
{
	ode_GEOMETRY	*geometry = get_geom( self );

	CheckPositiveNonZeroNumber( NUM2DBL(newRadius), "radius" );
	dGeomSphereSetRadius( geometry->id, (dReal)NUM2DBL(newRadius) );

	return rb_float_new( (dReal)dGeomSphereGetRadius(geometry->id) );
}


/* --- ODE::Geometry::Box -------------------- */

/*
 * ODE::Geometry::Box::new( lx, ly, lz, space=nil )
 * --
 * Create a new rectangular collision geometry with the specified side lengths,
 * and insert it into the specified space, if given.
 */
static VALUE
ode_geometry_box_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	VALUE			lx, ly, lz, spaceObj;
	dSpaceID		space = 0;
	ode_GEOMETRY	*geometry = 0;

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	/* Fetch the ode_GEOMETRY pointer */
	geometry = get_geom(self);
	if ( !geometry ) rb_bug( "Superclass's initialize didn't return a valid Geometry." );
	debugMsg(( "Box::initialize: Geometry is <%p>", geometry ));
	
	debugMsg(( "Box::initialize: Scanning arguments." ));
	if ( rb_scan_args(argc, argv, "31", &lx, &ly, &lz, &spaceObj) == 4 ) {
		SetContainer( spaceObj, space, geometry );
	}

	CheckPositiveNonZeroNumber( NUM2DBL(lx), "X" );
	CheckPositiveNonZeroNumber( NUM2DBL(ly), "Y" );
	CheckPositiveNonZeroNumber( NUM2DBL(lz), "Z" );

	debugMsg(( "Creating new Box geometry." ));
	geometry->id = (dGeomID)dCreateBox( space,
										(dReal)NUM2DBL(lx),
										(dReal)NUM2DBL(ly),
										(dReal)NUM2DBL(lz) );

	/* Set the ode_GEOMETRY pointer as the data pointer of the dGeomID */
	dGeomSetData( geometry->id, geometry );

	return self;
}


/*
 * ODE::Geometry::Box#lengths
 * --
 * Returns the box's lengths as a 3-element array.
 */
static VALUE
ode_geometry_box_lengths( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		result;

	dGeomBoxGetLengths( geometry->id, result );

	return rb_ary_new3( 3,
						rb_float_new(result[0]),
						rb_float_new(result[1]),
						rb_float_new(result[2]) );
}


/*
 * ODE::Geometry::Box#lengths=( lengths )
 * --
 * Set the box's radius to <tt>lengths</tt>, which can be any object which
 * returns an array with 3 numeric values when <tt>to_ary</tt> is called on it,
 * such a Math3d::Vector3 or an Array with 3 numeric values.
 */
static VALUE
ode_geometry_box_lengths_eq( self, lengths )
	 VALUE self, lengths;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	VALUE			lengthsArray;
	dVector3		result;

	debugMsg(( "Setting box lengths: Got %d argument/s.", RARRAY(lengths)->len ));

	/* Normalize the dimensions into an array */
	if ( RARRAY(lengths)->len == 1 ) 
		lengthsArray = ode_obj_to_ary3( *(RARRAY(lengths)->ptr), "lengths" );
	else
		lengthsArray = ode_obj_to_ary3( lengths, "lengths" );

	debugMsg(( "Arguments normalized. Setting box lengths." ));

	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(lengthsArray)->ptr  )), "lx" );
	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(lengthsArray)->ptr+1)), "ly" );
	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(lengthsArray)->ptr+2)), "lz" );

	dGeomBoxSetLengths( geometry->id,
						(dReal)NUM2DBL(*(RARRAY(lengthsArray)->ptr    )),
						(dReal)NUM2DBL(*(RARRAY(lengthsArray)->ptr + 1)),
						(dReal)NUM2DBL(*(RARRAY(lengthsArray)->ptr + 2)) );

	debugMsg(( "Lengths set." ));

	dGeomBoxGetLengths( geometry->id, result );
	return rb_ary_new3( 3,
						rb_float_new(result[0]),
						rb_float_new(result[1]),
						rb_float_new(result[2]) );
}


/*
 * ODE::Geometry::Box#lx
 * --
 * Returns the box's X-axis length.
 */
static VALUE
ode_geometry_box_length_x( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		result;

	dGeomBoxGetLengths( geometry->id, result );

	return rb_float_new(result[X]);
}


/*
 * ODE::Geometry::Box#lx=( length )
 * --
 * Sets the box's X-axis length.
 */
static VALUE
ode_geometry_box_length_x_eq( self, newLength )
	 VALUE self, newLength;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		lengths;

	CheckPositiveNonZeroNumber( NUM2DBL(newLength), "length" );

	dGeomBoxGetLengths( geometry->id, lengths );

	lengths[X] = (dReal)NUM2DBL( newLength );
	dGeomBoxSetLengths( geometry->id, lengths[X], lengths[Y], lengths[Z] );

	return rb_float_new(lengths[X]);
}


/*
 * ODE::Geometry::Box#ly
 * --
 * Returns the box's Y-axis length.
 */
static VALUE
ode_geometry_box_length_y( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		result;

	dGeomBoxGetLengths( geometry->id, result );

	return rb_float_new(result[Y]);
}


/*
 * ODE::Geometry::Box#ly=( length )
 * --
 * Sets the box's Y-axis length.
 */
static VALUE
ode_geometry_box_length_y_eq( self, newLength )
	 VALUE self, newLength;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		lengths;

	CheckPositiveNonZeroNumber( NUM2DBL(newLength), "length" );

	dGeomBoxGetLengths( geometry->id, lengths );

	lengths[Y] = (dReal)NUM2DBL( newLength );
	dGeomBoxSetLengths( geometry->id, lengths[X], lengths[Y], lengths[Z] );

	return rb_float_new(lengths[Y]);
}


/*
 * ODE::Geometry::Box#lz
 * --
 * Returns the box's Z-axis length.
 */
static VALUE
ode_geometry_box_length_z( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		result;

	dGeomBoxGetLengths( geometry->id, result );

	return rb_float_new(result[Z]);
}


/*
 * ODE::Geometry::Box#lz=( length )
 * --
 * Sets the box's Z-axis length.
 */
static VALUE
ode_geometry_box_length_z_eq( self, newLength )
	 VALUE self, newLength;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		lengths;

	CheckPositiveNonZeroNumber( NUM2DBL(newLength), "length" );

	dGeomBoxGetLengths( geometry->id, lengths );

	lengths[Z] = (dReal)NUM2DBL( newLength );
	dGeomBoxSetLengths( geometry->id, lengths[X], lengths[Y], lengths[Z] );

	return rb_float_new(lengths[Z]);
}


/* --- ODE::Geometry::Plane ------------------------------ */

/*
 * ODE::Geometry::Plane::new( a, b, c, d, space=nil )
 * --
 * Create a new planar collision geometry with the specified parameters, and
 * insert it into the specified space, if given. The plane equation is
 *
 *     a*x + b*y + c*z = d
 *
 * The plane's normal vector is (a,b,c), and it must have length 1. Planes are
 * non-placeable geoms. This means that, unlike placeable geoms, planes do not
 * have an assigned position and rotation. This means that the parameters
 * (a,b,c,d) are always in global coordinates. In other words it is assumed that
 * the plane is always part of the static environment and not tied to any
 * movable object.
 */
static VALUE
ode_geometry_plane_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	VALUE			a, b, c, d, spaceObj;
	dSpaceID		space = 0;
	ode_GEOMETRY	*geometry = 0;

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	/* Fetch the ode_GEOMETRY pointer */
	geometry = get_geom(self);
	if ( !geometry ) rb_bug( "Superclass's initialize didn't return a valid Geometry." );
	debugMsg(( "Plane::initialize: Geometry is <%p>", geometry ));
	
	debugMsg(( "Plane::initialize: Scanning arguments." ));
	if ( rb_scan_args(argc, argv, "41", &a, &b, &c, &d, &spaceObj) == 5 ) {
		SetContainer( spaceObj, space, geometry );
	}

	debugMsg(( "Creating new Plane geometry." ));
	geometry->id = (dGeomID)dCreatePlane( space,
										  (dReal)NUM2DBL(a),
										  (dReal)NUM2DBL(b),
										  (dReal)NUM2DBL(c),
										  (dReal)NUM2DBL(d) );

	/* Set the ode_GEOMETRY pointer as the data pointer of the dGeomID */
	debugMsg(( "Setting geom data for plane <%p> to <%p>",
			   geometry->id, geometry ));
	dGeomSetData( geometry->id, geometry );

	return self;
}


/*
 * ODE::Geometry::Plane#params
 * --
 * Returns the plane's params as a 4-element array (a,b,c,d).
 */
static VALUE
ode_geometry_plane_params( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector4		result;

	dGeomPlaneGetParams( geometry->id, result );

	return rb_ary_new3( 4,
						rb_float_new(result[0]),
						rb_float_new(result[1]),
						rb_float_new(result[2]),
						rb_float_new(result[3]) );
}


/*
 * ODE::Geometry::Plane#params=( params )
 * --
 * Set the plane's radius to <tt>params</tt>, which can be any object which
 * returns an array with 3 numeric values when <tt>to_ary</tt> is called on it,
 * such a Math3d::Vector3 or an Array with 3 numeric values.
 */
static VALUE
ode_geometry_plane_params_eq( self, params )
	 VALUE self, params;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	VALUE			paramsArray;
	dVector4		result;

	debugMsg(( "Setting plane params: Got %d argument/s.", RARRAY(params)->len ));

	/* Normalize the dimensions into an array */
	if ( RARRAY(params)->len == 1 ) 
		paramsArray = ode_obj_to_ary4( *(RARRAY(params)->ptr), "params" );
	else
		paramsArray = ode_obj_to_ary4( params, "params" );

	debugMsg(( "Arguments normalized. Setting plane params." ));

	dGeomPlaneSetParams( geometry->id,
						 (dReal)NUM2DBL(*(RARRAY(paramsArray)->ptr    )),
						 (dReal)NUM2DBL(*(RARRAY(paramsArray)->ptr + 1)),
						 (dReal)NUM2DBL(*(RARRAY(paramsArray)->ptr + 2)),
						 (dReal)NUM2DBL(*(RARRAY(paramsArray)->ptr + 3)) );

	debugMsg(( "Params set." ));

	dGeomPlaneGetParams( geometry->id, result );
	return rb_ary_new3( 4,
						rb_float_new(result[0]),
						rb_float_new(result[1]),
						rb_float_new(result[2]),
						rb_float_new(result[3]) );
}


/* --- ODE::Geometry::Capsule ------------------------------ */

/*
 * ODE::Geometry::Capsule::new( radius, length, space=nil )
 * --
 * Create a new capsule collision geometry with the specified
 * parameters, and insert it into the specified space, if given.
 *
 * A capsule is like a normal cylinder except it has half-sphere caps at
 * its ends. This feature makes the internal collision detection code
 * particularly fast and accurate. The cylinder's length, not counting the caps,
 * is given by length. The cylinder is aligned along the geom's local Z
 * axis. The radius of the caps, and of the cylinder itself, is given by radius.
 */
static VALUE
ode_geometry_capsule_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	VALUE			radius, length, spaceObj;
	dSpaceID		space = 0;
	ode_GEOMETRY	*geometry = 0;

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	/* Fetch the ode_GEOMETRY pointer */
	geometry = get_geom(self);
	if ( !geometry ) rb_bug( "Superclass's initialize didn't return a valid Geometry." );
	debugMsg(( "Capsule::initialize: Geometry is <%p>", geometry ));
	
	debugMsg(( "Capsule::initialize: Scanning arguments." ));
	if ( rb_scan_args(argc, argv, "21", &radius, &length, &spaceObj) == 3 ) {
		SetContainer( spaceObj, space, geometry );
	}

	CheckPositiveNonZeroNumber( NUM2DBL(radius), "radius" );
	CheckPositiveNonZeroNumber( NUM2DBL(length), "length" );

	debugMsg(( "Creating new Capsule geometry." ));
	geometry->id = (dGeomID)dCreateCCylinder( space,
											  (dReal)NUM2DBL(radius),
											  (dReal)NUM2DBL(length) );

	/* Set the ode_GEOMETRY pointer as the data pointer of the dGeomID */
	dGeomSetData( geometry->id, geometry );

	return self;
}


/*
 * ODE::Geometry::Capsule#params
 * --
 * Returns the capsule's params as a 2-element array (radius,length).
 */
static VALUE
ode_geometry_capsule_params( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			radius, length;

	dGeomCCylinderGetParams( geometry->id, &radius, &length );

	return rb_ary_new3( 2,
						rb_float_new(radius),
						rb_float_new(length) );
}


/*
 * ODE::Geometry::Capsule#params=( radius, length )
 * --
 * Set the capsule's <tt>radius</tt> and <tt>length</tt> to the
 * specified values.
 */
static VALUE
ode_geometry_capsule_params_eq( self, args )
	 VALUE self, args;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			results[2];

	/* Unwrap inner array (when called like #params = 1, 2) */
	if ( RARRAY(args)->len == 1 ) args = *(RARRAY(args)->ptr);

	/* If the args VALUE isn't an array, or is an array with more or less than 2
	   elements, raise an ArgumentError. */
	if ( TYPE(args) != T_ARRAY )
		rb_raise( rb_eArgError, "wrong number of arguments (1 for 2)" );
	else if ( RARRAY(args)->len != 2 )
		rb_raise( rb_eArgError, "wrong number of arguments (%d for 2)",
				  RARRAY(args)->len );

	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(args)->ptr    )), "radius" );
	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(args)->ptr + 1)), "length" );

	dGeomCCylinderSetParams( geometry->id,
							 (dReal)NUM2DBL(*(RARRAY(args)->ptr    )),
							 (dReal)NUM2DBL(*(RARRAY(args)->ptr + 1)) );

	dGeomCCylinderGetParams( geometry->id, results, results+1 );
	return rb_ary_new3( 2,
						rb_float_new(results[0]),
						rb_float_new(results[1]) );
}


/*
 * ODE::Geometry::Capsule#radius
 * --
 * Get the cylinder's radius.
 */
static VALUE
ode_geometry_capsule_radius( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];

	dGeomCCylinderGetParams( geometry->id, params, params+1 );

	return rb_float_new( params[0] );
}


/*
 * ODE::Geometry::Capcyl#radius=( newValue )
 * --
 * Set the capsule's radius.
 */
static VALUE
ode_geometry_capsule_radius_eq( self, newRadius )
	 VALUE self, newRadius;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];

	CheckPositiveNonZeroNumber( NUM2DBL(newRadius), "radius" );
	dGeomCCylinderGetParams( geometry->id, params, params+1 );

	params[0] = (dReal)NUM2DBL( newRadius );

	dGeomCCylinderSetParams( geometry->id, params[0], params[1] );
	return rb_float_new( params[0] );
}


/*
 * ODE::Geometry::Capsule#length
 * --
 * Get the cylinder's length.
 */
static VALUE
ode_geometry_capsule_length( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];
	
	dGeomCCylinderGetParams( geometry->id, params, params+1 );

	return rb_float_new( params[1] );
}


/*
 * ODE::Geometry::Capcyl#length=( newValue )
 * --
 * Set the capsule's length.
 */
static VALUE
ode_geometry_capsule_length_eq( self, newLength )
	 VALUE self, newLength;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];

	CheckPositiveNonZeroNumber( NUM2DBL(newLength), "length" );
	dGeomCCylinderGetParams( geometry->id, params, params+1 );

	params[1] = (dReal)NUM2DBL( newLength );

	dGeomCCylinderSetParams( geometry->id, params[0], params[1] );
	return rb_float_new( params[1] );
}


/* --- ODE::Geometry::Cylinder ------------------------------ */

/*
 * ODE::Geometry::Cylinder::new( radius, length, space=nil )
 * --
 * Create a new regular cylinder collision geometry with the specified
 * parameters, and insert it into the specified space, if given. This requires
 * the 'dCylinder' extension to ODE. Note that this type of cylinder is aligned
 * along the geom's local *Y* axis, instead of the Z axis as Capsule is.
 */
static VALUE
ode_geometry_cylinder_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
#ifdef HAVE_ODE_DCYLINDER_H
	VALUE			radius, length, spaceObj;
	dSpaceID		space = 0;
	ode_GEOMETRY	*geometry = 0;

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	/* Fetch the ode_GEOMETRY pointer */
	geometry = get_geom(self);
	if ( !geometry ) rb_bug( "Superclass's initialize didn't return a valid Geometry." );
	debugMsg(( "Cylinder::initialize: Geometry is <%p>", geometry ));
	
	debugMsg(( "Cylinder::initialize: Scanning arguments." ));
	if ( rb_scan_args(argc, argv, "21", &radius, &length, &spaceObj) == 3 ) {
		SetContainer( spaceObj, space, geometry );
	}

	CheckPositiveNonZeroNumber( NUM2DBL(radius), "radius" );
	CheckPositiveNonZeroNumber( NUM2DBL(length), "length" );

	debugMsg(( "Creating new Cylinder geometry." ));
	geometry->id = (dGeomID)dCreateCylinder( space,
											 (dReal)NUM2DBL(radius),
											 (dReal)NUM2DBL(length) );

	/* Set the ode_GEOMETRY pointer as the data pointer of the dGeomID */
	dGeomSetData( geometry->id, geometry );

	return self;
#else
	rb_notimplement();
#endif
}


/* These only get implemented if they have the necessary underlying
   functionality. */
#if HAVE_ODE_DCYLINDER_H

/*
 * ODE::Geometry::Cylinder#params
 * --
 * Returns the cylinder's params as a 2-element array (radius,length).
 */
static VALUE
ode_geometry_cylinder_params( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			radius, length;

	dGeomCylinderGetParams( geometry->id, &radius, &length );

	return rb_ary_new3( 2,
						rb_float_new(radius),
						rb_float_new(length) );
}


/*
 * ODE::Geometry::Cylinder#params=( radius, length )
 * --
 * Set the cylinder's <tt>radius</tt> and <tt>length</tt> to the specified
 * values.
 */
static VALUE
ode_geometry_cylinder_params_eq( self, args )
	 VALUE self, args;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			results[2];

	/* Unwrap inner array (when called like #params = 1, 2) */
	if ( RARRAY(args)->len == 1 ) args = *(RARRAY(args)->ptr);

	/* If the args VALUE isn't an array, or is an array with more or less than 2
	   elements, raise an ArgumentError. */
	if ( TYPE(args) != T_ARRAY )
		rb_raise( rb_eArgError, "wrong number of arguments (1 for 2)" );
	else if ( RARRAY(args)->len != 2 )
		rb_raise( rb_eArgError, "wrong number of arguments (%d for 2)",
				  RARRAY(args)->len );

	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(args)->ptr    )), "radius" );
	CheckPositiveNonZeroNumber( NUM2DBL(*(RARRAY(args)->ptr + 1)), "length" );

	dGeomCylinderSetParams( geometry->id,
							(dReal)NUM2DBL(*(RARRAY(args)->ptr    )),
							(dReal)NUM2DBL(*(RARRAY(args)->ptr + 1)) );

	dGeomCylinderGetParams( geometry->id, results, results+1 );
	return rb_ary_new3( 2,
						rb_float_new(results[0]),
						rb_float_new(results[1]) );
}


/*
 * ODE::Geometry::Cylinder#radius
 * --
 * Get the cylinder's radius.
 */
static VALUE
ode_geometry_cylinder_radius( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];
	
	dGeomCylinderGetParams( geometry->id, params, params+1 );

	return rb_float_new( params[0] );
}


/*
 * ODE::Geometry::Cylinder#radius=( newValue )
 * --
 * Set the cylinder's radius.
 */
static VALUE
ode_geometry_cylinder_radius_eq( self, newRadius )
	 VALUE self, newRadius;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];

	CheckPositiveNonZeroNumber( NUM2DBL(newRadius), "radius" );
	dGeomCylinderGetParams( geometry->id, params, params+1 );

	params[0] = (dReal)NUM2DBL( newRadius );

	dGeomCylinderSetParams( geometry->id, params[0], params[1] );
	return rb_float_new( params[0] );
}


/*
 * ODE::Geometry::Cylinder#length
 * --
 * Get the cylinder's length.
 */
static VALUE
ode_geometry_cylinder_length( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];
	
	dGeomCylinderGetParams( geometry->id, params, params+1 );

	return rb_float_new( params[1] );
}


/*
 * ODE::Geometry::Cylinder#length=( newValue )
 * --
 * Set the cylinder's length.
 */
static VALUE
ode_geometry_cylinder_length_eq( self, newLength )
	 VALUE self, newLength;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dReal			params[2];

	CheckPositiveNonZeroNumber( NUM2DBL(newLength), "length" );
	dGeomCylinderGetParams( geometry->id, params, params+1 );

	params[1] = (dReal)NUM2DBL( newLength );

	dGeomCylinderSetParams( geometry->id, params[0], params[1] );
	return rb_float_new( params[1] );
}

#endif /* HAVE_ODE_DCYLINDER_H */



/* --- ODE::Geometry::Ray ------------------------------ */

static VALUE ode_geometry_ray_start_point_eq( VALUE, VALUE );
static VALUE ode_geometry_ray_direction_point_eq( VALUE, VALUE );


/*
 * ODE::Geometry::Ray#initialize( length[, space[, start, direction]] )
 * --
 * Create a new Ray object of the specified +length+ and insert it into the
 * specified +space+, if non-nil. If the optional +start+ and +direction+
 * arguments are given, use them to set the ray's start position and direction.
 */
static VALUE
ode_geometry_ray_init( argc, argv, self )
	 int argc;
	 VALUE *argv, self;
{
	VALUE			length, start, direction, spaceObj;
	dSpaceID		space = 0;
	ode_GEOMETRY	*geometry = 0;

	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	/* Fetch the ode_GEOMETRY pointer */
	geometry = get_geom(self);
	if ( !geometry ) rb_bug( "Superclass's initialize didn't return a valid Geometry." );
	debugMsg(( "Ray::initialize: Geometry is <%p>", geometry ));
	
	debugMsg(( "Ray::initialize: Scanning arguments." ));
	rb_scan_args( argc, argv, "13", &length, &spaceObj, &start, &direction );

	/* If a container space was specified get the dSpaceID for it */
	if ( RTEST(spaceObj) ) {
		SetContainer( spaceObj, space, geometry );
	}

	/* Make sure the length is a positive non-zero number */
	CheckPositiveNonZeroNumber( NUM2DBL(length), "length" );

	/* Create the ray */
	debugMsg(( "Creating new Ray geometry." ));
	geometry->id = (dGeomID)dCreateRay( space,
										(dReal)NUM2DBL(length) );

	/* Set the ode_GEOMETRY pointer as the data pointer of the dGeomID */
	dGeomSetData( geometry->id, geometry );

	/* Set start point and/or direction if they are non-nil */
	if (RTEST( start )) ode_geometry_ray_start_point_eq( self, start );
	if (RTEST( direction )) ode_geometry_ray_direction_point_eq( self, direction );

	return self;
}


/*
 * ODE::Geometry::Ray#length
 * --
 * 
 */
static VALUE
ode_geometry_ray_length( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	return rb_float_new( dGeomRayGetLength(geometry->id) );
}


/*
 * ODE::Geometry::Ray#length=
 * --
 * 
 */
static VALUE
ode_geometry_ray_length_eq( self, length )
	 VALUE self, length;
{
	ode_GEOMETRY	*geometry = get_geom( self );

	CheckPositiveNonZeroNumber( NUM2DBL(length), "length" );
	dGeomRaySetLength( geometry->id, (dReal)NUM2DBL(length) );

	return ode_geometry_ray_length( self );
}


/*
 * ODE::Geometry::Ray#startPoint
 * --
 * 
 */
static VALUE
ode_geometry_ray_start_point( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		start, dir;
	VALUE			pos;

	dGeomRayGet( geometry->id, start, dir );
	Vec3ToOdePosition( start, pos );
	
	return pos;
}


/*
 * ODE::Geometry::Ray#startPoint=
 * --
 * 
 */
static VALUE
ode_geometry_ray_start_point_eq( self, point )
	 VALUE self, point;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	VALUE			pointAry;
	dVector3		start, dir;

	pointAry = ode_obj_to_ary3( point, "start point" );

	dGeomRayGet( geometry->id, start, dir );
	dGeomRaySet( geometry->id,
				 (dReal)NUM2DBL( *(RARRAY(pointAry)->ptr) ),
				 (dReal)NUM2DBL( *(RARRAY(pointAry)->ptr + 1) ),
				 (dReal)NUM2DBL( *(RARRAY(pointAry)->ptr + 2) ),
				 dir[0],
				 dir[1],
				 dir[2] );

	/* It no longer matters (as of the latest Ruby-1.8) what gets returned here,
	   so just return nil */
	return Qnil;
}


/*
 * ODE::Geometry::Ray#directionPoint
 * --
 * 
 */
static VALUE
ode_geometry_ray_direction_point( self )
	 VALUE self;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	dVector3		start, dir;
	VALUE			pos;

	dGeomRayGet( geometry->id, start, dir );
	Vec3ToOdePosition( dir, pos );
	
	return pos;
}


/*
 * ODE::Geometry::Ray#directionPoint=
 * --
 * 
 */
static VALUE
ode_geometry_ray_direction_point_eq( self, point )
	 VALUE self, point;
{
	ode_GEOMETRY	*geometry = get_geom( self );
	VALUE			pointAry;
	dVector3		start, dir;

	pointAry = ode_obj_to_ary3( point, "direction point" );

	dGeomRayGet( geometry->id, start, dir );
	dGeomRaySet( geometry->id,
				 start[0],
				 start[1],
				 start[2],
				 (dReal)NUM2DBL( *(RARRAY(pointAry)->ptr) ),
				 (dReal)NUM2DBL( *(RARRAY(pointAry)->ptr + 1) ),
				 (dReal)NUM2DBL( *(RARRAY(pointAry)->ptr + 2) ) );

	/* It no longer matters (as of the latest Ruby-1.8) what gets returned here,
	   so just return nil */
	return Qnil;
}




/* --------------------------------------------------
 * Initialize
 * -------------------------------------------------- */

void ode_init_geometry() {
	/* Kluge to make Rdoc see the class in this file */
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeGeometry		= rb_define_class_under( ode_mOde, "Geometry", rb_cObject );
	ode_cOdeGeometryPlane	= rb_define_class_under( ode_cOdeGeometry, "Plane", ode_cOdeGeometry );

	ode_cOdePlaceable		= rb_define_class_under( ode_cOdeGeometry, "Placeable", ode_cOdeGeometry );
	ode_cOdeGeometrySphere	= rb_define_class_under( ode_cOdeGeometry, "Sphere", ode_cOdePlaceable );
	ode_cOdeGeometryBox		= rb_define_class_under( ode_cOdeGeometry, "Box", ode_cOdePlaceable );
	ode_cOdeGeometryCapCyl	= rb_define_class_under( ode_cOdeGeometry, "Capsule", ode_cOdePlaceable );
	ode_cOdeGeometryRay		= rb_define_class_under( ode_cOdeGeometry, "Ray", ode_cOdeRay );
	ode_cOdeGeometryCylinder = rb_define_class_under( ode_cOdeGeometry, "Cylinder", ode_cOdePlaceable );

	ode_cOdeGeometryTransform = rb_define_class_under( ode_cOdeGeometry, "Transform", ode_cOdeGeometry );
	ode_cOdeGeometryTransformGroup = rb_define_class_under( ode_cOdeGeometry, "TransformGroup", ode_cOdeGeometry );

	ode_cOdeSpace			= rb_define_class_under( ode_mOde, "Space", ode_cOdeGeometry );
	ode_cOdeHashSpace		= rb_define_class_under( ode_mOde, "HashSpace", ode_cOdeSpace );
#endif

	/* Constructor */
	rb_define_alloc_func( ode_cOdeGeometry, ode_geometry_s_alloc );

	/* Initializer */
	rb_define_method( ode_cOdeGeometry, "initialize", ode_geometry_init, -1 );
	rb_enable_super ( ode_cOdeGeometry, "initialize" );

	/* Instance methods */

	/* ODE::Geometry */
	rb_define_method( ode_cOdeGeometry, "container", ode_geometry_container, 0 );

	rb_define_method( ode_cOdeGeometry, "aabb", ode_geometry_aabb, 0 );
	rb_define_alias ( ode_cOdeGeometry, "boundingBox", "aabb" );
	rb_define_alias ( ode_cOdeGeometry, "bounding_box", "aabb" );
	rb_define_method( ode_cOdeGeometry, "isSpace?", ode_geometry_space_p, 0 );
	rb_define_alias ( ode_cOdeGeometry, "is_space?", "isSpace?" );

	rb_define_method( ode_cOdeGeometry, "categoryMask", ode_geometry_category_mask, 0 );
	rb_define_alias ( ode_cOdeGeometry, "category_mask", "categoryMask" );
	rb_define_alias ( ode_cOdeGeometry, "categoryBits", "categoryMask" );
	rb_define_alias ( ode_cOdeGeometry, "category_bits", "categoryMask" );
	rb_define_method( ode_cOdeGeometry, "categoryMask=", ode_geometry_category_mask_eq, 1 );
	rb_define_alias ( ode_cOdeGeometry, "category_mask=", "categoryMask=" );
	rb_define_alias ( ode_cOdeGeometry, "categoryBits=", "categoryMask=" );
	rb_define_alias ( ode_cOdeGeometry, "category_bits=", "categoryMask=" );

	rb_define_method( ode_cOdeGeometry, "collideMask", ode_geometry_collide_mask, 0 );
	rb_define_alias ( ode_cOdeGeometry, "collide_mask", "collideMask" );
	rb_define_alias ( ode_cOdeGeometry, "collideBits", "collideMask" );
	rb_define_alias ( ode_cOdeGeometry, "collide_bits", "collideMask" );
	rb_define_method( ode_cOdeGeometry, "collideMask=", ode_geometry_collide_mask_eq, 1 );
	rb_define_alias ( ode_cOdeGeometry, "collide_mask=", "collideMask=" );
	rb_define_alias ( ode_cOdeGeometry, "collideBits=", "collideMask=" );
	rb_define_alias ( ode_cOdeGeometry, "collide_bits=", "collideMask=" );
	
	rb_define_method( ode_cOdeGeometry, "enable", ode_geometry_enable, 0 );
	rb_define_method( ode_cOdeGeometry, "disable", ode_geometry_disable, 0 );
	rb_define_method( ode_cOdeGeometry, "enabled?", ode_geometry_enabled_p, 0 );

	rb_define_method( ode_cOdeGeometry, "surface", ode_geometry_surface, 0 );
	rb_define_method( ode_cOdeGeometry, "surface=", ode_geometry_surface_eq, 1 );
	rb_define_method( ode_cOdeGeometry, "body", ode_geometry_body, 0 );
	rb_define_method( ode_cOdeGeometry, "body=", ode_geometry_body_eq, 0 );

	/* Collision */
	rb_define_method( ode_cOdeGeometry, "collideWith", ode_geometry_collide, -1 );
	rb_define_method( ode_cOdeGeometry, "intersectWith", ode_geometry_isect, -1 );

	/* ODE::Geometry::Placeable */
	rb_define_method( ode_cOdePlaceable, "body", ode_geometry_placeable_body, 0 );
	rb_define_method( ode_cOdePlaceable, "body=", ode_geometry_placeable_body_eq, 1 );
	rb_define_method( ode_cOdePlaceable, "position", ode_geometry_placeable_position, 0 );
	rb_define_method( ode_cOdePlaceable, "position=", ode_geometry_placeable_position_eq, -2 );
	rb_define_method( ode_cOdePlaceable, "rotation", ode_geometry_placeable_rotation, 0 );
	rb_define_method( ode_cOdePlaceable, "rotation=", ode_geometry_placeable_rotation_eq, -2 );

	/* ODE::Geometry::Sphere */
	rb_define_method( ode_cOdeGeometrySphere, "initialize", ode_geometry_sphere_init, -1 );
	rb_enable_super ( ode_cOdeGeometrySphere, "initialize" );

	rb_define_method( ode_cOdeGeometrySphere, "radius", ode_geometry_sphere_radius, 0 );
	rb_define_method( ode_cOdeGeometrySphere, "radius=", ode_geometry_sphere_radius_eq, 1 );

	/* ODE::Geometry::Box */
	rb_define_method( ode_cOdeGeometryBox, "initialize", ode_geometry_box_init, -1 );
	rb_enable_super ( ode_cOdeGeometryBox, "initialize" );

	rb_define_method( ode_cOdeGeometryBox, "lengths", ode_geometry_box_lengths, 0 );
	rb_define_method( ode_cOdeGeometryBox, "lengths=", ode_geometry_box_lengths_eq, -2 );
	rb_define_method( ode_cOdeGeometryBox, "lx", ode_geometry_box_length_x, 0 );
	rb_define_method( ode_cOdeGeometryBox, "lx=", ode_geometry_box_length_x_eq, 1 );
	rb_define_method( ode_cOdeGeometryBox, "ly", ode_geometry_box_length_y, 0 );
	rb_define_method( ode_cOdeGeometryBox, "ly=", ode_geometry_box_length_y_eq, 1 );
	rb_define_method( ode_cOdeGeometryBox, "lz", ode_geometry_box_length_z, 0 );
	rb_define_method( ode_cOdeGeometryBox, "lz=", ode_geometry_box_length_z_eq, 1 );

	/* ODE::Geometry::Plane */
	rb_define_method( ode_cOdeGeometryPlane, "initialize", ode_geometry_plane_init, -1 );
	rb_enable_super ( ode_cOdeGeometryPlane, "initialize" );

	rb_define_method( ode_cOdeGeometryPlane, "params", ode_geometry_plane_params, 0 );
	rb_define_method( ode_cOdeGeometryPlane, "params=", ode_geometry_plane_params_eq, -2 );
	/* :TODO: Other convenience accessors? normalVector(=)? */

	/* ODE::Geometry::Capsule */
	rb_define_method( ode_cOdeGeometryCapCyl, "initialize", ode_geometry_capsule_init, -1 );
	rb_enable_super ( ode_cOdeGeometryCapCyl, "initialize" );

	rb_define_method( ode_cOdeGeometryCapCyl, "params", ode_geometry_capsule_params, 0 );
	rb_define_method( ode_cOdeGeometryCapCyl, "params=", ode_geometry_capsule_params_eq, -2 );
	rb_define_method( ode_cOdeGeometryCapCyl, "radius", ode_geometry_capsule_radius, 0 );
	rb_define_method( ode_cOdeGeometryCapCyl, "radius=", ode_geometry_capsule_radius_eq, 1 );
	rb_define_method( ode_cOdeGeometryCapCyl, "length", ode_geometry_capsule_length, 0 );
	rb_define_method( ode_cOdeGeometryCapCyl, "length=", ode_geometry_capsule_length_eq, 1 );

	/* ODE::Geometry::Cylinder */
	rb_define_method( ode_cOdeGeometryCylinder, "initialize", ode_geometry_cylinder_init, -1 );
	rb_enable_super ( ode_cOdeGeometryCylinder, "initialize" );

#if HAVE_ODE_DCYLINDER_H
	rb_define_method( ode_cOdeGeometryCylinder, "params", ode_geometry_cylinder_params, 0 );
	rb_define_method( ode_cOdeGeometryCylinder, "params=", ode_geometry_cylinder_params_eq, -2 );
	rb_define_method( ode_cOdeGeometryCylinder, "radius", ode_geometry_cylinder_radius, 0 );
	rb_define_method( ode_cOdeGeometryCylinder, "radius=", ode_geometry_cylinder_radius_eq, 1 );
	rb_define_method( ode_cOdeGeometryCylinder, "length", ode_geometry_cylinder_length, 0 );
	rb_define_method( ode_cOdeGeometryCylinder, "length=", ode_geometry_cylinder_length_eq, 1 );
#endif

	/* ODE::Geometry::Ray */
	rb_define_method( ode_cOdeGeometryRay, "initialize", ode_geometry_ray_init, -1 );
	rb_enable_super ( ode_cOdeGeometryRay, "initialize" );

	rb_define_method( ode_cOdeGeometryRay, "length", ode_geometry_ray_length, 0 );
	rb_define_method( ode_cOdeGeometryRay, "length=", ode_geometry_ray_length_eq, 1 );
	rb_define_method( ode_cOdeGeometryRay, "startPoint", ode_geometry_ray_start_point, 0 );
	rb_define_alias ( ode_cOdeGeometryRay, "start_point", "startPoint" );
	rb_define_method( ode_cOdeGeometryRay, "startPoint=", ode_geometry_ray_start_point_eq, -2 );
	rb_define_alias ( ode_cOdeGeometryRay, "start_point=", "startPoint=" );
	rb_define_method( ode_cOdeGeometryRay, "directionPoint", ode_geometry_ray_direction_point, 0 );
	rb_define_alias ( ode_cOdeGeometryRay, "direction_point", "direction_point" );
	rb_define_method( ode_cOdeGeometryRay, "directionPoint=", ode_geometry_ray_direction_point_eq, -2 );
	rb_define_alias ( ode_cOdeGeometryRay, "direction_point=", "directionPoint=" );
	
	
}

