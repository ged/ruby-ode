#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class Joint_tests < ODE::TestCase

	JointClasses = [
		ODE::BallJoint,
		ODE::HingeJoint,
		ODE::Hinge2Joint,
		ODE::SliderJoint,
		ODE::FixedJoint,
		#	ODE::ContactJoint, 
	]

	Tolerance = ODE::Precision == 'dDOUBLE' ? 1e-10 : 1e-5

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

	# Test joint feedback
	def test_05_joint_feedback
		testHeader "Test joint feedback"

		# Setup
		debugMsg "Creating a HingeJoint."
		joint = ODE::HingeJoint::new( @world )
		debugMsg "Creating two bodies to attach"
		body1 = @world.createBody
		body1.position = 1, 0, 0
		body2 = @world.createBody
		body2.position = -1, 0, 0
		debugMsg "Attaching the joint to the bodies"
		joint.attach( body1, body2 )
		debugMsg "Stepping the world."
		@world.step( 0.01 );

		# Tests
		rval = nil
		debugMsg "Testing the feedback function without setting it first"
		assert_nothing_raised { rval = joint.feedback }
		assert_nil rval, "Un-feedbacked joint returned <#{rval.inspect}> "\
			"instead of nil for feedback."
		assert_nothing_raised { rval = joint.feedback? }
		assert_equal false, rval
		assert_nothing_raised { rval = joint.feedback_enabled? }
		assert_equal false, rval

		# Test turning feedback on, and then doing it when it's already on.
		debugMsg "Turning feedback on and testing to see if it's enabled."
		assert_nothing_raised { joint.feedback = true }
		assert_nothing_raised { rval = joint.feedback? }
		assert_equal true, rval

		# Test actual feedback values, which should be mostly 0, since no force
		# has been applied to anything.
		debugMsg "Testing feedback structure"
		@world.step( 0.01 )
		assert_nothing_raised { rval = joint.feedback }
		assert_instance_of Hash, rval

		debugMsg "Feedback = %s" % rval.inspect
		assert rval.key?( :body1 ), "Feedback hash doesn't contain a key :body1"
		assert rval.key?( :body2 ), "Feedback hash doesn't contain a key :body2"

		# Test the feedback hash's members
		[:body1, :body2].each {|outerKey|
			debugMsg "   ...testing %s's half of the feedback hash" % outerKey.inspect
			assert_instance_of Hash, rval[outerKey],
				"%s feedback isn't a Hash" % outerKey.inspect

			{ :force => ODE::Force, :torque => ODE::Torque }.each {|innerKey, klass|
				debugMsg "      ...testing %s's %s value" % [ outerKey.inspect, innerKey.inspect ]
				assert rval[outerKey].key?( innerKey ),
					"%s feedback hash doesn't contain a %s key" %
					[ outerKey.inspect, innerKey.inspect ]

				debugMsg "         ...has a %s key" % innerKey.inspect
				testVal = rval[outerKey][innerKey]
				assert_instance_of klass, testVal

				[ :x, :y, :z ].each {|axis|
					debugMsg "         ...testing delta of %s axis." % axis.inspect
					assert_in_delta 0.0, testVal.send(axis), Tolerance
				}
			}
		}

		debugMsg "Done testing joint feedback."
		$stderr.flush
	end

end


