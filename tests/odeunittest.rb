#!/usr/bin/ruby
# 
# Test case class
# 
# == Synopsis
# 
#   
# 
# == Author
# 
# Michael Granger <ged@FaerieMUD.org>
# 
# Copyright (c) 2002 The FaerieMUD Consortium. All rights reserved.
# 
# This module is free software. You may use, modify, and/or redistribute this
# software under the terms of the Perl Artistic License. (See
# http://language.perl.com/misc/Artistic.html)
# 
# == Version
#
#  $Id: odeunittest.rb,v 1.2 2002/11/23 21:48:56 deveiant Exp $
# 

basedir = File::dirname(File::dirname( __FILE__ ))
$LOAD_PATH.unshift "#{basedir}/ext", "#{basedir}/lib"

require "test/unit"
require "ode"

### Test case class
module ODE
	class TestCase < Test::Unit::TestCase

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
			$stderr.puts "%sDEBUG>>> %s %s" %
				[ ansiCode('dark', 'white'), msgs.join(''), ansiCode('reset') ]
			$stderr.flush
		end


		### Output the specified <tt>msgs</tt> joined together to
		### <tt>STDOUT</tt>.
		def message( *msgs )
			$stderr.puts msgs.join('')
			$stderr.flush
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
		def testHeader( desc )
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


		### Additional assertion methods

		### Passes if <tt>actual</tt> matches the given <tt>regexp</tt>.
		def assert_match( regexp, actual, message=nil )
			_wrap_assertion {
				assert(regexp.kind_of?(Regexp), "The first parameter to assert_matches should be a Regexp.")
				full_message = build_message(message, actual, regexp) {
					| arg1, arg2 |
					"Expected <#{arg1}> to match #{arg2.inspect}"
				}
				assert_block(full_message) {
					regexp.match( actual )
				}
			}
		end



		### This was copied from test/unit.rb because I can't call it from
		### here. Grrr... one shouldn't make methods useful to subclassers
		### private.

		def _wrap_assertion # :nodoc:
			@_assertion_wrapped ||= false
			if (!@_assertion_wrapped)
				@_assertion_wrapped = true
				begin
					add_assertion
					return yield
				ensure
					@_assertion_wrapped = false
				end
			else    
				return yield
			end
		end

	end # module TestCase
end # module ODE

