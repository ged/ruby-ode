#!/usr/bin/ruby -w

$: << "src"

require "ode"

puts "Creating world."
world = ODE::World.new
puts "Creating a ball joint in #{world}."
join = ODE::BallJoint.new(world)
puts "Destroying the world."
world = nil
puts "Starting garbage collection."
GC.start
puts "Destroying the ball joint."
joint = nil
puts "Starting garbage collection."
GC.start

