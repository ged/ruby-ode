/*
 *		(>>>FILE<<<) - ODE Ruby Binding - ODE::(>>>class<<<) class
 *		$Id: TEMPLATE.c.tpl,v 1.2 2002/11/16 06:54:33 deveiant Exp $
 *		Time-stamp: <15-Nov-2002 23:54:11 deveiant>
 *
 *		Authors:
 *		  * (>>>USER_NAME<<<) <(>>>AUTHOR<<<)>
 *
 *		Copyright (c) (>>>YEAR<<<) The FaerieMUD Consortium. All rights reserved.
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


/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Macros
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Memory-management functions
 * -------------------------------------------------- */



/* --------------------------------------------------
 *	Instance Methods
 * -------------------------------------------------- */



/* --------------------------------------------------
 * Initializer
 * -------------------------------------------------- */

void ode_init_(>>>class<<<)()
{
	static char
		rcsid[]		= "$Id: TEMPLATE.c.tpl,v 1.2 2002/11/16 06:54:33 deveiant Exp $",
		revision[]	= "$Revision: 1.2 $";

	VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );

	// Constants
	rb_define_const( ode_c(>>>1<<<), "Version", vstr );
	rb_define_const( ode_c(>>>0<<<), "Rcsid", rb_str_new2(rcsid) );

	// Constructor
	rb_define_singleton_method( ode_c(>>>POINT<<<), "new", ode_(>>>class<<<)_new, 0 );
	
}

