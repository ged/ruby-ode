/*
 *		ode.h - ODE Ruby Binding - Header file
 *		$Id: ode.h,v 1.2 2002/03/20 14:22:24 deveiant Exp $
 *
 *		Author: Michael Granger <ged@FaerieMUD.org>
 *		Copyright (c) 2001 The FaerieMUD Consortium. All rights reserved.
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

#include <ruby.h>
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

// Modules
extern VALUE ode_mOde;

// Classes
extern VALUE ode_cOdeWorld;
extern VALUE ode_cOdeBody;
extern VALUE ode_cOdeRotation;
extern VALUE ode_cOdeMass;
extern VALUE ode_cOdeJointGroup;
extern VALUE ode_cOdeJoint;
extern VALUE ode_cOdeBallJoint;
extern VALUE ode_cOdeHingeJoint;
extern VALUE ode_cOdeHinge2Joint;
extern VALUE ode_cOdeContactJoint;
extern VALUE ode_cOdeSliderJoint;
extern VALUE ode_cOdeFixedJoint;

//extern VALUE ode_mOdeSpace;

// Exception classes
extern VALUE ode_cOdeObsoleteJointError;



/* -------------------------------------------------------
 *	Structures
 * ------------------------------------------------------- */

// ODE::Body struct
typedef struct {
  dBodyID	id;
  VALUE		world;
} ode_BODY;

// ODE::Joint struct
typedef struct {
  dJointID	id;
  VALUE		jointGroup;
  VALUE		world;
  VALUE		body1;
  VALUE		body2;
  VALUE		surface; // Only for ContactJoints
} ode_JOINT;

// JointGroup linked list entry
typedef struct jointListNode {
  struct jointListNode *next;
  VALUE joint;
} ode_JOINTLIST;

// ODE::JointGroup struct
typedef struct {
  dJointGroupID	id;
  ode_JOINTLIST	*jointList;
} ode_JOINTGROUP;

  


/* -------------------------------------------------------
 *	Macros
 * ------------------------------------------------------- */

// Macros
#define _index(i,j) ((i)*4+(j))

// Debugging macro
#if DEBUG
#	define debugMsg(f)	ode_debug f
#else /* ! DEBUG */
#  define debugMsg(f) 
#endif /* DEBUG */


// Macro for unwrapping World structs
#define GetWorld( r, s ) {\
  Data_Get_Struct( (r), dWorldID, (dWorldID)(s) );\
  if ( (s) == NULL ) rb_fatal( "Null world pointer." );\
}

// Macro for unwrapping Body structs
#define GetBody( r, s ) {\
  Data_Get_Struct( (r), ode_BODY, (s) );\
  if ( (s) == NULL ) rb_fatal( "Null body pointer." );\
}

// Macro for unwrapping Mass structs
#define GetMass( r, s ) {\
	Data_Get_Struct( (r), dMass, (s) );\
	if ( (s) == NULL ) rb_fatal( "Null mass pointer." );\
}

// Macro for unwrapping RotationMatrix structs
#define GetRotationMatrix( r, s ) {\
    Data_Get_Struct( (r), dMatrix3, (s) );\
	if ( (s) == NULL ) rb_fatal( "Null rotation matrix pointer." );\
}

// Macro for unwrapping Joint structs
#define GetJoint( r, s ) {\
    Data_Get_Struct( (r), ode_JOINT, (s) );\
	if ( (s) == NULL ) rb_fatal( "Null joint pointer." );\
}

// Macro for unwrapping JointGroup structs
#define GetJointGroup( r, s ) {\
    Data_Get_Struct( (r), dJointGroupID, (dJointGroupID)(s) );\
	if ( (s) == NULL ) rb_fatal( "Null jointGroup pointer." );\
}


// Macro that raises an exception if the specified Joint struct's obsoleteFlag is set..
#define CheckForObsoleteJoint( j ) {\
	if ( RTEST(rb_iv_get(j,"@obsolete")) ) \
		 rb_raise( ode_cOdeObsoleteJointError,\
				   "Cannot use joint which has been marked "\
				   "obsolete." );\
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
//extern void ode_init_space		_(( void ));
//extern void ode_init_				_(( void ));


/* -------------------------------------------------------
 * Global method function declarations
 * ------------------------------------------------------- */

// Generic utility functions
extern VALUE ode_matrix3_to_rArray		_(( dMatrix3 ));
extern void ode_rotation_to_dMatrix3	_(( VALUE, dMatrix3 ));
extern void ode_copy_array				_(( dReal *, dReal *, int ));

/* ODE::Body class */
extern VALUE ode_body_new				_(( VALUE, VALUE ));
extern VALUE ode_world_body_create		_(( VALUE ));

/* ODE::Mass class */
extern VALUE ode_mass_new				_(( int, VALUE *, VALUE ));
extern VALUE ode_mass_new_from_body		_(( dMass * ));

/* ODE::Rotation class */
extern VALUE ode_rotation_new			_(( int, VALUE *, VALUE ));
extern VALUE ode_rotation_new_from_body	_(( dMatrix3 ));

/* ODE::Joint class */
extern VALUE ode_joint_make_obsolete	_(( VALUE ));

/* ODE::JointGroup class */
extern void ode_jointGroup_register_joint _(( VALUE, VALUE ));


#endif /* _R_ODE_H */

