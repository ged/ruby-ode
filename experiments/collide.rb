#!/usr/bin/ruby -w

$LOAD_PATH.unshift( "ext", "lib" )
require 'ode'

require '../utils'
include UtilityFunctions

# A very small test of the collision system.

header "Experiment: Collision!"

# Globals
$topSpace = ODE::HashSpace::new
$world = ODE::World::new
$world.gravity = 0, 0, -0.5
$world.cfm = 1e-5
$contacts = ODE::JointGroup::new( ODE::ContactJoint, $world )

# Defaults
DefaultPosition = ODE::Position::new( 0, 0, 0 )

# Classes
class Box < ODE::Body

	def initialize( world, name, length=2, width=3, height=4, position=DefaultPosition )
		super( world )

		@name = name
		@space = ODE::Space::new( $topSpace )
		@geom = ODE::Geometry::Box::new( 2, 3, 4, @space )

		self.position = ODE::Position::new( 0, 0, 0 )
		self.mass = ODE::Mass::newBox( length, width, height, 1.5 )
	end

	attr_accessor :space, :geom

end


# Main program

message "Creating ground plane..."
ground = ODE::Geometry::Plane::new( 0, 0, 1, 0 )
$topSpace << ground
message "done."

message "Creating first collision geom at the origin..."
box1 = Box::new( $world, "Box1" )
$topSpace << box1.geom
box2 = Box::new( $world, "Box2", 3, 4, 2, ODE::Position::new(0, 10, 0) )
$topSpace << box2.geom


def nearCallback( geom1, geom2, *args )
	message "Near callback for %s vs. %s" % [ geom1.inspect, geom2.inspect ]

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
			message "Contact: %s x %s"

			contact.surface.mu = ODE::Infinity
			contact.surface.mu2 = 0
			contact.surface.bounce = 0.5
			contact.surface.bounceVelocity = 0.1

			cj = $contacts.createJoint( $world, contact )
			cj.attach( geom1.body, geom2.body );
		}
	end
end


5000.times {|tick|
	topLevelSpace.eachAdjacentPair( method(:nearCallback) )
	world.step( 0.05 )

	$contacts.empty
	message "Locations: %s  %s" % [ box1.position, box2.position ]
}

