#!/usr/bin/ruby -w

require '../utils'
include UtilityFunctions

require 'ode'
include ODE

# An experiment to work out Ruby-idiomish names for the collision methods by
# implementing the example traversing 'near' callback in the docs. It's not
# really meant to do anything useful, more just to work out the Ruby idiom for
# the collision system.

header "Experiment: Near callback"

def nearCallback( geom1, geom2, *args )
	if geom1.isSpace? || geom2.isSpace?
		
		# Intersect the two geoms, recursing for each pair that could
		# potentially collide.
		geom1.intersectWith( geom2, method(:nearCallback) )

		# Collide all geoms internal to the space(s)
		geom1.eachAdjacentPair( method(:nearCallback) ) if geom1.isSpace?
		geom2.eachAdjacentPair( method(:nearCallback) ) if geom2.isSpace?

    else
		# Generate contact points between the two geoms (maximum of 3)
		geom1.collideWith( geom2, 3 ) {|contact|
			# ... build ContactJoints, connect bodies with them, etc.
		}
	end
end


100.times {|tick|
	topLevelSpace.eachAdjacentPair( method(:nearCallback) )
}
