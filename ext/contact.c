/*
 *		contact.c - ODE Ruby Binding - ODE::Contact class
 *		$Id: contact.c,v 1.1 2002/11/23 23:08:10 deveiant Exp $
 *		Time-stamp: <18-Nov-2002 23:54:19 deveiant>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2002 The FaerieMUD Consortium. All rights reserved.
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

/* This class wraps the dContact struct:
 * --
 *  typedef struct dContact {
 *    dSurfaceParameters surface;
 *    dContactGeom geom;
 *    dVector3 fdir1;
 *  } dContact;
 */


/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */

void ode_contact_gc_mark( ode_CONTACT * );
void ode_contact_gc_free( ode_CONTACT * );
VALUE ode_contact_new_from_cgeom( VALUE, dContactGeom * );

/* --------------------------------------------------
 * Macros
 * -------------------------------------------------- */

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
 * Class Methods
 * -------------------------------------------------- */

/*
 * ODE::Contact::new()
 * --
 * Create a contact (collision) object.
 */
VALUE
ode_contact_alloc( klass )
	 VALUE klass;
{
	dContactGeom	*cgeom;

	cgeom = ALLOCA_N( dContactGeom, 1 );
	MEMZERO( cgeom, dContactGeom, 1 );

	return ode_contact_new_from_cgeom( klass, cgeom );
}


/* Internal initializer */
VALUE
ode_contact_new_from_cgeom( klass, cgeom )
	 VALUE			klass;
	 dContactGeom	*cgeom;
{
	ode_CONTACT			*contactStruct;
	VALUE				contactObj, surfaceObj;
	dSurfaceParameters	*surface;

	/* Create the ruby object wrapped around the top-level struct */
	contactObj = Data_Make_Struct( klass, ode_CONTACT,
								   ode_contact_gc_mark, 
								   ode_contact_gc_free,
								   contactStruct );
	contactStruct->object = contactObj;

	/* Create a new surface object and add that to the new object's surface
	   member. */
	surfaceObj = rb_class_new_instance( ode_cOdeSurface, 0, 0 );
	GetSurface( surfaceObj, surface );
	contactStruct->surface = surfaceObj;

	/* Allocate and fill the mid-level struct */
	contactStruct->contact = ALLOC( dContact );
	contactStruct->contact->surface = *surface;
	contactStruct->contact->fdir1[0] = 0.f;
	contactStruct->contact->fdir1[1] = 0.f;
	contactStruct->contact->fdir1[2] = 0.f;

	/* Allocate a new bottom-level struct and copy the data over from the one on
	   the stack. */
	memcpy( &contactStruct->contact->geom, cgeom, sizeof(dContactGeom) );

	return contactObj;
}


VALUE
ode_contact_init( argc, argv, self )
	 int	argc;
	 VALUE	*argv, self;
{
	rb_notimplement();
	return self;
}


/* --------------------------------------------------
 * Memory-management functions
 * -------------------------------------------------- */

/*
 * GC mark function
 */
void
ode_contact_gc_mark( contactStruct )
	 ode_CONTACT	*contactStruct;
{
	if ( contactStruct->surface ) rb_gc_mark( contactStruct->surface );
	if ( contactStruct->geometry1 ) rb_gc_mark( contactStruct->geometry1 );
	if ( contactStruct->geometry2 ) rb_gc_mark( contactStruct->geometry2 );
}


/* 
 * GC free function
 */
void
ode_contact_gc_free( contactStruct )
	 ode_CONTACT	*contactStruct;
{
	free( contactStruct->contact );

	contactStruct->surface = 0;
	contactStruct->geometry1 = 0;
	contactStruct->geometry2 = 0;

	free( contactStruct );
}

/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_contact()
{
	static char
		rcsid[]		= "$Id: contact.c,v 1.1 2002/11/23 23:08:10 deveiant Exp $",
		revision[]	= "$Revision: 1.1 $";

	VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );

	// Constants
	rb_define_const( ode_cOdeContact, "Version", vstr );
	rb_define_const( ode_cOdeContact, "Rcsid", rb_str_new2(rcsid) );

	// Surface mode flag constants
	rb_define_const( ode_cOdeContact, "Mu2",		INT2FIX(0x001) );
	rb_define_const( ode_cOdeContact, "FDir1",		INT2FIX(0x002) );
	rb_define_const( ode_cOdeContact, "UseFrictionDirection", INT2FIX(0x002) );
	rb_define_const( ode_cOdeContact, "Bounce",		INT2FIX(0x004) );
	rb_define_const( ode_cOdeContact, "SoftERP",	INT2FIX(0x008) );
	rb_define_const( ode_cOdeContact, "SoftCFM",	INT2FIX(0x010) );
	rb_define_const( ode_cOdeContact, "Motion1",	INT2FIX(0x020) );
	rb_define_const( ode_cOdeContact, "Motion2",	INT2FIX(0x040) );
	rb_define_const( ode_cOdeContact, "Slip1",		INT2FIX(0x080) );
	rb_define_const( ode_cOdeContact, "Slip2",		INT2FIX(0x100) );

	rb_define_const( ode_cOdeContact, "Approx0",	INT2FIX(0x0000) );
	rb_define_const( ode_cOdeContact, "PyramidFrictionNeither", INT2FIX(0x0000) );
	rb_define_const( ode_cOdeContact, "Approx1_1",	INT2FIX(0x1000) );
	rb_define_const( ode_cOdeContact, "PyramidFriction1", INT2FIX(0x1000) );
	rb_define_const( ode_cOdeContact, "Approx1_2",	INT2FIX(0x2000) );
	rb_define_const( ode_cOdeContact, "PyramidFriction2", INT2FIX(0x2000) );
	rb_define_const( ode_cOdeContact, "Approx1",	INT2FIX(0x3000) );
	rb_define_const( ode_cOdeContact, "PyramidFrictionBoth", INT2FIX(0x3000) );

	// Constructor
	rb_define_singleton_method( ode_cOdeContact, "allocate", ode_contact_alloc, 0 );

	// Initializer
	rb_define_method( ode_cOdeContact, "initialize", ode_contact_init, -1 );

	// Accessors
	
	// :TODO: Call fdir1 'frictionDirection'

	
}

