#!/usr/bin/ruby -w

require '../utils'
include UtilityFunctions

# Minimal test case for a segfault in Ruby-20021117.

header "Experiment: Proc.inspect segfault minimal testcase"

message "Setting up struct class"
FlagUnit = Struct::new( "FlagUnit", :op, :input, :result, :predicate )
message "Creating struct"
test = FlagUnit::new( :mu2?, Proc::new {"foo"},	TypeError, false )
message "Inspecting struct"
puts test.inspect

