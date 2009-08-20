#!/usr/bin/ruby

require 'ode'
require 'ode/vector'

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
class AngularVelocity < ODE::Vector; end
