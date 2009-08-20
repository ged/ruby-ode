/*
 *		contact.c - ODE Ruby Binding - ODE::Contact class
 *		$Id$
 *		Time-stamp: <27-Jul-2005 20:12:27 ged>
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


/* This class wraps a struct that looks like:
 * --
 *  typedef struct {
 *  	dContact	*contact;
 *  	VALUE		object, surface, geometry1, geometry2;
 *  } ode_CONTACT;
 *
 * which contains:
 * --
 *  struct dContact {
 *    dSurfaceParameters	surface;
 *    dContactGeom			geom;
 *    dVector3				fdir1;
 *  };
 *
 * which itself in turn contains:
 * --
 *  struct dContactGeom {
 *    dVector3	pos;       // contact position
 *    dVector3	normal;    // normal vector
 *    dReal		depth;     // penetration depth
 *    dGeomID	g1,g2;     // the colliding geoms
 *  };
 */

/* --------------------------------------------------
 * Macros
 * -------------------------------------------------- */

#define IsContact( obj ) rb_obj_is_kind_of( (obj), ode_cOdeContact )


/* --------------------------------------------------
 * Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static ode_CONTACT *
ode_contact_alloc()
{
	ode_CONTACT *ptr = ALLOC( ode_CONTACT );
	ptr->contact	= NULL;
	ptr->object		= Qnil;
	ptr->surface	= Qnil;

	return ptr;
}


/*
 * GC mark function
 */
void
ode_contact_gc_mark( ptr )
	 ode_CONTACT	*ptr;
{
	debugMsg(( "Marking an ODE::Contact." ));

	if ( ptr ) {
		debugMsg(( "Marking Contact <%p>.", ptr ));

		if ( ptr->surface )   rb_gc_mark( ptr->surface );
	}

	else {
		debugMsg(( "Not marking NULL pointer." ));
	}
}


/* 
 * GC free function
 */
