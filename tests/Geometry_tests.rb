#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class GeometryTestCase < ODE::TestCase

	Tolerance = ODE::Precision == 'dDOUBLE' ? 1e-5 : 1e-2

	# Test unit struct -- makes passing test data around a bit more readble.
	GeomUnit = Struct::new( "GeomUnit", :classObj, :op, :input, :expected )

	def self.normalizePlaneVector( a, b, c, d )
		sideSum = a*a + b*b + c*c
		if sideSum.nonzero?
			factor = Math::sqrt( 1.0/sideSum )

			return [ a*factor, b*factor, c*factor, d*factor ]
		else
			return [ 1.0, 0.0, 0.0, 0.0 ]
		end
	end


	# Test data for flag/option accessors (Mapped into FlagUnit objects)
	GeomTests = {
		# Class
		#	method
		#		args, result
		ODE::Geometry::Sphere => {
			:__proto_args__	=> [1],

			:new	=> {
				1			=> ODE::Geometry::Sphere,
				0			=> RangeError,
				-1			=> RangeError,
				#[1,mockSpace]	=> ODE::Geometry::Sphere,
			},

			:radius => {
				1			=> 1.0,
				0			=> RangeError,
				-1			=> RangeError,
			},
		},

		ODE::Geometry::Box => {
			:__proto_args__	=> [1,1,1],

			:new	=> {
				[1,1,1]		=> ODE::Geometry::Box,
				[0,1,1]		=> RangeError,
				[1,0,1]		=> RangeError,
				[1,1,0]		=> RangeError,
				[-1,1,1]	=> RangeError,
				[1,-1,1]	=> RangeError,
				[1,1,-1]	=> RangeError,
				[]			=> ArgumentError,
				[1]			=> ArgumentError,
				[1,1]		=> ArgumentError,

				#[1,1,1,mockSpace]	=> ODE::Geometry::Box,
			},

			:lengths	=> {
				[1,1,1]		=> [1.0,1.0,1.0],
				[0,1,1]		=> RangeError,
				[1,0,1]		=> RangeError,
				[1,1,0]		=> RangeError,
				[-1,1,1]	=> RangeError,
				[1,-1,1]	=> RangeError,
				[1,1,-1]	=> RangeError,
				[]			=> ArgumentError,
				[1]			=> ArgumentError,
				[1,1]		=> ArgumentError,
			},

			:lx		=> {
				1			=> 1.0,
				0			=> RangeError,
				-1			=> RangeError,
			},

			:ly		=> {
				1			=> 1.0,
				0			=> RangeError,
				-1			=> RangeError,
			},

			:lz		=> {
				1			=> 1.0,
				0			=> RangeError,
				-1			=> RangeError,
			},
		},

		ODE::Geometry::Plane => {
			:__proto_args__	=> [0,0,1,0],

			:new	=> {
				[1,1,1,1]	=> ODE::Geometry::Plane,
				[0,1,1,1]	=> ODE::Geometry::Plane,
				[1,0,1,0]	=> ODE::Geometry::Plane,
				[0,0,0,0]	=> ODE::Geometry::Plane,
				[1,1,0,-1]	=> ODE::Geometry::Plane,
				[-1,1,1,0]	=> ODE::Geometry::Plane,
				[1,-1,1,0]	=> ODE::Geometry::Plane,
				[1,1,-1,0]	=> ODE::Geometry::Plane,
				[]			=> ArgumentError,
				[1]			=> ArgumentError,
				[1,0]		=> ArgumentError,
				[1,0,0]		=> ArgumentError,

				#[1,0,0,0,mockSpace]	=> ODE::Geometry::Plane,
			},

			:params	=> {
				[1,1,1,1]	=> normalizePlaneVector(1,1,1,1),
				[0,1,1,1]	=> normalizePlaneVector(0,1,1,1),
				[1,0,1,0]	=> normalizePlaneVector(1,0,1,0),
				[0,0,0,0]	=> normalizePlaneVector(0,0,0,0),
				[1,1,0,-1]	=> normalizePlaneVector(1,1,0,-1),
				[-1,1,1,0]	=> normalizePlaneVector(-1,1,1,0),
				[1,-1,1,0]	=> normalizePlaneVector(1,-1,1,0),
				[1,1,-1,0]	=> normalizePlaneVector(1,1,-1,0),
				[]			=> ArgumentError,
				[1]			=> ArgumentError,
				[1,0]		=> ArgumentError,
				[1,0,0]		=> ArgumentError,
			},
		},

		ODE::Geometry::CappedCylinder => {
			:__proto_args__	=> [1,1],

			:new	=> {
				[1,1]		=> ODE::Geometry::CappedCylinder,
				[0,1]		=> RangeError,
				[1,0]		=> RangeError,
				[-1,0]		=> RangeError,
				[0,-1]		=> RangeError,
				[]			=> ArgumentError,
				[1]			=> ArgumentError,
			},

			:params	=> {
				[1,1]		=> [1.0,1.0],
				[0,1]		=> RangeError,
				[1,0]		=> RangeError,
				[-1,0]		=> RangeError,
				[0,-1]		=> RangeError,
				[]			=> ArgumentError,
				[1]			=> ArgumentError,
			},

			:radius	=> {
				1			=> 1.0,
				0			=> RangeError,
				-1			=> RangeError,
				[]			=> TypeError,
				$stderr		=> TypeError,
			},

			:length	=> {
				1			=> 1.0,
				0			=> RangeError,
				-1			=> RangeError,
				[]			=> TypeError,
				$stderr		=> TypeError,
			},
		},
	}

	# Auto-generate test methods for each op
	GeomTests.keys.each_with_index {|klass, i|
		debugMsg "Creating test methods for #{klass.name} (i = #{i})"

		GeomTests[klass].keys.each_with_index {|op, n|
			next if op.to_s =~ /^__/

			debugMsg "Creating test method for #{op.to_s} of #{klass.name} (i = #{i}, n = #{n})"
			methodName = "test_%d_%s_%s" % [ i*10 + n, klass.name.gsub(/:+/, '_'), op.to_s ]

			eval <<-"EOCODE"
			def #{methodName}
				unit = GeomTests[#{klass.name}][:#{op.to_s}]
				testHeader "Testing '#{op.to_s}' method for '#{klass.name}'."
				runGeomTest( #{klass.name}, :#{op.to_s}, unit )
			end
			EOCODE
		}
	}


	### Run the unit test of the specified op on the given class
	def runGeomTest( klass, op, unit )
		config = {}

		# Constructor test
		if op == :new
			unit.collect {|input, expected|
				GeomUnit::new(klass, op, input, expected)
			}.each {|test|
				runInstantiationTest( test )
			}

		# Accesor test
		else
			# Get the proto args from the test hash
			args = GeomTests[klass][:__proto_args__]
			@obj = klass.new( *args )

			# Generate symbols for the get/set methods for this flag
			# from the symbol being tested.
			getter = op
			setter = ("%s=" % op.to_s).intern

			debugMsg "Using testing method: %s" %
				[ getter, setter ].
				collect {|sym| sym.inspect}.join(", ")

			# Test to be sure the interface is right
			assert_respond_to @obj, getter, "Test: %s#%s" % [ klass.name, op.inspect ]
			assert_respond_to @obj, setter, "Test: %s#%s" % [ klass.name, op.inspect ]

			# Now map all the test values into FlagUnit struct objects and
			# iterate over them.
			unit.collect {|input,expected|
				GeomUnit::new( klass, op, input, expected )
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


end

