/*
 *		ode.h - ODE Ruby Binding - Header file
 *		$Id$
 *		Time-stamp: <18-Feb-2003 09:56:41 deveiant>
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


#ifndef _R_ODE_H
#define _R_ODE_H 1

#include <stdlib.h>
#include <stdio.h>

#include <ruby.h>
#include <intern.h>				/* For rb_dbl2big() */
#include <version.h>			/* Check version for alloc framework */

#include <ode/ode.h>

/* Debugging functions/macros */
#ifdef HAVE_STDARG_PROTOTYPES
#include <stdarg.h>
#define va_init_list(a,b) va_start(a,b)
extern void ode_debug(const char *fmt, ...);
#else
#include <varargs.h>
#define va_init_list(a,b) va_start(a)
extern void ode_debug(fmt, va_alist);
#endif


/* -------------------------------------------------------
 * Globals
 * ------------------------------------------------------- */

/* 
 * Hack to make up for various Ruby objects being static for some reason.
 */
extern VALUE ruby_cMethod;
extern VALUE ruby_eLocalJumpError;


/*
 * Modules
 */
extern VALUE ode_mOde;

/*
 * Exception classes
 */
extern VALUE ode_eOdeObsoleteJointError;
extern VALUE ode_eOdeGeometryError;


/*
 * Utility/data classes
 */
extern VALUE ode_cOdeVector;
extern VALUE ode_cOdeQuaternion;
extern VALUE ode_cOdePosition;
extern VALUE ode_cOdeLinearVelocity;
extern VALUE ode_cOdeAngularVelocity;
extern VALUE ode_cOdeForce;
extern VALUE ode_cOdeTorque;


/*
 * Primary Classes
 */
extern VALUE ode_cOdeWorld;
extern VALUE ode_cOdeBody;
extern VALUE ode_cOdeJointGroup;
extern VALUE ode_cOdeJoint;
extern VALUE ode_cOdeBallJoint;
extern VALUE ode_cOdeFixedJoint;
extern VALUE ode_cOdeUniversalJoint;
extern VALUE ode_cOdeContactJoint;

extern VALUE ode_cOdeParamJoint;
extern VALUE ode_cOdeHingeJoint;
extern VALUE ode_cOdeHinge2Joint;
extern VALUE ode_cOdeSliderJoint;
extern VALUE ode_cOdeAMotorJoint;

extern VALUE ode_cOdeMass;
extern VALUE ode_cOdeMassBox;
extern VALUE ode_cOdeMassSphere;
extern VALUE ode_cOdeMassCapCyl;

extern VALUE ode_cOdeContact;

extern VALUE ode_cOdeGeometry;
extern VALUE ode_cOdePlaceable;
extern VALUE ode_cOdeGeometrySphere;
extern VALUE ode_cOdeGeometryBox;
extern VALUE ode_cOdeGeometryPlane;
extern VALUE ode_cOdeGeometryCapCyl;
extern VALUE ode_cOdeGeometryCylinder; /* Optional ODE extension */
extern VALUE ode_cOdeGeometryRay;
extern VALUE ode_cOdeGeometryTransform;
extern VALUE ode_cOdeGeometryTransformGroup; /* Optional ODE extension */
extern VALUE ode_cOdeSpace;
extern VALUE ode_cOdeHashSpace;

extern VALUE ode_cOdeSurface;
extern VALUE ode_cOdeContact;



/* -------------------------------------------------------
 *	Structures
 * ------------------------------------------------------- */

/* ODE::Body struct */
typedef struct {
	dBodyID			id;
	VALUE			object, world, mass;
} ode_BODY;

/* ODE::Mass object */
typedef struct {
	dMass			*massptr;
	VALUE			body;
} ode_MASS;

/* ODE::Joint structs */
typedef struct {
	dJointID		id;
	dJointFeedback	*feedback;
	VALUE			object, jointGroup, world, body1, body2, fbhash, contact, obsolete;
} ode_JOINT;

/* JointGroup linked list entry */
typedef struct jointListNode {
	struct jointListNode *next;
	VALUE		joint;
} ode_JOINTLIST;

/* ODE::JointGroup struct */
typedef struct {
	dJointGroupID	id;
	ode_JOINTLIST	*jointList;
} ode_JOINTGROUP;

