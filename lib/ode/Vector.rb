#!/usr/bin/ruby
# 
# This file contains the ODE::Vector class, instances of which represent a
# 3rd-order vector of some sort. It is the superclass for classes like
# ODE::AngularVelocity, ODE::LinearVelocity, and ODE::ForceVector.
# 
# This class is mostly just a thin elaboration of the Math3d::Vector3 class.
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
#  $Id: Vector.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
# 

require 'math3d'
require 'forwardable'

module ODE

	### Instance of this class represent a quantity that has a magnitude and a direction..
	class Vector

		extend Forwardable

		### Class constants
		Version = /([\d\.]+)/.match( %q{$Revision: 1.1 $} )[1]
		Rcsid = %q$Id: Vector.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $

		DelegateClass = Math3d::Vector3
		NonDelegatedMethods = %w{initialize inspect}
		DelegatedMethods = DelegateClass.ancestors.
			find_all {|klass| klass.name =~ /Math3d/}.
			collect {|klass| klass.instance_methods }.
			flatten.uniq - NonDelegatedMethods

		### Create a new Vector object
		def initialize( *args )
			@data = DelegateClass::new( *args )
		end


		######
		public
		######

		# Delegate header methods to the header object
		def_delegators :@data, *DelegatedMethods

		# The underlying vector data object
		attr_reader :data

	end # class Vector

end # module ODE

