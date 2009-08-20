/*
 *		surface.c - ODE Ruby Binding - ODE::Surface class
 *		$Id$
 *		Time-stamp: <04-Feb-2003 15:33:25 deveiant>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2002, 2003 The FaerieMUD Consortium.
 *
 *		This work is licensed under the Creative Commons Attribution License. To
 *		view a copy of this license, visit
 *		http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
 *		Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
 *
 */

#include "ode.h"

/*
 * This class wraps the surface parameters struct:
 * --
 *  typedef struct dSurfaceParameters {
 *    // must always be defined
 *    int mode;
 *    dReal mu;
 *  
 *    // only defined if the corresponding flag is set in mode
 *    dReal mu2;
 *    dReal bounce;
 *    dReal bounce_vel;
 *    dReal soft_erp;
 *    dReal soft_cfm;
 *    dReal motion1,motion2;
 *    dReal slip1,slip2;
 *  } dSurfaceParameters;
 */

/*
 * The masks which may be set in the mode member:
 * --
 *  enum {
 *    dContactMu2		= 0x001,
 *    dContactFDir1		= 0x002,
 *    dContactBounce	= 0x004,
 *    dContactSoftERP	= 0x008,
 *    dContactSoftCFM	= 0x010,
 *    dContactMotion1	= 0x020,
 *    dContactMotion2	= 0x040,
 *    dContactSlip1		= 0x080,
 *    dContactSlip2		= 0x100,
 *  
 *    dContactApprox0	= 0x0000,
 *    dContactApprox1_1	= 0x1000,
 *    dContactApprox1_2	= 0x2000,
 *    dContactApprox1	= 0x3000
 *  };
*/

/* --------------------------------------------------
 * Macros and constants
 * -------------------------------------------------- */

#define IsSurface( obj ) rb_obj_is_kind_of( (obj), ode_cOdeSurface )



/* --------------------------------------------------
 *	Memory-management functions
 * -------------------------------------------------- */

/*
 * Allocation function
 */
static dSurfaceParameters *
ode_surface_alloc()
{
	dSurfaceParameters *ptr = ALLOC( dSurfaceParameters );

	ptr->mode		= 0;
	ptr->mu			= dInfinity;

	ptr->mu2		= 0.f;
	ptr->bounce		= 0.f;
	ptr->bounce_vel	= 0.f;
	ptr->soft_erp	= 0.f;
	ptr->soft_cfm	= 0.f;
	ptr->motion1	= 0.f;
	ptr->motion2	= 0.f;
	ptr->slip1		= 0.f;
	ptr->slip2		= 0.f;

	debugMsg(( "Allocated a dSurfaceParameters <%p>", ptr ));
	return ptr;
}


/*
 * Object validity checker. Returns the data pointer.
 */
