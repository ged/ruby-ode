#!/usr/bin/ruby

require "walkit/cli_script"
require "ode"

class Rotation_test < Walkit::Testclass

	X = 0
	Y = 1
	Z = 2

	def setup
		super
	end

	def teardown
		super
	end

	def test_00_create_no_args
		rotation = nil
		vet { 
			assert_no_exception {rotation = ODE::Rotation.new}
			assert_instance_of( ODE::Rotation, rotation ) 
		}
	end

	def test_01_create_three_args
		rotation = nil
		vet { 
			assert_no_exception {rotation = ODE::Rotation.new( 0.5, -0.5, 0.5 )}
			assert_instance_of( ODE::Rotation, rotation ) 
		}
	end

	def test_02_create_four_args
		rotation = nil
		vet { 
			assert_no_exception {rotation = ODE::Rotation.new( 1, 2, 3, 0.5 )}
			assert_instance_of( ODE::Rotation, rotation ) 
		}
	end

	def test_03_create_six_args
		rotation = nil
		vet { 
			assert_no_exception {rotation = ODE::Rotation.new( 1,2,3,1,2,3 )}
			assert_instance_of( ODE::Rotation, rotation ) 
		}
	end

	def test_04_to_matrix
		rotation = ODE::Rotation.new
		matrix = nil
		vet { assert_no_exception {matrix = rotation.to_matrix} }
		vet {
			assert_equals( 3, matrix.length )
			assert_equals( 4, matrix[0].length )
			assert_equals( 4, matrix[1].length )
			assert_equals( 4, matrix[2].length )
			# :TODO: Test for correctness
		}
	end

	def test_05_to_quaternion
		rotation = ODE::Rotation.new
		quaternion = nil
		vet { assert_no_exception {quaternion = rotation.to_quaternion} }
		vet {
			assert_equals( 4, quaternion.length )
			# :TODO: Test for correctness, too.
		}
	end

end

if $0 == __FILE__
    Walkit::Cli_script.new.select([Rotation_test], $*.shift)
end

