#!/usr/bin/ruby

$LOAD_PATH.unshift "lib", "ext"

require '../utils'
include UtilityFunctions

require 'ode'

# Experiment to test containment and memory management in collision geometries.

def newSpace( container=nil )
	klass = [ ODE::HashSpace, ODE::Space ][ rand(2).ceil.to_i ]

	if container
		klass.new( container )
	else
		klass.new
	end
end

header "Experiment: 99tiers -- test nesting and memory-management of ODE::Spaces"
topSpace = newSpace()

5.times {
	sp = newSpace( topSpace )

	10.times {
		subsp = newSpace( sp )
		
		20.times {
			subsubsp = newSpace( subsp )
		}
	}
}

puts "Space: " + topSpace.inspect
			