static dSurfaceParameters *
check_surface( self )
	 VALUE	self;
{
	debugMsg(( "Checking a Surface object (%d).", self ));
	Check_Type( self, T_DATA );

    if ( !IsSurface(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Surface)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static dSurfaceParameters *
get_surface( self )
	 VALUE self;
{
	dSurfaceParameters *ptr = check_surface( self );

	debugMsg(( "Fetching a dSurfaceParameters (%p).", ptr ));
	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized surface" );

	return ptr;
}


/*
 * Publicly-usable surface-fetcher.
 */
dSurfaceParameters *
ode_get_surface( self )
	 VALUE self;
{
	return get_surface(self);
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * allocate()
 * --
 * Allocate a new ODE::Surface object.
 */
static VALUE
ode_surface_s_alloc( klass )
{
	debugMsg(( "Wrapping an uninitialized ODE::Surface pointer." ));
	return Data_Wrap_Struct( klass, 0, xfree, 0 );
}


/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */

static VALUE
ode_surface_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	dSurfaceParameters	*surface;
	VALUE				mu;

	DATA_PTR(self) = surface = ode_surface_alloc();

	/* Set the mode if it was specified. */
	if ( rb_scan_args(argc, argv, "01", &mu) ) {
		CheckPositiveNumber( NUM2DBL(mu), "mu" );
		surface->mu = (dReal)NUM2DBL( mu );
	}

	rb_call_super( 0, 0 );

	return self;
}


/*
 * Average two dReals.
 */
static dReal
average( val1, val2 )
	 dReal	val1, val2;
{
	if ( val1 == dInfinity || val2 == dInfinity ) return dInfinity;
	return (dReal)(( val1 + val2 ) / 2.0);
}


/*
 * |( otherSurface )
 * --
 * Combine the receiver with the <tt>otherSurface</tt> and return the new
 * surface.
 */
static VALUE
ode_surface_combine( self, otherSurface )
	 VALUE self, otherSurface;
{
	dSurfaceParameters *surface	= get_surface( self );
	dSurfaceParameters *other	= get_surface( otherSurface );

	VALUE newSurface = rb_class_new_instance( 0, 0, CLASS_OF(self) );
	dSurfaceParameters *ptr = get_surface( newSurface );

	/* This is by no means perfect, and probably not even mathematically or
	   physically correct, but it does the job for now. Suggestions for a better
	   way to do this welcomed. */
	ptr->mode		= surface->mode | other->mode;
	ptr->mu			= average( surface->mu, other->mu );
	ptr->mu2		= average( surface->mu2, other->mu2 );

	ptr->bounce		= average( surface->bounce, other->bounce );
	ptr->bounce_vel	= average( surface->bounce_vel, other->bounce_vel );
	ptr->soft_erp	= average( surface->soft_erp, other->soft_erp );
	ptr->soft_cfm	= average( surface->soft_cfm, other->soft_cfm );
	ptr->motion1	= average( surface->motion1, other->motion1 );
	ptr->motion2	= average( surface->motion2, other->motion2 );
	ptr->slip1		= average( surface->slip1, other->slip1 );
	ptr->slip2		= average( surface->slip2, other->slip2 );

	return newSurface;
}

/*
 * mode()
 * --
 * Get the surface's contact flags. See the constants in the ODE::Contact class
 * for possible legal values.
 */
static VALUE
ode_surface_get_mode( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	return INT2FIX( surface->mode );
}


/*
 * mode=( Integer )
 * --
 * Set the surface's contact flags. See the constants in the ODE::Contact class
 * for possible legal values. Note that the mode value may also be altered by
 * setting the corresponding values for the given contact flags. Eg., calling
 * ODE::Contact#mu2= with a non-nil value will automatically add
 * ODE::Contact::Mu2 to the surface's contact mode, while calling it with nil or
 * false will remove the flag. Setting this to a value inconsistant with the
 * contact values may have undesired results.
 */
static VALUE
ode_surface_set_mode( self, newMode )
	 VALUE self, newMode;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	CheckPositiveNumber( NUM2INT(newMode), "mode" );
	surface->mode = NUM2INT( newMode );

	return INT2FIX( surface->mode );
}


/*
 * mu()
 * --
 * Get the surface's Coulomb friction coefficient.
 */
static VALUE
ode_surface_get_mu( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	return rb_float_new( surface->mu );
}


/*
 * mu=( Float )
 * --
 * Set the surface's Coulomb friction coefficient. This must be in the range 0.0
 * to ODE::Infinity. 0.0 results in a frictionless contact, and ODE::Infinity
 * results in a contact that never slips. Note that frictionless contacts are
 * less time consuming to compute than ones with friction, and infinite friction
 * contacts can be cheaper than contacts with finite friction.
 */
static VALUE
ode_surface_set_mu( self, newMu )
	 VALUE self, newMu;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	CheckPositiveNumber( NUM2DBL(newMu), "mu" );
	surface->mu = (dReal)NUM2DBL( newMu );

	return rb_float_new( surface->mu );
}



/*
 * mu2()
 * --
 * Get the surface's Coulomb friction coefficient for friction direction 2. This
 * must be in the range 0.0 to ODE::Infinity. If not set, the surface's first
 * coefficient will be used for both directions.
 */
static VALUE
ode_surface_get_mu2( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactMu2 )
		return rb_float_new( surface->mu2 );
	else
		return Qnil;
}


/*
 * mu2=( Float )
 * --
 * Set the surface's Coulomb friction coefficient for friction direction 2. This
 * must be in the range 0.0 to ODE::Infinity. If not set, the surface's first
 * coefficient will be used for both directions.
 */
