/*
 * Experiment to test return value of Joint#anchor with no attached bodies.
 *
 */

#include <ode/ode.h>
#include <stdio.h>

int main( void ) {
	dWorldID	world = dWorldCreate();
	dJointID	joint = dJointCreateBall( world, 0 );
	dVector3	pos;
	
	printf( "Create world and joint. Setting joint anchor to [4,11.18,-1.2]...\n" );
	dJointSetBallAnchor( joint, 4.0, 11.18, -1.2 );

	printf( "Done. Fetching set values...\n" );
	dJointGetBallAnchor( joint, pos );

	printf( "Anchor is at: [%0.5f, %0.5f, %0.5f]", pos[0], pos[1], pos[2] );

	dJointDestroy( joint );
	dWorldDestroy( world );

	return 0;
}