/* ODE::Geometry struct (for ODE::Spaces, too) */
typedef struct {
	dGeomID			id;
	VALUE			object, body, surface, container;
} ode_GEOMETRY;  

/* ODE::Contact struct */
typedef struct {
	dContact		*contact;
	VALUE			object, surface;
} ode_CONTACT;

/* Callback data for collision system */
typedef struct {
	VALUE			callback;
	VALUE			args;
} ode_CALLBACK;



/* -------------------------------------------------------
 *	Macros
 * ------------------------------------------------------- */

/* Convert x,y to index of a 4xn array */
#define _index(i,j) ((i)*4+(j))

/* Debugging macro */
#if DEBUG
#	define debugMsg(f)	ode_debug f
#else /* ! DEBUG */
#  define debugMsg(f) 
#endif /* DEBUG */


/* Macro for unwrapping World structs */
#define GetWorld( r, s ) {\
	(s) = ode_get_world( r ); \
}

/* Macro for unwrapping Body structs */
#define GetBody( r, s ) {\
	(s) = ode_get_body( r ); \
}

/* Macro for unwrapping Mass structs */
#define GetMass( r, s ) {\
	CheckKindOf( r, ode_cOdeMass );\
	(s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null mass pointer." );\
}

/* Macro for unwrapping Joint structs */
#define GetJoint( r, s ) {\
	CheckKindOf( r, ode_cOdeJoint );\
    (s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null joint pointer." );\
}

/* Macro for unwrapping dJointGroupID structs */
#define GetJointGroup( r, s ) {\
	CheckKindOf( r, ode_cOdeJointGroup );\
	(s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null jointGroup pointer." );\
}

/* Macro for unwrapping dContact structs */
#define GetContact( r, s ) {\
	(r) = ode_get_contact( s ); \
}

/* Macro for unwrapping dSurfaceParameters structs */
#define GetSurface( r, s ) {\
	(r) = ode_get_surface( s ); \
}

/* Macro for unwrapping ode_GEOMETRY structs */
#define GetGeometry( r, s ) {\
	(r) = ode_get_geom( s ); \
}


#define IsSpace( obj ) rb_obj_is_kind_of( (obj), ode_cOdeSpace )
#define IsWorld( obj ) rb_obj_is_kind_of( (obj), ode_cOdeWorld )
#define IsBody( obj ) rb_obj_is_kind_of( (obj), ode_cOdeBody )
#define IsJoint( obj ) rb_obj_is_kind_of( (obj), ode_cOdeJoint )
#define IsJointGroup( obj ) rb_obj_is_kind_of( (obj), ode_cOdeJointGroup )
#define IsSurface( obj ) rb_obj_is_kind_of( (obj), ode_cOdeSurface )
#define IsMass( obj ) rb_obj_is_kind_of( (obj), ode_cOdeMass )


/* Test that obj is .kind_of?( klass ) and raise a TypeError if not. */
#define CheckKindOf( obj, klass ) {\
	if ( ! rb_obj_is_kind_of(obj, klass) ) \
		 rb_raise( rb_eTypeError, \
				   "no implicit conversion to %s from %s", \
				   rb_class2name(klass), \
				   rb_class2name(CLASS_OF( obj )) ); \
}

/* Test that the specified var contains a number which is greater than or equal 
   to 0. If the test fails, raise a RangeError with a message built from the 
   given name. */
#define CheckPositiveNumber( var, name ) {\
	if ( (var) < 0 ) \
		rb_raise( rb_eRangeError, \
				  "Illegal value for parameter '" name \
				  "' (%0.1f): must be a non-negative number.", \
				  (var) ); \
}

/* Test that the specified var contains a number which is greater than 0. If the 
   test fails, raise a RangeError with a message built from the given name. */
#define CheckPositiveNonZeroNumber( var, name ) {\
	if ( (var) <= 0 ) \
		rb_raise( rb_eRangeError, \
				  "Illegal value for parameter '" name \
				  "' (%0.1f): must be a positive non-zero number.", \
				  (var) ); \
}

/* Test that the specified var contains a number which is greater than or equal
   to min, but less than or equal to max, inclusive. If the test fails, raise a
   RangeError with a message built from the given name. */
#define CheckNumberBetween( var, name, min, max ) {\
	if ( (var) < (min) || (var) > (max) ) \
		rb_raise( rb_eRangeError, \
				  "Illegal value for parameter '" name \
				  "' (%0.1f): must be between %0.5f and %0.5f.", \
				  (var), (min), (max) ); \
}

