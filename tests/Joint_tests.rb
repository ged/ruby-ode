#!/usr/bin/ruby

require "walkit/cli_script"
require "ode"

class Joint_tests < Walkit::Testclass

	JointClasses = [ODE::BallJoint, ODE::HingeJoint, ODE::Hinge2Joint,
		ODE::SliderJoint, ODE::ContactJoint, ODE::FixedJoint]
	JointGroupMaxSize = (16*1024)
	@world = nil
	
	def setup
		@world = ODE::World.new
	end

	def teardown
		@world = nil
	end

	def test_00_create
		vet { assert_exception(RuntimeError) { ODE::Joint.new } }

		JointClasses.each {|klass|
			vet {
				joint = nil
				assert_exception(ArgumentError) { klass.new } 
				assert_no_exception { joint = klass.new(@world) }
				assert_instance_of( klass, joint )
			}
		}
	end

	def test_01_create_jointGroup
		jointGroup = nil
		vet { assert_exception(ArgumentError) { ODE::JointGroup.new } }
		vet { assert_exception(TypeError) { ODE::JointGroup.new("world") } }
		vet { assert_no_exception { ODE::JointGroup.new(JointGroupMaxSize) } }
	end

	def test_02_create_with_jointGroup
		JointClasses.each {|klass|
			vet {
				jointGroup = ODE::JointGroup.new(JointGroupMaxSize)
				joint = nil
				assert_exception(TypeError) { klass.new(@world, "jointGroup") } 
				assert_no_exception { joint = klass.new(@world, jointGroup) }
				assert_instance_of( klass, joint )
			}
		}
	end


end

if $0 == __FILE__
    Walkit::Cli_script.new.select([Joint_tests], $*.shift)
end

