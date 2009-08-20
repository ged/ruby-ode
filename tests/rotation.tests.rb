#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class Rotation_test < ODE::TestCase

	X = 0
	Y = 1
	Z = 2

	IdentityMatrix = [
		[1.000000, 0.000000, 0.000000, 0.000000],
		[0.000000, 1.000000, 0.000000, 0.000000],
		[0.000000, 0.000000, 1.000000, 0.000000],
		[0.000000, 0.000000, 0.000000, 1.000000]
	]

	def test_00_create_no_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Quaternion.new}
		assert_instance_of( ODE::Quaternion, rotation ) 
	end

	def test_01_create_two_args
		rotation = nil
		assert_nothing_raised {
			axis = ODE::Vector::new(0.5, -0.5, 0.5)
			angle = 1.4
			rotation = ODE::Quaternion.new( axis, angle )
		}
		assert_instance_of( ODE::Quaternion, rotation ) 
	end

	def test_02_create_four_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Quaternion.new( 1, 2, 3, 0.5 )}
		assert_instance_of( ODE::Quaternion, rotation ) 
	end

	def test_04_to_matrix
		rotation = ODE::Quaternion.new
		matrix = nil
		assert_nothing_raised {matrix = rotation.to_matrix} 
		assert_instance_of ODE::Matrix, matrix

		IdentityMatrix.each_with_index {|row,y|
			row.each_with_index {|col,x|
				assert_in_delta col, matrix[y,x], 5e-5
			}
		}
	end

	def test_05_to_vector
		rotation = ODE::Quaternion::new
		vector = nil
		assert_nothing_raised {vector = rotation.to_vector} 
		assert_instance_of ODE::Vector, vector
		assert_in_delta( 1.0, vector.length, 5e-5 )
	end

end

