#!/usr/bin/ruby
# 
# Test case class
# 
# == Synopsis
# 
#   
# 
# == Authors
# 
# * Michael Granger <ged@FaerieMUD.org>
# 
# Copyright (c) 2002, 2003 The FaerieMUD Consortium.
# 
# This work is licensed under the Creative Commons Attribution License. To
# view a copy of this license, visit
# http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
# Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
# 
# == Version
#
#  $Id: odeunittest.rb,v 1.5 2003/06/27 13:37:59 deveiant Exp $
# 

basedir = File::dirname(File::dirname( __FILE__ ))
$LOAD_PATH.unshift "#{basedir}/ext", "#{basedir}/lib"

require "test/unit"
require "ode"

### Test case class
module ODE
	class TestCase < Test::Unit::TestCase

		@@methodCounter = 0

		# Set some ANSI escape code constants (Shamelessly stolen from Perl's
		# Term::ANSIColor by Russ Allbery <rra@stanford.edu> and Zenin <zenin@best.com>
		AnsiAttributes = {
			'clear'      => 0,
			'reset'      => 0,
			'bold'       => 1,
			'dark'       => 2,
			'underline'  => 4,
			'underscore' => 4,
			'blink'      => 5,
			'reverse'    => 7,
			'concealed'  => 8,

			'black'      => 30,   'on_black'   => 40, 
			'red'        => 31,   'on_red'     => 41, 
			'green'      => 32,   'on_green'   => 42, 
			'yellow'     => 33,   'on_yellow'  => 43, 
			'blue'       => 34,   'on_blue'    => 44, 
			'magenta'    => 35,   'on_magenta' => 45, 
			'cyan'       => 36,   'on_cyan'    => 46, 
			'white'      => 37,   'on_white'   => 47
		}

		# ANSI escape to move to the previous line and clear it
		ErasePreviousLine = "\033[A\033[K"


		### Inheritance callback -- adds @setupMethods and @teardownMethods ivars
		### and accessors to the inheriting class.
		def self.inherited( klass )
			klass.module_eval {
				@setupMethods = []
				@teardownMethods = []

				class << self
					attr_accessor :setupMethods
					attr_accessor :teardownMethods
				end
			}
		end


		### Returns a String containing the specified ANSI escapes suitable for
		### inclusion in another string. The <tt>attributes</tt> should be one
		### or more of the keys of AnsiAttributes.
		def self.ansiCode( *attributes )
			attr = attributes.collect {|a| AnsiAttributes[a] ? AnsiAttributes[a] : nil}.compact.join(';')
			if attr.empty? 
				return ''
			else
				return "\e[%sm" % attr
			end
		end


		### Output the specified <tt>msgs</tt> joined together to
		### <tt>STDERR</tt> if <tt>$DEBUG</tt> is set.
		def self.debugMsg( *msgs )
			return unless $DEBUG
			self.message "%sDEBUG>>> %s %s" %
				[ ansiCode('dark', 'white'), msgs.join(''), ansiCode('reset') ]
		end


		### Output the specified <tt>msgs</tt> joined together to
		### <tt>STDOUT</tt>.
		def self.message( *msgs )
			$stderr.puts msgs.join('')
			$stderr.flush
		end


		### Add a setup block for the current testcase
		def self.addSetupBlock( &block )
			@@methodCounter += 1
			newMethodName = "setup_#{@@methodCounter}".intern
			define_method( newMethodName, &block )
			self.setupMethods.push newMethodName
		end

			
		### Add a teardown block for the current testcase
		def self.addTeardownBlock( &block )
			@@methodCounter += 1
			newMethodName = "teardown_#{@@methodCounter}".intern
			define_method( newMethodName, &block )
			self.teardownMethods.unshift newMethodName
		end


		#############################################################
		###	I N S T A N C E   M E T H O D S
		#############################################################

		### Run dynamically-added setup methods
		def setup( *args )
			self.class.setupMethods.each {|sblock|
				self.send( sblock )
			}
		end


		### Backward-compatibility method for older versions of Test::Unit
		def set_up( *args )
			setup( *args )
		end


		### Run dynamically-added teardown methods
		def teardown( *args )
			self.class.teardownMethods.each {|tblock|
				self.send( tblock )
			}
		end


		### Backward-compatibility method for older version of Test::Unit
		def tear_down( *args )
			teardown( *args )
		end


		### Instance alias for the like-named class method.
		def message( *msgs )
			self.class.message( *msgs )
		end


		### Instance-alias for the like-named class method
		def ansiCode( *attributes )
			self.class.ansiCode( *attributes )
		end


		### Instance alias for the like-named class method
		def debugMsg( *msgs )
			self.class.debugMsg( *msgs )
		end


		### Replace the previous line with the specified <tt>msgs</tt>.
		def replaceMessage( *msgs )
			$stderr.print ErasePreviousLine
			message( *msg )
		end


		### Output a separator line made up of <tt>length</tt> of the specified
		### <tt>char</tt>.
		def writeLine( length=75, char="-" )
			$stderr.puts "\r" + (char * length )
		end


		### Output a header for delimiting tests
		def printTestHeader( desc )
			return unless $VERBOSE || $DEBUG
			message "%s>>> %s <<<%s" % 
				[ ansiCode('bold','yellow','on_blue'), desc, ansiCode('reset') ]
		end


		### Try to force garbage collection to start.
		def collectGarbage
			a = []
			1000.times { a << {} }
			a = nil
			GC.start
		end


		### Output the name of the test as it's running if in verbose mode.
		def run( result )
			$stderr.puts self.name if $VERBOSE || $DEBUG
			super
		end

	end # module TestCase
end # module ODE

