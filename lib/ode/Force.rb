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
#  $Id: Force.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
# 



module ODE

	### Instances of this class represent a 3-dimensional linear force vector
	### that can be applied to an ODE::Body to move it around.
	class Force < ODE::Vector

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: Force.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $

	end # class Force

end # module ODE

