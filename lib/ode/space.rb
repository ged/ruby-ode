#!/usr/bin/ruby
# 
# This file contains the ODE::Space class. Instance of this class
# are collision spaces..
# 
# == Synopsis
# 
#   
# 
# == Author
# 
# Michael Granger <ged@FaerieMUD.org>
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

	### Instance of this class are collision spaces..
	class Space

		### Class constants
		Version ||= /([\d\.]+)/.match( %q{$Revision: 1.2 $} )[1]
		Rcsid ||= %q$Id$


		######
		public
		######

		### Returns a human-readable string containing a representation of the
		### Space suitable for debugging or tracing.
		def inspect
			return "<%s 0x%x: geometries=[%s]>" % [
				self.class.name,
				self.respond_to?( :object_id ) ? self.object_id * 2 : self.id * 2,
				self.geometries.collect {|geom| geom.inspect}.join(", "),
			]
		end


	end # class Space

end # module ODE