void
ode_contact_gc_free( ptr )
	 ode_CONTACT	*ptr;
{
	debugMsg(( "Freeing an ODE::Contact." ));

	if ( ptr ) {
		debugMsg(( "Freeing Contact <%p>.", ptr ));

		xfree( ptr->contact );
		ptr->contact = NULL;
		ptr->surface = Qnil;

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
static ode_CONTACT *
check_contact( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Contact object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsContact(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Contact)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_CONTACT *
get_contact( self )
	 VALUE self;
{
	ode_CONTACT *ptr = check_contact( self );

	debugMsg(( "Fetching an ode_CONTACT (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized contact" );

	return ptr;
}


/*
 * Publicly-usable contact-fetcher.
 */
ode_CONTACT *
ode_get_contact( self )
	 VALUE self;
{
	return get_contact(self);
}


/*
 * Set the contact's innermost struct to a copy of the specified
 * dContactGeom. Used in the collision functions in geometry.c.
 */
void
ode_contact_set_cgeom( self, cgeom )
	 VALUE			self;
	 dContactGeom	*cgeom;
{
	ode_CONTACT *ptr = get_contact( self );

	if ( !cgeom ) rb_bug("NULL cgeom pointer.");
	memcpy( &ptr->contact->geom, cgeom, sizeof(dContactGeom) );
}


/*
 * Check the ode_CONTACT's contact geom for setted-ness, raising an error if
 * it's not been set.
 */
static void
check_contact_geom( ptr )
	 ode_CONTACT	*ptr;
{
	debugMsg(( "Checking an ODE::Contact <%p> for completeness.", ptr ));

	if ( !ptr->contact->geom.g1 || !ptr->contact->geom.g2 ) {
		debugMsg(( "Contact is not fully initialized.", ptr ));

		rb_raise( rb_eRuntimeError, "Cannot use a partially-initialized ODE::Contact." );
	}

	else {
		debugMsg(( "Contact is fully initialized (<%p>, <%p>).",
				   ptr->contact->geom.g1,
				   ptr->contact->geom.g2 ));
	}
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * allocate()
 * --
 * Create a contact (collision) object.
 */
static VALUE
ode_contact_s_alloc( klass )
	 VALUE klass;
{
	debugMsg(( "Wrapping an uninitialized ODE::Contact pointer." ));
	return Data_Wrap_Struct( klass, ode_contact_gc_mark, ode_contact_gc_free, 0 );
}




/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */

/*
 * initialize( pos=nil, normal=nil, depth=nil, geom1=nil, geom2=nil )
 * --
 * Instantiate and return a new ODE::Contact object, optionally filling in the
 * specified parts of the contact geometry.
 */
static VALUE
ode_contact_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	int		argCount;
	VALUE	pos, normal, depth, g1, g2;

	debugMsg(( "ODE::Contact init." ));

	if ( !check_contact(self) ) {
		ode_CONTACT *ptr;

		DATA_PTR(self) = ptr = ode_contact_alloc();
		ptr->object = self;

		/* Create a new surface object and add that to the new object's surface
		   member. */
		ptr->surface = rb_class_new_instance( 0, 0, ode_cOdeSurface );

		/* Allocate and fill the mid-level struct */
		ptr->contact = ALLOC( dContact );
		ptr->contact->surface = *(ode_get_surface( ptr->surface ));
		ptr->contact->fdir1[0] = 0.f;
		ptr->contact->fdir1[1] = 0.f;
		ptr->contact->fdir1[2] = 0.f;

		/* Set the pointer to the bottom struct to NULL, as it should be filled
		   in by a geometry later. */
		MEMZERO( &ptr->contact->geom, dContactGeom, 1 );

		return self;
	}

	/* If there are initializer arguments, call the accessors to set the ones provided. */
	if (( argCount = rb_scan_args(argc, argv, "05", &pos, &normal, &depth, &g1, &g2) )) {
		if ( pos )		rb_funcall( self, rb_intern("pos="), 1, &pos );
		if ( normal )	rb_funcall( self, rb_intern("normal="), 1, &normal );
		if ( depth )	rb_funcall( self, rb_intern("depth="), 1, &depth );
		if ( g1 )		rb_funcall( self, rb_intern("geom1="), 1, &g1 );
		if ( g2 )		rb_funcall( self, rb_intern("geom2="), 1, &g2 );
	}


	debugMsg(( "Calling super()" ));
	rb_call_super( 0, 0 );
	debugMsg(( "Back from super()" ));

	return self;
}


/*
 * surface
 * --
 * Fetch the object that describes the properties of the colliding surfaces (an
 * ODE::Surface object).
 */
static VALUE
ode_contact_surface( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	return ptr->surface;
}


/*
 * surface=( newSurface )
 * --
 * Set the surface parameters of the collision to the specified
 * <tt>newSurface</tt> (an ODE::Surface object).
 */
static VALUE
ode_contact_surface_eq( self, surface )
	 VALUE	self, surface;
{
	ode_CONTACT			*ptr = get_contact( self );
	dSurfaceParameters	*surfacePtr = ode_get_surface( surface );

	ptr->surface = surface;
	ptr->contact->surface = *surfacePtr;

	return surface;
}


/*
 * geom1
 * --
 * Returns one of the pair of geometry objects (a deriviative of
 * ODE::Geometry)involved in the collision.
 */
static VALUE
ode_contact_geom1( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	ode_GEOMETRY *geom;

	geom = (ode_GEOMETRY *)dGeomGetData( ptr->contact->geom.g1 );
	return geom->object;
}


/*
 * geom1=( geometry )
 * --
 * Sets one of the pair of geometry objects involved in the collision to the
 * specified <tt>geometry</tt> (a deriviative of ODE::Geometry).
 */
static VALUE
ode_contact_geom1_eq( self, geometry )
	 VALUE	self, geometry;
{
	ode_CONTACT		*ptr = get_contact( self );
	ode_GEOMETRY	*geom = ode_get_geom( geometry );

	ptr->contact->geom.g1 = geom->id;
	return geom->object;
}


/*
 * geom2
 * --
 * Returns one of the pair of geometry objects (a deriviative of
 * ODE::Geometry)involved in the collision.
 */
static VALUE
ode_contact_geom2( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	ode_GEOMETRY *geom;

	geom = (ode_GEOMETRY *)dGeomGetData( ptr->contact->geom.g2 );
	return geom->object;
}


/*
 * geom2=( geometry )
 * --
 * Sets one of the pair of geometry objects involved in the collision to the
 * specified <tt>geometry</tt> (a deriviative of ODE::Geometry).
 */
static VALUE
ode_contact_geom2_eq( self, geometry )
	 VALUE	self, geometry;
{
	ode_CONTACT		*ptr = get_contact( self );
	ode_GEOMETRY	*geom = ode_get_geom( geometry );

	ptr->contact->geom.g2 = geom->id;
	return geom->object;
}


/*
 * geometries
 * --
 * Returns both geometry objects (ODE::Geometry deriviatives) involved in the
 * collision as an Array.
 */
static VALUE
ode_contact_geom_ary( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	ode_GEOMETRY *geom1, *geom2;

	geom1 = (ode_GEOMETRY *)dGeomGetData( ptr->contact->geom.g1 );
	geom2 = (ode_GEOMETRY *)dGeomGetData( ptr->contact->geom.g2 );

	return rb_ary_new3( 2, geom1->object, geom2->object );
}


/*
 * geometries=( g1, g2 )
 * --
 * Sets both geometry objects (ODE::Geometry deriviatives) involved in the
 * collision.
 */
static VALUE
ode_contact_geom_ary_eq( self, args )
	 VALUE	self, args;
{
	ode_CONTACT		*ptr = get_contact( self );
	VALUE			geom1, geom2;
	ode_GEOMETRY	*g1, *g2;

	/* Check to be sure 2 args were given */
	if ( RARRAY(args)->len != 2 )
		rb_raise( rb_eArgError, "wrong number of arguments (%d for 2)",
				  RARRAY(args)->len );

	/* Fetch what is needed from the arguments */
	geom1 = *(RARRAY(args)->ptr);
	g1 = ode_get_geom( geom1 );
	geom2 = *(RARRAY(args)->ptr+1);
	g2 = ode_get_geom( geom2 );

	/* Set the geometries in the inner structs */
	ptr->contact->geom.g1 = g1->id;
	ptr->contact->geom.g2 = g2->id;

	return rb_ary_new3( 2, geom1, geom2 );
}


/*
 * pos
 * --
 * Returns the position of the contact in global coordinates as an ODE::Position
 * object.
 */
static VALUE
ode_contact_pos( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	VALUE		pos;

	check_contact_geom( ptr );
	Vec3ToOdePosition( ptr->contact->geom.pos, pos );

	return pos;
}


/*
 * pos=( position )
 * --
 * Set the position of the contact in global coordinates to the specified
 * <tt>position</tt>, which can be any object which returns an Array of three
 * numbers when #to_ary is called on it (eg., Array, ODE::Vector, ODE::Position,
 * etc.).
 */
static VALUE
ode_contact_pos_eq( self, newPos )
	 VALUE	self, newPos;
{
	ode_CONTACT *ptr = get_contact( self );
	VALUE posAry = ode_obj_to_ary3( newPos, "position" );

	SetVec3FromArray( ptr->contact->geom.pos, posAry );

	return posAry;
}


/*
 * normal
 * --
 * Get the normal vector (as an ODE::Vector), which is a unit length vector that
 * is, generally speaking, perpendicular to the contact surface.
 */
static VALUE
ode_contact_normal( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	VALUE		normal;

	check_contact_geom( ptr );
	Vec3ToOdeVector( ptr->contact->geom.normal, normal );

	return normal;
}


/*
 * normal=( normal )
 * --
 * Set the contact's normal vector to <tt>normal</tt> which can be any object
 * which returns three numbers when #to_ary is called on it (eg., an
 * ODE::Vector, an Array of three numbers, etc.).
 */
static VALUE
ode_contact_normal_eq( self, normal )
	 VALUE	self, normal;
{
	ode_CONTACT *ptr = get_contact( self );
	VALUE		normalAry = ode_obj_to_ary3( normal, "contact normal" );
	
	SetVec3FromArray( ptr->contact->geom.normal, normalAry );
	
	return normalAry;
}


/*
 * depth
 * --
 * Return the depth to which the two bodies inter-penetrate each other. If the
 * depth is zero then the two bodies have a grazing contact, i.e. they "only
 * just" touch. However, this is rare - the simulation is not perfectly accurate
 * and will often step the bodies too far so that the depth is nonzero.
 */
static VALUE
ode_contact_depth( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	return rb_float_new( ptr->contact->geom.depth );
}


/*
 * depth=( depth )
 * --
 * Set the penetration depth of the contact.
 */
static VALUE
ode_contact_depth_eq( self, depth )
	 VALUE self, depth;
{
	ode_CONTACT *ptr = get_contact( self );
	ptr->contact->geom.depth = (dReal)NUM2DBL( depth );
	return rb_float_new( ptr->contact->geom.depth );
}


/*
 * fdir1
 * --
 * Returns the "first friction direction" vector that defines a direction along
 * which frictional force is applied if the contact's
 * surface#useFrictionDirection? flag is true. If useFrictionDirection? is
 * false, this setting is unused, though it can still be set.
 */
static VALUE
ode_contact_fdir1( self )
	 VALUE	self;
{
	ode_CONTACT *ptr = get_contact( self );
	VALUE		fdir1;

	check_contact_geom( ptr );
	Vec3ToOdeVector( ptr->contact->fdir1, fdir1 );

	return fdir1;
}


/*
 * fdir1=( vector )
 * --
 * Sets the "first friction direction" vector that defines a direction along
 * which frictional force is applied if the contact's
 * surface#useFrictionDirection? flag is true. If useFrictionDirection? is
 * false, this setting is unused, though it can still be set.
 */
static VALUE
ode_contact_fdir1_eq( self, direction )
	 VALUE	self, direction;
{
	ode_CONTACT *ptr = get_contact( self );
	VALUE		fdirAry = ode_obj_to_ary3( direction, "direction" );
	
	SetVec3FromArray( ptr->contact->fdir1, fdirAry );
	
	return fdirAry;
}



/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_contact()
{
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeContact = rb_define_class_under( ode_mOde, "Contact", rb_cObject );
#endif

	/* Surface mode flag constants */
	rb_define_const( ode_cOdeContact, "Mu2",					INT2FIX(0x001) );
	rb_define_const( ode_cOdeContact, "FDir1",					INT2FIX(0x002) );
	rb_define_const( ode_cOdeContact, "UseFrictionDirection",	INT2FIX(0x002) );
	rb_define_const( ode_cOdeContact, "Bounce",					INT2FIX(0x004) );
	rb_define_const( ode_cOdeContact, "SoftERP",				INT2FIX(0x008) );
	rb_define_const( ode_cOdeContact, "SoftCFM",				INT2FIX(0x010) );
	rb_define_const( ode_cOdeContact, "Motion1",				INT2FIX(0x020) );
	rb_define_const( ode_cOdeContact, "Motion2",				INT2FIX(0x040) );
	rb_define_const( ode_cOdeContact, "Slip1",					INT2FIX(0x080) );
	rb_define_const( ode_cOdeContact, "Slip2",					INT2FIX(0x100) );

	rb_define_const( ode_cOdeContact, "Approx0",				INT2FIX(0x0000) );
	rb_define_const( ode_cOdeContact, "PyramidFrictionNeither", INT2FIX(0x0000) );
	rb_define_const( ode_cOdeContact, "Approx1_1",				INT2FIX(0x1000) );
	rb_define_const( ode_cOdeContact, "PyramidFriction1",		INT2FIX(0x1000) );
	rb_define_const( ode_cOdeContact, "Approx1_2",				INT2FIX(0x2000) );
	rb_define_const( ode_cOdeContact, "PyramidFriction2",		INT2FIX(0x2000) );
	rb_define_const( ode_cOdeContact, "Approx1",				INT2FIX(0x3000) );
	rb_define_const( ode_cOdeContact, "PyramidFrictionBoth",	INT2FIX(0x3000) );

	/* Allocator */
	rb_define_alloc_func( ode_cOdeContact, ode_contact_s_alloc );

	/* Initializer */
	rb_define_method( ode_cOdeContact, "initialize", ode_contact_init, -1 );
	rb_enable_super ( ode_cOdeContact, "initialize" );

	/* Accessors */
	rb_define_method( ode_cOdeContact, "surface", ode_contact_surface, 0 );
	rb_define_method( ode_cOdeContact, "surface=", ode_contact_surface_eq, 1 );

	rb_define_method( ode_cOdeContact, "geom1", ode_contact_geom1, 0 );
	rb_define_method( ode_cOdeContact, "geom1=", ode_contact_geom1_eq, 1 );
	rb_define_method( ode_cOdeContact, "geom2", ode_contact_geom2, 0 );
	rb_define_method( ode_cOdeContact, "geom2=", ode_contact_geom2_eq, 1 );
	rb_define_method( ode_cOdeContact, "geometries", ode_contact_geom_ary, 0 );
	rb_define_method( ode_cOdeContact, "geometries=", ode_contact_geom_ary_eq, -2 );

	rb_define_method( ode_cOdeContact, "pos", ode_contact_pos, 0 );
	rb_define_alias ( ode_cOdeContact, "position", "pos" );
	rb_define_method( ode_cOdeContact, "pos=", ode_contact_pos_eq, -2 );
	rb_define_alias ( ode_cOdeContact, "position=", "pos=" );
	rb_define_method( ode_cOdeContact, "normal", ode_contact_normal, 0 );
	rb_define_alias ( ode_cOdeContact, "normalVector", "normal" );
	rb_define_alias ( ode_cOdeContact, "normal_vector", "normal" );
	rb_define_method( ode_cOdeContact, "normal=", ode_contact_normal_eq, -2 );
	rb_define_alias ( ode_cOdeContact, "normalVector=", "normal=" );
	rb_define_alias ( ode_cOdeContact, "normal_vector=", "normal=" );

	rb_define_method( ode_cOdeContact, "depth", ode_contact_depth, 0 );
	rb_define_alias ( ode_cOdeContact, "penetrationDepth", "depth" );
	rb_define_alias ( ode_cOdeContact, "penetration_depth", "depth" );
	rb_define_method( ode_cOdeContact, "depth=", ode_contact_depth_eq, 1 );
	rb_define_alias ( ode_cOdeContact, "penetrationDepth=", "depth=" );
	rb_define_alias ( ode_cOdeContact, "penetration_depth=", "depth=" );

	rb_define_method( ode_cOdeContact, "fdir1", ode_contact_fdir1, 0 );
	rb_define_alias ( ode_cOdeContact, "frictionDirection", "fdir1" );
	rb_define_alias ( ode_cOdeContact, "friction_direction", "fdir1" );
	rb_define_method( ode_cOdeContact, "fdir1=", ode_contact_fdir1_eq, -2 );
	rb_define_alias ( ode_cOdeContact, "frictionDirection=", "fdir1=" );
	rb_define_alias ( ode_cOdeContact, "friction_direction=", "fdir1=" );

	/* Load the ruby half of the class */
	rb_require( "ode/contact" );
}