/* Make a copy of a dReal array  */
#define CopyDRealArray( original, copy, depth ) {\
	memcpy( (copy), (original), sizeof(dReal)*(depth) );\
}

/* Turn a dVector3 into an ODE::Vector */
#define Vec3ToOdeVector( vec, odevec ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *((vec) + i) );\
	  (odevec) = rb_class_new_instance( 3, axes, ode_cOdeVector );\
  } while (0);\
}

/* Update a current ODE::Vector with the values from a dVector3 */
#define SetOdeVectorFromVec3( vec, odevec ) {\
  rb_ary_store( (odevec), 0, rb_float_new(*(vec)) );\
  rb_ary_store( (odevec), 1, rb_float_new(*(vec)+1) );\
  rb_ary_store( (odevec), 2, rb_float_new(*(vec)+2) );\
}

#define SetVec3FromArray( vec, ary ) {\
	*(vec)   = (dReal)NUM2DBL( *(RARRAY(ary)->ptr  ) ); \
	*(vec+1) = (dReal)NUM2DBL( *(RARRAY(ary)->ptr+1) ); \
	*(vec+2) = (dReal)NUM2DBL( *(RARRAY(ary)->ptr+2) ); \
}

/* Turn a dVector3 into an ODE::Force */
#define Vec3ToOdeForce( vec, odeforce ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *((vec) + i) );\
	  (odeforce) = rb_class_new_instance( 3, axes, ode_cOdeForce );\
  } while (0);\
}


/* Turn a dVector3 into an ODE::Torque */
#define Vec3ToOdeTorque( vec, odetorque ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *((vec) + i) );\
	  (odetorque) = rb_class_new_instance( 3, axes, ode_cOdeTorque );\
  } while (0);\
}

/* Turn a dVector3 into an ODE::Position */
#define Vec3ToOdePosition( vec, oedipus ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *((vec) + i) );\
	  (oedipus) = rb_class_new_instance( 3, axes, ode_cOdePosition );\
  } while (0);\
}



/* -------------------------------------------------------
 * Initializer functions
 * ------------------------------------------------------- */
extern void ode_init_world			_(( void ));
extern void ode_init_body			_(( void ));
extern void ode_init_rotation		_(( void ));
extern void ode_init_mass			_(( void ));
extern void ode_init_joints			_(( void ));
extern void ode_init_jointGroup		_(( void ));
extern void ode_init_space			_(( void ));
extern void ode_init_contact		_(( void ));
extern void ode_init_surface		_(( void ));
extern void ode_init_geometry		_(( void ));

/* -------------------------------------------------------
 * Global method function declarations
 * ------------------------------------------------------- */

/* Generic utility functions */
extern VALUE ode_matrix3_to_rArray			_(( dMatrix3 ));
extern VALUE ode_vector3_to_rArray			_(( dVector3 ));
extern VALUE ode_obj_to_ary3				_(( VALUE, const char * ));
extern VALUE ode_obj_to_ary4				_(( VALUE, const char * ));
extern void ode_quaternion_to_dMatrix3		_(( VALUE, dMatrix3 ));
extern void ode_near_callback				_(( ode_CALLBACK *, dGeomID, dGeomID ));
extern void ode_check_arity					_(( VALUE, int ));

/* ODE::Mass class */
extern void ode_mass_set_body				_(( VALUE, VALUE ));

/* ODE::JointGroup class */
extern void ode_jointGroup_register_joint	_(( VALUE, VALUE ));

/* ODE::Contact class */
extern void ode_contact_set_cgeom			_(( VALUE, dContactGeom * ));

/* Fetchers */
extern ode_GEOMETRY *ode_get_geom			_(( VALUE ));
extern ode_GEOMETRY *ode_get_space			_(( VALUE ));
extern ode_BODY *ode_get_body				_(( VALUE ));
extern dWorldID ode_get_world				_(( VALUE ));
extern dSurfaceParameters *ode_get_surface	_(( VALUE ));
extern ode_CONTACT *ode_get_contact			_(( VALUE ));
extern ode_JOINT *ode_get_joint				_(( VALUE ));
extern ode_JOINTGROUP *ode_get_jointGroup	_(( VALUE ));
extern ode_MASS *ode_get_mass				_(( VALUE ));

#endif /* _R_ODE_H */

