/*
 *		mass.c - ODE Ruby Binding - Mass object class
 *		$Id: mass.c,v 1.4 2003/02/04 11:27:49 deveiant Exp $
 *		Time-stamp: <04-Feb-2003 03:42:53 deveiant>
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
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static ode_MASS *
ode_mass_alloc()
{
	ode_MASS	*ptr = ALLOC( ode_MASS );
	dMass		*massptr = ALLOC( dMass );

	dMassSetZero( massptr );

	ptr->massptr = massptr;
	ptr->body = Qnil;

	debugMsg(( "Allocated a dMass <%p>", ptr ));
	return ptr;
}


/*
 * GC Mark function
 */
static void
ode_mass_gc_mark( ptr )
	 ode_MASS *ptr;
{

	debugMsg(( "Marking an ODE::Mass" ));

	if ( ptr ) {
		debugMsg(( "Marking Mass <%p>", ptr ));
		rb_gc_mark( ptr->body );
	}
	else {
		debugMsg(( "Not marking uninitialized ode_MASS" ));
	}
}


/*
 * GC Free function
 */
static void
ode_mass_gc_free( ptr )
	 ode_MASS *ptr;
{
	debugMsg(( "Freeing an ODE::Mass." ));

	if ( ptr ) {

		/* If the mass belongs to a body, don't free the dMass * part, as it's
		   part of the dBodyID struct. If there is no associated body object, we
		   have to free the dMass ourselves. This may fall down during global
		   destruction, but we don't actually care at that point, anyway. */
/* 		if ( RTEST(ptr->body) ) { */
/* 			ptr->body = Qnil; */
/* 		} else { */
 			xfree( ptr->massptr );
/* 		} */

		debugMsg(( "Freeing ode_MASS <%p>", ptr ));
		xfree( ptr );
		ptr = NULL;
	}

	else {
		debugMsg(( "Not freeing uninitialized ode_MASS" ));
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ode_MASS *
check_mass( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Mass object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsMass(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Mass)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_MASS *
get_mass( self )
	 VALUE self;
{
	ode_MASS *ptr = check_mass( self );

	debugMsg(( "Fetching a ode_MASS (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized mass" );

	return ptr;
}


/*
 * Publicly-usable mass-fetcher.
 */
ode_MASS *
ode_get_mass( self )
	 VALUE self;
{
	return get_mass(self);
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * allocate()
 * --
 * Allocate a new ODE::Mass object.
 */
static VALUE
ode_mass_s_alloc( klass )
{
	debugMsg(( "Wrapping an uninitialized ODE::Mass pointer." ));
	return Data_Wrap_Struct( klass, ode_mass_gc_mark, ode_mass_gc_free, 0 );
}


/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */

/*
 * ODE::Mass#initialize( mass )
 * --
 * Initialize an ODE::Mass object.
 */
static VALUE
ode_mass_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_MASS	*ptr;
	VALUE		mass;

	if ( !(ptr = check_mass(self)) ) {
		DATA_PTR(self) = ptr = ode_mass_alloc();
	}

	/* Set the mode if it was specified. */
	if ( rb_scan_args(argc, argv, "01", &mass) ) {
		CheckPositiveNonZeroNumber( NUM2DBL(mass), "mass" );
		dMassAdjust( ptr->massptr, (dReal)NUM2DBL(mass) );
	}

	rb_call_super( 0, 0 );
	return self;
}


/*
 * Set the body associated with this mass. Called from ode_body_mass() in
 * body.c.
 */
void
ode_mass_set_body( self, body )
	 VALUE self, body;
{
	ode_MASS	*ptr = get_mass( self );
	ode_BODY	*bodyptr = ode_get_body( body );

	if ( !RTEST(ptr->body) ) {
		debugMsg(( "Freeing standalone dMass *<%p>", ptr->massptr ));
		xfree( ptr->massptr );
	}

	ptr->body = body;
	dBodyGetMass( bodyptr->id, ptr->massptr );
}



/*
 * ODE::Mass#totalMass
 * --
 * Return the total mass of the body this mass applies to.
 */
static VALUE
ode_mass_totalmass( self )
	 VALUE self;
{
	ode_MASS *ptr = get_mass( self );
	return rb_float_new( ptr->massptr->mass );
}


/*
 * ODE::Mass#centerOfGravity
 * --
 * Return the center of gravity position in the body frame as an ODE::Position.
 */
static VALUE
ode_mass_cog( self )
	 VALUE self;
{
	ode_MASS	*ptr = get_mass( self );
	VALUE		pos;

	Vec3ToOdePosition( ptr->massptr->c, pos );
	return pos;
}


/* inertia() */
static VALUE
ode_mass_inertia( self )
	 VALUE self;
{
	ode_MASS		*ptr;

	GetMass( self, ptr );
	return ode_matrix3_to_rArray( ptr->massptr->I );
}



/* adjust( newmass ) or mass=newmass */
static VALUE
ode_mass_adjust( self, newmass )
	 VALUE self, newmass;
{
	ode_MASS		*ptr;

	GetMass( self, ptr );
	dMassAdjust( ptr->massptr, NUM2DBL(newmass) );

	return Qtrue;
}


/* translate( x, y, z ) */
static VALUE
ode_mass_translate( self, x, y, z )
	 VALUE self, x, y, z;
{
	ode_MASS		*ptr;

	GetMass( self, ptr );
	dMassTranslate( ptr->massptr, NUM2DBL(x), NUM2DBL(y), NUM2DBL(z) );

	return Qtrue;
}


/* rotate( ) */
static VALUE
ode_mass_rotate( self, rotation )
	 VALUE	self, rotation;
{
	ode_MASS		*ptr;
	dMatrix3		dmatrix;

	if ( ! rb_obj_is_kind_of(rotation, ode_cOdeQuaternion) )
		rb_raise( rb_eTypeError, "no implicit conversion from %s",
				  rb_class2name(CLASS_OF( rotation )) );

	/* Get the dMatrix3 from the quaternion object and use it to rotate the
	   mass */
	GetMass( self, ptr );
	ode_quaternion_to_dMatrix3( rotation, dmatrix );
	dMassRotate( ptr->massptr, dmatrix );

	return Qtrue;
}


/* --- ODE::Mass::Sphere ------------------------------ */

/*
 * ODE::Mass::Sphere#initialize( radius, density[, totalmass] )
 * --
 * Given a <tt>radius</tt> and a <tt>density</tt>, create a spherical Mass
 * object. If the optional <tt>totalmass</tt> parameter is given, adjust them so
 * the total mass is <tt>totalmass</tt> before returning.
 */
static VALUE
ode_mass_sphere_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_MASS	*ptr;
	VALUE		radius, density, totalmass;

	rb_scan_args( argc, argv, "21", &radius, &density, &totalmass );

	rb_call_super( 0, 0 );
	ptr = get_mass( self );

	CheckPositiveNonZeroNumber( NUM2DBL(radius), "radius" );
	CheckPositiveNonZeroNumber( NUM2DBL(density), "density" );
	dMassSetSphere( ptr->massptr, NUM2DBL(radius), NUM2DBL(density) );

	/* If a totalmass argument was given, check and set it */
	if ( RTEST(totalmass) ) {
		CheckPositiveNonZeroNumber( NUM2DBL(totalmass), "totalmass" );
		dMassAdjust( ptr->massptr, (dReal)NUM2DBL(totalmass) );
	}

	return self;
}


/* --- ODE::Mass::CappedCylinder ------------------------------ */

/*
 * ODE::Mass::CappedCylinder#initialize( density, direction, radius, length[, totalmass] )
 * --
 * Create and return a capped cylinder Mass object of the given <tt>radius</tt>
 * and <tt>length</tt> with the specified density, with the center of mass at
 * (0,0,0) relative to the body. The cylinder's long axis is oriented along the
 * body's x, y or z axis according to the value of <tt>direction</tt> (1=x, 2=y,
 * 3=z). If the optional <tt>totalmass</tt> parameter is given, adjust the
 * values so the total mass is <tt>totalmass</tt> before returning.
 */
static VALUE
ode_mass_ccyl_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_MASS	*ptr;
	VALUE		density, direction, radius, length, totalmass;

	rb_scan_args( argc, argv, "41", &density, &direction, &radius, &length, &totalmass );

	if ( NUM2INT(direction) < 1 || NUM2INT(direction) > 3 )
		rb_raise( rb_eArgError,
				  "Direction argument '%d' out of bounds. Must be between 1 and 3.",
				  NUM2INT(direction) );

	CheckPositiveNonZeroNumber( NUM2DBL(density), "density" );
	CheckPositiveNonZeroNumber( NUM2DBL(radius), "radius" );
	CheckPositiveNonZeroNumber( NUM2DBL(length), "length" );

	rb_call_super( 0, 0 );
	ptr = get_mass( self );

	dMassSetCappedCylinder( ptr->massptr,
							(dReal)NUM2DBL(density),
							NUM2INT(direction),
							(dReal)NUM2DBL(radius),
							(dReal)NUM2DBL(length) );

	/* If a totalmass argument was given, check and set it */
	if ( RTEST(totalmass) ) {
		CheckPositiveNonZeroNumber( NUM2DBL(totalmass), "totalmass" );
		dMassAdjust( ptr->massptr, (dReal)NUM2DBL(totalmass) );
	}

	return self;
}


/* --- ODE::Mass::Box ------------------------------ */

/*
 * ODE::Mass::Box#initialize( density, sideX, sideY, sideZ[, totalmass] )
 * --
 * Create and return a box Mass of the given dimensions and density, with the
 * center of mass at (0,0,0) relative to the body. If the optional
 * <tt>totalmass</tt> parameter is given, adjust the values so the total mass is
 * <tt>totalmass</tt> before returning.
 */
static VALUE
ode_mass_box_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	ode_MASS	*ptr;
	VALUE		density, lx, ly, lz, totalmass;

	rb_scan_args( argc, argv, "41", &density, &lx, &ly, &lz, &totalmass );

	CheckPositiveNonZeroNumber( NUM2DBL(density), "density" );
	CheckPositiveNonZeroNumber( NUM2DBL(lx), "sideX" );
	CheckPositiveNonZeroNumber( NUM2DBL(ly), "sideY" );
	CheckPositiveNonZeroNumber( NUM2DBL(lz), "sideZ" );

	rb_call_super( 0, 0 );
	ptr = get_mass( self );

	dMassSetBox( ptr->massptr,
				 NUM2DBL(density),
				 NUM2DBL(lx),
				 NUM2DBL(ly),
				 NUM2DBL(lz) );

	/* If a totalmass argument was given, check and set it */
	if ( RTEST(totalmass) ) {
		CheckPositiveNonZeroNumber( NUM2DBL(totalmass), "totalmass" );
		dMassAdjust( ptr->massptr, (dReal)NUM2DBL(totalmass) );
	}

	return self;
}




/* Mass initializer */
void
ode_init_mass(void)
{

	/* Allocator */
#ifdef NEW_ALLOC
	rb_define_alloc_func( ode_cOdeMass, ode_mass_s_alloc );
#else
	rb_define_singleton_method( ode_cOdeMass, "allocate", ode_mass_s_alloc, 0 );
#endif

	/* Initializer */
	rb_define_method( ode_cOdeMass, "initialize", ode_mass_init, -1 );
	rb_enable_super( ode_cOdeMass, "initialize" );

	/* Instance methods */
	rb_define_method( ode_cOdeMass, "totalMass", ode_mass_totalmass, 0 );
	rb_define_alias ( ode_cOdeMass, "total_mass", "totalMass" );
	rb_define_method( ode_cOdeMass, "adjust", ode_mass_adjust, 1 );
	rb_define_alias ( ode_cOdeMass, "totalMass=", "adjust" );
	rb_define_alias ( ode_cOdeMass, "total_mass=", "adjust" );
	rb_define_method( ode_cOdeMass, "cog", ode_mass_cog, 0 );
	rb_define_alias ( ode_cOdeMass, "centerOfGravity", "cog" );
	rb_define_alias ( ode_cOdeMass, "center_of_gravity", "cog" );
	rb_define_method( ode_cOdeMass, "inertia", ode_mass_inertia, 0 );
	rb_define_method( ode_cOdeMass, "translate", ode_mass_translate, 3 );
	rb_define_method( ode_cOdeMass, "rotate", ode_mass_rotate, 1 );

	/* ODE::Mass::Sphere */
	rb_define_method( ode_cOdeMassSphere, "initialize", ode_mass_sphere_init, -1 );
	rb_enable_super( ode_cOdeMassSphere, "initialize" );

	/* ODE::Mass::CappedCylinder */
	rb_define_method( ode_cOdeMassCapCyl, "initialize", ode_mass_ccyl_init, -1 );
	rb_enable_super( ode_cOdeMassCapCyl, "initialize" );

	/* ODE::Mass::Box */
	rb_define_method( ode_cOdeMassBox, "initialize", ode_mass_box_init, -1 );
	rb_enable_super( ode_cOdeMassBox, "initialize" );
  
}

