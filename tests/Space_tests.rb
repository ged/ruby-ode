#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class SpaceTestCase < ODE::TestCase

	Tolerance = ODE::Precision == 'dDOUBLE' ? 1e-5 : 1e-2

	# Test unit struct -- makes passing test data around a bit more readble.
	SpaceUnit = Struct::new( "SpaceUnit", :classObj, :op, :input, :expected )

	# Persistant top-level space for testing containment
	ProtoSpace = ODE::Space::new

	# Test data for flag/option accessors (Mapped into FlagUnit objects)
	SpaceTests = {

		# Arguments to use in the default constructor (all tests except those
		# with more-explicit proto args).
		:__proto_args__ => [],

		# op
		#	input	=> result
		:new	=> {
			[]				=> :__class__,
			[1]				=> TypeError,
			[nil]			=> TypeError,
			[ProtoSpace]	=> :__class__,
		},

	}

	# Auto-generate test methods for each op
	[ ODE::Space, ODE::HashSpace ].each {|klass|
		SpaceTests.keys.each_with_index {|op, n|
			next if op.to_s =~ /^__/

			debugMsg "Creating test method for %s##{op.to_s} (n = #{n})" % klass.name
			methodName = "test_%d_%s_%s" %
				[ 10 + n, klass.name.gsub(/:+/, '_'), op.to_s ]

			eval <<-"EOCODE"
			def #{methodName}
				unit = SpaceTests[:#{op.to_s}]
				testHeader "Testing '#{op.to_s}' method of #{klass.name}."
				runSpaceTest( #{klass.name}, :#{op.to_s}, unit )
			end
			EOCODE
		}
	}

	### Run the unit test of the specified op on the given class
	def runSpaceTest( klass, op, unit )
		config = {}

		# Constructor test
		if op == :new
			unit.collect {|input, expected|
				expected = klass if expected == :__class__
				SpaceUnit::new( klass, op, input, expected)
			}.each {|test|
				runInstantiationTest( test )
			}

		# Accesor test
		else
			# Get the proto args from the test hash
			args = SpaceTests["__#{op.to_s}_proto_args__".intern] || SpaceTests[:__proto_args__]
			@obj = klass.new( *args )

			# Generate symbols for the get/set methods for this flag
			# from the symbol being tested.
			getter = op
			setter = ("%s=" % op.to_s).intern

			debugMsg "Using testing methods: %s" %
				[ getter, setter ].collect {|sym| sym.inspect}.join(", ")

			# Test to be sure the interface is right
			assert_respond_to @obj, getter, "Test: %s#%s" % [ klass.name, op.inspect ]
			assert_respond_to @obj, setter, "Test: %s#%s" % [ klass.name, op.inspect ]

			# Now map all the test values into FlagUnit struct objects and
			# iterate over them.
			unit.collect {|input,expected|
				expected = klass if expected == :__class__
				SpaceUnit::new( klass, op, input, expected )
			}.each {|test|
				runTest( test, getter, setter )
			}
		end
	end


	### If the expected result is a Float, test it with a tolerance, else test
	### it for equality.
	def assertSimilar( expected, actual, errmsg=nil )
		if expected != ODE::Infinity && expected.kind_of?( Float )
			assert_in_delta expected, actual, Tolerance, errmsg
		elsif expected.is_a?( Array )
			assert_kind_of Array, actual
			assert_equal expected.length, actual.length

			# Recurse for each value
			expected.each_index {|i| assertSimilar(expected[i], actual[i], errmsg)}
		else
			assert_equal expected, actual, errmsg
		end
	end


	### Run the instantiator test on the specified class
	def runInstantiationTest( test )
		if test.expected < Exception
			assert_raises( test.expected ) { test.classObj.new(*test.input) }
		else
			rval = nil
			assert_nothing_raised( "Test instantiation: #{test.classObj.name}: #{test.inspect}" ) {
				rval = test.classObj.new( *test.input )
			}

			assert_instance_of test.expected, rval,
				"Test instantiation: #{test.classObj.name}: #{test.inspect}"
		end
	end


	### Run get/set tests for the given test struct.
	def runTest( test, getter, setter )
		debugMsg "Running tests for #{test.inspect}"
		
		runSetterTests( test, setter )
		runGetterTests( test, getter ) unless
			test.expected.kind_of?( Class ) && test.expected < Exception

	rescue Exception => err
		Kernel::raise err.exception( "In test: #{test.inspect}: #{err.message}" )
	end


	### Run the 'set' accessor part of a test using the specified setter method
	### and test specification struct.
	def runSetterTests( test, setter )
		debugMsg "Running setter tests for method #{setter.inspect} for #{test.inspect}"
		rval = nil

		# If the test is meant to raise an exception, assert that.
		if test.expected.kind_of?( Class ) && test.expected < Exception
			assert_raises( test.expected, "Test set method." ) {
				@obj.send( setter, test.input )
			}

		# Otherwise, make sure it doesn't raise and test the return
		# value.
		else
			assert_nothing_raised( "Test set method." ) {
				rval = @obj.send( setter, test.input )
			}
			assertSimilar( test.expected, rval, "Setter return value was #{rval.inspect}." )
		end
	end


	### Run the 'get' accessor part of a test using the specified getter method
	### and test specification struct. If <tt>expectedReturn</tt> is non-nil,
	### the return value should be the value it contains instead of the test's
	### .expected member.
	def runGetterTests( test, getter, expectedReturn=nil )
		debugMsg "Running getter tests method #{getter.inspect} for #{test.inspect}"
		rval = nil

		assert_nothing_raised( "Test get method: #{test.inspect}" ) {
			rval = @obj.send( getter )
		}

		assertSimilar( test.expected, rval, "Getter return value was #{rval.inspect}" )
	end



	#################################################################
	###	S E T U P / T E A R D O W N   M E T H O D S
	#################################################################



	#################################################################
	###	T E S T S
	#################################################################

	# :TODO: Test deep containment/marking functions/geom+space interaction

end

