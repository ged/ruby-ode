#!/usr/bin/ruby
#
# $Id$
# Time-stamp: <28-Nov-2004 00:30:32 ged>
#
# Authors:
#   # Michael Granger <ged@FaerieMUD.org>
#
# Copyright (c) 2002, 2003, 2004 The FaerieMUD Consortium.
#
# This work is licensed under the Creative Commons Attribution License. To
# view a copy of this license, visit
# http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
# Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
#

require 'mkmf'
require 'rbconfig'
include Config

dir_config( "ode" )

### Print an error message and exit with an error condition
def abort( msg )
	$stderr.puts( msg )
	exit 1
end

### There has to be a better way to do this.



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
$CFLAGS << ' -Wall'
# Add C++ libraries.
$libs = append_library($libs, "stdc++")

# Make sure we have the ODE library and header available
have_header( "ode/ode.h" ) or
	abort( "Can't find the ode/ode.h header." )
have_library( "ode", "dWorldCreate", "ode/ode.h") or
	abort( "Can't find the ODE library." )
have_library_no_append( "ode", "dBodyGetForce") or
	abort( "Can't find a recent enough version of the ODE library." )
have_library_no_append( "ode", "dSpaceCollide2" ) or
	abort( "This library uses the new collision system, which your "\
		   "ODE library doesn't support." )
# Test for dGeomEnable (which hasn't as of this writing, been implemented yet,
# AFAICT).
if have_library_no_append( "ode", "dGeomEnable" )
	$CFLAGS << ' -DHAVE_DGEOMENABLE'
else
	puts "  Excluding geom.enable/disable (not yet in libode)"
end

# Test for optional features (stuff in the contrib/ directory)
if have_library_no_append( "ode", "dCreateGeomTransformGroup" )
	puts "  Enabling optional Geometry Transform Group extension"
	$CFLAGS << " -DHAVE_GEOM_TRANSFORM_GROUP"
end

if have_library_no_append( "ode", "dCreateCylinder")
	puts "  Enabling optional Cylinder geometry class extension"
	$CFLAGS << " -DHAVE_CYLINDER_GEOM"
end

puts "  Ruby 1.8.x allocation framework"
$CFLAGS << " -DNEW_ALLOC"
	

# Write the Makefile
create_makefile( "ode" )

# Add the 'depend' target to the end of the Makefile
File.open( "Makefile", "a" ) {|make|
	make.print <<-EOF

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM *.c > depend

EOF
}
