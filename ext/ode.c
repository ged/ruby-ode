/*
 *		ode.c - ODE Ruby Binding
 *		$Id: ode.c,v 1.3 2002/11/23 23:08:45 deveiant Exp $
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

#include "ode.h"


/* -------------------------------------------------------
 * Globals
 * ------------------------------------------------------- */
VALUE ode_mOde;

VALUE ode_eOdeObsoleteJointError;

VALUE ode_cOdeVector;
VALUE ode_cOdeRotation;
VALUE ode_cOdePosition;
VALUE ode_cOdeLinearVelocity;
VALUE ode_cOdeAngularVelocity;
VALUE ode_cOdeForce;
VALUE ode_cOdeTorque;

VALUE ode_cOdeWorld;
VALUE ode_cOdeBody;
VALUE ode_cOdeJointGroup;
VALUE ode_cOdeJoint;
VALUE ode_cOdeBallJoint;
VALUE ode_cOdeHingeJoint;
VALUE ode_cOdeHinge2Joint;
VALUE ode_cOdeSliderJoint;
VALUE ode_cOdeFixedJoint;
VALUE ode_cOdeUniversalJoint;
VALUE ode_cOdeContactJoint;
VALUE ode_cOdeAMotorJoint;
VALUE ode_cOdeMass;

VALUE ode_cOdeContact;
VALUE ode_cOdePlaceable;
VALUE ode_cOdeGeometry;
VALUE ode_cOdeGeometrySphere;
VALUE ode_cOdeGeometryBox;
VALUE ode_cOdeGeometryPlane;
VALUE ode_cOdeGeometryCapCyl;
VALUE ode_cOdeGeometryCylinder;	/* Optional ODE extension */
VALUE ode_cOdeGeometryTransform;
VALUE ode_cOdeGeometryTransformGroup; /* Optional ODE extension */
VALUE ode_cOdeSpace;
VALUE ode_cOdeHashSpace;

VALUE ode_cOdeSurface;
VALUE ode_cOdeContact;


/* 
 * Hack to work around rb_cMethod being static.
 */
VALUE ruby_cMethod;


/*
 * Classes/modules from the Math3d Library
 */
VALUE math3d_cMatrix4;
VALUE math3d_cRotation;
VALUE math3d_cVector;
VALUE math3d_cVector2;
VALUE math3d_cVector3;
VALUE math3d_cVector4;
VALUE math3d_cLineSeg;
VALUE math3d_cPlane;
VALUE math3d_cBound;
VALUE math3d_mFrust;
VALUE math3d_cFrust;
VALUE math3d_cOrtho;
VALUE math3d_mMath3d;



/* --------------------------------------------------
 * Utility functions
 * -------------------------------------------------- */

void
#ifdef HAVE_STDARG_PROTOTYPES
ode_debug(const char *fmt, ...)
#else
 ode_debug(fmt, va_alist)
	 const char *fmt;
	 va_dcl
#endif
{
	char		buf[BUFSIZ], buf2[BUFSIZ];
	va_list	args;

	if (!RTEST(ruby_verbose)) return;

	snprintf( buf, BUFSIZ, "ODE Debug>>> %s", fmt );

	va_init_list( args, fmt );
	vsnprintf( buf2, BUFSIZ, buf, args );
	fputs( buf2, stderr );
	fputs( "\n", stderr );
	fflush( stderr );
	va_end( args );
}


// Convert an ODE dMatrix3 to a Ruby Array object
VALUE
ode_matrix3_to_rArray( matrix )
	 dMatrix3 matrix;
{
	VALUE	rArray = rb_ary_new2( 9 );
	int		index;

	for ( index = 0; index <= 8; index += 4 )
		rb_ary_push( rArray,
					 rb_ary_new3(4,
								 rb_float_new( *(matrix + (index+0)) ),
								 rb_float_new( *(matrix + (index+1)) ),
								 rb_float_new( *(matrix + (index+2)) )) );

	return rArray;
}


