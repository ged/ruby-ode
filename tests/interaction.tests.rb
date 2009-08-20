#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class Interaction_tests < ODE::TestCase

	def test_00_world_scope
		body = nil

		# Test to be sure the world going out of scope doesn't hork our body
		# object, which is still in scope.
		body = ODE::World.new.createBody

		100.times { ODE::World.new ; GC.start }
		assert_nothing_raised { body.position }
	end

	def test_01_simple
		assert true
	end

end


