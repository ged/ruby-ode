#!/usr/bin/ruby
#
#	Test suite for Rode
#
#


$LOAD_PATH.unshift "src", "ext"

require 'find'
require 'test/unit'
require 'test/unit/testsuite'
require 'test/unit/ui/console/testrunner'

require 'ode'

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
	next unless file =~ /_tests.rb$/
	next if pattern && pattern !~ file

	$stderr.puts "Found '#{file}'"
	require "#{file}"
}

class MUESTests
	class << self
		def suite
			suite = Test::Unit::TestSuite.new( "Ruby-ODE" )
			classes = []

			ObjectSpace.each_object( Class ) {|klass|
				if klass < Test::Unit::TestCase
					classes << klass
				end
			}

			$stderr.puts "WARNING: No matching testcases found." if classes.empty?
			classes.each {|klass|
				suite.add( klass.suite )
			}
			return suite
		end
	end
end

Test::Unit::UI::Console::TestRunner.new( MUESTests ).start




