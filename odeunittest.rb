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
#  $Id: odeunittest.rb,v 1.1 2002/03/20 05:28:37 deveiant Exp $
# 

if File.directory? "src"
	$:.unshift "src", "ext"
elsif File.directory? "../src"
	$:.unshift "../src", "../ext", ".."
end

require "test/unit"

### Test case class
module ODE
	class TestCase < Test::Unit::TestCase

		def ansicode( *codes )
			return "\033[#{codes.collect {|x| sprintf '%02d',x}.join(':')}m"
		end

		### Add a debugging message to the test output if -w is turned on
		def debugMsg( *messages )
			return unless $DEBUG
			$stderr.puts messages.join('')
			$stderr.flush
		end

		### Output a header for delimiting tests
		def testHeader( desc )
			debugMsg( ansicode(1,33) + ">>> " + desc + " <<<" + ansicode(0) )
		end

		### Try to force garbage collection to start.
		def collectGarbage
			a = []
			1000.times { a << {} }
			a = nil
			GC.start
		end

	end # module TestCase
end # module ODE

