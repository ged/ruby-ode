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
# Copyright (c) 2003 The FaerieMUD Consortium. All rights reserved.
# 
# This module is free software. You may use, modify, and/or redistribute this
# software under the terms of the Perl Artistic License. (See
# http://language.perl.com/misc/Artistic.html)
# 
# == Version
#
#  $Id: Space.rb,v 1.1 2003/02/04 11:28:21 deveiant Exp $
# 

require 'ode'

module ODE

	### Instance of this class are collision spaces..
	class Space

		### Class constants
		Version ||= /([\d\.]+)/.match( %q{$Revision: 1.1 $} )[1]
		Rcsid ||= %q$Id: Space.rb,v 1.1 2003/02/04 11:28:21 deveiant Exp $


		######
		public
		######

		### Returns a human-readable string containing a representation of the
		### Space suitable for debugging or tracing.
		def inspect
			return "<%s 0x%x: geometries=[%s]>" % [
				self.class.name,
				self.object_id * 2,
				self.geometries.collect {|geom| geom.inspect}.join(", "),
			]
		end


	end # class Space

end # module ODE

