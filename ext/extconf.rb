#!/usr/bin/ruby

require "mkmf"
dir_config( "ode" )

### Print an error message and exit with an error condition
def abort( msg )
	$stderr.puts( msg )
	exit 1
end

### Version of have_library() that doesn't append (for checking a library that
### we already found, but may not be recent enough)
def have_library_no_append(lib, func="main")
  printf "checking for %s() in -l%s... ", func, lib
  STDOUT.flush

  if func && func != ""
    libs = append_library($libs, lib)
    if /mswin32|mingw/ =~ RUBY_PLATFORM
      r = try_link(<<"SRC", libs)
#include <windows.h>
#include <winsock.h>
int main() { return 0; }
int t() { #{func}(); return 0; }
SRC
      unless r
        r = try_link(<<"SRC", libs)
#include <windows.h>
#include <winsock.h>
int main() { return 0; }
int t() { void ((*p)()); p = (void ((*)()))#{func}; return 0; }
SRC
      end
    else
      r = try_link(<<"SRC", libs)
int main() { return 0; }
int t() { #{func}(); return 0; }
SRC
    end
    unless r
      print "no\n"
      return false
    end
  else
    libs = append_library($libs, lib)
  end

  # $libs = libs
  print "yes\n"
  return true
end

# Add necessary flags for compiling with ODE headers 
$CFLAGS << ' -DdDOUBLE'
$CFLAGS << ' -DBUILD_MODE=debug'
$CFLAGS << ' -DPLATFORM=linux'
$CFLAGS << ' -Wall'
$CFLAGS << ' -Wno-comment' # For Ruby's missing.h
$CFLAGS << ' -DDEBUG'

# Make sure we have the ODE library and header available
have_library( "ode", "dWorldCreate" ) or
	abort( "Can't find the ODE library." )
have_library_no_append( "ode", "dBodyGetForce" ) or
	abort( "Can't find a recent enough version of the ODE library." )
have_library_no_append( "ode", "dSpaceCollide2" ) or
	abort( "This library uses the new collision system, which your "\
		   "ODE library doesn't support." )
have_header( "ode/ode.h" ) or
	abort( "Can't find the ode/ode.h header." )

# Test for optional features (stuff in the contrib/ directory)
have_header( "ode/dCylinder.h" )
have_header( "ode/GeomTransformGroup.h" )

# Write the Makefile
create_makefile( "ode" )

# Add the 'depend' target to the end of the Makefile
File.open( "Makefile", "a" ) {|make|
	make.print <<-EOF

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM *.c > depend

EOF
}


