#!/usr/bin/ruby -w

$LOAD_PATH.unshift "lib", "ext", "../lib", "../ext"
require 'ode'

require 'utils'
include UtilityFunctions

# Function pointer trickery test.

header "Experiment: Function pointer trickery in ParameterizedJoint methods."

message "Creating the World..."
w = ODE::World::new
message "done.\n"

message "Creating a Hinge2Joint..."
j = ODE::Hinge2Joint::new( w )
message "done.\n"

message "Getting vel for the Hinge2Joint..."
v = j.vel
puts "Vel = #{v}"
message "done.\n\n"

message "Getting vel2 for the Hinge2Joint..."
v = j.vel2
puts "Vel2 = #{v}"
message "done.\n\n"

