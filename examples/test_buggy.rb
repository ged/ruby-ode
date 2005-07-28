#!/usr/bin/ruby
#
# test_buggy.rb - A (sort of) port of the test_buggy.cpp test in the ODE
# distribution
#
# $Id$
# Time-stamp: <04-Feb-2003 15:21:35 deveiant>
#
# Authors:
#   # Michael Granger <ged@FaerieMUD.org>
#
# Copyright (c) 2002, 2003 The FaerieMUD Consortium.
#
# This work is licensed under the Creative Commons Attribution License. To
# view a copy of this license, visit
# http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
# Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.
#

basedir = File::dirname(File::dirname(__FILE__))
$LOAD_PATH.unshift "#{basedir}/lib", "#{basedir}/ext"

begin
	require 'ncurses'
rescue LoadError
	$stderr.puts "Sorry. This example requires the 'ncurses' module to run."
	exit( 1 )
end

require '../utils'
include UtilityFunctions

require 'ode'
include ODE

# This is a port of the test_buggy ODE example, just to see how things
# would/will work.
module Example
	class Car
		include ODE

		RadToDeg = 180.0 / Math::PI

		Chassis = Struct::new( "CarChassis", :body, :geometry )
		Wheel = Struct::new( "CarWheel", :body, :geometry, :hinge )
		DefaultPosition = Position::new( 0, 0, 0.5 )

		def initialize( world, length=0.7, width=0.5, height=0.2, wheelRadius=0.24,
						chassisMass=1, wheelMass=0.2, position=DefaultPosition )

			@space = Space::new

			@chassis = Chassis::new
			@chassis.body = world.createBody
			@chassis.body.position = position
			@chassis.body.mass = Mass::Box::new( length, width, height, chassisMass )

			@chassis.geometry = Geometry::Box::new( length, width, height, @space )
			@chassis.geometry.body = @chassis.body

			@wheels = (0..2).to_a.collect {|i|
				# Create the wheel body
				body = world.createBody
				body.quaternion = 1, 0, 0, (ODE::Pi*1.5)
				body.mass = Mass::Sphere::new( 1, wheelRadius, wheelMass )

				# Position each wheel appropriately
				case i
				when 1
					body.position = (0.5 * length), 0, (position.z - height * 0.5);

				when 2
					body.position = (-0.5 * length), (width * 0.5), (position.z - height * 0.5);

				when 3
					body.position = (-0.5 * length), (width * -0.5), (position.z - height * 0.5);
				end

				# Create the wheel's collision geometry
				geom = Geometry::Sphere::new( wheelRadius, @space )
				geom.body = body

				# Create and configure a joint, and then use it to connect the wheel
				# to the chassis
				hinge = Hinge2Joint::new( world )
				hinge.attach( @chassis.body, body )
				hinge.anchor = body.position.x, body.position.y, body.position.z
				hinge.axis1 = 0, 0, 1
				hinge.axis2 = 0, 1, 0
				hinge.suspensionERP = 0.4
				hinge.suspensionCFM = 0.8

				Wheel::new( body, geom, hinge )
			}

			# Lock rear wheels along the steering axis
			@wheels[1..2].each {|wheel|
				wheel.hinge.loStop = 0
				wheel.hinge.hiStop = 0
			}

		end

		######
		public
		######

		# The car's top-level geometry space
		attr_reader :space

		# The car's chassis struct
		attr_reader :chassis

		# The car's wheel struct array
		attr_reader :wheels


		### Drive the car
		def drive( steering, speed )
			self.steer( steering )
			self.throttle( speed )
		end


		### Steer the car's front wheel
		def steer( steering )
			vel = steering - @wheels[0].hinge.angle1
			if ( vel > 0.1 ) then vel = 0.1 end
			if ( vel < -0.1 ) then vel = -0.1 end

			vel *= 10
			@wheels[0].hinge.velocity = vel
			@wheels[0].hinge.fMax = 0.2
			@wheels[0].hinge.loStop = -0.75
			@wheels[0].hinge.hiStop = 0.75
			@wheels[0].hinge.fudgeFactor = 0.1
		end		


		### Adjust the speed of the car's engine by the car's speed.
		def throttle( speed )
			@wheels[0].hinge.vel2 = -speed
			@wheels[0].hinge.fMax2 = 0.1
		end


		### Add the car to the specified ODE::Space.
		def >>( space )
			if space.is_a?( ODE::Space )
				self.addToSpace( space )

			else
				raise "Cannot add the car to a %s" % space.class.name
			end
		end

		### Add the car to the specified collision space
		def addToSpace( space )
			space << @space
		end

		### Remove the car from the specified collision space
		def removeFromSpace( space )
			space.removeGeometry( @space )
		end

		### Return a string describing the car's location relative to the origin.
		def location
			pos = @chassis.body.position
			return "%d from origin at (%0.1f, %0.1f, %0.1f)" %
				[ pos.distance(ODE::Position::Origin), pos.x, pos.y, pos.z ]
		end


		### Return a string describing the car's rotation relative to the default
		### orientation.
		def rotation
			quat = @chassis.body.rotation
			return "R: %0.1f  P: %0.1f  Y: %0.1f" %
				[ quat.roll * RadToDeg, quat.pitch * RadToDeg, quat.yaw * RadToDeg ]
		end

	end # class Car


	class Ramp

		include ODE

		def initialize( world, length=2, width=1.5, height=1 )
			@body = world.createBody
			@geometry = Geometry::Box::new( length, width, height )
			@geometry.body = @body
			@geometry.position = 2, 0, -0.34
			@geometry.rotation = 0, 1, 0, -0.15
		end

		# The physical body of the ramp
		attr_reader :body

		# The collision geometry of the ramp
		attr_reader :geometry


		### Add the car to the specified ODE::Space.
		def >>( space )
			if space.is_a?( ODE::Space )
				self.addToSpace( space )
			else
				raise "Cannot add the ramp to a %s" % space.class.name
			end

			return self
		end

		### Add the car to the specified collision space
		def addToSpace( space )
			space << @geometry
		end

		### Remove the car from the specified collision space
		def removeFromSpace( space )
			space.removeGeometry( @geometry )
		end

	end # class Ramp
