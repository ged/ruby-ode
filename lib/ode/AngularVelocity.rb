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
#  $Id: AngularVelocity.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
# 

require 'ode/Vector'

module ODE

	### Instance of this class represent an ODE::Body object's motion about one
	### or more of its axes in an ODE::World simulation.
	class AngularVelocity < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: AngularVelocity.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $

	end # class AngularVelocity
end # module ODE

