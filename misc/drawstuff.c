/*
 *		simulation.c - ODE Ruby Binding - ODE::Simulation class (a wrapper
 *						around ODE's drawstuff library)
 *		$Id: drawstuff.c,v 1.1 2003/02/04 11:32:11 deveiant Exp $
 *		Time-stamp: <04-Feb-2003 03:30:37 deveiant>
 *
 *		Authors:
 *		  * Michael Granger <ged@FaerieMUD.org>
 *
 *		Copyright (c) 2002, 2003 The FaerieMUD Consortium. All rights reserved.
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

#include <ruby.h>
#include <intern.h>
#include <drawstuff/drawstuff.h>

#error "This isn't finished."

/*
 * This class wraps a struct that is used to build the callback function struct
 * from drawstuff:
 *
 * 	typedef struct dsFunctions {
 * 		int version;			// put DS_VERSION here 
 * 		// version 1 data
 * 		void (*start)();		// called before sim loop starts 
 * 		void (*step) (int pause);	// called before every frame 
 * 		void (*command) (int cmd);	// called if a command key is pressed 
 * 		void (*stop)();		// called after sim loop exits 
 * 		// version 2 data 
 * 		char *path_to_textures;	// if nonzero, path to texture files 
 * 	} dsFunctions;
 * 	
 */


/* --------------------------------------------------
 *  Forward declarations
 * -------------------------------------------------- */

/* Callback data for collision system */
typedef struct {
	VALUE			callback;
	VALUE			args;
} ode_SIM_CALLBACK;


typedef struct {
	ode_CALLBACK	*start, *step, *command, *stop;
} ode_SIMULATION;


/*
 * Allocation function
 */
static ode_SIMULATION *
ode_simulation_alloc()
{
}


/*
 * GC Mark function
 */
static void
ode_simulation_gc_mark( ptr )
	 ode_SIMULATION *ptr;
{

	if ( ptr ) {
	}

	else {
	}
}


/*
 * GC Free function
 */
static void
ode_simulation_gc_free( ptr )
	 ode_SIMULATION *ptr;
{
	if ( ptr ) {
	}

	else {
	}
}


/*
 * Object validity checker. Returns the data pointer.
 */
static ode_SIMULATION *
check_simulation( self )
	 VALUE	self;
{
	Check_Type( self, T_DATA );

    if ( !IsSimulation(self) ) {
		rb_raise( rb_eTypeError, "wrong argument type %s (expected ODE::Simulation)",
				  rb_class2name(CLASS_OF( self )) );
    }
	
	return DATA_PTR( self );
}


/*
 * Fetch the data pointer and check it for sanity.
 */
static ode_SIMULATION *
get_simulation( self )
	 VALUE self;
{
	ode_SIMULATION *ptr = check_simulation( self );

	if ( !ptr )
		rb_raise( rb_eRuntimeError, "uninitialized simulation" );

	return ptr;
}



/* --------------------------------------------------
 * Class Methods
 * -------------------------------------------------- */

/*
 * allocate()
 * --
 * Allocate a new ODE::Simulation object.
 */
static VALUE
ode_simulation_s_alloc( klass )
{
	debugMsg(( "Wrapping an uninitialized ODE::Simulation pointer." ));
	return Data_Wrap_Struct( klass, ode_simulation_gc_mark, ode_simulation_gc_free, 0 );
}



/* --------------------------------------------------
 * Instance Methods
 * -------------------------------------------------- */

/*
 * initialize( )
 * --
 * Create a simulation.
 */
static VALUE
ode_simulation_init( self, world )
	 VALUE self, world;
{
	debugMsg(( "ODE::Simulation init." ));

	if ( !check_simulation(self) ) {
	}

	/* Can't initialize twice. */
	else {
		rb_raise( rb_eRuntimeError,
				  "Cannot re-initialize a simulation." );
	}

	return self;
}




/* the main() function should fill in the dsFunctions structure then
 * call this.
 */
