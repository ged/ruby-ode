/*
 *		(>>>FILE<<<) - ODE Ruby Binding - ODE::(>>>class<<<) class
 *		$Id$
 *		Time-stamp: <27-Jul-2005 22:44:56 ged>
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
	// Constructor
	rb_define_singleton_method( ode_c(>>>POINT<<<), "new", ode_(>>>class<<<)_new, 0 );
}

