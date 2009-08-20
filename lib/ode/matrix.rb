#!/usr/bin/ruby

require 'matrix'

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
class ODE::Matrix < ::Matrix; end

