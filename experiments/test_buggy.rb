#!/usr/bin/ruby -w

require '../utils'
include UtilityFunctions

require 'ode'
include ODE

# This is a port of the test_buggy ODE example, just to see how things
# would/will work.

header "Experiment: Port of the test_buggy example, without graphics"

class Car
	include ODE

	Chassis = Struct::new( "CarChassis", :body, :geometry )
	Wheel = Struct::new( "CarWheel", :body, :geometry, :hinge )
	DefaultPosition = Position::new( 0, 0, 0.5 )

	def initialize( length=0.7, width=0.5, height=0.2, wheelRadius=0.24,
				    chassisMass=1, wheelMass=0.2, position=DefaultPosition )

		@space = SimpleSpace::new

		@chassis = Chassis::new
		@chassis.body = Body::new
		@chassis.body.position = position
		@chassis.body.mass = Mass::newBox( length, width, height, chassisMass )

		@chassis.geometry = Geometry::Box::new( length, width, height, @space )
		@chassis.geometry.body = @chassis.body

		@wheels = (0..2).to_a.collect {|i|
			# Create the wheel body
			body = Body::new
			body.quaternion = 1, 0, 0, (ODE::PI*1.5)
			body.mass = Mass::newSphere( 1, wheelRadius, wheelMass )

			# Position each wheel appropriately
			case i
			when 1
				body.position = (0.5 * length), 0, (position.z - height * 0.5);

			when 2
				body.position = (-0.5 * length), (width * 0.5), (position.z - height * 0.5);

			when 3
				body.position = (-0.5 * length), (width * -0.5), (position.z - height * 0.5);
			end

			# Create the wheel's collision geometry
			geom = Geometry::Sphere::new( wheelRadius, @space )
			geom.body = body

			# Create and configure a joint, and then use it to connect the wheel
			# to the chassis
			hinge = Hinge2Joint::new
			hinge.attach( @chassis, body )
			hinge.anchor = body.position.x, body.position.y, body.position.z
			hinge.axis1 = 0, 0, 1
			hinge.axis2 = 0, 1, 0
			hinge.suspensionERP = 0.4
			hinge.suspensionCFM = 0.8
			
			Wheel::new( body, geom, hinge )
		}

		# Lock rear wheels along the steering axis
		@wheels[1..2].each {|wheel|
			wheel.hinge.loStop = 0
			wheel.hinge.hiStop = 0
		}

	end

	######
	public
	######

	# The car's top-level geometry space
	attr_reader :space

	# The car's chassis struct
	attr_reader :chassis

	# The car's wheel struct array
	attr_reader :wheels


	### Drive the car
	def drive( steering, speed )
		self.steer( steering )
		self.throttle( speed )
	end
	

	### Steer the car's front wheel
	def steer( steering )
		vel = steering - @wheel[0].hinge.angle1
		if ( vel > 0.1 ) then vel = 0.1 end
		if ( vel < -0.1 ) then vel = -0.1 end

		vel *= 10
		@wheels[0].hinge.vel = vel
		@wheels[0].hinge.fMax = 0.2
		@wheels[0].hinge.loStop = -0.75
		@wheels[0].hinge.hiStop = 0.75
		@wheels[0].hinge.fudgeFactor = 0.1
	end		


	### Adjust the speed of the car's engine by the car's speed.
	def throttle( speed )
		@wheels[0].hinge.vel2 = -speed
		@wheels[0].hinge.fMax2 = 0.1
	end


	### Add the car to the specified object (either an ODE::World or an
	### ODE::Space).
	def >>( container )
		case container
		when ODE::World
			self.addToWorld( container )

		when ODE::Space
			self.addToSpace( container )

		else
			raise "Cannot add the car to a %s" % container.class.name
		end
	end

	### Add the car to the specified world.
	def addToWorld( world )
		world << @chassis.body << @wheels.collect {|wheel| wheel.body}
	end

	### Remove the car from the specified world.
	def removeFromWorld( world )
		world.removeBodies( @wheels.collect {|wheel| wheel.body} )
		world.removeBodies( @chassis.body )
	end

	### Add the car to the specified collision space
	def addToSpace( space )
		space << @space
	end

	### Remove the car from the specified collision space
	def removeFromSpace( space )
		space.removeGeometry( @space )
	end

end


class Ramp

	include ODE

	def initialize( length=2, width1.5, height=1 )
		@body = Body::new
		@geometry = Geometry::Box::new( length, width, height )
		@geometry.body = @body
		@geometry.position = 2, 0, -0.34
		@geometry.rotation = 0, 1, 0, -0.15
	end

	# The physical body of the ramp
	attr_reader :body

	# The collision geometry of the ramp
	attr_reader :ramp


	### Add the car to the specified object (either an ODE::World or an
	### ODE::Space).
	def >>( container )
		case container
		when ODE::World
			self.addToWorld( container )

		when ODE::Space
			self.addToSpace( container )

		else
			raise "Cannot add the ramp to a %s" % container.class.name
		end

		return self
	end

	### Add the car to the specified world.
	def addToWorld( world )
		world << @body
	end

	### Remove the car from the specified world.
	def removeFromWorld( world )
		world.removeBodies( @body )
	end

	### Add the car to the specified collision space
	def addToSpace( space )
		space << @geometry
	end

	### Remove the car from the specified collision space
	def removeFromSpace( space )
		space.removeGeometry( @geometry )
	end

end



#####################################################################
###	M A I N   P R O G R A M
#####################################################################

# Define some categories for culling collisions
module Category
	MOVING		= (1 << 0)
	STATIONARY	= (1 << 1)
end

# Create the simulation world
world = World::new
world.gravity = 0, 0, -0.5

# Create a joint group for contact joints
contactJoints = JointGroup::new

# Create the collision space and add the ground plane
space = HashSpace::new
ground = Geometry::Plane::new( 0, 0, 1, 0 )
space << ground
ground.categoryMask = Category::STATIONARY
ground.collideMask = Category::MOVING

# Create the car and add it to the world and the collision space
car = Car::new
car >> world >> space
car.space.categoryMask = Category::MOVING
car.space.collideMask = Category::STATIONARY

# Create the ramp
ramp = Ramp::new
ramp >> world >> space
ramp.geometry.categoryMask = Category::STATIONARY
ramp.geometry.collideMask = Category::MOVING


quitFlag = pauseFlag = false
until ( quitFlag )

	unless ( pauseFlag )

		# Update the car's parts
		car.drive( steer, throttle )

		# Set up the collision function
		space.eachNearPair {|geom1, geom2|
			geom1.collideWith( geom2, 10 ).each {|contact|
				contact.surface.mode = (Contact::Slip1|Contact::Slip2|
										Contact::SoftERP|Contact::SoftCFM|
										Contact::Approx1)
				contact.surface.mu = ODE::Infinity
				contact.surface.slip1 = 0.1
				contact.surface.slip2 = 0.1
				contact.surface.soft_erp = 0.5
				contact.surface.soft_cfm = 0.3

				contactJoint = ContactJoint::new( world, contactJoints, contact )
				contactJoint.attach( contact.geom1.body, contact.geom2.body )
			}
		}

		# Step the world and remove any contact joints created in this cycle
		world.step( 0.05 )
		contactJoints.empty
	end


	### Drawing routines go here when they exist.

end


