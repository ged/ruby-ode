#!/usr/bin/ruby
# 
# This file contains the ODE::Contact class. Instance of this class
# express a contact between two geometries in an ODE physics simulation..
# 
# == Synopsis
# 
#   
# 
# == Authors
# 
# * Michael Granger <ged@FaerieMUD.org>
# 
# Copyright (c) 2003 The FaerieMUD Consortium.
# 
# This work is licensed under the Creative Commons Attribution License. To
# view a copy of this license, visit
# http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
# Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
# 
# == Version
#
#  $Id$
# 

require 'ode'

module ODE

	### The contact class.
	class Contact

		######
		public
		######

		### Returns a string containing a human-readable representation of the
		### contact, oriented towards debugging or tracing.
		def inspect
			"<%s 0x%x: surface=%s, geom1=%s, geom2=%s>" % [
				self.class.name,
				self.object_id * 2,
				self.surface.inspect,
				self.geom1.inspect,
				self.geom2.inspect
			]
		end


		### Return a human-readable string oriented towards english
		### descriptions.
		def to_s
			"contact between a %s and a %s at %s (depth = %0.2f)" % [
				self.geom1.to_s,
				self.geom2.to_s,
				self.pos.to_s,
				self.depth,
			]
		end

	end # class Contact

end # module ODE

