/*
 *		ode.h - ODE Ruby Binding - Header file
 *		$Id: ode.h,v 1.1 2001/12/28 01:10:42 deveiant Exp $
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

// Macros
#define _index(i,j) ((i)*4+(j))


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


/*
 *	Structures
 */
typedef struct {
  dBodyID	id;
  VALUE		world;
} ode_BODY;

typedef struct {
  dJointID	id;
  VALUE		jointGroup;
  VALUE		world;
  VALUE		body1;
  VALUE		body2;
} ode_JOINT;


/*
 *	Macros
 */

#define GetWorld( r, s ) {\
  Data_Get_Struct( (r), dWorldID, (dWorldID)(s) );\
  if ( (s) == NULL ) rb_fatal( "Null world pointer." );\
}

#define GetBody( r, s ) {\
  Data_Get_Struct( (r), ode_BODY, (s) );\
  if ( (s) == NULL ) rb_fatal( "Null body pointer." );\
}

#define GetMass( r, s ) {\
	Data_Get_Struct( (r), dMass, (s) );\
	if ( (s) == NULL ) rb_fatal( "Null mass pointer." );\
}

#define GetRotationMatrix( r, s ) {\
    Data_Get_Struct( (r), dMatrix3, (s) );\
	if ( (s) == NULL ) rb_fatal( "Null rotation matrix pointer." );\
}

#define GetJoint( r, s ) {\
    Data_Get_Struct( (r), ode_JOINT, (s) );\
	if ( (s) == NULL ) rb_fatal( "Null joint pointer." );\
}

#define GetJointGroup( r, s ) {\
    Data_Get_Struct( (r), dJointGroupID, (dJointGroupID)(s) );\
	if ( (s) == NULL ) rb_fatal( "Null jointGroup pointer." );\
}




/*
 * Declarations
 */

// Generic functions
extern VALUE ode_matrix3_to_rArray	_(( dMatrix3 ));
extern void ode_copy_array			_(( dReal *, dReal *, int ));


// World class
extern void ode_init_world			_(( void ));
extern VALUE ode_world_new			_(( VALUE, VALUE ));
extern VALUE ode_world_init			_(( VALUE, VALUE ));
extern void ode_world_gc_free		_(( dWorldID ));
extern VALUE ode_world_gravity		_(( VALUE,VALUE ));
extern VALUE ode_world_gravity_eq	_(( VALUE,VALUE ));
extern VALUE ode_world_erp			_(( VALUE,VALUE ));
extern VALUE ode_world_erp_eq		_(( VALUE,VALUE ));
extern VALUE ode_world_cfm			_(( VALUE,VALUE ));
extern VALUE ode_world_cfm_eq		_(( VALUE,VALUE ));
extern VALUE ode_world_step			_(( VALUE,VALUE ));


// World Body factory method (in body.c)
extern VALUE ode_world_body_create	_(( VALUE ));


// Body class
extern VALUE ode_body_new						_(( VALUE, VALUE ));
extern void ode_init_body						_(( void ));
extern VALUE ode_body_init						_(( VALUE ));
extern void ode_body_gc_free					_(( ode_BODY * ));
extern void ode_body_gc_mark					_(( ode_BODY * ));

extern VALUE ode_body_position					_(( VALUE, VALUE ));
extern VALUE ode_body_position_eq				_(( VALUE, VALUE ));
extern VALUE ode_body_rotation					_(( VALUE, VALUE ));
extern VALUE ode_body_rotation_eq				_(( VALUE, VALUE ));

extern VALUE ode_body_linearVelocity			_(( VALUE, VALUE ));
extern VALUE ode_body_linearVelocity_eq			_(( VALUE, VALUE ));
extern VALUE ode_body_angularVelocity			_(( VALUE, VALUE ));
extern VALUE ode_body_angularVelocity_eq		_(( VALUE, VALUE ));

extern VALUE ode_body_mass						_(( VALUE, VALUE ));
extern VALUE ode_body_mass_eq					_(( VALUE, VALUE ));

extern VALUE ode_body_add_force					_(( VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_body_add_torque				_(( VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_body_add_rel_force				_(( VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_body_add_rel_torque			_(( VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_body_add_force_at_pos			_(( VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_body_add_rel_force_at_pos		_(( VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_body_add_rel_force_at_rel_pos	_(( VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE ));


// Rotation class
extern void ode_init_rotation			_(( void ));
extern VALUE ode_rotation_new			_(( int, VALUE*, VALUE ));
extern VALUE ode_rotation_new_from_body	_(( dMatrix3 ));
extern VALUE ode_rotation_init			_(( int, VALUE*, VALUE ));
extern VALUE ode_rotation_to_quaternion	_(( VALUE, VALUE ));
extern VALUE ode_rotation_to_matrix		_(( VALUE, VALUE ));
extern void ode_rotation_to_dMatrix3	_(( VALUE, dMatrix3 ));


// Mass class
extern void ode_init_mass			_(( void ));
extern VALUE ode_mass_new			_(( int, VALUE*, VALUE ));
extern VALUE ode_mass_init			_(( int, VALUE*, VALUE ));
extern VALUE ode_mass_mass			_(( VALUE ));
extern VALUE ode_mass_cog			_(( VALUE ));
extern VALUE ode_mass_inertia		_(( VALUE ));
extern VALUE ode_mass_adjust		_(( VALUE, VALUE ));
extern VALUE ode_mass_translate		_(( VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_mass_rotate		_(( VALUE, VALUE ));
extern VALUE ode_mass_new_sphere	_(( VALUE, VALUE, VALUE ));
extern VALUE ode_mass_new_ccyl		_(( VALUE, VALUE, VALUE, VALUE, VALUE ));
extern VALUE ode_mass_new_box		_(( VALUE, VALUE, VALUE, VALUE, VALUE ));


// Joint class
extern void ode_init_joints			_(( void ));
extern VALUE ode_joint_new			_(( int, VALUE *, VALUE, dJointID ));
extern void ode_joint_gc_mark		_(( ode_JOINT * ));
extern void ode_joint_gc_free		_(( ode_JOINT * ));
extern VALUE ode_joint_init			_(( VALUE ));


// JointGroup class
extern void ode_init_jointGroup		_(( void ));
extern VALUE ode_jointGroup_new		_(( VALUE, VALUE ));
extern VALUE ode_jointGroup_init	_(( VALUE ));
extern VALUE ode_jointGroup_empty	_(( VALUE ));
extern void ode_jointGroup_gc_free	_(( dJointGroupID ));


// Space class

//extern void ode_init_space	_(( void ));
//extern void ode_init_body	_(( void ));


#endif /* _R_ODE_H */

