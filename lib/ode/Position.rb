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
#  $Id: Position.rb,v 1.2 2003/02/04 11:28:45 deveiant Exp $
# 

require 'ode/Vector'

module ODE

	### Instances of this class represent the position of a point in an
	### ODE::World simulation.
	class Position < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q{$Revision: 1.2 $} )[1]
		Rcsid = %q$Id: Position.rb,v 1.2 2003/02/04 11:28:45 deveiant Exp $

		Origin = new(0,0,0)

		### Create and return a new Position object with the specified
		### coordinates.
		def initialize( x=0, y=0, z=0 )
			super( x, y, z )
		end

		
		### Returns the receiver's distance from the <tt>other</tt>
		### ODE::Position.
		def distance( other=Origin )
			raise TypeError, "no implicit conversion from %s to %s" %
				[ other.class.name, self.class.name ] unless
				other.is_a?( ODE::Position )

			return Math::sqrt( (self.x - other.x) ** 2 +
							   (self.y - other.y) ** 2 +
							   (self.z - other.z) ** 2 )
							  
		end

		### Returns a human-readable string representing the position
		def to_s
			"|x = %0.2f, y = %0.2f, z = %0.2f|" %
				[self.x, self.y, self.z]
		end

	end # class Position
end # module ODE
