#!/usr/bin/ruby

begin
	require "odeunittest"
rescue
	require "../odeunittest"
end

require "ode"

class Joint_tests < ODE::TestCase

	JointClasses = [
		ODE::BallJoint,
		ODE::HingeJoint,
		ODE::Hinge2Joint,
		ODE::SliderJoint,
		ODE::FixedJoint,
		#	ODE::ContactJoint, 
	]

	@world = nil

	# Setup preconditions
	def set_up
		@world = ODE::World.new
	end

	# Remove preconditions
	def tear_down
		@world = nil
		collectGarbage()
	end
	
	# Create joint objects without a JointGroup
	def test_00_create
		testHeader "Test creation of Joints without JointGroups"

		assert_raises(RuntimeError) { ODE::Joint.new } 

		JointClasses.each {|klass|
			joint = nil

			debugMsg "Create: Testing the #{klass.name} class."
			assert_raises(ArgumentError) { klass.new } 
			assert_nothing_raised { joint = klass.new(@world) }
			assert_instance_of( klass, joint )

			debugMsg "Clobbering joint"
			$stderr.flush
			joint = nil
			collectGarbage()
		}
	end

	# Create an empty JointGroup
	def test_01_create_jointGroup
		testHeader "Test creation of empty JointGroups"

		jointGroup = nil

		assert_raises(TypeError) { ODE::JointGroup.new("world") } 
		collectGarbage()

		assert_nothing_raised { ODE::JointGroup.new } 
		collectGarbage()

		assert_nothing_raised { ODE::JointGroup.new(ODE::SliderJoint) } 
		collectGarbage()
	end

	# Create joints in a JointGroup
	def test_02_create_with_jointGroup
		testHeader "Test creation of Joints with JointGroups"

		JointClasses.each {|klass|
			joint = nil

			debugMsg "Creating a JointGroup."
			jointGroup = ODE::JointGroup.new

			assert_raises(TypeError) { klass.new(@world, "jointGroup") } 

			debugMsg "Create with JointGroup: Testing the '#{klass.name}' class."
			assert_nothing_raised { joint = klass.new(@world, jointGroup) }
			assert_instance_of( klass, joint )

			debugMsg "Clobbering joint"
			joint = nil
			collectGarbage()
		}
	end

	# Create joints in a JointGroup
	def test_03_create_many_with_jointGroup
		testHeader "Test creation of Many Joints with a single JointGroup"

		debugMsg "Creating a JointGroup."
		jointGroup = ODE::JointGroup.new

		JointClasses.each {|klass|
			joint = nil

			debugMsg "Create Many with JointGroup: Adding a '#{klass.name}' object."
			assert_nothing_raised { joint = klass.new(@world, jointGroup) }
			assert_instance_of( klass, joint )

			debugMsg "Clobbering joint"
			joint = nil
			collectGarbage()
		}

		debugMsg "Clobbering jointGroup"
		jointGroup = nil
		collectGarbage()
	end

	# Test joint obsoletion
	def test_04_joint_obsoletion
		testHeader "Test obsoletion of joints after destroying JointGroup"

		debugMsg "Creating a JointGroup."
		jointGroup = ODE::JointGroup.new
		joints = []

		JointClasses.each {|klass|
			debugMsg "Obsoletion: Adding a '#{klass.name}'."
			joints.push klass.new(@world, jointGroup)
		}

		debugMsg "Emptying jointGroup"
		jointGroup.empty

		joints.each {|j|
			assert j.obsolete?
			assert_raises( ODE::ObsoleteJointError ) { j.attachedBodies }
		}
	end

end


