#!/usr/bin/ruby

if File.directory? "src"
	$:.unshift "src"
elsif File.directory? "../src"
	$:.unshift "../src"
end

require "test/unit"
require "ode"

class World_test < Test::Unit::TestCase

	def set_up
		@world = ODE::World.new
	end

	def tear_down
		@world = nil
	end

	def test_00_new
		assert_instance_of( ODE::World, @world )
	end

	def test_01_gravity_get
		gravity = nil

		assert_nothing_raised { gravity = @world.gravity } 
		assert_instance_of( Array, gravity ) 
		assert_in_delta( gravity[0], 0.0, 0.01 ) 
		assert_in_delta( gravity[1], 0.0, 0.01 ) 
		assert_in_delta( gravity[2], 0.0, 0.01 ) 
	end

	def test_02_gravity_set
		assert_nothing_raised { @world.gravity = [1.0,1.0,1.0] }
		assert_in_delta( @world.gravity[0], 1.0, 0.001 )
		assert_in_delta( @world.gravity[1], 1.0, 0.001 )
		assert_in_delta( @world.gravity[2], 1.0, 0.001 )

		assert_nothing_raised { @world.gravity = [1,1,1] }
		assert_in_delta( @world.gravity[0], 1.0, 0.001 )
		assert_in_delta( @world.gravity[1], 1.0, 0.001 )
		assert_in_delta( @world.gravity[2], 1.0, 0.001 )

		assert_nothing_raised { @world.gravity = [-0.2,-1.0,4.0] }
		assert_in_delta( @world.gravity[0], -0.2, 0.001 )
		assert_in_delta( @world.gravity[1], -1.0, 0.001 )
		assert_in_delta( @world.gravity[2],  4.0, 0.001 )

		assert_nothing_raised { @world.gravity = 0,1,2 }
		assert_in_delta( @world.gravity[0], 0.0, 0.001 )
		assert_in_delta( @world.gravity[1], 1.0, 0.001 )
		assert_in_delta( @world.gravity[2], 2.0, 0.001 )

		assert_raises( TypeError ) { @world.gravity = 0,1 }

		assert_raises( TypeError ) { @world.gravity = {'x' => 1} } 
		assert_raises( TypeError ) { @world.gravity = "none" } 
	end

	def test_03_erp_get
		erp = nil
		assert_nothing_raised { erp = @world.erp } 
		assert_in_delta( @world.erp, 0.2, 0.001 ) 
	end

	def test_04_erp_set
		assert_nothing_raised { @world.erp = 0.15 } 
		assert_in_delta( @world.erp, 0.15, 0.001 ) 
		assert_raises( TypeError ) { @world.erp = {'x' => 1} } 
		assert_raises( TypeError ) { @world.erp = "none" } 
	end

	def test_05_cfm_get
		cfm = nil
		assert_nothing_raised { cfm = @world.cfm } 
		assert_in_delta( @world.cfm, 1e-10, 1e-10 ) 
	end

	def test_06_cfm_set
		assert_nothing_raised { @world.cfm = 1e-5 } 
		assert_in_delta( @world.cfm, 1e-5, 1e-10 ) 
		assert_raises( TypeError ) { @world.cfm = {'x' => 1} } 
		assert_raises( TypeError ) { @world.cfm = "none" } 
	end

	def test_07_step
		assert_nothing_raised { @world.step(0.05) } 
		assert_raises( TypeError ) { @world.step("one") } 
	end
end


