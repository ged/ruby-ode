#!/usr/bin/ruby

$LOAD_PATH.unshift File::dirname(__FILE__)
require "odeunittest"

class JointGroupTestCase < ODE::TestCase

	def setup 
		@world = ODE::World.new
		super
	end
	alias_method :set_up, :setup

	def teardown 
		@world = nil
		super
	end
	alias_method :tear_down, :teardown


	### Test instantiation
	def test_00_create
		testHeader "Test creation of empty JointGroups"
		jointGroup = nil

		assert_raises(TypeError) { ODE::JointGroup.new("world") } 
		collectGarbage()

		assert_raises(TypeError) { ODE::JointGroup.new(ODE::SliderJoint, "world") } 
		collectGarbage()

		assert_nothing_raised { ODE::JointGroup.new } 
		collectGarbage()

		assert_nothing_raised { ODE::JointGroup.new(ODE::SliderJoint, @world) } 
		collectGarbage()

		# Add new setup/teardown code
		self.class.addSetupBlock {
			@group = ODE::JointGroup::new
		}
		self.class.addTeardownBlock {
			@group = nil
		}
	end

	
	### Test empty on an empty group
	def test_01_empty_on_empty_group
		testHeader "Test #empty on already-empty JointGroup"
		assert_nothing_raised {
			@group.empty
		}
	end


	### Test factory methods
	def test_02_factory_methods
		testHeader "Test factory methods"
		rval = nil
		joints = []

		assert_nothing_raised { rval = @group.factory? }
		assert_equal false, rval

		assert_nothing_raised { @group.factoryClass = ODE::BallJoint }
		assert !@group.factory?, "JointGroup is a factory despite not having a factory world set."

		assert_nothing_raised { @group.factoryWorld = @world }
		assert @group.factory?, "JointGroup is not a factory despite having both requirements set."

		assert_nothing_raised { rval = @group.factoryClass }
		assert_equal rval, ODE::BallJoint
		assert_nothing_raised { rval = @group.factoryWorld }
		assert_equal rval, @world

		5.times do |i|
			assert_nothing_raised { joints << @group.createJoint }
			assert_equal i+1, joints.length
		end

		assert_nothing_raised { @group.factoryClass = nil }
		assert !@group.factory?, "JointGroup is a factory despite not having a factory class set."

		assert_raises( RuntimeError ) { rval = @group.createJoint }

		# Add new setup/teardown code
		self.class.addSetupBlock {
			@group.factoryClass = ODE::BallJoint
			@group.factoryWorld = @world
		}
	end


	### Test empty? predicate
	def test_03_empty_p
		testHeader "Test #empty? predicate method"
		rval = nil
		joints = []

		assert_nothing_raised { rval = @group.empty? }
		assert_equal true, rval
		
		joints << @group.createJoint

		assert_nothing_raised { rval = @group.empty? }
		assert_equal false, rval
	end


	### Test empty method (with joints)
	def test_04_empty
		testHeader "Test #empty"
		joints = []

		5.times { joints << @group.createJoint }

		assert_nothing_raised { @group.empty }
		assert @group.empty?, "JointGroup not empty after .empty() was called."
	end


end

