#!/usr/bin/ruby
# 
# This file contains the Ruby half of the ODE::JointGroup class.
# 
# == Synopsis
# 
#   require 'ode'
#	require 'ode/SurfaceLibrary'
#
#	contactGroup = ODE::JointGroup.new( world, ODE::ContactJoint )
#
#	joint1 = contactGroup.createJoint( ODE::SL::ConcreteSurface )
#	joint2 = contactGroup.createJoint( ODE::SL::GlassSurface )
#
#	joint1.attach( body1, body2 )
#	joint2.attach( body2, body3 )
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
#  $Id: JointGroup.rb,v 1.1 2002/03/20 05:57:06 deveiant Exp $
# 


module ODE

	### Ruby half of the ODE::JointGroup class.
	class JointGroup

		### Class constants
		Version = /([\d\.]+)/.match( %q$Revision: 1.1 $ )[1]
		Rcsid = %q$Id: JointGroup.rb,v 1.1 2002/03/20 05:57:06 deveiant Exp $

		### Create a new ODE::JointGroup object. If the <tt>world</tt> and
		### <tt>jointClass</tt> arguments are given, the JointGroup can act as a
		### factory for creating and adding new joints to the group via
		### #createJoint.
		def initialize( jointClass=nil, world=nil )
			raise TypeError, "No implicit conversion of #{jointClass.type.name} to Class" unless
				(jointClass.nil? || jointClass.kind_of?( Class ))
			raise TypeError, "No implicit conversion of #{world.type.name} to ODE::World" unless
				(world.nil? || world.kind_of?( ODE::World ))

			@factoryClass = jointClass
			@factoryWorld = world
		end


		######
		public
		######

		### Returns true if the JointGroup can act as a factory for member joints.
		def factory?
			return ! @factoryClass.nil? && ! @factoryWorld.nil?
		end

		### Create a new joint in the receiving JointGroup with the specified
		### +arguments+. If the jointClass and world arguments weren't specified in
		### this JointGroup's constructor, an exception is raised.
		def createJoint( *arguments )
			raise RuntimeError, "Not a factory JointGroup" unless self.factory?

			return @factoryClass.new( @factoryWorld, self, *arguments )
		end


		#########
		protected
		#########


	end # class jointGroup
end # module ODE
