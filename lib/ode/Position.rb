#!/usr/bin/ruby
# 
# This file contains the ODE::Position class, instances of which represent a
# position point in an ODE::World simulation.
#
# == Synopsis
# 
#	require 'ode'
#
#   pos = ODE::Position::new( 12.1, 19, 13 )
#	body.position = pos
#
#   body.position = 12.1, 19, 13
#   body.position
#   # => <ODE::Position: 12.10000, 19.00000, 13.00000>
# 
#	puts "Body is at: %0.2f x %0.2f x %0.2f" % [ pos.x, pos.y, pos.z ]
#	# -> Body is at: 12.10 x 19.00 x 13.00
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
#  $Id: Position.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
# 

require 'ode/Vector'

module ODE

	### Instances of this class represent the position of a point in an
	### ODE::World simulation.
	class Position < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q{$Revision: 1.1 $} )[1]
		Rcsid = %q$Id: Position.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $

		### Create and return a new Position object with the specified
		### coordinates.
		def initialize( x=0, y=0, z=0 )
			super
		end


	end # class Position
end # module ODE
