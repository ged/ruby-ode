#!/usr/bin/ruby
# 
# This file contains the ODE::Torque class, instances of which represent
# 3-dimensional angular force vectors that can be applied to an ODE::Body to
# rotate it in place.
# 
# == Synopsis
# 
#   
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
#  $Id: Torque.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
# 

require 'ode/Vector'

module ODE

	### Instance of this class represent 3-dimensional angular force vectors
	### that can be applied to an ODE::Body to rotate it in place.
	class Torque < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: Torque.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $

	end # class Torque

end # module ODE