static VALUE
ode_surface_set_mu2( self, newMu2 )
	 VALUE self, newMu2;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newMu2 )) {
		dReal mu2 = NUM2DBL( newMu2 );

		if ( mu2 != dInfinity && mu2 < 0 )
			rb_raise( rb_eRangeError,
					  "Illegal value (%0.2f); must be between 0 and ODE::Infinity",
					  mu2 );
	
		surface->mu2 = mu2;
		surface->mode |= dContactMu2;

		return rb_float_new( surface->mu2 );
	} else {
		surface->mode ^= (surface->mode & dContactMu2);
		surface->mu2 = 0.f;

		return Qnil;
	}
}


/*
 * mu2?()
 * --
 * Predicate method to test if the Coulomb friction coefficient for friction
 * direction 2 is set for this surface.
 */
static VALUE
ode_surface_mu2_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactMu2 )
		return Qtrue;
	else
		return Qfalse;
}



/*
 * bounce()
 * --
 * Get the surface's elasticity as the coefficient of restitution, which will be
 * a value between 0 and 1, inclusive. 0 means the surfaces are not elastic at
 * all; 1 is perfect elasticity.
 */
static VALUE
ode_surface_get_bounce( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactBounce )
		return rb_float_new( surface->bounce );
	else
		return Qnil;
}


/*
 * bounce=( Float )
 * --
 * Set the surface's elasticity as the coefficient of restitution; the value
 * must be between 0 and 1, inclusive. 0 means the surfaces are not elastic at
 * all; 1 is perfect elasticity.
 */
static VALUE
ode_surface_set_bounce( self, newBounce )
	 VALUE self, newBounce;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newBounce )) {
		dReal bounce = NUM2DBL( newBounce );

		if ( bounce < 0 || bounce > 1 )
			rb_raise( rb_eRangeError,
					  "Value (%0.2f) out of bounds; must be between 0 and 1, inclusive.",
					  bounce );

		surface->bounce = bounce;
		surface->mode |= dContactBounce;

		return rb_float_new( surface->bounce );
	} else {
		surface->mode ^= (surface->mode & dContactBounce);
		surface->bounce = 0.f;

		return Qnil;
	}
}


/*
 * bounce_vel()
 * --
 * Get the minimum incoming velocity necessary for bounce (in m/s). Incoming
 * velocities below this will effectively have a bounce parameter of 0.
 */
static VALUE
ode_surface_get_bounce_vel( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactBounce )
		return rb_float_new( surface->bounce_vel );
	else
		return Qnil;
}


/*
 * bounce_vel=( Float )
 * --
 * Set the minimum incoming velocity necessary for bounce (in m/s). Incoming
 * velocities below this will effectively have a bounce parameter of 0.
 */
static VALUE
ode_surface_set_bounce_vel( self, velocity )
	 VALUE self, velocity;
{
	dSurfaceParameters	*surface = get_surface(self);
	dReal vel = NUM2DBL( velocity );

	if ( vel < 0 )
		rb_raise( rb_eRangeError, "Value (%0.2f) out of bounds; must be "
				  "a positive value.", vel );

	surface->bounce_vel = vel;

	return rb_float_new( surface->bounce_vel );
}


/*
 * bounce?()
 * --
 * Predicate method to test if the elasticity (bounciness) of the surface has
 * been set.
 */
static VALUE
ode_surface_bounce_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactBounce )
		return Qtrue;
	else
		return Qfalse;
}



/*
 * softERP()
 * --
 * Get the surface's contact normal "softness" error reduction parameter.
 */
static VALUE
ode_surface_get_soft_erp( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSoftERP )
		return rb_float_new( surface->soft_erp );
	else
		return Qnil;
}


/*
 * softERP=( Float )
 * --
 * Set the surface's contact normal "softness" error reduction
 * parameter. Typical values are in the range 0.1 to 0.8.
 */
static VALUE
ode_surface_set_soft_erp( self, newSoftERP )
	 VALUE self, newSoftERP;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newSoftERP )) {
		dReal erp = NUM2DBL( newSoftERP );

		if ( erp < 0 || erp > 1 )
			rb_raise( rb_eRangeError,
					  "Value (%0.2f) out of bounds; must be between 0 and 1, inclusive.",
					  erp );

		surface->soft_erp = erp;
		surface->mode |= dContactSoftERP;

		return rb_float_new( surface->soft_erp );
	} else {
		surface->mode ^= (surface->mode & dContactSoftERP);
		surface->soft_erp = 0.f;

		return Qnil;
	}
}


