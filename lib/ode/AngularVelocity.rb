#!/usr/bin/ruby
# 
# This file contains the ODE::AngularVelocity class. Instance of this class
# represent an ODE::Body object's motion about one or more of its axes in an
# ODE::World simulation.
# 
# == Synopsis
# 
#   # Are two bodies rotating similarly?
#	body1.angularVelocity.almost_equal( body2.angularVelocity, 0.5 )
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
#  $Id: AngularVelocity.rb,v 1.2 2003/02/04 11:28:45 deveiant Exp $
# 

require 'ode/Vector'

module ODE

	### Instance of this class represent an ODE::Body object's motion about one
	### or more of its axes in an ODE::World simulation.
	class AngularVelocity < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.2 $ )[1]
		Rcsid = %q$Id: AngularVelocity.rb,v 1.2 2003/02/04 11:28:45 deveiant Exp $

	end # class AngularVelocity
end # module ODE

