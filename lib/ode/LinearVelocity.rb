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
#  $Id: LinearVelocity.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
# 

require 'ode/Vector'

module ODE

	### Instances of this class represent the linear velocity of a ODE::Body in
	### an ODE::World simulation.
	class LinearVelocity < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: LinearVelocity.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $

	end # class LinearVelocity
end # module ODE
