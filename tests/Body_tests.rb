#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class BodyTestCase < ODE::TestCase

	X = 0
	Y = 1
	Z = 2

	def set_up
		@world = ODE::World.new
		@body = @world.createBody
	end

	def tear_down
		@world = nil
	end

	# dBodyID dBodyCreate (dWorldID);
	def test_00_create
		body = nil
		assert_nothing_raised {body = ODE::Body.new(@world) }
		assert_nothing_raised {body = @world.createBody}
		assert_instance_of( ODE::Body, body )
	end


	# const dReal * dBodyGetPosition   (dBodyID);
	def test_01_position
		position = nil

		assert_nothing_raised {position = @body.position} 
		assert_instance_of( ODE::Position, position ) 
		assert_in_delta( position[X], 0.0, 0.001 ) 
		assert_in_delta( position.x, 0.0, 0.001 ) 
		assert_in_delta( position[Y], 0.0, 0.001 ) 
		assert_in_delta( position.y, 0.0, 0.001 ) 
		assert_in_delta( position[Z], 0.0, 0.001 ) 
		assert_in_delta( position.z, 0.0, 0.001 ) 
	end

	# void dBodySetPosition   (dBodyID, dReal x, dReal y, dReal z);
	def test_02_position_eq
		assert_nothing_raised {@body.position = 1.0, 10.0, 100.0} 
		assert_nothing_raised {@body.position = [1.0, 10.0, 100.0]} 
		assert_in_delta( @body.position[X], 1.0, 0.001 ) 
		assert_in_delta( @body.position[Y], 10.0, 0.001 ) 
		assert_in_delta( @body.position[Z], 100.0, 0.001 ) 

		assert_nothing_raised {@body.position = -1.0, 16, 1e10} 
		assert_in_delta( @body.position[X], -1.0, 0.001 ) 
		assert_in_delta( @body.position[Y], 16.0, 0.001 ) 
		assert_in_delta( @body.position[Z], 1e10, 0.001 ) 
	end

	# const dReal * dBodyGetRotation   (dBodyID);
	def test_03_rotation
		rotation = nil

		assert_nothing_raised { rotation = @body.rotation } 
		assert_instance_of ODE::Rotation, rotation
	end

	# void dBodySetRotation   (dBodyID, const dMatrix3 R);
	def test_04_rotation_eq
		rotation = ODE::Rotation::new

		assert_nothing_raised { @body.rotation = rotation }
	end

	# const dReal * dBodyGetLinearVel  (dBodyID);
	def test_05_linearVelocity
		lVel = nil
		
		assert_nothing_raised { lVel = @body.linearVelocity } 
		assert_in_delta( @body.linearVelocity[X], 0.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity[Y], 0.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity[Z], 0.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity.x, 0.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity.y, 0.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity.z, 0.0, 0.001 ) 
	end

	# void dBodySetLinearVel  (dBodyID, dReal x, dReal y, dReal z);
	def test_06_linearVelocity_eq
		assert_nothing_raised { @body.linearVelocity = 1.0, 10.0, 100.0 } 
		assert_in_delta( @body.linearVelocity[X], 1.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity[Y], 10.0, 0.001 ) 
		assert_in_delta( @body.linearVelocity[Z], 100.0, 0.001 ) 

		assert_raises( NoMethodError ) { @body.linearVelocity = "broken" } 
		assert_raises( NoMethodError ) { @body.linearVelocity = 1 } 
	end

	# const dReal * dBodyGetAngularVel (dBodyID);
	def test_07_angularVelocity
		aVel = nil
		
		assert_nothing_raised { aVel = @body.angularVelocity } 
		assert_in_delta( @body.angularVelocity[X], 0.0, 0.001 ) 
		assert_in_delta( @body.angularVelocity[Y], 0.0, 0.001 ) 
		assert_in_delta( @body.angularVelocity[Z], 0.0, 0.001 ) 
	end

	# void dBodySetAngularVel (dBodyID, dReal x, dReal y, dReal z);
	def test_08_angularVelocity_eq

		assert_nothing_raised {@body.angularVelocity = [1.0, 10.0, 100.0]} 
		assert_in_delta( @body.angularVelocity[X], 1.0, 0.001 ) 
		assert_in_delta( @body.angularVelocity[Y], 10.0, 0.001 ) 
		assert_in_delta( @body.angularVelocity[Z], 100.0, 0.001 ) 

		assert_raises( NoMethodError ) { @body.angularVelocity = "broken" } 
		assert_raises( NoMethodError ) { @body.angularVelocity = 1 } 
	end

	# void dBodySetQuaternion (dBodyID, const dQuaternion q);
	def test_08a_quaternion 
		
	end

	def test_08b_quaternion_eq 
	end

	def test_09_addForce
		assert_nothing_raised {@body.addForce( 1.0, 0.0, 0.0 )} 
		assert_nothing_raised {@body.addForce( 0, 1, 0 )} 
		assert_raises( TypeError ) { @body.addForce("aString",1,2) } 
		assert_raises( ArgumentError ) { @body.addForce } 
	end

	def test_09a_getForce
		origFVec = nil
		modFVec = nil

		assert_nothing_raised { origFVec = @body.getForce }
		assert_instance_of ODE::Force, origFVec
		@body.addForce( 1,0,0 )
		assert_nothing_raised { modFVec = @body.getForce }
		assert_not_equal modFVec, origFVec
	end

	def test_10_addTorque
		assert_nothing_raised {@body.addTorque( 1.0, 0.0, 0.0 )} 
		assert_nothing_raised {@body.addTorque( 0, 1, 0 )} 
		assert_raises( TypeError ) { @body.addTorque("aString",1,2) } 
		assert_raises( ArgumentError ) { @body.addTorque } 
	end

	def test_10a_getTorque
		origTVec = nil
		modTVec = nil

		assert_nothing_raised { origTVec = @body.getTorque }
		assert_instance_of ODE::Torque, origTVec
		@body.addTorque( 1,0,0 )
		assert_nothing_raised { modTVec = @body.getTorque }
		assert_not_equal modTVec, origTVec
	end

	def test_11_addRelForce
		assert_nothing_raised {@body.addRelForce( 1.0, 0.0, 0.0 )} 
		assert_nothing_raised {@body.addRelForce( 0, 1, 0 )} 
		assert_raises( TypeError ) { @body.addRelForce("aString",1,2) } 
		assert_raises( ArgumentError ) { @body.addRelForce } 
	end

	def test_12_addRelTorque
		assert_nothing_raised { @body.addRelTorque( 1.0, 0.0, 0.0 ) } 
		assert_nothing_raised { @body.addRelTorque( 0, 1, 0 ) } 
		assert_raises( TypeError ) { @body.addRelTorque("aString",1,2) } 
		assert_raises( ArgumentError ) { @body.addRelTorque } 
	end

	def test_13_addForceAtPosition
		point	= ODE::Position::new( 1.0, 0.0, 0.0 )
		force	= ODE::Force::new( 0.2, 0.3, 0.0 )

		assert_nothing_raised {@body.addForceAtPosition( [1.0, 0.0, 0.0], [0.2, 0.3, 0.0] )} 
		assert_nothing_raised {@body.addForceAtPosition( [0, 1, 0], [2, 1, 3] )} 
		assert_nothing_raised {@body.addForceAtPosition( force, point )} 

		assert_raises( TypeError ) { @body.addForceAtPosition(["aString",1,2],[3,4,5]) } 
		assert_raises( ArgumentError ) { @body.addForceAtPosition( 1, 2, 1 ) } 
		assert_raises( ArgumentError ) { @body.addForceAtPosition } 
	end

	def test_14_addRelForceAtPosition
		point	= ODE::Position::new( 1.0, 0.0, 0.0 )
		force	= ODE::Force::new( 0.2, 0.3, 0.0 )

		assert_nothing_raised {@body.addRelForceAtPosition( [1.0, 0.0, 0.0], [0.2, 0.3, 0.0] )} 
		assert_nothing_raised {@body.addRelForceAtPosition( [0, 1, 0], [2, 1, 3] )} 
		assert_nothing_raised {@body.addRelForceAtPosition( force, point )}

		assert_raises( TypeError ) { @body.addRelForceAtPosition(["aString",1,2],[3,4,5]) } 
		assert_raises( ArgumentError ) { @body.addRelForceAtPosition( 1, 2, 1 ) } 
		assert_raises( ArgumentError ) { @body.addRelForceAtPosition } 
	end

	def test_15_addRelForceAtRelPosition
		point	= ODE::Position::new( 1.0, 0.0, 0.0 )
		force	= ODE::Force::new( 0.2, 0.3, 0.0 )

		assert_nothing_raised {@body.addRelForceAtRelPosition( [1.0, 0.0, 0.0], [0.2, 0.3, 0.0] )} 
		assert_nothing_raised {@body.addRelForceAtRelPosition( [0, 1, 0], [2, 1, 3] )} 
		assert_nothing_raised {@body.addRelForceAtRelPosition( force, point )} 

		assert_raises( TypeError ) { @body.addRelForceAtRelPosition(["aString",1,2],[3,4,5]) } 
		assert_raises( ArgumentError ) { @body.addRelForceAtRelPosition( 1, 2, 1 ) } 
		assert_raises( ArgumentError ) { @body.addRelForceAtRelPosition } 
	end

	def test_16_enabled
		rval = nil
		assert_nothing_raised {rval = @body.enabled?}
		assert rval
	end

	def test_17_enable_disable
		assert_equal true, @body.enabled?
		assert_nothing_raised {@body.disable}
		assert_equal false, @body.enabled?
		assert_nothing_raised {@body.enable}
		assert_equal true, @body.enabled?

	end
end