// Convert an ODE dVector3 to a Ruby Array object
VALUE
ode_vector3_to_rArray( vector )
	 dVector3 vector;
{
	VALUE	ary = rb_ary_new2( 3 );
	int		index;

	for ( index = 0; index <= 2; index++ )
		rb_ary_store( ary, index, rb_float_new(*(vector+index)) );

	return ary;
}


// Convert an object to a 3-element array
VALUE
ode_obj_to_ary3( obj, name )
	 VALUE		obj;
	 const char	*name;
{
	VALUE	ary;

	ary = rb_funcall( obj, rb_intern("to_ary"), 0, 0 );
	debugMsg(( "ode_obj_to_ary3: to_ary returned '%s'",
			   STR2CSTR(rb_funcall( ary, rb_intern("inspect"), 0, 0 )) ));

	rb_funcall( ary, rb_intern("flatten!"), 0, 0 );
	if ( TYPE(ary) != T_ARRAY )
		rb_raise( rb_eArgError, "no implicit conversion from %s to Array for %s",
				  rb_class2name(CLASS_OF( obj )),
				  name );
	if ( RARRAY(ary)->len != 3 )
		rb_raise( rb_eArgError, "wrong number of elements in %s (%d for 3)",
				  name,
				  RARRAY(ary)->len );

	return ary;
}

// Convert an object to a 4-element array
VALUE
ode_obj_to_ary4( obj, name )
	 VALUE		obj;
	 const char	*name;
{
	VALUE	ary;

	ary = rb_funcall( obj, rb_intern("to_ary"), 0, 0 );
	debugMsg(( "ode_obj_to_ary4: to_ary returned '%s'",
			   STR2CSTR(rb_funcall( ary, rb_intern("inspect"), 0, 0 )) ));

	rb_funcall( ary, rb_intern("flatten!"), 0, 0 );
	if ( TYPE(ary) != T_ARRAY )
		rb_raise( rb_eArgError, "no implicit conversion from %s to Array for %s",
				  rb_class2name(CLASS_OF( obj )),
				  name );
	if ( RARRAY(ary)->len != 4 )
		rb_raise( rb_eArgError, "wrong number of elements in %s (%d for 4)",
				  name,
				  RARRAY(ary)->len );

	return ary;
}

// Convert an ODE::Rotation object to a dMatrix3
void
ode_rotation_to_dMatrix3( rotation, matrix )
	 VALUE		rotation;
	 dMatrix3	matrix;
{
	VALUE			rarray;
	
	// Fetch the rotation as axis & angle
	rarray = rb_funcall( rotation, rb_intern("to_ary"), 0, 0 );

	if ( TYPE(rarray) != T_ARRAY )
		rb_bug( "Rotation#to_ary did not return an array" );
	else if ( RARRAY(rarray)->len != 4 )
		rb_bug( "Rotation array only contains %d components instead of 4.",
				RARRAY(rarray)->len );

	debugMsg(( "Rotation -> array: %s",
			   STR2CSTR(rb_funcall( rarray, rb_intern("inspect"), 0, 0 )) ));
	
	// Convert to a dMatrix3 from the axis & angle
	dRFromAxisAndAngle( matrix,
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr  )),
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr+1)),
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr+2)),
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr+3)) );
}


/* --------------------------------------------------
 * Initialization function
 * -------------------------------------------------- */
void
Init_ode()
{
	static char
		rcsid[]		= "$Id: ode.c,v 1.3 2002/11/23 23:08:45 deveiant Exp $",
		revision[]	= "$Revision: 1.3 $";

	VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );
	VALUE features	= rb_hash_new();

	// Modules
	ode_mOde = rb_define_module( "ODE" );

	// Module constants
	rb_obj_freeze( vstr );
	rb_define_const( ode_mOde, "Version", vstr );
	vstr = rb_str_new2( rcsid );
	rb_obj_freeze( vstr );
	rb_define_const( ode_mOde, "Rcsid", vstr );

	rb_define_const( ode_mOde, "PI", rb_float_new(M_PI) );
	rb_define_const( ode_mOde, "Infinity", rb_float_new(dInfinity) );