/*
 * softERP?()
 * --
 * Predicate method to test if the surface's contact normal "softness" error
 * reduction parameter is enabled.
 */
static VALUE
ode_surface_soft_erp_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSoftERP )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * softCFM()
 * --
 * Get the surface's contact normal "softness" constraint force mixing
 * parameter.
 */
static VALUE
ode_surface_get_soft_cfm( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSoftCFM )
		return rb_float_new( surface->soft_cfm );
	else
		return Qnil;
}


/*
 * softCFM=( Float )
 * --
 * Set the surface's contact normal "softness" constraint force mixing
 * parameter. Typical values are in the range 1e-9 to 1.
 */
static VALUE
ode_surface_set_soft_cfm( self, newSoftCFM )
	 VALUE self, newSoftCFM;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newSoftCFM )) {
		dReal cfm = NUM2DBL( newSoftCFM );

		if ( cfm < 0 || cfm > 1 )
			rb_raise( rb_eRangeError,
					  "Value (%0.2f) out of bounds; must be between 0 and 1, inclusive.",
					  cfm );

		surface->soft_cfm = cfm;
		surface->mode |= dContactSoftCFM;

		return rb_float_new( surface->soft_cfm );
	} else {
		surface->mode ^= (surface->mode & dContactSoftCFM);
		surface->soft_cfm = 0.f;

		return Qnil;
	}
}


/*
 * softCFM?()
 * --
 * Predicate method to test if the surface's contact normal "softness"
 * constraint force mixing parameter is enabled.
 */
static VALUE
ode_surface_soft_cfm_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSoftCFM )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * motion1()
 * --
 * Get the surface's velocity (m/s) in friction direction 1.
 */
static VALUE
ode_surface_get_motion1( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactMotion1 )
		return rb_float_new( surface->motion1 );
	else
		return Qnil;
}


/*
 * motion1=( Float )
 * --
 * Set the surface's velocity (m/s) in friction direction 1 to the given value.
 */
static VALUE
ode_surface_set_motion1( self, newMotion1 )
	 VALUE self, newMotion1;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newMotion1 )) {
		dReal velocity = NUM2DBL( newMotion1 );

		surface->motion1 = velocity;
		surface->mode |= dContactMotion1;

		return rb_float_new( surface->motion1 );
	} else {
		surface->mode ^= (surface->mode & dContactMotion1);
		surface->motion1 = 0.f;

		return Qnil;
	}
}


/*
 * motion1?()
 * --
 * Predicate method to test if the surface has a velocity in friction direction
 * 1. If <tt>true</tt>, the contact surface is assumed to be moving
 * independently of the motion of the bodies -- like a conveyor belt running
 * over the surface.
 */
static VALUE
ode_surface_motion1_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactMotion1 )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * motion2()
 * --
 * Get the surface's velocity (m/s) in friction direction 2.
 */
static VALUE
ode_surface_get_motion2( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactMotion2 )
		return rb_float_new( surface->motion2 );
	else
		return Qnil;
}


/*
 * motion2=( Float )
 * --
 * Set the surface's velocity (m/s) in friction direction 2 to the given value.
 */
static VALUE
ode_surface_set_motion2( self, newMotion2 )
	 VALUE self, newMotion2;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newMotion2 )) {
		dReal velocity = NUM2DBL( newMotion2 );

		surface->motion2 = velocity;
		surface->mode |= dContactMotion2;

		return rb_float_new( surface->motion2 );
	} else {
		surface->mode ^= (surface->mode & dContactMotion2);
		surface->motion2 = 0.f;

		return Qnil;
	}
}


/*
 * motion2?()
 * --
 * Predicate method to test if the surface has a velocity in friction direction
 * 2. If <tt>true</tt>, the contact surface is assumed to be moving
 * independently of the motion of the bodies -- like a conveyor belt running
 * over the surface.
 */
static VALUE
ode_surface_motion2_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactMotion2 )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * slip1()
 * --
 * Get the surface's first-order slip in friction direction 1.
 */
static VALUE
ode_surface_get_slip1( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSlip1 )
		return rb_float_new( surface->slip1 );
	else
		return Qnil;
}


