#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class Mass_test < ODE::TestCase

	X = 1
	Y = 2
	Z = 3

	def test_00_new_no_args
		mass = nil
		assert_nothing_raised {mass = ODE::Mass.new}
		assert_instance_of( ODE::Mass, mass ) 
	end

	def test_02_new_sphere
		mass = nil
		assert_raises( TypeError ) {
			mass = ODE::Mass::Sphere::new( 1, "that" )
		}
		assert_nothing_raised {
			mass = ODE::Mass::Sphere::new( 1.0, 2.0 )
		}
		assert_kind_of( ODE::Mass, mass )
		assert_instance_of( ODE::Mass::Sphere, mass )
		assert_nothing_raised {
			mass = ODE::Mass::Sphere::new( 1, 2 )
		}
		assert_kind_of( ODE::Mass, mass )
		assert_instance_of( ODE::Mass::Sphere, mass )
	end

	def test_03_new_capsule
		mass = nil
		assert_raises( ArgumentError ) {
			mass = ODE::Mass::Capsule::new( 2, 4, 1, 2 )
		}
		assert_raises( TypeError ) {
			mass = ODE::Mass::Capsule::new( 1, "that", 1, 2 )
		}
		assert_nothing_raised {
			mass = ODE::Mass::Capsule::new( 1.0, Y, 2, 4 )
		}
		assert_kind_of( ODE::Mass, mass )
		assert_instance_of( ODE::Mass::Capsule, mass )
		assert_nothing_raised {
			mass = ODE::Mass::Capsule::new( 2, Z, 1, 2 )
		}
		assert_kind_of( ODE::Mass, mass )
		assert_instance_of( ODE::Mass::Capsule, mass )
	end

	def test_04_new_box
		mass = nil
		assert_raises( TypeError ) {
			mass = ODE::Mass::Box::new( 1, "that", 2, 2 )
		}
		assert_raises( ArgumentError ) {
			mass = ODE::Mass::Box::new( 1 )
		}
		assert_nothing_raised {
			mass = ODE::Mass::Box::new( 1.0, 2.0, 2.0, 2.0 )
		}
		assert_kind_of( ODE::Mass, mass )
		assert_instance_of( ODE::Mass::Box, mass )
		assert_nothing_raised {
			mass = ODE::Mass::Box::new( 1, 2, 2, 2 )
		}
		assert_kind_of( ODE::Mass, mass )
		assert_instance_of( ODE::Mass::Box, mass )
	end

	def test_05_adjust
		mass = ODE::Mass.new( 3 )
		assert_raises( TypeError ) {
			mass.adjust( "frank" )
		}
		assert_raises( ArgumentError ) {
			mass.adjust()
		}

		assert_nothing_raised { mass.adjust( 1.2 ) }
		assert_in_delta( mass.totalMass, 1.2, 0.001 )

		assert_nothing_raised { mass.totalMass = 1.5 }
		assert_in_delta( mass.totalMass, 1.5, 0.001 )
	end

	def test_06_translate
		mass = ODE::Mass.new( 3 )
		assert_raises( TypeError ) { mass.translate( "bob","dick","jane" ) }
		assert_raises( ArgumentError ) { mass.translate( 1 ) }

		assert_nothing_raised {mass.translate( 0.2,0.3,0.1 )}
	end

	def test_07_rotate
		mass = ODE::Mass.new( 3 )
		rotation = ODE::Quaternion::new( 61.2, 2.3, 0.8, 23 )

		assert_raises( TypeError ) { mass.rotate( "bob" ) }
		assert_raises( ArgumentError ) { mass.translate() }

		assert_nothing_raised { mass.rotate( rotation ) }
	end
end