#ifdef dDOUBLE
	vstr = rb_str_new2("dDOUBLE");
#else
	vstr = rb_str_new2("dSINGLE");
#endif	
	rb_obj_freeze( vstr );
	rb_const_set( ode_mOde, rb_intern("Precision"), vstr );

#ifdef HAVE_ODE_DCYLINDER_H
	rb_hash_aset( features, ID2SYM(rb_intern("dCylinder")), Qtrue );
#else
	rb_hash_aset( features, ID2SYM(rb_intern("dCylinder")), Qfalse );
#endif	
#ifdef HAVE_ODE_GEOMTRANSFORMGROUP_H
	rb_hash_aset( features, ID2SYM(rb_intern("GeomTransformGroup")), Qtrue );
#else
	rb_hash_aset( features, ID2SYM(rb_intern("GeomTransformGroup")), Qfalse );
#endif	
	rb_obj_freeze( features );
	rb_const_set( ode_mOde, rb_intern("Features"), features );


	// Find the 'Method' class (as rb_cMethod is static to eval.c for some
	// reason).
	ruby_cMethod		= rb_const_get( rb_cObject,		rb_intern("Method") );


	// Require the math3d library
	rb_require( "math3d" );
  
	// Load the Math3d constants so we can use them
	math3d_mMath3d		= rb_const_get( rb_cObject,		rb_intern("Math3d") ); 
	math3d_cMatrix4		= rb_const_get( math3d_mMath3d,	rb_intern("Matrix4") );
	math3d_cRotation	= rb_const_get( math3d_mMath3d,	rb_intern("Rotation") );
	math3d_cVector		= rb_const_get( math3d_mMath3d,	rb_intern("Vector") );
	math3d_cVector2		= rb_const_get( math3d_mMath3d,	rb_intern("Vector2") );
	math3d_cVector3		= rb_const_get( math3d_mMath3d,	rb_intern("Vector3") );
	math3d_cVector4		= rb_const_get( math3d_mMath3d,	rb_intern("Vector4") );
	math3d_cLineSeg		= rb_const_get( math3d_mMath3d,	rb_intern("LineSeg") );
	math3d_cPlane		= rb_const_get( math3d_mMath3d,	rb_intern("Plane") );
	math3d_cBound		= rb_const_get( math3d_mMath3d,	rb_intern("Bound") );
	math3d_mFrust		= rb_const_get( math3d_mMath3d,	rb_intern("FrustumModule") );
	math3d_cFrust		= rb_const_get( math3d_mMath3d,	rb_intern("Frustum") );
	math3d_cOrtho		= rb_const_get( math3d_mMath3d,	rb_intern("Ortho") );
  

	// Load ruby half of the class library and fetch the class objects
	rb_require( "ode/Vector" );
	ode_cOdeVector			= rb_const_get( ode_mOde, rb_intern("Vector") );

	rb_require( "ode/AngularVelocity" );
	ode_cOdeAngularVelocity	= rb_const_get( ode_mOde, rb_intern("AngularVelocity") );

	rb_require( "ode/Force" );
	ode_cOdeForce			= rb_const_get( ode_mOde, rb_intern("Force") );

	rb_require( "ode/LinearVelocity" );
	ode_cOdeLinearVelocity	= rb_const_get( ode_mOde, rb_intern("LinearVelocity") );

	rb_require( "ode/Position" );
	ode_cOdePosition		= rb_const_get( ode_mOde, rb_intern("Position") );

	rb_require( "ode/Rotation" );
	ode_cOdeRotation		= rb_const_get( ode_mOde, rb_intern("Rotation") );

	rb_require( "ode/Torque" );
	ode_cOdeTorque			= rb_const_get( ode_mOde, rb_intern("Torque") );

	// Add some ODE constants to a few of the classes
	rb_define_const( ode_cOdeRotation, "INFINITESIMAL", INT2FIX(0) );
	rb_define_const( ode_cOdeRotation, "FINITE",		INT2FIX(1) );


	// Define exception class/es
	ode_eOdeObsoleteJointError =
		rb_define_class_under( ode_mOde, "ObsoleteJointError", rb_eRuntimeError );

	// Define the ODE classes
	ode_cOdeWorld			= rb_define_class_under( ode_mOde, "World", rb_cObject );
	ode_cOdeBody			= rb_define_class_under( ode_mOde, "Body", rb_cObject );
	ode_cOdeJointGroup		= rb_define_class_under( ode_mOde, "JointGroup", rb_cObject );
	ode_cOdeJoint			= rb_define_class_under( ode_mOde, "Joint", rb_cObject );
	ode_cOdeBallJoint		= rb_define_class_under( ode_mOde, "BallJoint", ode_cOdeJoint );
	ode_cOdeHingeJoint		= rb_define_class_under( ode_mOde, "HingeJoint", ode_cOdeJoint );
	ode_cOdeHinge2Joint		= rb_define_class_under( ode_mOde, "Hinge2Joint", ode_cOdeJoint );
	ode_cOdeSliderJoint		= rb_define_class_under( ode_mOde, "SliderJoint", ode_cOdeJoint );
	ode_cOdeFixedJoint		= rb_define_class_under( ode_mOde, "FixedJoint", ode_cOdeJoint );
	ode_cOdeUniversalJoint	= rb_define_class_under( ode_mOde, "UniversalJoint", ode_cOdeJoint );
	ode_cOdeContactJoint	= rb_define_class_under( ode_mOde, "ContactJoint", ode_cOdeJoint );
	ode_cOdeAMotorJoint		= rb_define_class_under( ode_mOde, "AngularMotorJoint", ode_cOdeJoint );
	ode_cOdeMass			= rb_define_class_under( ode_mOde, "Mass", rb_cObject );

	// ODE Collision classes
	ode_cOdeContact			= rb_define_class_under( ode_mOde, "Contact", rb_cObject );

	ode_cOdeGeometry		= rb_define_class_under( ode_mOde, "Geometry", rb_cObject );
	ode_cOdeGeometryPlane	= rb_define_class_under( ode_cOdeGeometry, "Plane", ode_cOdeGeometry );

	ode_cOdePlaceable		= rb_define_class_under( ode_cOdeGeometry, "Placeable", ode_cOdeGeometry );
	ode_cOdeGeometrySphere	= rb_define_class_under( ode_cOdeGeometry, "Sphere", ode_cOdePlaceable );
	ode_cOdeGeometryBox		= rb_define_class_under( ode_cOdeGeometry, "Box", ode_cOdePlaceable );
	ode_cOdeGeometryCapCyl	= rb_define_class_under( ode_cOdeGeometry, "CappedCylinder", ode_cOdePlaceable );
	ode_cOdeGeometryCylinder = rb_define_class_under( ode_cOdeGeometry, "Cylinder", ode_cOdePlaceable );

	ode_cOdeGeometryTransform = rb_define_class_under( ode_cOdeGeometry, "Transform", ode_cOdeGeometry );
	ode_cOdeGeometryTransformGroup = rb_define_class_under( ode_cOdeGeometry, "TransformGroup", ode_cOdeGeometry );

	ode_cOdeSpace			= rb_define_class_under( ode_mOde, "Space", ode_cOdeGeometry );
	ode_cOdeHashSpace		= rb_define_class_under( ode_mOde, "HashSpace", ode_cOdeSpace );

	ode_cOdeContact			= rb_define_class_under( ode_mOde, "Contact", rb_cObject );
	ode_cOdeSurface			= rb_define_class_under( ode_mOde, "Surface", rb_cObject );


	// Init the other modules
	ode_init_world();
	// ode_init_space();
	ode_init_body();
	ode_init_mass();
	ode_init_joints();
	ode_init_jointGroup();
	ode_init_contact();
	ode_init_surface();
	ode_init_geometry();
	ode_init_space();
/* 	ode_init_geometry_transform(); */
/* 	ode_init_geometry_transform_group(); */
}
