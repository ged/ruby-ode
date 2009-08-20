/*
 *		ode.c - ODE Ruby Binding
 *		$Id$
 *		Time-stamp: <27-Jul-2005 19:25:53 ged>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2001-2005 The FaerieMUD Consortium.
 *
 *		This work is licensed under the Creative Commons Attribution License. To
 *		view a copy of this license, visit
 *		http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
 *		Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
 *
 */

#include "ode.h"


/* -------------------------------------------------------
 * Globals
 * ------------------------------------------------------- */
VALUE ode_mOde;

VALUE ode_eOdeObsoleteJointError;
VALUE ode_eOdeGeometryError;

VALUE ode_cOdeVector;
VALUE ode_cOdeQuaternion;
VALUE ode_cOdePosition;
VALUE ode_cOdeLinearVelocity;
VALUE ode_cOdeAngularVelocity;
VALUE ode_cOdeForce;
VALUE ode_cOdeTorque;
VALUE ode_cOdeMatrix;

VALUE ode_cOdeWorld;
VALUE ode_cOdeBody;
VALUE ode_cOdeJointGroup;
VALUE ode_cOdeJoint;
VALUE ode_cOdeBallJoint;
VALUE ode_cOdeFixedJoint;
VALUE ode_cOdeUniversalJoint;
VALUE ode_cOdeContactJoint;

VALUE ode_cOdeParamJoint;
VALUE ode_cOdeHingeJoint;
VALUE ode_cOdeHinge2Joint;
VALUE ode_cOdeSliderJoint;
VALUE ode_cOdeAMotorJoint;

VALUE ode_cOdeMass;
VALUE ode_cOdeMassBox;
VALUE ode_cOdeMassSphere;
VALUE ode_cOdeMassCapsule;

VALUE ode_cOdeContact;
VALUE ode_cOdePlaceable;
VALUE ode_cOdeGeometry;
VALUE ode_cOdeGeometrySphere;
VALUE ode_cOdeGeometryBox;
VALUE ode_cOdeGeometryPlane;
VALUE ode_cOdeGeometryCapCyl;
VALUE ode_cOdeGeometryCylinder;	/* Optional ODE extension */
VALUE ode_cOdeGeometryRay;
VALUE ode_cOdeGeometryTransform;
VALUE ode_cOdeGeometryTransformGroup; /* Optional ODE extension */
VALUE ode_cOdeSpace;
VALUE ode_cOdeHashSpace;

VALUE ode_cOdeSurface;
VALUE ode_cOdeContact;

/* 
 * Hack to work around various Ruby variables being static.
 */
VALUE ruby_cMethod;
VALUE ruby_eLocalJumpError;



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

	if (!RTEST(ruby_debug)) return;

	snprintf( buf, BUFSIZ, "ODE Debug>>> %s", fmt );

	va_init_list( args, fmt );
	vsnprintf( buf2, BUFSIZ, buf, args );
	fputs( buf2, stderr );
	fputs( "\n", stderr );
	fflush( stderr );
	va_end( args );
}


/* Convert an ODE dMatrix3 to a Ruby Array object */
inline VALUE
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


/* Convert an ODE dVector3 to a Ruby Array object */
inline VALUE
ode_vector3_to_rArray( vector )
	 dVector3 vector;
{
	VALUE	ary = rb_ary_new2( 3 );
	int		index;

	for ( index = 0; index <= 2; index++ )
		rb_ary_store( ary, index, rb_float_new(*(vector+index)) );

	return ary;
}


/* Convert an object to a 3-element array */
inline VALUE
ode_obj_to_ary3( obj, name )
	 VALUE		obj;
	 const char	*name;
{
	VALUE	ary;

	if ( !rb_respond_to(obj, rb_intern("to_ary")) )
		rb_raise( rb_eTypeError, "no implicit conversion from %s to Array for %s",
				  rb_class2name(CLASS_OF( obj )),
				  name );
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

/* Convert an object to a 4-element array */
inline VALUE
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

/* Convert an ODE::Quaternion object to a dMatrix3 */
inline void
ode_quaternion_to_dMatrix3( quaternion, matrix )
	 VALUE		quaternion;
	 dMatrix3	matrix;
{
	VALUE			rarray;
	
	/* Fetch the quaternion as axis & angle */
	rarray = rb_funcall( quaternion, rb_intern("to_ary"), 0, 0 );

	if ( TYPE(rarray) != T_ARRAY )
		rb_bug( "Quaternion#to_ary did not return an array" );
	else if ( RARRAY(rarray)->len != 4 )
		rb_bug( "Quaternion array only contains %d components instead of 4.",
				RARRAY(rarray)->len );

	debugMsg(( "Quaternion -> array: %s",
			   STR2CSTR(rb_funcall( rarray, rb_intern("inspect"), 0, 0 )) ));
	
	/* Convert to a dMatrix3 from the axis & angle */
	dRFromAxisAndAngle( matrix,
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr  )),
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr+1)),
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr+2)),
						(dReal) NUM2DBL(*(RARRAY(rarray)->ptr+3)) );
}


