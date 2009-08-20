#!/usr/bin/ruby
# 
# This file contains the ODE::LinearVelocity class. Instances of this class
# represent the velocity of a ODE::Body in an ODE::World simulation.
#
# == Synopsis
#
#   # Are the two bodies moving in similar directions?
#   body1.linearVelocity.almost_equal( body2.linearVelocity, 0.5 )
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
#  $Id$
# 

require 'ode/vector'

module ODE

	### Instances of this class represent the linear velocity of a ODE::Body in
	### an ODE::World simulation.
	class LinearVelocity < ODE::Vector

		### Create and return a new LinearVelocity object with the specified
		### coordinates.
		def initialize( x=0, y=0, z=0 )
			super( x, y, z )
		end

	end # class LinearVelocity
end # module ODE
