#!/usr/bin/ruby

if File.directory? "src"
	$:.unshift "src"
elsif File.directory? "../src"
	$:.unshift "../src"
end

require "test/unit"
require "ode"

class Interaction_test < Test::Unit::TestCase

	def test_00_world_scope
		body = nil

		# Test to be sure the world going out of scope doesn't hork our body
		# object, which is still in scope.
		body = ODE::World.new.createBody

		100.times { ODE::World.new ; GC.start }
		assert_nothing_raised { body.position }
	end

	def test_01_simple
	end

end

if $0 == __FILE__
    Walkit::Cli_script.new.select([Interaction_test], $*.shift)
end

