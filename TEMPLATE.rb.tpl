#!/usr/bin/ruby
# 
# This file contains the ODE::(>>>FILE_SANS<<<) class. Instance of this class
# (>>>description<<<).
# 
# == Synopsis
# 
#   (>>>POINT<<<)
# 
# == Author
# 
# Michael Granger <ged@FaerieMUD.org>
# 
# Copyright (c) (>>>YEAR<<<) The FaerieMUD Consortium. All rights reserved.
# 
# This module is free software. You may use, modify, and/or redistribute this
# software under the terms of the Perl Artistic License. (See
# http://language.perl.com/misc/Artistic.html)
# 
# == Version
#
#  $Id: TEMPLATE.rb.tpl,v 1.1 2002/11/23 21:43:05 deveiant Exp $
# 

(>>>MARK<<<)

module ODE

	### Instance of this class (>>>description<<<).
	class (>>>FILE_SANS<<<) < (>>>superclass<<<)

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: TEMPLATE.rb.tpl,v 1.1 2002/11/23 21:43:05 deveiant Exp $

		### Create a new ODE::(>>>FILE_SANS<<<) object.
		def initialize
		end


		######
		public
		######


		#########
		protected
		#########


	end # class (>>>FILE_SANS<<<)

end # module ODE

