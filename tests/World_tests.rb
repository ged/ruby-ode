#!/usr/bin/ruby

require "walkit/cli_script"
require "ode"

class World_test < Walkit::Testclass

	def setup
		@world = ODE::World.new
	end

	def teardown
		@world = nil
	end

	def test_00_new
		vet { assert_instance_of( ODE::World, @world ) }
	end

	def test_01_gravity_get
		gravity = nil

		vet { assert_no_exception { gravity = @world.gravity } }
		vet { assert_instance_of( Array, gravity ) }
		vet { assert_equal_float( gravity[0], 0.0, 0.01 ) }
		vet { assert_equal_float( gravity[1], 0.0, 0.01 ) }
		vet { assert_equal_float( gravity[2], 0.0, 0.01 ) }
	end

	def test_02_gravity_set
		vet {
			assert_no_exception { @world.gravity = [1.0,1.0,1.0] }
			assert_equal_float( @world.gravity[0], 1.0, 0.001 )
			assert_equal_float( @world.gravity[1], 1.0, 0.001 )
			assert_equal_float( @world.gravity[2], 1.0, 0.001 )
		}
		vet {
			assert_no_exception { @world.gravity = [1,1,1] }
			assert_equal_float( @world.gravity[0], 1.0, 0.001 )
			assert_equal_float( @world.gravity[1], 1.0, 0.001 )
			assert_equal_float( @world.gravity[2], 1.0, 0.001 )
		}
		vet {
			assert_no_exception { @world.gravity = [-0.2,-1.0,4.0] }
			assert_equal_float( @world.gravity[0], -0.2, 0.001 )
			assert_equal_float( @world.gravity[1], -1.0, 0.001 )
			assert_equal_float( @world.gravity[2],  4.0, 0.001 )
		}
		vet {
			assert_no_exception { @world.gravity = 0,1,2 }
			assert_equal_float( @world.gravity[0], 0.0, 0.001 )
			assert_equal_float( @world.gravity[1], 1.0, 0.001 )
			assert_equal_float( @world.gravity[2], 2.0, 0.001 )
		}
		vet {
			assert_exception( TypeError ) { @world.gravity = 0,1 }
		}
		vet { assert_exception( TypeError ) { @world.gravity = {'x' => 1} } }
		vet { assert_exception( TypeError ) { @world.gravity = "none" } }
	end

	def test_03_erp_get
		erp = nil
		vet { assert_no_exception { erp = @world.erp } }
		vet { assert_equal_float( @world.erp, 0.2, 0.001 ) }
	end

	def test_04_erp_set
		vet { assert_no_exception { @world.erp = 0.15 } }
		vet { assert_equal_float( @world.erp, 0.15, 0.001 ) }
		vet { assert_exception( TypeError ) { @world.erp = {'x' => 1} } }
		vet { assert_exception( TypeError ) { @world.erp = "none" } }
	end

	def test_05_cfm_get
		cfm = nil
		vet { assert_no_exception { cfm = @world.cfm } }
		vet { assert_equal_float( @world.cfm, 1e-10, 1e-10 ) }
	end

	def test_06_cfm_set
		vet { assert_no_exception { @world.cfm = 1e-5 } }
		vet { assert_equal_float( @world.cfm, 1e-5, 1e-10 ) }
		vet { assert_exception( TypeError ) { @world.cfm = {'x' => 1} } }
		vet { assert_exception( TypeError ) { @world.cfm = "none" } }
	end

	def test_07_step
		vet { assert_no_exception { @world.step(0.05) } }
		vet { assert_exception( TypeError ) { @world.step("one") } }
	end
end

if $0 == __FILE__
    Walkit::Cli_script.new.select([World_test], $*.shift)
end