/*
 * slip1=( Float )
 * --
 * Set the surface's first-order slip in friction direction 1 to the given value.
 */
static VALUE
ode_surface_set_slip1( self, newSlip1 )
	 VALUE self, newSlip1;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newSlip1 )) {
		dReal coefficient = NUM2DBL( newSlip1 );

		if ( coefficient < 0 || coefficient > 1 )
			rb_raise( rb_eRangeError,
					  "Value (%0.2f) out of bounds; must be between 0 and 1, inclusive.",
					  coefficient );

		surface->slip1 = coefficient;
		surface->mode |= dContactSlip1;

		return rb_float_new( surface->slip1 );
	} else {
		surface->mode ^= (surface->mode & dContactSlip1);
		surface->slip1 = 0.f;

		return Qnil;
	}
}


/*
 * slip1?()
 * --
 * Predicate method to test if the surface defines a first-order slip for
 * friction direction 1.
 */
static VALUE
ode_surface_slip1_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSlip1 )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * slip2()
 * --
 * Get the surface's first-order slip in friction direction 2.
 */
static VALUE
ode_surface_get_slip2( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSlip2 )
		return rb_float_new( surface->slip2 );
	else
		return Qnil;
}


/*
 * slip2=( Float )
 * --
 * Set the surface's first-order slip in friction direction 2 to the given value.
 */
static VALUE
ode_surface_set_slip2( self, newSlip2 )
	 VALUE self, newSlip2;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( newSlip2 )) {
		dReal coefficient = NUM2DBL( newSlip2 );

		if ( coefficient < 0 || coefficient > 1 )
			rb_raise( rb_eRangeError,
					  "Value (%0.2f) out of bounds; must be between 0 and 1, inclusive.",
					  coefficient );

		surface->slip2 = coefficient;
		surface->mode |= dContactSlip2;

		return rb_float_new( surface->slip2 );
	} else {
		surface->mode ^= (surface->mode & dContactSlip2);
		surface->slip2 = 0.f;

		return Qnil;
	}
}


/*
 * slip2?()
 * --
 * Predicate method to test if the surface defines a first-order slip for
 * friction direction 2.
 */
static VALUE
ode_surface_slip2_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactSlip2 )
		return Qtrue;
	else
		return Qfalse;
}


/* --------------------------------------------------
 * Mode-only convenience accessors
 * -------------------------------------------------- */


/*
 * useFrictionDirection=( trueOrFalse )
 * --
 * If set to a true value, the 'frictionDirection' vector of any ODE::Contact
 * this surface is applied to will be used as friction direction 1; otherwise
 * direction 1 will be computed to be perpendicular to the contact normal (in
 * which case its resulting orientation is not defined).
 */
static VALUE
ode_surface_set_fdir1_mode( self, setting )
	VALUE self, setting;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if (RTEST( setting ))
		surface->mode |= dContactFDir1;
	else
		surface->mode ^= (surface->mode & dContactFDir1);

	return RTEST(setting) ? Qtrue : Qfalse;
}

/*
 * useFrictionDirection?
 * --
 * Returns true if the 'frictionDirection' vector of any ODE::Contact this
 * surface is applied to will be used as friction direction 1.
 */
static VALUE
ode_surface_fdir1_mode_p( self )
	VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactFDir1 )
		return Qtrue;
	else 
		return Qfalse;
}


/*
 * frictionModel=( flags )
 * --
 * Set the friction model to be used for either or both friction directions. The
 * <tt>flags</tt> argument can be one of
 * ODE::Contact::PyramidFriction{Neither,1,2,Both}. Any friction direction which
 * does not use the pyramid friction approximation will use normal "box
 * friction". ODE::Contact::PyramidFrictionNeither is the default.
 */
static VALUE
ode_surface_set_friction_model( self, setting )
	VALUE self, setting;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	surface->mode ^= (surface->mode & dContactApprox1);
	surface->mode |= (FIX2INT(setting) & (dContactApprox1));

	return INT2FIX( surface->mode & dContactApprox1 );
}


/*
 * pyramidFriction1=( trueOrFalse )
 * --
 * Turn the pyramid friction approximation on or off for friction direction 1.
 */
