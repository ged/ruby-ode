#!/usr/bin/ruby

require "walkit/cli_script"
require "ode"

class Mass_test < Walkit::Testclass

	X = 1
	Y = 2
	Z = 3

	@@MassArray = [ 1, 0,0,0, 1,1,1,0,0,0 ]

	def setup
		super
	end

	def teardown
		super
	end

	def test_00_new_no_args
		mass = nil
		vet { 
			assert_no_exception {mass = ODE::Mass.new}
			assert_instance_of( ODE::Mass, mass ) 
		}
	end

	def test_01_new_ten_args
		mass = nil
		vet {
			assert_no_exception {
				mass = ODE::Mass.new *@@MassArray
			}
			assert_instance_of( ODE::Mass, mass ) 
		}
	end

	def test_02_new_sphere
		mass = nil
		vet {
			assert_exception( TypeError ) {
				mass = ODE::Mass.newSphere( 1, "that" )
			}
		}
		vet {
			assert_no_exception {
				mass = ODE::Mass.newSphere( 1.0, 2.0 )
			}
			assert_instance_of( ODE::Mass, mass )
		}
		vet {
			assert_no_exception {
				mass = ODE::Mass.newSphere( 1, 2 )
			}
			assert_instance_of( ODE::Mass, mass )
		}
	end

	def test_03_new_cappedcylinder
		mass = nil
		vet {
			assert_exception( ArgumentError ) {
				mass = ODE::Mass.newCappedCylinder( 2, 4, 1, 2 )
			}
			assert_exception( TypeError ) {
				mass = ODE::Mass.newCappedCylinder( 1, "that", 1, 2 )
			}
		}
		vet {
			assert_no_exception {
				mass = ODE::Mass.newCappedCylinder( 1.0, Y, 2, 4 )
			}
			assert_instance_of( ODE::Mass, mass )
		}
		vet {
			assert_no_exception {
				mass = ODE::Mass.newCappedCylinder( 2, Z, 1, 2 )
			}
			assert_instance_of( ODE::Mass, mass )
		}
	end

	def test_04_new_box
		mass = nil
		vet {
			assert_exception( TypeError ) {
				mass = ODE::Mass.newBox( 1, "that", 2, 2 )
			}
			assert_exception( ArgumentError ) {
				mass = ODE::Mass.newBox( 1 )
			}
		}
		vet {
			assert_no_exception {
				mass = ODE::Mass.newBox( 1.0, 2.0, 2.0, 2.0 )
			}
			assert_instance_of( ODE::Mass, mass )
		}
		vet {
			assert_no_exception {
				mass = ODE::Mass.newBox( 1, 2, 2, 2 )
			}
			assert_instance_of( ODE::Mass, mass )
		}
	end

	def test_05_adjust
		mass = ODE::Mass.new *@@MassArray
		vet {
			assert_exception( TypeError ) {
				mass.adjust( "frank" )
			}
			assert_exception( ArgumentError ) {
				mass.adjust()
			}
		}

		vet {
			assert_no_exception { mass.adjust( 1.2 ) }
			assert_equal_float( mass.mass, 1.2, 0.001 )
		}

		vet {
			assert_no_exception { mass.mass = 1.5 }
			assert_equal_float( mass.mass, 1.5, 0.001 )
		}
	end

	def test_06_translate
		mass = ODE::Mass.new *@@MassArray
		vet {
			assert_exception( TypeError ) {
				mass.translate( "bob","dick","jane" )
			}
			assert_exception( ArgumentError ) {
				mass.translate( 1 )
			}
		}

		vet {
			assert_no_exception {mass.translate( 0.2,0.3,0.1 )}
		}
	end

	def test_07_rotate
		mass = ODE::Mass.new *@@MassArray
		rotation = ODE::Rotation.new( 1.2, 2.3, 0.8 )

		vet {
			assert_exception( TypeError ) {
				mass.rotate( "bob" )
			}
			assert_exception( ArgumentError ) {
				mass.translate()
			}
		}

		vet {
			assert_no_exception{ mass.rotate( rotation ) }
		}
	end
end

if $0 == __FILE__
    Walkit::Cli_script.new.select([Mass_test], $*.shift)
end

