#!/usr/bin/ruby

if File.directory? "src"
	$:.unshift "src"
elsif File.directory? "../src"
	$:.unshift "../src"
end

require "test/unit"
require "ode"

class Rotation_test < Test::Unit::TestCase

	X = 0
	Y = 1
	Z = 2

	def test_00_create_no_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Rotation.new}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_01_create_three_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Rotation.new( 0.5, -0.5, 0.5 )}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_02_create_four_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Rotation.new( 1, 2, 3, 0.5 )}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_03_create_six_args
		rotation = nil
		assert_nothing_raised {rotation = ODE::Rotation.new( 1,2,3,1,2,3 )}
		assert_instance_of( ODE::Rotation, rotation ) 
	end

	def test_04_to_matrix
		rotation = ODE::Rotation.new
		matrix = nil
		assert_nothing_raised {matrix = rotation.to_matrix} 
		assert_equal( 3, matrix.length )
		assert_equal( 4, matrix[0].length )
		assert_equal( 4, matrix[1].length )
		assert_equal( 4, matrix[2].length )
		# :TODO: Test for correctness
	end

	def test_05_to_quaternion
		rotation = ODE::Rotation.new
		quaternion = nil
		assert_nothing_raised {quaternion = rotation.to_quaternion} 
		assert_equal( 4, quaternion.length )
		# :TODO: Test for correctness, too.
	end

end