void dsSimulationLoop (int argc, char **argv,
		       int window_width, int window_height,
		       struct dsFunctions *fn);

/* these functions display an error message then exit. they take arguments
 * in the same way as printf(), except you do not have to add a terminating
 * '\n'. Debug() tries to dump core or start the debugger.
 */
void dsError (char *msg, ...);
void dsDebug (char *msg, ...);

/* dsPrint() prints out a message. it takes arguments in the same way as
 * printf() (i.e. you must add a '\n' at the end of every line).
 */
void dsPrint (char *msg, ...);

/* set and get the camera position. xyz is the cameria position (x,y,z).
 * hpr contains heading, pitch and roll numbers in degrees. heading=0
 * points along the x axis, pitch=0 is looking towards the horizon, and
 * roll 0 is "unrotated".
 */
void dsSetViewpoint (float xyz[3], float hpr[3]);
void dsGetViewpoint (float xyz[3], float hpr[3]);

/* stop the simulation loop. calling this from within dsSimulationLoop()
 * will cause it to exit and return to the caller. it is the same as if the
 * user used the exit command. using this outside the loop will have no
 * effect.
 */
void dsStop();

/* change the way objects are drawn. these changes will apply to all further
 * dsDrawXXX() functions. the texture number must be a DS_xxx texture
 * constant. the red, green, and blue number are between 0 and 1.
 * alpha is between 0 and 1 - if alpha is not specified it's assubed to be 1.
 * the current texture is colored according to the current color.
 * at the start of each frame, the texture is reset to none and the color is
 * reset to white.
 */
void dsSetTexture (int texture_number);
void dsSetColor (float red, float green, float blue);
void dsSetColorAlpha (float red, float green, float blue, float alpha);

/* draw objects.
 *   - pos[] is the x,y,z of the center of the object.
 *   - R[] is a 3x3 rotation matrix for the object, stored by row like this:
 *        [ R11 R12 R13 0 ]
 *        [ R21 R22 R23 0 ]
 *        [ R31 R32 R33 0 ]
 *   - sides[] is an array of x,y,z side lengths.
 *   - all cylinders are aligned along the z axis.
 */
void dsDrawBox (const float pos[3], const float R[12], const float sides[3]);
void dsDrawSphere (const float pos[3], const float R[12], float radius);
void dsDrawTriangle (const float pos[3], const float R[12],
		     const float *v0, const float *v1, const float *v2);
void dsDrawCylinder (const float pos[3], const float R[12],
		     float length, float radius);
void dsDrawCappedCylinder (const float pos[3], const float R[12],
			   float length, float radius);
void dsDrawLine (const float pos1[3], const float pos2[3]);

/* these drawing functions are identical to the ones above, except they take
 * double arrays for `pos' and `R'.
 */
void dsDrawBoxD (const double pos[3], const double R[12],
		 const double sides[3]);
void dsDrawSphereD (const double pos[3], const double R[12],
		    const float radius);
void dsDrawCylinderD (const double pos[3], const double R[12],
		      float length, float radius);
void dsDrawCappedCylinderD (const double pos[3], const double R[12],
			    float length, float radius);
void dsDrawLineD (const double pos1[3], const double pos2[3]);

/* Set the drawn quality of the objects. Higher numbers are higher quality,
 * but slower to draw. This must be set before the first objects are drawn to
 * be effective.
 */
void dsSetSphereQuality (int n);		/* default = 1 */
void dsSetCappedCylinderQuality (int n);	/* default = 3 */



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

void ode_init_DrawStuff()
{
	static char
		rcsid[]		= "$Id: drawstuff.c,v 1.1 2003/02/04 11:32:11 deveiant Exp $",
		revision[]	= "$Revision: 1.1 $";

	VALUE vstr		= rb_str_new( (revision+11), strlen(revision) - 11 - 2 );

	/* Constants */
	rb_define_const( ode_cSimulation, "Version", vstr );
	rb_define_const( ode_cSimulation, "Rcsid", rb_str_new2(rcsid) );

	
}