static VALUE
ode_surface_set_friction_model1( self, setting )
	VALUE self, setting;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( setting )) 
		surface->mode |= dContactApprox1_1;

	return (surface->mode & dContactApprox1_1) ? Qtrue : Qfalse;
}


/*
 * pyramidFriction1?
 * --
 * Returns <tt>true</tt> if friction direction 1 of the surface will use the
 * friction pyramid approximation.
 */
static VALUE
ode_surface_friction_model1_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactApprox1_1 )
		return Qtrue;
	else
		return Qfalse;
}


/*
 * pyramidFriction1=( trueOrFalse )
 * --
 * Turn the pyramid friction approximation on or off for friction direction 1.
 */
static VALUE
ode_surface_set_friction_model2( self, setting )
	VALUE self, setting;
{
	dSurfaceParameters	*surface = get_surface(self);

	if (RTEST( setting )) 
		surface->mode |= dContactApprox1_2;

	return (surface->mode & dContactApprox1_2) ? Qtrue : Qfalse;
}


/*
 * pyramidFriction2?
 * --
 * Returns <tt>true</tt> if friction direction 2 of the surface will use the
 * friction pyramid approximation.
 */
static VALUE
ode_surface_friction_model2_p( self )
	 VALUE self;
{
	dSurfaceParameters	*surface = get_surface(self);
	
	if ( surface->mode & dContactApprox1_2 )
		return Qtrue;
	else
		return Qfalse;
}



