#!/usr/bin/ruby -w

$LOAD_PATH.unshift "lib", "ext"
require 'ode'

require '../utils'
include UtilityFunctions

# Minimal test case for a segfault in Ruby-20021117.

header "Experiment: Proc.to_s segfault minimal testcase"

# Test unit struct -- makes passing test data around a bit more readble.
FlagUnit = Struct::new( "FlagUnit", :op, :input, :result, :predicate )

# Test data for flag/option accessors (Mapped into FlagUnit objects)
FlagTests = {
	# Flag/accessor (:op)
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


FlagTests.keys.each {|k|
	FlagTests[k].each_with_index {|unit,i|
		message "#{k.to_s} ##{i}:: "
		unitStruct = FlagUnit::new( k, *unit )

		message unitStruct.inspect + "\n"
	}
}



