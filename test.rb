#!/usr/bin/ruby
#
#	Test suite for Rode
#
#

$LOAD_PATH.unshift "lib", "ext"

require 'find'
require 'test/unit'
require 'test/unit/testsuite'
# begin
# 	require 'test/unit/ui/fox/testrunner'
# rescue
	require 'test/unit/ui/console/testrunner'
#end

if ARGV.length.nonzero?
	patsrc = "(" + ARGV.join('|') + ")"
	$stderr.puts "Looking for tests matching /#{patsrc}/"
	pattern = /#{patsrc}/i
else
	pattern = nil
end

### Load all the tests from the tests dir
Find.find("tests") {|file|
	Find.prune if file =~ %r{(^|\/)[\.#]} or file =~ /~$/
	Find.prune if file =~ /TEMPLATE/
	next if File.stat( file ).directory?
	next unless file =~ /tests.rb$/
	next if pattern && pattern !~ file

	$stderr.puts "Found '#{file}'"
	require "#{file}"
}

class ODETests

	def initialize
		@suite = Test::Unit::TestSuite.new( "Ruby-ODE" )
		classes = []

		ObjectSpace.each_object( Class ) {|klass|
			if klass < Test::Unit::TestCase
				classes << klass
			end
		}

		$stderr.puts "WARNING: No matching testcases found." if classes.empty?
		classes.each {|klass|
			@suite.add( klass.suite )
		}
	end

	attr_accessor :suite

	def size
		self.suite.size
	end

	def run( *args )
		self.suite.run( *args )
	end

	def to_s
		"Ruby-ODE Test Suite (%d tests)" % self.size
	end
end


if ENV.key?( "DISPLAY" ) && Object::const_defined?( :Fox )
	Test::Unit::UI::Fox::TestRunner.run( ODETests::new )
else
	Test::Unit::UI::Console::TestRunner.run( ODETests::new )
end