/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_surface() {
	/* Kluge to make Rdoc see the class in this file */
#if FOR_RDOC_PARSER
	ode_mOde = rb_define_module( "ODE" );
	ode_cOdeSurface			= rb_define_class_under( ode_mOde, "Surface", rb_cObject );
#endif

	/* Constructor */
	rb_define_alloc_func( ode_cOdeSurface, ode_surface_s_alloc );

	/* Initializers */
	rb_define_method( ode_cOdeSurface, "initialize", ode_surface_init, -1 );
	rb_enable_super ( ode_cOdeSurface, "initialize" );

	/* Operators */
	rb_define_method( ode_cOdeSurface, "|", ode_surface_combine, 1 );
	
	/* Accessors */
	rb_define_method( ode_cOdeSurface, "mode", ode_surface_get_mode, 0 );
	rb_define_method( ode_cOdeSurface, "mode=", ode_surface_set_mode, 1 );
	rb_define_method( ode_cOdeSurface, "mu", ode_surface_get_mu, 0 );
	rb_define_method( ode_cOdeSurface, "mu=", ode_surface_set_mu, 1 );

	rb_define_method( ode_cOdeSurface, "mu2", ode_surface_get_mu2, 0 );
	rb_define_method( ode_cOdeSurface, "mu2=", ode_surface_set_mu2, 1 );
	rb_define_method( ode_cOdeSurface, "mu2?", ode_surface_mu2_p, 0 );

	rb_define_method( ode_cOdeSurface, "bounce", ode_surface_get_bounce, 0 );
	rb_define_method( ode_cOdeSurface, "bounce=", ode_surface_set_bounce, 1 );
	rb_define_method( ode_cOdeSurface, "bounceVelocity", ode_surface_get_bounce_vel, 0 );
	rb_define_alias ( ode_cOdeSurface, "bounce_velocity", "bounceVelocity" );
	rb_define_alias ( ode_cOdeSurface, "bounce_vel", "bounceVelocity" );
	rb_define_method( ode_cOdeSurface, "bounceVelocity=", ode_surface_set_bounce_vel, 1 );
	rb_define_alias ( ode_cOdeSurface, "bounce_velocity=", "bounceVelocity=" );
	rb_define_alias ( ode_cOdeSurface, "bounce_vel=", "bounceVelocity=" );
	rb_define_method( ode_cOdeSurface, "bounce?", ode_surface_bounce_p, 0 );

	rb_define_method( ode_cOdeSurface, "softERP", ode_surface_get_soft_erp, 0 );
	rb_define_alias ( ode_cOdeSurface, "soft_erp", "softERP" );
	rb_define_method( ode_cOdeSurface, "softERP=", ode_surface_set_soft_erp, 1 );
	rb_define_alias ( ode_cOdeSurface, "soft_erp=", "softERP=" );
	rb_define_method( ode_cOdeSurface, "softERP?", ode_surface_soft_erp_p, 0 );
	rb_define_alias ( ode_cOdeSurface, "soft_erp?", "softERP?" );

	rb_define_method( ode_cOdeSurface, "softCFM", ode_surface_get_soft_cfm, 0 );
	rb_define_alias ( ode_cOdeSurface, "soft_cfm", "softCFM" );
	rb_define_method( ode_cOdeSurface, "softCFM=", ode_surface_set_soft_cfm, 1 );
	rb_define_alias ( ode_cOdeSurface, "soft_cfm=", "softCFM=" );
	rb_define_method( ode_cOdeSurface, "softCFM?", ode_surface_soft_cfm_p, 0 );
	rb_define_alias ( ode_cOdeSurface, "soft_cfm?", "softCFM?" );

	rb_define_method( ode_cOdeSurface, "motion1", ode_surface_get_motion1, 0 );
	rb_define_method( ode_cOdeSurface, "motion1=", ode_surface_set_motion1, 1 );
	rb_define_method( ode_cOdeSurface, "motion1?", ode_surface_motion1_p, 0 );

	rb_define_method( ode_cOdeSurface, "motion2", ode_surface_get_motion2, 0 );
	rb_define_method( ode_cOdeSurface, "motion2=", ode_surface_set_motion2, 1 );
	rb_define_method( ode_cOdeSurface, "motion2?", ode_surface_motion2_p, 0 );

	rb_define_method( ode_cOdeSurface, "slip1", ode_surface_get_slip1, 0 );
	rb_define_method( ode_cOdeSurface, "slip1=", ode_surface_set_slip1, 1 );
	rb_define_method( ode_cOdeSurface, "slip1?", ode_surface_slip1_p, 0 );

	rb_define_method( ode_cOdeSurface, "slip2", ode_surface_get_slip2, 0 );
	rb_define_method( ode_cOdeSurface, "slip2=", ode_surface_set_slip2, 1 );
	rb_define_method( ode_cOdeSurface, "slip2?", ode_surface_slip2_p, 0 );

	/* Flag-only modes */

	/* Friction direction specifier (a flag for a member of the contact struct) */
	rb_define_method( ode_cOdeSurface, "useFrictionDirection=", ode_surface_set_fdir1_mode, 1 );
	rb_define_alias ( ode_cOdeSurface, "use_friction_direction=", "useFrictionDirection=" );
	rb_define_alias ( ode_cOdeSurface, "fDir1=", "useFrictionDirection=" );
	rb_define_method( ode_cOdeSurface, "useFrictionDirection?", ode_surface_fdir1_mode_p, 0 );
	rb_define_alias ( ode_cOdeSurface, "use_friction_direction?", "useFrictionDirection?" );
	rb_define_alias ( ode_cOdeSurface, "fDir1?", "useFrictionDirection?" );

	/* Friction model flag; if either direction is set, "friction pyramid
	   approximation 1" is used instead of constraint-force-limit approximation
	   for that direction. */
	rb_define_method( ode_cOdeSurface, "frictionModel=", ode_surface_set_friction_model, 1 );
	rb_define_alias ( ode_cOdeSurface, "friction_model=", "frictionModel=" );
	rb_define_alias ( ode_cOdeSurface, "approx1=", "frictionModel=" );

	rb_define_method( ode_cOdeSurface, "pyramidFriction1=", ode_surface_set_friction_model1, 0 );
	rb_define_alias ( ode_cOdeSurface, "pyramid_friction1=", "pyramidFriction1=" );
	rb_define_method( ode_cOdeSurface, "pyramidFriction1?", ode_surface_friction_model1_p, 0 );
	rb_define_alias ( ode_cOdeSurface, "pyramid_friction1?", "pyramidFriction1?" );

	rb_define_method( ode_cOdeSurface, "pyramidFriction2=", ode_surface_set_friction_model2, 0 );
	rb_define_alias ( ode_cOdeSurface, "pyramid_friction2=", "pyramidFriction2=" );
	rb_define_method( ode_cOdeSurface, "pyramidFriction2?", ode_surface_friction_model2_p, 0 );
	rb_define_alias ( ode_cOdeSurface, "pyramid_friction2?", "pyramidFriction2?" );

	
}


