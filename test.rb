#!/usr/bin/ruby
#
#	Test suite for Rode
#
#

BEGIN { $LOAD_PATH.unshift "tests", "lib", "ext" }

require 'find'
require 'test/unit'
require 'test/unit/testsuite'
require 'test/unit/ui/console/testrunner'
require 'odeunittest'

if Object::const_defined?( :Fox )
 	require 'test/unit/ui/fox/testrunner'
end

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
			if klass < ODE::TestCase
				classes << klass
			end
		}

		$stderr.puts "WARNING: No matching testcases found." if classes.empty?
		if @suite.respond_to?( :<< )
			classes.each {|klass| @suite << klass.suite }
		elsif @suite.respond_to?( :add )
			classes.each {|klass| @suite.add(klass.suite) }
		else
			raise "This test suite requires a Test::Unit >= 1.5"
		end

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

$defout.sync = true

if ENV.key?( "DISPLAY" ) && Object::const_defined?( :Fox )
	Test::Unit::UI::Fox::TestRunner.run( ODETests::new )
else
	Test::Unit::UI::Console::TestRunner.run( ODETests::new )
end