end # module Example


#####################################################################
###	M A I N   P R O G R A M
#####################################################################

include Example

# Define some categories for culling collisions
CATEGORY_MOVING		= (1 << 0)
CATEGORY_STATIONARY	= (1 << 1)

# Create the simulation world
world = World::new
world.gravity = 0, 0, -0.5

# Create a joint group for contact joints
contactJoints = JointGroup::new

# Create the collision space and add the ground plane
space = HashSpace::new
ground = Geometry::Plane::new( 0, 0, 1, 0 )
space << ground
ground.categoryMask = CATEGORY_STATIONARY
ground.collideMask = CATEGORY_MOVING

# Create the car and add it to the world and the collision space
car = Car::new( world )
car >> space
car.space.categoryMask = CATEGORY_MOVING
car.space.collideMask = CATEGORY_STATIONARY

# Create the ramp
ramp = Ramp::new( world )
ramp >> space
ramp.geometry.categoryMask = CATEGORY_STATIONARY
ramp.geometry.collideMask = CATEGORY_MOVING


quitFlag = pauseFlag = false
contacts = []
loopCount = 0

logfile = File::open( "trace.#{Process.pid}", "w" )
$stderr.reopen( logfile )

# Init the screen and set up terminal and windows
Ncurses.initscr
begin
	$pauseWindow = nil

	def printCenter( row, msg, window=Ncurses.stdscr )
		cols = []; lines = []
		window.getmaxyx( lines, cols )
		col = (cols[0] - msg.length) / 2
		row = lines[0] if row > lines[0]
		window.mvprintw( row, col, msg )
	end

	def showPauseWindow 
		msg = "* PAUSED *"
		starty = (Ncurses.LINES - 3) / 2
		startx = (Ncurses.COLS - msg.length - 4) / 2
		
		$pauseWindow = Ncurses.newwin( 3, msg.length + 4, starty, startx )
		$pauseWindow.attr_set( Ncurses::A_STANDOUT, Ncurses.COLOR_PAIR(3), nil )
		$pauseWindow.box( 0, 0 )
		$pauseWindow.mvwprintw( 1, 2, msg )

		$pauseWindow.refresh
	end

	def hidePauseWindow 
		$pauseWindow.wborder( 32,32,32,32, 32,32,32,32 )
		$pauseWindow.clear
		$pauseWindow.refresh
		$pauseWindow.delwin
	end


	if $VERBOSE
		Ncurses.trace( Ncurses::TRACE_MAXIMUM )
	end

	Ncurses.cbreak
	Ncurses.stdscr.keypad( true )
	Ncurses.nonl
	Ncurses.noecho
	Ncurses.clear
	Ncurses.curs_set( 0 )

	$stderr.puts "Terminal initialized."

	if ( Ncurses.has_colors? )
		bg = Ncurses::COLOR_BLACK
		Ncurses.start_color
		if (Ncurses.respond_to?("use_default_colors"))
			if (Ncurses.use_default_colors == Ncurses::OK)
				bg = -1
			end
		end
		Ncurses.init_pair(1, Ncurses::COLOR_BLUE, bg);
		Ncurses.init_pair(2, Ncurses::COLOR_CYAN, bg);
		Ncurses.init_pair(3, Ncurses::COLOR_WHITE, bg);
		Ncurses.init_pair(4, Ncurses::COLOR_RED, Ncurses::COLOR_WHITE);
		$stderr.puts "Color initialized."
	else
		$stderr.puts "No color support."
	end

	# Print a banner
	printCenter( Ncurses.LINES/2, "ODE Buggy Test" )
	printCenter( Ncurses.LINES/2 + 1, "Press any key to continue..." )
	Ncurses::getch

	# Now clear the screen and turn off blocking for input
	Ncurses.clear
	Ncurses.curs_set( 0 )
	Ncurses.stdscr.nodelay( true )
	Ncurses.timeout( 0 )

	titleWindow = Ncurses.newwin( 3, Ncurses.COLS, 0, 0 )
	titleWindow.color_set( 2, nil )
	titleWindow.box( 0, 0 )
	printCenter( 1, "Experiment: ncurses test_buggy", titleWindow )
	titleWindow.refresh

	carWindow = Ncurses.newwin( 15, Ncurses.COLS / 2, 4, 0 )
	carWindow.color_set( 2, nil )
	carWindow.box( 0, 0 )
	printCenter( 0, "Car", carWindow )
	carWindow.refresh

	traceWindow = Ncurses.newwin( 15, Ncurses.COLS / 2, 4, Ncurses.COLS / 2 )
	traceWindow.color_set( 2, nil )
	traceWindow.box( 0, 0 )
	printCenter( 0, "Trace", traceWindow )
	traceWindow.refresh

	steer, throttle = 0.0, 0.0

	until ( quitFlag )

		char = Ncurses.getch()
		$stderr.puts "Read char #{char}..." if char >= 0

		case char
		when 'q'[0], 'Q'[0], "\e"[0]
			quitFlag = true

		when Ncurses::KEY_LEFT
			steer -= 0.5

		when Ncurses::KEY_RIGHT
			steer += 0.5

		when Ncurses::KEY_UP
			throttle += 0.3

		when Ncurses::KEY_DOWN
			throttle -= 0.3

		when 'p'[0]
			pauseFlag = pauseFlag ? false : true
			if pauseFlag
				showPauseWindow()
			else
				hidePauseWindow()
			end
		end

		unless ( pauseFlag )
			loopCount += 1
			traceWindow.mvprintw( 2, 3, ">>> Loop #{loopCount} <<<" )
			traceWindow.move( 3, 3 )

			# Update the car's parts
			car.drive( steer, throttle )
	
			# Set up the collision function
			space.eachAdjacentPair {|geom1, geom2, *data|
				geom1.collideWith( geom2, 10 ) {|contact|
					traceWindow.printw( "Got contact '#{contact.inspect}'.\n" )
					surface = contact.surface
					surface.mode = (Contact::Slip1|Contact::Slip2|
									Contact::SoftERP|Contact::SoftCFM|
									Contact::Approx1)
					surface.mu = ODE::Infinity
					surface.slip1 = 0.1
					surface.slip2 = 0.1
					surface.soft_erp = 0.5
					surface.soft_cfm = 0.3

					contactJoint = ContactJoint::new( world, contact, contactJoints )
					contactJoint.attach( contact.geom1.body, contact.geom2.body )

					contacts << contactJoint
				}
			}

			# Step the world and remove any contact joints created in this cycle
			world.step( 0.05 )
			contactJoints.empty
			contacts.clear

			# Update the car window with the new numbers
			carWindow.mvwprintw( 2, 3, car.location )
			carWindow.clrtoeol
			carWindow.mvwprintw( 3, 3, car.rotation )
			carWindow.clrtoeol
			carWindow.mvwprintw( 4, 3, "Throttle: %0.1f" % throttle )
			carWindow.clrtoeol
			carWindow.mvwprintw( 5, 3, "Steer: %0.1f" % steer )
			carWindow.clrtoeol

			titleWindow.box( 0, 0 )
			titleWindow.noutrefresh
			traceWindow.box( 0, 0 )
			traceWindow.noutrefresh
			carWindow.box( 0, 0 )
			carWindow.noutrefresh
			Ncurses.doupdate
		end

		quitFlag = true if loopCount >= 100_000
	end

rescue => e
	$stderr.puts "%s:\n\t%s" % [ e.message, e.backtrace.join("\n\t") ]
	
	begin
		errorWindow = Ncurses.newwin( Ncurses.LINES-1, Ncurses.COLS-1, 0, 0 )
		errorWindow.color_set( 4, nil )

		errorWindow.box( 0, 0 )
		printCenter( 0, " Error ", errorWindow )
		errorWindow.mvwprintw( 2, 3, e.message )
		e.backtrace.each_with_index do |frame,i|
			errorWindow.mvwprintw( 3 + i, 7, frame )
		end

		errorWindow.mvwprintw( 2 + e.backtrace.length + 2, 3, "Any key to exit..." )
		errorWindow.refresh

		Ncurses.curs_set( 0 )
		Ncurses.stdscr.nodelay( false )
		Ncurses.timeout( -1 )

		Ncurses.getch
	rescue => e
		$stderr.puts "%s:\n\t%s" % [ e.message, e.backtrace.join("\n\t") ]
	end
ensure
	Ncurses.curs_set(1)
	Ncurses.endwin()
end

