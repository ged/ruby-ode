/*
 *		ode.h - ODE Ruby Binding - Header file
 *		$Id: ode.h,v 1.3 2002/11/23 23:08:45 deveiant Exp $
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2001, 2002 The FaerieMUD Consortium. All rights reserved.
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


#ifndef _R_ODE_H
#define _R_ODE_H 1

#include <stdlib.h>

#include <ruby.h>
#include <intern.h>				/* For rb_dbl2big() */

#include <ode/ode.h>

// Debugging functions/macros
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
 * Hack to make up for rb_cMethod being static for some reason.
 */
extern VALUE ruby_cMethod;

/*
 * Modules
 */
extern VALUE ode_mOde;

/*
 * Exception classes
 */
extern VALUE ode_eOdeObsoleteJointError;


/*
 * Utility/data classes
 */
extern VALUE ode_cOdeVector;
extern VALUE ode_cOdeRotation;
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
extern VALUE ode_cOdeHingeJoint;
extern VALUE ode_cOdeHinge2Joint;
extern VALUE ode_cOdeSliderJoint;
extern VALUE ode_cOdeFixedJoint;
extern VALUE ode_cOdeUniversalJoint;
extern VALUE ode_cOdeContactJoint;
extern VALUE ode_cOdeAMotorJoint;

extern VALUE ode_cOdeMass;


extern VALUE ode_cOdeContact;
extern VALUE ode_cOdeGeometry;
extern VALUE ode_cOdePlaceable;
extern VALUE ode_cOdeGeometrySphere;
extern VALUE ode_cOdeGeometryBox;
extern VALUE ode_cOdeGeometryPlane;
extern VALUE ode_cOdeGeometryCapCyl;
extern VALUE ode_cOdeGeometryCylinder; /* Optional ODE extension */
extern VALUE ode_cOdeGeometryTransform;
extern VALUE ode_cOdeGeometryTransformGroup; /* Optional ODE extension */
extern VALUE ode_cOdeSpace;
extern VALUE ode_cOdeHashSpace;

extern VALUE ode_cOdeSurface;
extern VALUE ode_cOdeContact;




/*
 * Classes/modules from the Math3d Library
 */
extern VALUE math3d_cMatrix4;
extern VALUE math3d_cRotation;
extern VALUE math3d_cVector;
extern VALUE math3d_cVector2;
extern VALUE math3d_cVector3;
extern VALUE math3d_cVector4;
extern VALUE math3d_cLineSeg;
extern VALUE math3d_cPlane;
extern VALUE math3d_cBound;
extern VALUE math3d_mFrust;
extern VALUE math3d_cFrust;
extern VALUE math3d_cOrtho;
extern VALUE math3d_mMath3d;


/* -------------------------------------------------------
 *	Structures
 * ------------------------------------------------------- */

// ODE::Body struct
typedef struct {
	dBodyID			id;
	VALUE			object, world;
} ode_BODY;

// ODE::Joint struct
typedef struct {
	dJointID		id;
	dJointFeedback	*feedback;
	VALUE			joint, jointGroup, world, body1, body2, fbhash, contact;
} ode_JOINT;

// JointGroup linked list entry
typedef struct jointListNode {
	struct jointListNode *next;
	VALUE		joint;
} ode_JOINTLIST;

// ODE::JointGroup struct
typedef struct {
	dJointGroupID	id;
	ode_JOINTLIST	*jointList;
} ode_JOINTGROUP;

// ODE::Geometry struct (for ODE::Spaces, too)
typedef struct {
	dGeomID			id;
	VALUE			object, body, container;
} ode_GEOMETRY;  

// ODE::Contact struct
typedef struct {
	dContact		*contact;
	VALUE			object, surface, geometry1, geometry2;
} ode_CONTACT;



/* -------------------------------------------------------
 *	Macros
 * ------------------------------------------------------- */

// Convert x,y to index of a 4xn array
#define _index(i,j) ((i)*4+(j))

// Debugging macro
#if DEBUG
#	define debugMsg(f)	ode_debug f
#else /* ! DEBUG */
#  define debugMsg(f) 
#endif /* DEBUG */


// Macro for unwrapping World structs
#define GetWorld( r, s ) {\
	(s) = ode_get_world( r ); \
}

// Macro for unwrapping Body structs
#define GetBody( r, s ) {\
	(s) = ode_get_body( r ); \
}