/*
 * Test the arity of the specified block, making sure that it's either of
 * variable arity or at least the arity specified, and raising an ArgumentError
 * if not.
 */
inline void
ode_check_arity( block, minArity )
	 VALUE		block;
	 const int	minArity;
{
	int arity = NUM2INT(rb_funcall( block, rb_intern("arity"), 0 ));

	if ( arity > -1 && arity < minArity )
		rb_raise( rb_eArgError,
				  "Not enough parameters for block (requires at least %d, got %d)",
				  minArity, arity );
}


/*
 * Return a string containing the class name associated with a given dGeomID.
 */ 
static const char*
ode_get_geom_class( obj )
	 dGeomID	obj;
{
	char *class;

	switch( dGeomGetClass(obj) ) {

	case dSphereClass:
		class = "Sphere";
		break;

	case dBoxClass:
		class = "Box";
		break;

	case dCCylinderClass:
		class = "Capped cylinder";
		break;

	case dCylinderClass:
		class = "Regular flat-ended cylinder";
		break;

	case dPlaneClass:
		class = "Infinite plane (non-placeable)";
		break;

	case dGeomTransformClass:
		class = "Geometry transform";
		break;

	case dRayClass:
		class = "Ray";
		break;

	case dTriMeshClass:
		class = "Triangle mesh";
		break;

	case dSimpleSpaceClass:
		class = "Simple space";
		break;

	case dHashSpaceClass:
		class = "Hash table based space";
		break;

	default:
		class = "(Unknown)";
	}

	return class;
}


/*
 * Delegator for collision "near" callbacks. Called from
 * ode_space_each_adjacent_pair() and ode_geometry_isect() when two geometries
 * in a space are potentially colliding.
*/
void
ode_near_callback( callback, o1, o2 )
	 ode_CALLBACK	*callback;
	 dGeomID		o1, o2;
{
	ode_GEOMETRY		*geom1, *geom2;
	static const char	*class1, *class2;

	class1 = ode_get_geom_class( o1 );
	class2 = ode_get_geom_class( o2 );

	debugMsg(( "In near callback with %p (%s) and %p (%s).", o1, class1, o2, class2 ));

	geom1 = dGeomGetData( o1 );
	geom2 = dGeomGetData( o2 );

	rb_funcall( callback->callback, rb_intern("call"), 3,
				geom1->object, geom2->object, callback->args );
}




/* --------------------------------------------------
 * Initialization function
 * -------------------------------------------------- */
