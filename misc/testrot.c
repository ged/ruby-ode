
#include <ode/ode.h>
#include <stdio.h>


int
rindex( i, j )
	 int i, j;
{
  return ((i) * 4 + j );
}

int
main( void )
{
  dMatrix3	rotation;
  int		x;

  dRSetIdentity( rotation );

  for ( x = 0; x <= 2; x++ ) {
	printf( "%d: %0.3f %0.3f %0.3f %0.3f\n",
			x,
			rotation[ rindex(x,0) ],
			rotation[ rindex(x,1) ],
			rotation[ rindex(x,2) ],
			rotation[ rindex(x,3) ] );
  }

  return 0;
}


