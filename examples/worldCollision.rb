#!/usr/bin/ruby -w
#
# worldCollision.rb
#
# $Id: worldCollision.rb,v 1.1 2003/02/04 11:12:40 deveiant Exp $
# Time-stamp: <04-Feb-2003 03:59:18 deveiant>
#
# This is a little experiment to work out Ruby-idiomish names for the collision
# methods by implementing the example traversing 'near' callback in the
# docs. It's not really meant to do anything useful, more just to work out the
# Ruby idiom for the collision system. It just drops a box onto a sphere.
#
# Authors:
#   # Michael Granger <ged@FaerieMUD.org>
#
# Copyright (c) 2002, 2003 The FaerieMUD Consortium.
#
# This work is licensed under the Creative Commons Attribution License. To
# view a copy of this license, visit
# http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
# Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
#

base = File::dirname( File::dirname(__FILE__) )
$LOAD_PATH.unshift "#{base}/lib", "#{base}/ext"

require '../utils'
include UtilityFunctions

require 'ode'
include ODE


header "Experiment: World collision -- drop a box on a sphere"

# Create the world object and set its gravity to earth-normal
$world = ODE::World::new
$world.gravity = 0, 0, -9.81

# Create a joint group to keep all the contact joints in
$contactJoints = ODE::JointGroup::new( ODE::ContactJoint, $world )

# Create a containing collision space and a ground plane, and stick the plane in
# the space.
$topLevelSpace = ODE::HashSpace::new
$ground = ODE::Geometry::Plane::new( 0, 0, 1, 0 )
$topLevelSpace << $ground

# Define a callback method that will handle the 3 kinds of ODE collison: space
# vs space (intersect), space vs. geom (recursive adjacency), and geom vs. geom
# (collide).
def nearCallback( geom1, geom2, *args )
	if geom1.isSpace? || geom2.isSpace?
		
		# Intersect the two geoms, recursing for each pair that could
		# potentially collide.
		geom1.intersectWith( geom2, &method(:nearCallback) )

		# Collide all geoms internal to the space(s)
		geom1.eachAdjacentPair( &method(:nearCallback) ) if geom1.isSpace?
		geom2.eachAdjacentPair( &method(:nearCallback) ) if geom2.isSpace?

    else
		# Generate contact points between the two geoms (maximum of 3)
		geom1.collideWith( geom2, 3 ) {|contact|
			$stderr.puts "Got %s" % contact.to_s

			# Set the surface parameters for the contact
			surface = contact.surface
			surface.mode = (Contact::Slip1|Contact::Slip2|
							Contact::SoftERP|Contact::SoftCFM|
							Contact::Approx1)
			surface.mu = ODE::Infinity
			surface.slip1 = 0.1
			surface.slip2 = 0.1
			surface.soft_erp = 0.5
			surface.soft_cfm = 0.3

			# Create a new contact joint in the joint group and attach it to the
			# bodies involved.
			contactJoint = $contactJoints.createJoint( contact )
			contactJoint.attach( contact.geom1.body, contact.geom2.body )
		}
	end
end


# Create a box collision geometry and its associated object, set its mass and
# position, and add it to the collision space.
box = ODE::Geometry::Box::new( 1, 2, 3 )
box.body = ODE::Body::new( $world )
box.body.mass = ODE::Mass::Box::new( 1, 1, 2, 3, 15 )
box.position = 0, 0, 15
$topLevelSpace << box

# Create a sphere collision geometry and its associated object, set its mass and
# position, and add it to the collision space.
sphere = ODE::Geometry::Sphere::new( 3 )
sphere.body = ODE::Body::new( $world )
sphere.body.mass = ODE::Mass::Sphere::new( 3, 1, 5 )
sphere.position = 0, 0, 3
$topLevelSpace << sphere

# Now step the world 500 times with 0.5 second granularity, calculating
# collisions for each step and reporting the objects' dispositions after each
# step.
500.times {|tick|
	$topLevelSpace.eachAdjacentPair( &method(:nearCallback) )
	$world.step( 0.5 )
	$contactJoints.empty

	$stderr.puts "In loop #{tick}: Box: #{box.position.to_s}, Sphere: #{sphere.position.to_s}"
}
