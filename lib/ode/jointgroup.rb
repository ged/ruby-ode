#!/usr/bin/ruby
# 
# This file contains the Ruby half of the ODE::JointGroup class.
# 
# == Synopsis
# 
#   require 'ode'
#
#	hingeGroup = ODE::JointGroup::new( world, ODE::Joint::Hinge )
#
#	joint1 = hingeGroup.createJoint
#	joint2 = hingeGroup.createJoint
#
#	joint1.attach( body1, body2 )
#	joint2.attach( body2, body3 )
# 
# == Authors
# 
# * Michael Granger <ged@FaerieMUD.org>
# 
# Copyright (c) 2002, 2003 The FaerieMUD Consortium. 
# 
# This work is licensed under the Creative Commons Attribution License. To
# view a copy of this license, visit
# http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
# Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
# 
# == Version
#
#  $Id$
# 

require 'ode'

module ODE

	### Ruby half of the ODE::JointGroup class.
	class JointGroup

		######
		public
		######

		# The ODE::Joint class to instantiate with #createJoint.
		attr_reader :factoryClass

		# The ODE::World to create new joints in
		attr_reader :factoryWorld


		### Set the factory class for this JointGroup.
		def factoryClass=( jointClass )
			raise TypeError, "No implicit conversion of #{jointClass.class.name} to Class" unless
				(jointClass.nil? || jointClass.kind_of?( Class ))
			@factoryClass = jointClass
		end

		### Set the factory world for this JointGroup
		def factoryWorld=( world )
			raise TypeError, "No implicit conversion of #{world.class.name} to ODE::World" unless
				(world.nil? || world.kind_of?( ODE::World ))
			@factoryWorld = world
		end


		### Returns true if the JointGroup can act as a factory for member joints.
		def factory?
			return ! @factoryClass.nil? && ! @factoryWorld.nil?
		end


		### Create a new joint in the receiving JointGroup with the specified
		### +arguments+. If the jointClass and world arguments weren't specified in
		### this JointGroup's constructor, an exception is raised.
		def createJoint( *arguments )
			raise RuntimeError, "Not a factory JointGroup" unless self.factory?
			arguments.push( self )
			return @factoryClass.new( @factoryWorld, *arguments )
		end

	end # class jointGroup
end # module ODE
