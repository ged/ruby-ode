#!/usr/bin/ruby

$: << "src"
require "ode"

r = ODE::Rotation.new( 0.2, 0.4, 2.3 )
m = ODE::Mass.new

m.adjust( 1.5 )
m.translate( 1.2, 1.5, 2.0 )
m.rotate( r )

puts "Passed."