// Macro for unwrapping Mass structs
#define GetMass( r, s ) {\
	CheckKindOf( r, ode_cOdeMass );\
	(s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null mass pointer." );\
}

// Macro for unwrapping RotationMatrix structs
#define GetRotationMatrix( r, s ) {\
    (s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null rotation matrix pointer." );\
}

// Macro for unwrapping Joint structs
#define GetJoint( r, s ) {\
	CheckKindOf( r, ode_cOdeJoint );\
    (s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null joint pointer." );\
}

// Macro for unwrapping dJointGroupID structs
#define GetJointGroup( r, s ) {\
	CheckKindOf( r, ode_cOdeJointGroup );\
	(s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null jointGroup pointer." );\
}

// Macro for unwrapping dContact structs
#define GetContact( r, s ) {\
	CheckKindOf( r, ode_cOdeContact );\
    (s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null contact pointer." );\
}

// Macro for unwrapping dSurfaceParameters structs
#define GetSurface( r, s ) {\
	CheckKindOf( r, ode_cOdeSurface );\
    (s) = DATA_PTR(r);\
	if ( (s) == NULL ) rb_fatal( "Null surface pointer." );\
}

// Macro for unwrapping ode_GEOMETRY structs
#define GetGeometry( r, s ) {\
	(r) = ode_get_geom( s ); \
}



// Macro that raises an exception if the specified Joint struct's obsoleteFlag
// is set..
#define CheckForObsoleteJoint( j ) {\
	if ( RTEST(rb_iv_get(j,"@obsolete")) ) \
		 rb_raise( ode_eOdeObsoleteJointError,\
				   "Cannot use joint which has been marked "\
				   "obsolete." );\
}

// Test that obj is .kind_of?( klass ) and raise a TypeError if not.
#define CheckKindOf( obj, klass ) {\
	if ( ! rb_obj_is_kind_of(obj, klass) ) \
		 rb_raise( rb_eTypeError, \
				   "no implicit conversion to %s from %s", \
				   rb_class2name(klass), \
				   rb_class2name(CLASS_OF( obj )) ); \
}

// Test that the specified var contains a number which is greater than or equal
// to 0. If the test fails, raise a RangeError with a message built from the
// given name.
#define CheckPositiveNumber( var, name ) {\
	if ( (var) < 0 ) \
		rb_raise( rb_eRangeError, \
				  "Illegal value for parameter '" name \
				  "' (%0.1f): must be a non-negative number.", \
				  (var) ); \
}

// Test that the specified var contains a number which is greater than 0. If the
// test fails, raise a RangeError with a message built from the given name.
#define CheckPositiveNonZeroNumber( var, name ) {\
	if ( (var) <= 0 ) \
		rb_raise( rb_eRangeError, \
				  "Illegal value for parameter '" name \
				  "' (%0.1f): must be a positive non-zero number.", \
				  (var) ); \
}

// Make a copy of a dReal array 
#define CopyDRealArray( original, copy, depth ) {\
	memcpy( copy, original, sizeof(dReal)*depth );\
}

// Turn a dVector3 into an ODE::Vector
#define Vec3ToOdeVector( vec, odevec ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *(vec + i) );\
	  odevec = rb_class_new_instance( 3, axes, ode_cOdeVector );\
  } while (0);\
}

// Update a current ODE::Vector with the values from a dVector3
#define SetOdeVectorFromVec3( vec, odevec ) {\
  rb_ary_store( odevec, 0, rb_float_new(*vec) );\
  rb_ary_store( odevec, 1, rb_float_new(*vec+1) );\
  rb_ary_store( odevec, 2, rb_float_new(*vec+2) );\
}

// Turn a dVector3 into an ODE::Force
#define Vec3ToOdeForce( vec, odeforce ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *(vec + i) );\
	  odeforce = rb_class_new_instance( 3, axes, ode_cOdeForce );\
  } while (0);\
}


// Turn a dVector3 into an ODE::Torque
#define Vec3ToOdeTorque( vec, odetorque ) {\
  do {\
	  VALUE		axes[3];\
	  int		i;\
	  for ( i = 0; i <= 2; i++ )\
		  axes[i] = rb_float_new( *(vec + i) );\
	  odetorque = rb_class_new_instance( 3, axes, ode_cOdeTorque );\
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
//extern void ode_init_				_(( void ));
extern void ode_init_contact		_(( void ));
extern void ode_init_surface		_(( void ));
extern void ode_init_geometry		_(( void ));


/* -------------------------------------------------------
 * Global method function declarations
 * ------------------------------------------------------- */

// Generic utility functions
extern VALUE ode_matrix3_to_rArray		_(( dMatrix3 ));
extern VALUE ode_vector3_to_rArray		_(( dVector3 ));
extern VALUE ode_obj_to_ary3			_(( VALUE, const char * ));
extern VALUE ode_obj_to_ary4			_(( VALUE, const char * ));
extern void ode_rotation_to_dMatrix3	_(( VALUE, dMatrix3 ));

/* ODE::Mass class */
extern VALUE ode_mass_new				_(( int, VALUE *, VALUE ));
extern VALUE ode_mass_new_from_body		_(( dMass * ));

/* ODE::Joint class */
extern VALUE ode_joint_make_obsolete	_(( VALUE ));

/* ODE::JointGroup class */
extern void ode_jointGroup_register_joint _(( VALUE, VALUE ));

/* ODE::Contact class */
extern VALUE ode_contact_new_from_geom	_(( VALUE, dContactGeom * ));

/* Fetchers */
extern ode_GEOMETRY *ode_get_geom			_(( VALUE ));
extern ode_GEOMETRY *ode_get_space			_(( VALUE ));
extern ode_BODY *ode_get_body				_(( VALUE ));
extern dWorldID ode_get_world				_(( VALUE ));
extern dSurfaceParameters *ode_get_surface	_(( VALUE ));

#endif /* _R_ODE_H */

