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
		assert_nothing_raised {rotation = ODE::Rotation.new}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_01_create_two_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Rotation.new( Math3d::Vector3::new(0.5, -0.5, 0.5), 1.4 )}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_02_create_four_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Rotation.new( 1, 2, 3, 0.5 )}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_04_to_matrix
		rotation = ODE::Rotation.new
		matrix = nil
		assert_nothing_raised {matrix = rotation.to_matrix} 
		assert_instance_of Math3d::Matrix4, matrix

		IdentityMatrix.each_with_index {|row,y|
			row.each_with_index {|col,x|
				assert_equal col, matrix[y,x]
			}
		}
	end

	def test_05_to_quaternion
		rotation = ODE::Rotation.new
		quaternion = nil
		assert_nothing_raised {quaternion = rotation.to_quaternion} 
		assert_instance_of Math3d::Vector4, quaternion
		assert_in_delta( 1.0, quaternion.length, 0.0005 )
	end

end

