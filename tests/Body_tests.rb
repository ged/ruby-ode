#!/usr/bin/ruby

require "walkit/cli_script"
require "ode"

class Body_test < Walkit::Testclass

	X = 0
	Y = 1
	Z = 2

	def setup
		@world = ODE::World.new
	end

	def teardown
		@world = nil
	end

	def test_00_create
		body = nil
		vet { assert_no_exception {body = ODE::Body.new(@world) } }
		vet { assert_no_exception {body = @world.createBody} }
		vet { assert_instance_of( ODE::Body, body ) }
	end

	def test_01_position
		body = @world.createBody
		position = nil

		vet { assert_no_exception {position = body.position} }
		vet { assert_instance_of( ::Array, position ) }
		vet { assert_equal_float( position[X], 0.0, 0.001 ) }
		vet { assert_equal_float( position[Y], 0.0, 0.001 ) }
		vet { assert_equal_float( position[Z], 0.0, 0.001 ) }
	end

	def test_02_position=
		body = @world.createBody

		vet { assert_no_exception {body.position = [1.0, 10.0, 100.0]} }
		vet { assert_equal_float( body.position[X], 1.0, 0.001 ) }
		vet { assert_equal_float( body.position[Y], 10.0, 0.001 ) }
		vet { assert_equal_float( body.position[Z], 100.0, 0.001 ) }

		vet { assert_no_exception {body.position = -1.0, 16, 1e10} }
		vet { assert_equal_float( body.position[X], -1.0, 0.001 ) }
		vet { assert_equal_float( body.position[Y], 16.0, 0.001 ) }
		vet { assert_equal_float( body.position[Z], 1e10, 0.001 ) }
	end

	def test_03_rotation
		body = @world.createBody
		rotation = nil

		vet { assert_no_exception {rotation = body.rotation} }
		vet { assert_instance_of( ODE::Rotation, rotation ) }
	end

	def test_04_rotation=
		body = @world.createBody
		rotation = ODE::Rotation.new( 1.0, 1.0, 1.0 );

		vet { assert_no_exception {body.rotation = rotation} }
	end

	def test_05_linearVelocity
		body = @world.createBody
		lVel = nil
		
		vet { assert_no_exception { lVel = body.linearVelocity } }
		vet { assert_equal_float( body.linearVelocity[X], 0.0, 0.001 ) }
		vet { assert_equal_float( body.linearVelocity[Y], 0.0, 0.001 ) }
		vet { assert_equal_float( body.linearVelocity[Z], 0.0, 0.001 ) }
	end

	def test_06_linearVelocity=
		body = @world.createBody

		vet { assert_no_exception {body.linearVelocity = [1.0, 10.0, 100.0]} }
		vet { assert_equal_float( body.linearVelocity[X], 1.0, 0.001 ) }
		vet { assert_equal_float( body.linearVelocity[Y], 10.0, 0.001 ) }
		vet { assert_equal_float( body.linearVelocity[Z], 100.0, 0.001 ) }

		vet { assert_exception( TypeError ) { body.linearVelocity = "broken" } }
		vet { assert_exception( TypeError ) { body.linearVelocity = 1 } }
	end

	def test_07_angularVelocity
		body = @world.createBody
		aVel = nil
		
		vet { assert_no_exception { aVel = body.angularVelocity } }
		vet { assert_equal_float( body.angularVelocity[X], 0.0, 0.001 ) }
		vet { assert_equal_float( body.angularVelocity[Y], 0.0, 0.001 ) }
		vet { assert_equal_float( body.angularVelocity[Z], 0.0, 0.001 ) }
	end

	def test_08_angularVelocity=
		body = @world.createBody

		vet { assert_no_exception {body.angularVelocity = [1.0, 10.0, 100.0]} }
		vet { assert_equal_float( body.angularVelocity[X], 1.0, 0.001 ) }
		vet { assert_equal_float( body.angularVelocity[Y], 10.0, 0.001 ) }
		vet { assert_equal_float( body.angularVelocity[Z], 100.0, 0.001 ) }

		vet { assert_exception( TypeError ) { body.angularVelocity = "broken" } }
		vet { assert_exception( TypeError ) { body.angularVelocity = 1 } }
	end

	def test_09_addForce
		body = @world.createBody

		vet { assert_no_exception {body.addForce( 1.0, 0.0, 0.0 )} }
		vet { assert_no_exception {body.addForce( 0, 1, 0 )} }
		vet { assert_exception( TypeError ) { body.addForce("aString",1,2) } }
		vet { assert_exception( ArgumentError ) { body.addForce } }
	end

	def test_10_addTorque
		body = @world.createBody

		vet { assert_no_exception {body.addTorque( 1.0, 0.0, 0.0 )} }
		vet { assert_no_exception {body.addTorque( 0, 1, 0 )} }
		vet { assert_exception( TypeError ) { body.addTorque("aString",1,2) } }
		vet { assert_exception( ArgumentError ) { body.addTorque } }
	end

	def test_11_addRelForce
		body = @world.createBody

		vet { assert_no_exception {body.addRelForce( 1.0, 0.0, 0.0 )} }
		vet { assert_no_exception {body.addRelForce( 0, 1, 0 )} }
		vet { assert_exception( TypeError ) { body.addRelForce("aString",1,2) } }
		vet { assert_exception( ArgumentError ) { body.addRelForce } }
	end

	def test_12_addRelTorque
		body = @world.createBody

		vet { assert_no_exception {body.addRelTorque( 1.0, 0.0, 0.0 )} }
		vet { assert_no_exception {body.addRelTorque( 0, 1, 0 )} }
		vet { assert_exception( TypeError ) { body.addRelTorque("aString",1,2) } }
		vet { assert_exception( ArgumentError ) { body.addRelTorque } }
	end

	def test_13_addForceAtPosition
		body = @world.createBody

		vet { assert_no_exception {body.addForceAtPosition( 1.0, 0.0, 0.0, 0.2, 0.3, 0.0 )} }
		vet { assert_no_exception {body.addForceAtPosition( 0, 1, 0, 2, 1, 3 )} }
		vet { assert_exception( TypeError ) { body.addForceAtPosition("aString",1,2,3,4,5) } }
		vet { assert_exception( ArgumentError ) { body.addForceAtPosition( 1, 2, 1 ) } }
		vet { assert_exception( ArgumentError ) { body.addForceAtPosition } }
	end

	def test_14_addRelForceAtPosition
		body = @world.createBody

		vet { assert_no_exception {body.addRelForceAtPosition( 1.0, 0.0, 0.0, 0.2, 0.3, 0.0 )} }
		vet { assert_no_exception {body.addRelForceAtPosition( 0, 1, 0, 2, 1, 3 )} }
		vet { assert_exception( TypeError ) { body.addRelForceAtPosition("aString",1,2,3,4,5) } }
		vet { assert_exception( ArgumentError ) { body.addRelForceAtPosition( 1, 2, 1 ) } }
		vet { assert_exception( ArgumentError ) { body.addRelForceAtPosition } }
	end

	def test_15_addRelForceAtRelPosition
		body = @world.createBody

		vet { assert_no_exception {body.addRelForceAtRelPosition( 1.0, 0.0, 0.0, 0.2, 0.3, 0.0 )} }
		vet { assert_no_exception {body.addRelForceAtRelPosition( 0, 1, 0, 2, 1, 3 )} }
		vet { assert_exception( TypeError ) { body.addRelForceAtRelPosition("aString",1,2,3,4,5) } }
		vet { assert_exception( ArgumentError ) { body.addRelForceAtRelPosition( 1, 2, 1 ) } }
		vet { assert_exception( ArgumentError ) { body.addRelForceAtRelPosition } }
	end


end

if $0 == __FILE__
    Walkit::Cli_script.new.select([Body_test], $*.shift)
end

