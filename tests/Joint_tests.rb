#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class Joint_tests < ODE::TestCase

	# Numeric index of auto-generated tests
	$testCounter = 20

	# Build the lists of the joint classes to test
	JointClasses = ODE::constants.grep(/joint$/i).collect {|className|
		ODE::const_get(className)
	}.find_all {|klass| klass < ODE::Joint} - [ ODE::ContactJoint, ODE::ParameterizedJoint ]
	ParamJointClasses = JointClasses.find_all {|klass|
		klass < ODE::ParameterizedJoint
	}

	Tolerance = (ODE::Precision == 'dDOUBLE' ? 1e-10 : 1e-5)

	# Test unit struct -- makes passing test data around a bit more readble.
	JointUnit = Struct::new( "JointUnit", :joint, :op, :input, :result )

	# Datasets for common types of operations that get cloned for joint tests
	GenericDatasets = {
		:anchor => [
			[ [1.0,2,8],		ODE::Position ],
			[ [0,0,0],			ODE::Position ],
			[ [-1,0,1],			ODE::Position ],
			
			# Illegal values
			[ 1,				TypeError ],
			[ ["a", 1, 0],		TypeError ],
			[ nil,				TypeError ],
		],

		:axis => [
			[ [1.0,2,8],		ODE::Vector ],
			[ [0,0,0],			ODE::Vector ],
			[ [-1,0,1],			ODE::Vector ],
			
			# Illegal values
			[ 1,				TypeError ],
			[ ["a", 1, 0],		TypeError ],
			[ nil,				TypeError ],
		],
	}

	# :TODO: A lot of the individual joints still don't have coverage. This will
	# eventually be a matrix for testing those methods.
	# Test matrix for joint classes (Mapped into JointUnit objects)
	JointTests = {
		# JointClass
			# Method (:op)
				# input,			result
		ODE::BallJoint => {
			:anchor	=> GenericDatasets[:anchor].dup,
		},

		ODE::UniversalJoint => {
			:anchor	=> GenericDatasets[:anchor].dup,
			:axis1	=> GenericDatasets[:axis].dup,
			:axis2	=> GenericDatasets[:axis].dup,
		},
		
	}

	# Test matrix for param joints
	ParamJointTests = {
		# Flag/accessor (:op)
			# input,			result
		:CFM => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			# There are not useful values, but should they error?
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:bounce  => [
			[ 0,				0.0 ],
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			[ 1,				1.0 ],
			# Illegal values
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:fMax  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			[ 1.2,				1.2 ],
			[ 23.4,				23.4 ],
			[ 5,				5.0 ],
			# Illegal values
			[ -0.2,				RangeError ],
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:fudgeFactor  => [
			[ 0,				0.0 ],
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			[ 1,				1.0 ],
			# Illegal values
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:hiStop  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			[ ODE::Infinity,	ODE::Infinity ],
			[ ODE::Pi - 0.5,	ODE::Pi - 0.5  ],
			[ 1.2,				1.2 ],
			[ -0.2,				-0.2 ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:loStop  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			[ -ODE::Infinity,	-ODE::Infinity ],
			[ -ODE::Pi + 0.5,	-ODE::Pi + 0.5  ],
			[ 1.2,				1.2 ],
			[ -0.2,				-0.2 ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:stopCFM  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			# There are not useful values, but should they error?
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:stopERP  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			# There are not useful values, but should they error?
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:suspensionCFM  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			# There are not useful values, but should they error?
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:suspensionERP  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			# There are not useful values, but should they error?
			[ 1.2,				RangeError ],
			[ -0.2,				RangeError ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],

		:vel  => [
			[ 0.1,				0.1 ],
			[ 0.8,				0.8 ],
			[ 1.2,				1.2 ],
			[ 413.11,			413.11 ],
			# There are not useful values, but should they error?
			[ -0.2,				-0.2 ],
			# Illegal values
			[ :foo,				TypeError ],
			[ Object,			TypeError ],
			[ "foo",			TypeError ],
			[ nil,				TypeError ],
		],
	}

	# Auto-generate test methods for each joint class
	JointTests.keys.each_with_index {|klass, i|
		JointTests[klass].keys.each_with_index {|opname, j|

			# Build the name of the method to generate
			methodName = "test_%d_%s_%s" % [ $testCounter, klass.name.gsub(/:+/, '_'), opname.to_s ]
			$testCounter += 1

			# Build the method for the current class and op
			code = <<-"EOCODE"
			def #{methodName}
				unit = JointTests[#{klass.name}][:#{opname}]
				printTestHeader "Testing #{opname} for #{klass.name}##{opname}."
				runJointTest( #{klass.name}, :#{opname}, unit )
			end
			EOCODE

			begin
				eval code, nil, "#{methodName} (auto-generated)", __LINE__
			rescue => err
				Kernel::raise err, "Couldn't eval #{code}: #{err.message}"
			end
		}
	}

	# Auto-generate test methods for each parameter op for parameterized joints
	ParamJointTests.keys.each {|opname|
		ParamJointClasses.each {|klass|
			["", "2", "3"].each {|axis|

				# Check to see if the current type of joint has the axis we're
				# testing. If not, skip making this test.
				axes = klass.const_get( 'Axes' ).to_i
				break unless axis.empty? || axis.to_i <= axes

				# The suspension parameter is limited more than the rest...
				if opname.to_s =~ /suspension/i
					# :TODO: Hinge2 is the only joint that implements the
					# suspension parameters currently. This may change(?)
					next unless klass == ODE::Hinge2Joint

					# Only the first axis of the hinge2 joint can have
					# suspension.
					next unless axis == ""
				end

				# Build the name of the method to generate
				methodName = "test_%d_%s_%s%s" % [ $testCounter, klass.name.gsub(/:+/, '_'), opname.to_s, axis ]
				$testCounter += 1

				# Create the testing method for the current tested method, axis
				# and class.
				code = <<-"EOCODE"
				def #{methodName}
					unit = ParamJointTests[:#{opname}]
					printTestHeader "Testing param accessor for #{klass.name}##{opname}#{axis}."
					runJointTest( #{klass.name}, :#{opname}#{axis}, unit )
				end
				EOCODE

				begin
					eval code, nil, "#{methodName} (auto-generated)", __LINE__
				rescue => err
					Kernel::raise err, "Couldn't eval #{code}: #{err.message}"
				end
			}
		}
	}

	def runJointTest( klass, op, unit )
		config = {}
		joint = klass.new( @world )

		# Generate symbols for the get/set methods for this joint from the
		# symbol being tested.
		getter = op
		setter = ("%s=" % op.to_s).intern

		# If there's a hash immediately after the symbol, it contains
		# specifications for non-regular accessors.
		if unit.first.is_a?( Hash )
			config = unit.shift
			getter = config[:getter] if config.key?( :getter )
			setter = config[:setter] if config.key?( :setter )

			config[:prep].call( self ) if config.key?( :prep )
		end

		debugMsg "Using testing method: %s" %
			[ getter, setter ].
			collect {|sym| sym.inspect}.join(", ")

		# Test to be sure the interface is right
		assert_respond_to joint, getter, "Test: %s" % op.inspect if getter
		assert_respond_to joint, setter, "Test: %s" % op.inspect if setter

		# Make sure the setter's really a setter
		assert_raises( ArgumentError ) { joint.send(setter) }

		# Now map all the test values into JointUnit struct objects and
		# iterate over them.
		unit.collect {|ary| JointUnit::new(joint, op, *ary)}.each_with_index {|test, i|
			config[:pretest].call( test ) if config.key?( :pretest )
			expectedReturn = nil

			# If the expected result is an error, use the last non-exception
			# result value, as the setting should still be untouched.
			if test.result.kind_of?( Class ) && test.result < Exception
				expectedReturn = unit[ 0 .. i ].reverse.find {|oldtest|
					! (oldtest[1].kind_of?( Class ) && oldtest[1] < Exception)
				}[1]

				debugMsg "Testing with expected return of '#{expectedReturn.inspect}'"
			end

			debugMsg "Calling runTest with #{test.inspect}"
			runTest( test, getter, setter, expectedReturn )

			config[:posttest].call( test ) if config.key?( :posttest )
		}
	end


	### If the expected result is a Float, test it with a tolerance, else test
	### it for equality.
	def assertSimilar( expected, actual, errmsg=nil )
		if expected != ODE::Infinity && expected != -ODE::Infinity && expected.kind_of?( Float )
			assert_in_delta expected, actual, Tolerance, errmsg
		elsif expected.is_a?( Class )
			assert_instance_of expected, actual
		else
			assert_equal expected, actual, errmsg
		end
	end


	### Run get/set/predicate tests for the given test struct.
	def runTest( test, getter, setter, expectedReturn=nil )
		debugMsg "Running tests for #{test.inspect}"
		
		if setter
			runSetterTests( test, setter )
		end

		if getter
			runGetterTests( test, getter, expectedReturn )
		end

		debugMsg "Done with tests for #{test.inspect}"
	rescue Exception => err
		Kernel::raise err.exception( "In test: #{test.inspect}: #{err.message}" )
	end


	### Run the 'set' accessor part of a test using the specified setter method
	### and test specification struct.
	def runSetterTests( test, setter )
		debugMsg "Running setter tests for method #{setter.inspect} for #{test.inspect}"
		rval = nil

		# If the test is meant to raise an exception, assert that.
		if test.result.kind_of?( Class ) && test.result < Exception
			assert_raises( test.result, "Test set method: #{test.inspect}" ) {
				test.joint.send( setter, test.input )
			}

		# Otherwise, make sure it doesn't raise and test the return
		# value.
		else
			assert_nothing_raised( "Test set method: #{test.inspect}" ) {
				rval = test.joint.send( setter, test.input )
			}
			debugMsg "Testing for similarity between:"
			debugMsg "'#{test.result.inspect}' and '#{rval.inspect}'"
			assertSimilar( test.result, rval, "Setter return value: #{test.inspect}" )
		end

		debugMsg "Done with setter test."
	end


	### Run the 'get' accessor part of a test using the specified getter method
	### and test specification struct. If <tt>expectedReturn</tt> is non-nil,
	### the return value should be the value it contains instead of the test's
	### .result member.
	def runGetterTests( test, getter, expectedReturn=nil )
		debugMsg "Running getter tests method #{getter.inspect} for #{test.inspect}"
		rval = nil

		assert_nothing_raised( "Test get method: #{test.inspect}" ) {
			rval = test.joint.send( getter )
		}
		if test.result.is_a?( Class ) && test.result < Exception
			assertSimilar( expectedReturn, rval, "Getter return value: #{test.inspect}" )
		else
			assertSimilar( test.result, rval, "Getter return value: #{test.inspect}" )
		end
	end


	# Setup preconditions
	def setup
		@world = ODE::World.new
	end
	alias_method :set_up, :setup

	# Remove preconditions
	def teardown
		@world = nil
		collectGarbage()
	end
	alias_method :tear_down, :teardown

	
	# Create joint objects without a JointGroup
	def test_00_create
		printTestHeader "Test creation of Joints without JointGroups"

		assert_raises(ScriptError) { ODE::Joint::new } 

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

	# Create joints in a JointGroup
	def test_02_create_with_jointGroup
		printTestHeader "Test creation of Joints with JointGroups"

		JointClasses.each {|klass|
			joint = nil

			debugMsg "Creating a JointGroup."
			jointGroup = ODE::JointGroup.new

			assert_raises(TypeError, "klass = #{klass.name}") { klass.new(@world, "jointGroup") } 

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
		printTestHeader "Test creation of Many Joints with a single JointGroup"

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
		printTestHeader "Test obsoletion of joints after destroying JointGroup"

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

	# Test attach method
	def test_04_joint_attach 
		bodies = [ODE::Body::new(@world), ODE::Body::new(@world)]

		printTestHeader "Test joint attach"
		JointClasses.each {|klass|
			rval = nil
			joint = klass.new( @world )

			body1 = @world.createBody
			body1.position = 1,0,0
			body2 = @world.createBody
			body2.position = -1,0,0

			assert_nothing_raised {
				joint.attach( body1, body2 )
			}

			assert_nothing_raised {
				rval = joint.attachedBodies
			}
			assert rval.include?( body1 ),
				"Return value of attachedBodies doesn't include body1"
			assert rval.include?( body2 ),
				"Return value of attachedBodies doesn't include body2"
		}
	end		

	# Test joint feedback
	def test_05_joint_feedback
		printTestHeader "Test joint feedback"

		# Setup
		JointClasses.each {|klass|
			debugMsg "Creating a #{klass.name}."
			joint = klass.new( @world )
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

		}

		debugMsg "Done testing joint feedback."
		$stderr.flush
	end


	# Test FixedJoint#fix
	def test_05_fixedjoint_fix
		joint = ODE::FixedJoint::new( @world )

		rval = nil
		assert_nothing_raised { rval = joint.fix }
		assert rval, "FixedJoint#fix didn't return a true value"
	end

end


