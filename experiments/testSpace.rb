#!/usr/bin/ruby

$LOAD_PATH.unshift ".", "ext", "lib"
require 'utils'
include UtilityFunctions

header "Space/Geom tester."

message "Requiring 'ode'\n"
require 'ode'

message "Creating a top-level ODE::Space..."
topSpace = ODE::Space::new
message "done.\n"

message "Creating a heirarchy of ODE::Space objects beneath the top space "
message "...1"
spaces = [ ODE::Space::new(topSpace) ]
9.times do |i|
	message "...#{i+2}"
	spaces.push ODE::Space::new( spaces.last )
end
message spaces.inspect + "\n"

message "Creating an ODE::Geometry::Box in the space..."
box = ODE::Geometry::Box::new( 1,2,3, spaces.last )
message box.inspect + "\n"

message "Running GC: "
ObjectSpace::garbage_collect
message "done." + "\n"

message "Removing this context's reference to the top-level Space..."
topSpace = nil
message "Done. Running GC again: "
ObjectSpace::garbage_collect
message "done." + "\n"

message "Exiting.\n\n"


