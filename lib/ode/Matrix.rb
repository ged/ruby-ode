#!/usr/bin/ruby
# 
# This file contains the ODE::Matrix class. It's just an elaboration of the
# Matrix class that comes with Ruby's standard library.
# 
# == Synopsis
# 
#   ODE::Matrix[row0,
#				row1,
#				 :
#				rown]
# 
# == Authors
# 
# * Michael Granger <ged@FaerieMUD.org>
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
#  $Id: Matrix.rb,v 1.1 2003/02/04 11:28:21 deveiant Exp $
# 

require 'matrix'

module ODE

	### Instance of this class are matrices..
	class Matrix < ::Matrix

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: Matrix.rb,v 1.1 2003/02/04 11:28:21 deveiant Exp $

	end # class Matrix

end # module ODE

