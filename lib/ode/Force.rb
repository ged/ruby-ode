#!/usr/bin/ruby
# 
# This file contains the ODE::Force class, instances of which represent a
# 3-dimensional linear force vector that can be applied to an ODE::Body to move
# it around.
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
#  $Id: Force.rb,v 1.2 2003/02/04 11:28:45 deveiant Exp $
# 



module ODE

	### Instances of this class represent a 3-dimensional linear force vector
	### that can be applied to an ODE::Body to move it around.
	class Force < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.2 $ )[1]
		Rcsid = %q$Id: Force.rb,v 1.2 2003/02/04 11:28:45 deveiant Exp $

		### Create and return a new Force object with the specified
		### coordinates.
		def initialize( x=0, y=0, z=0 )
			super( x, y, z )
		end

	end # class Force

end # module ODE

