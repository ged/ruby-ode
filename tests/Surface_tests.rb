#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class Surface_test < ODE::TestCase

	Tolerance = ODE::Precision == 'dDOUBLE' ? 1e-10 : 1e-5

	# Test unit struct -- makes passing test data around a bit more readble.
	SurfaceFlagUnit = Struct::new( "SurfaceFlagUnit", :op, :input, :result, :predicate )

	# Test data for flag/option accessors (Mapped into SurfaceFlagUnit objects)
	SurfaceFlagTests = {
		# SurfaceFlag/accessor (:op)
			# :input,				:result,				:predicate
		:bounce => [
			[ 0.0,					0.0,					true ],
			[ 1.0,					1.0,					true ],
			[ 1.1,					RangeError,				true ],
			[ nil,					nil,					false ],
			[ 0.05,					0.05,					true ],
			[ false,				nil,					false ],
			[ -2.0,					RangeError,				false ],
			[ 2e-4,					2e-4,					true ],
			[ 14467.232373,			RangeError,				true ],
			
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

		:bounceVelocity => [
			{
				# The bounceVelocity won't show up unless bounce is turned on,
				# so turn it on before testing, and don't bother testing the
				# predicate.
				:predicate => nil,
				:prep => Proc::new {|testcase|
					testcase.surface.bounce = 0.5
				}
			},

			[ 0.1,					0.1,					nil ],
			[ 1,					1.0,					nil ],
			[ 115,					115.0,					nil ],
			[ 0,					0,						nil ],
			[ -5,					RangeError,				nil ],
			[ nil,					TypeError,				nil ],
			[ "foo",				TypeError,				nil ],
			[ ["foo"],				TypeError,				nil ],
			[ :foo,					TypeError,				nil ],
			[ $stderr,				TypeError,				nil ],
		],

		:softERP => [
			[ 0.1,					0.1,					true ],
			[ 0.8,					0.8,					true ],
			[ 0,					0.0,					true ],
			[ 1,					1.0,					true ],
			[ 11.141674,			RangeError,				true ],
			[ -2.0,					RangeError,				true ],
			[ false,				nil,					false ],
			[ 14467.232373,			RangeError,				false ],
			[ 4e-4,					4e-4,					true ],
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

		:softCFM => [
			[ 0.1,					0.1,					true ],
			[ 0.8,					0.8,					true ],
			[ 0,					0.0,					true ],
			[ 1,					1.0,					true ],
			[ 11.141674,			RangeError,				true ],
			[ -2.0,					RangeError,				true ],
			[ false,				nil,					false ],
			[ 14467.232373,			RangeError,				false ],
			[ 4e-4,					4e-4,					true ],
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

		:motion1 => [
			[ 0.1,					0.1,					true ],
			[ 0.8,					0.8,					true ],
			[ 0,					0.0,					true ],
			[ 1,					1.0,					true ],
			[ 11.141674,			11.141674,				true ],
			[ -2.0,					-2.0,					true ],
			[ false,				nil,					false ],
			[ 14467.232373,			14467.232373,			true ],
			[ 1444e-4,				1444e-4,				true ],
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

		:motion2 => [
			[ 0.1,					0.1,					true ],
			[ 0.8,					0.8,					true ],
			[ 0,					0.0,					true ],
			[ 1,					1.0,					true ],
			[ 11.141674,			11.141674,				true ],
			[ -2.0,					-2.0,					true ],
			[ false,				nil,					false ],
			[ 14467.232373,			14467.232373,			true ],
			[ 1444e-4,				1444e-4,				true ],
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

		:slip1   => [
			[ 0.1,					0.1,					true ],
			[ 0.8,					0.8,					true ],
			[ 0,					0.0,					true ],
			[ 1,					1.0,					true ],
			[ 11.141674,			RangeError,				true ],
			[ -2.0,					RangeError,				true ],
			[ false,				nil,					false ],
			[ 14467.232373,			RangeError,				false ],
			[ 4e-4,					4e-4,					true ],
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

		:slip2   => [
			[ 0.1,					0.1,					true ],
			[ 0.8,					0.8,					true ],
			[ 0,					0.0,					true ],
			[ 1,					1.0,					true ],
			[ 11.141674,			RangeError,				true ],
			[ -2.0,					RangeError,				true ],
			[ false,				nil,					false ],
			[ 14467.232373,			RangeError,				false ],
			[ 4e-4,					4e-4,					true ],
			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],
		
		:mu2 => [
			[ 0.0,					0.0,					true ],
			[ ODE::Infinity,		ODE::Infinity,			true ],
			[ 15,					15.0,					true ],
			[ nil,					nil,					false ],
			[ 11.141674,			11.141674,				true ],
			[ false,				nil,					false ],
			[ -2.0,					RangeError,				false ],
			[ 14467.232373,			14467.232373,			true ],

			# Make sure illegal values don't unset the mode
			[ -11.01,				RangeError,				true ],

			[ nil,					nil,					false ],
			[ "foo",				TypeError,				false ],
			[ ["foo"],				TypeError,				false ],
			[ :foo,					TypeError,				false ],
			[ $stderr,				TypeError,				false ],
		],

	}

	# Auto-generate test methods for each op
	SurfaceFlagTests.keys.each_with_index {|opname, i|
		methodName = "test_%d_%s" % [ i+10, opname.to_s ]

		eval <<-"EOCODE"
		def #{methodName}
			unit = SurfaceFlagTests[:#{opname}]
			testHeader "Testing flag accessor for '#{opname}'."
			runFlagTest( :#{opname}, unit )
		end
		EOCODE
	}

		
	def runFlagTest( op, unit )
		config = {}

		# Generate symbols for the get/set/predicate methods for this flag
		# from the symbol being tested.
		getter = op
		setter = ("%s=" % op.to_s).intern
		predicate = ("%s?" % op.to_s).intern

		# If there's a hash immediately after the symbol, it contains
		# specifications for non-regular accessors.
		if unit.first.is_a?( Hash )
			config = unit.shift
			getter = config[:getter] if config.key?( :getter )
			setter = config[:setter] if config.key?( :setter )
			predicate = config[:predicate] if config.key?( :predicate )

			config[:prep].call( self ) if config.key?( :prep )
		end

		debugMsg "Using testing method: %s" %
			[ getter, setter, predicate ].
			collect {|sym| sym.inspect}.join(", ")

		# Test to be sure the interface is right
		assert_respond_to @surface, getter, "Test: %s" % op.inspect if getter
		assert_respond_to @surface, setter, "Test: %s" % op.inspect if setter
		assert_respond_to @surface, predicate, "Test: %s" % op.inspect if predicate

		# Make sure the setter's really a setter
		assert_raises( ArgumentError ) { @surface.send(setter) }

		# Now map all the test values into FlagUnit struct objects and
		# iterate over them.
		unit.collect {|ary| SurfaceFlagUnit::new(op, *ary)}.each_with_index {|test, i|
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
			runTest( test, getter, setter, predicate, expectedReturn )

			config[:posttest].call( test ) if config.key?( :posttest )
		}
	end


	### If the expected result is a Float, test it with a tolerance, else test
	### it for equality.
	def assertSimilar( expected, actual, errmsg=nil )
		if expected != ODE::Infinity && expected.kind_of?( Float )
			assert_in_delta expected, actual, Tolerance, errmsg
		else
			assert_equal expected, actual, errmsg
		end
	end


	### Run get/set/predicate tests for the given test struct.
	def runTest( test, getter, setter, predicate, expectedReturn=nil )
		debugMsg "Running tests for #{test.inspect}"
		
		if setter
			runSetterTests( test, setter )
		end

		if getter
			runGetterTests( test, getter, expectedReturn )
		end

		if predicate
			runPredicateTests( test, predicate )
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
				@surface.send( setter, test.input )
			}

		# Otherwise, make sure it doesn't raise and test the return
		# value.
		else
			assert_nothing_raised( "Test set method: #{test.inspect}" ) {
				rval = @surface.send( setter, test.input )
			}
			assertSimilar( test.result, rval, "Setter return value: #{test.inspect}" )
		end
	end


	### Run the 'get' accessor part of a test using the specified getter method
	### and test specification struct. If <tt>expectedReturn</tt> is non-nil,
	### the return value should be the value it contains instead of the test's
	### .result member.
	def runGetterTests( test, getter, expectedReturn=nil )
		debugMsg "Running getter tests method #{getter.inspect} for #{test.inspect}"
		rval = nil

		assert_nothing_raised( "Test get method: #{test.inspect}" ) {
			rval = @surface.send( getter )
		}

		if test.result.is_a?( Class ) && test.result < Exception
			assertSimilar( expectedReturn, rval, "Getter return value: #{test.inspect}" )
		else
			assertSimilar( test.result, rval, "Getter return value: #{test.inspect}" )
		end
	end


	### Run the predicate part of a test using the specified predicate method
	### and test specification struct.
	def runPredicateTests( test, predicate )
		debugMsg "Running predicate tests with method #{predicate.inspect} for #{test.inspect}"
		rval = nil

		assert_nothing_raised( "Test predicate method: #{test.inspect}" ) {
			rval = @surface.send( predicate )
		}

		assert_equal test.predicate, rval, "Predicate return value: #{test.inspect}"
	end


	#################################################################
	###	S E T U P / T E A R D O W N   M E T H O D S
	#################################################################
	def setup 
		@surface = ODE::Surface::new
	end
	alias_method :set_up, :setup

	def teardown 
		@surface = nil
	end
	alias_method :tear_down, :teardown


	attr_reader :surface



	#################################################################
	###	T E S T S
	#################################################################

	# ODE::Surface#new
	def test_00_instance
		testHeader "Testing instance"
		assert_instance_of ODE::Surface, @surface
	end


	# ODE::Surface#mode
	def test_01_mode
		testHeader "Testing #mode, #mode="
		rval = nil

		# Test fetching the mode
		assert_nothing_raised { rval = @surface.mode }
		assert_equal 0, rval

		# Test setting the mode
		assert_nothing_raised { @surface.mode = 2 }
		assert_equal 2, @surface.mode

		# Test setting to illegal values
		assert_raises( TypeError ) { @surface.mode = "foo" }
		assert_raises( TypeError ) { @surface.mode = ["foo"] }
		assert_raises( TypeError ) { @surface.mode = Proc::new {"foo!"} }
		assert_raises( TypeError ) { @surface.mode = nil }
	end

	# ODE::Surface#mu
	def test_02_mu
		testHeader "Testing #mu, #mu="
		rval = nil

		# Test fetching mu
		assert_nothing_raised { rval = @surface.mu }
		assert_equal ODE::Infinity, rval

		# Test setting mu
		assert_nothing_raised { @surface.mu = 0.02 }
		assert_in_delta 0.02, @surface.mu, Tolerance

		# Test setting to illegal values
		assert_raises( TypeError ) { @surface.mu = "foo" }
		assert_raises( TypeError ) { @surface.mu = ["foo"] }
		assert_raises( TypeError ) { @surface.mu = Proc::new {"foo!"} }
		assert_raises( TypeError ) { @surface.mu = nil }
	end

	# ODE::Surface#useFrictionDirection
	def test_03_friction_direction
		testHeader "Testing #useFrictionDirection, #useFrictionDirection="
		rval = nil

		assert_respond_to @surface, :useFrictionDirection=
		assert_respond_to @surface, :fDir1=
		assert_respond_to @surface, :useFrictionDirection?
		assert_respond_to @surface, :fDir1?

		# Test the predicate
		assert_nothing_raised { rval = @surface.useFrictionDirection? }
		assert_equal false, rval

		# Test setting the mode
		assert_nothing_raised { @surface.useFrictionDirection = true }
		assert_equal true, @surface.useFrictionDirection?

		assert_nothing_raised { @surface.useFrictionDirection = false }
		assert_equal false, @surface.useFrictionDirection?

		assert_nothing_raised { @surface.useFrictionDirection = 1 }
		assert_equal true, @surface.useFrictionDirection?

		assert_nothing_raised { @surface.useFrictionDirection = nil }
		assert_equal false, @surface.useFrictionDirection?

	end

	# ODE::Surface#frictionModel=
	def test_03_friction_model
		testHeader "Testing #frictionModel=, #pyramidFriction{1,2}{?,=}"
		rval = nil

		assert_respond_to @surface, :frictionModel=
		assert_respond_to @surface, :pyramidFriction1=
		assert_respond_to @surface, :pyramidFriction1?
		assert_respond_to @surface, :pyramidFriction2=
		assert_respond_to @surface, :pyramidFriction2?

		# Test initial values
		assert_nothing_raised { rval = @surface.pyramidFriction1? }
		assert_equal false, rval, "Initial value of pyramidFriction1?"

		assert_nothing_raised { rval = @surface.pyramidFriction2? }
		assert_equal false, rval, "Initial value of pyramidFriction2?"

		# Test setting via 'mode'
		@surface.mode |= ODE::Contact::PyramidFrictionBoth

		assert_nothing_raised { rval = @surface.pyramidFriction1? }
		assert_equal true, rval, "Value of pyramidFriction1? after mode set"

		assert_nothing_raised { rval = @surface.pyramidFriction2? }
		assert_equal true, rval, "Value of pyramidFriction2? after mode set"
		
		# Test setting1 and unsetting 2 via 'frictionModel='
		@surface.frictionModel = ODE::Contact::PyramidFriction1
		assert (@surface.mode | ODE::Contact::PyramidFriction1).nonzero?

		assert_nothing_raised { rval = @surface.pyramidFriction1? }
		assert_equal true, rval, "Value of pyramidFriction1? after frictionModel= set"

		assert_nothing_raised { rval = @surface.pyramidFriction2? }
		assert_equal false, rval, "Value of pyramidFriction2? after frictionModel= set"

		# Test unsetting 1 and setting 2 via 'frictionModel='
		@surface.frictionModel = ODE::Contact::PyramidFriction2
		assert (@surface.mode | ODE::Contact::PyramidFriction2).nonzero?

		assert_nothing_raised { rval = @surface.pyramidFriction1? }
		assert_equal false, rval, "Value of pyramidFriction1? after frictionModel= set 2"

		assert_nothing_raised { rval = @surface.pyramidFriction2? }
		assert_equal true, rval, "Value of pyramidFriction2? after frictionModel= set 2"

		# Test setting some unrelated values (which shouldn't do anything)
		@surface.mode = 0
		assert_nothing_raised { @surface.frictionModel = ODE::Contact::Bounce }
		assert_equal false, @surface.pyramidFriction1?
		assert_equal false, @surface.pyramidFriction2?
		
	end

	def mnote
		debugMsg caller(1)[0]
	end
end