void
Init_ode()
{
	VALUE precision = Qnil;
	VALUE features	= rb_hash_new();

	ode_debug( "Loading Ruby ODE binding" );

	/* Modules */
	ode_mOde = rb_define_module( "ODE" );

	/* Module constants */
	rb_define_const( ode_mOde, "Pi", rb_float_new(M_PI) );
	rb_define_const( ode_mOde, "Infinity", rb_float_new(dInfinity) );
	rb_define_const( ode_mOde, "ROTATION_INFINITESIMAL", INT2FIX(0) );
	rb_define_const( ode_mOde, "ROTATION_FINITE", INT2FIX(1) );

#ifdef dDOUBLE
	precision = rb_str_new2("dDOUBLE");
#else
	precision = rb_str_new2("dSINGLE");
#endif	
	rb_obj_freeze( precision );
	rb_const_set( ode_mOde, rb_intern("Precision"), precision );

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


	/* Find the 'Method' and LocalJumpError classes (as rb_cMethod and
	   rb_eLocalJumpError are static for some reason). */
	ruby_cMethod		 = rb_const_get( rb_cObject,	rb_intern("Method") );
	ruby_eLocalJumpError = rb_const_get( rb_cObject,	rb_intern("LocalJumpError") );

	/* Load ruby half of the class library and fetch the class objects */
	rb_require( "ode/vector" );
	ode_cOdeVector			= rb_const_get( ode_mOde, rb_intern("Vector") );

	rb_require( "ode/angularvelocity" );
	ode_cOdeAngularVelocity	= rb_const_get( ode_mOde, rb_intern("AngularVelocity") );

	rb_require( "ode/force" );
	ode_cOdeForce			= rb_const_get( ode_mOde, rb_intern("Force") );

	rb_require( "ode/linearvelocity" );
	ode_cOdeLinearVelocity	= rb_const_get( ode_mOde, rb_intern("LinearVelocity") );

	rb_require( "ode/position" );
	ode_cOdePosition		= rb_const_get( ode_mOde, rb_intern("Position") );

	rb_require( "ode/quaternion" );
	ode_cOdeQuaternion		= rb_const_get( ode_mOde, rb_intern("Quaternion") );

	rb_require( "ode/torque" );
	ode_cOdeTorque			= rb_const_get( ode_mOde, rb_intern("Torque") );

	rb_require( "ode/matrix" );
	ode_cOdeMatrix			= rb_const_get( ode_mOde, rb_intern("Matrix") );


	/* Define exception class/es */
	ode_eOdeObsoleteJointError =
		rb_define_class_under( ode_mOde, "ObsoleteJointError", rb_eRuntimeError );
	ode_eOdeGeometryError =
		rb_define_class_under( ode_mOde, "GeometryError", rb_eRuntimeError );


	/* Define the ODE classes */
	ode_cOdeWorld			= rb_define_class_under( ode_mOde, "World", rb_cObject );
	ode_cOdeBody			= rb_define_class_under( ode_mOde, "Body", rb_cObject );
	ode_cOdeJointGroup		= rb_define_class_under( ode_mOde, "JointGroup", rb_cObject );
	ode_cOdeJoint			= rb_define_class_under( ode_mOde, "Joint", rb_cObject );
	ode_cOdeBallJoint		= rb_define_class_under( ode_mOde, "BallJoint", ode_cOdeJoint );
	ode_cOdeFixedJoint		= rb_define_class_under( ode_mOde, "FixedJoint", ode_cOdeJoint );
	ode_cOdeUniversalJoint	= rb_define_class_under( ode_mOde, "UniversalJoint", ode_cOdeJoint );
	ode_cOdeContactJoint	= rb_define_class_under( ode_mOde, "ContactJoint", ode_cOdeJoint );

	ode_cOdeParamJoint		= rb_define_class_under( ode_mOde, "ParameterizedJoint", ode_cOdeJoint );
	ode_cOdeHingeJoint		= rb_define_class_under( ode_mOde, "HingeJoint", ode_cOdeParamJoint );
	ode_cOdeHinge2Joint		= rb_define_class_under( ode_mOde, "Hinge2Joint", ode_cOdeParamJoint );
	ode_cOdeSliderJoint		= rb_define_class_under( ode_mOde, "SliderJoint", ode_cOdeParamJoint );
	ode_cOdeAMotorJoint		= rb_define_class_under( ode_mOde, "AngularMotorJoint", ode_cOdeParamJoint );

	ode_cOdeMass			= rb_define_class_under( ode_mOde, "Mass", rb_cObject );
	ode_cOdeMassBox			= rb_define_class_under( ode_cOdeMass, "Box", ode_cOdeMass );
	ode_cOdeMassSphere		= rb_define_class_under( ode_cOdeMass, "Sphere", ode_cOdeMass );
	ode_cOdeMassCapsule		= rb_define_class_under( ode_cOdeMass, "Capsule", ode_cOdeMass );

	/* ODE Collision classes */
	ode_cOdeContact			= rb_define_class_under( ode_mOde, "Contact", rb_cObject );

	ode_cOdeGeometry		= rb_define_class_under( ode_mOde, "Geometry", rb_cObject );
	ode_cOdeGeometryPlane	= rb_define_class_under( ode_cOdeGeometry, "Plane", ode_cOdeGeometry );

	ode_cOdePlaceable		= rb_define_class_under( ode_cOdeGeometry, "Placeable", ode_cOdeGeometry );
	ode_cOdeGeometrySphere	= rb_define_class_under( ode_cOdeGeometry, "Sphere", ode_cOdePlaceable );
	ode_cOdeGeometryBox		= rb_define_class_under( ode_cOdeGeometry, "Box", ode_cOdePlaceable );
	ode_cOdeGeometryCapCyl	= rb_define_class_under( ode_cOdeGeometry, "Capsule", ode_cOdePlaceable );
	ode_cOdeGeometryCylinder = rb_define_class_under( ode_cOdeGeometry, "Cylinder", ode_cOdePlaceable );
	ode_cOdeGeometryRay		= rb_define_class_under( ode_cOdeGeometry, "Ray", ode_cOdePlaceable );

	ode_cOdeGeometryTransform = rb_define_class_under( ode_cOdeGeometry, "Transform", ode_cOdeGeometry );
	ode_cOdeGeometryTransformGroup = rb_define_class_under( ode_cOdeGeometry, "TransformGroup", ode_cOdeGeometry );

	ode_cOdeSpace			= rb_define_class_under( ode_mOde, "Space", ode_cOdeGeometry );
	ode_cOdeHashSpace		= rb_define_class_under( ode_mOde, "HashSpace", ode_cOdeSpace );

	ode_cOdeContact			= rb_define_class_under( ode_mOde, "Contact", rb_cObject );
	ode_cOdeSurface			= rb_define_class_under( ode_mOde, "Surface", rb_cObject );

	/* Init the other modules */
	ode_init_world();
	ode_init_space();
	ode_init_body();
	ode_init_mass();
	ode_init_joints();
	ode_init_jointGroup();
	ode_init_contact();
	ode_init_surface();
	ode_init_geometry();
	ode_init_space();
/* 	ode_init_geometry_transform(); */
 	ode_init_geometry_transform_group();
}
