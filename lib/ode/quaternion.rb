#!/usr/bin/ruby
#
# This file contains the ODE::Quaternion (3D spinor) class. Instances of this
# class represent the orientation of an object in an ODE::World simulation.
#
# From the <b>The Matrix and Quaternions FAQ</b>
# (http://skal.planet-d.net/demo/matrixfaq.htm):
#
#   Quaternions extend the concept of rotation in three dimensions to
#   rotation in four dimensions. This avoids the problem of "gimbal-lock"
#   and allows for the implementation of smooth and continuous rotation.
#
#   In effect, they may be considered to add a additional rotation angle
#   to spherical coordinates ie. Longitude, Latitude and Rotation angles
#
#   A Quaternion is defined using four floating point values |x y z w|.
#
#   These are calculated from the combination of the three coordinates
#   of the rotation axis and the rotation angle.
#
# This class is based on the GLIT::Quat class from the alg3d library by Issac
# Trotts.
#
# == Synopsis
#
#
#
# == Authors
#
# Based on the GLIT::Quat class from the 'alg3d' library by Issac Trotts. Used
# under the following licensing terms:
#
#   alg3d.rb : an any-dimensional vector and 3D spinor (quaternion)
#
#   Copyright (c) 2001, 2003 Issac Trotts <itrotts at idolminds dot com>
#   Under the Ruby license
#
# Modifications, additional code, and any mistakes by Michael Granger
# <ged@FaerieMUD.org>.
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

require 'ode/Vector'
require 'ode/Matrix'

# Workaround for Ruby versions without copy_object:
unless Module::methods.include? "copy_object"
	Object.module_eval <<-EOF
		def copy_object( other )
			self.instance_variables.each {|ivar|
				val = eval(ivar)
				other.instance_eval("\#{ivar} = val")
			}
			return self
		end
		public :copy_object
	EOF
end

module ODE

	class Quaternion

		include Math

		### Class constants
		Version = /([\d\.]+)/.match( %q{$Revision: 1.3 $} )[1]
		Rcsid = %q$Id$

		X = 0; Y = 1; Z = 2; W = 3

		### Class methods
		class << self

			### Return the identity quaternion (multiplicative)
			def identity
				new( 0,0,0,1 )
			end


			### Construct a new Quaternion by rotating ODE::Vector <tt>f</tt> to
			### ODE::Vector <tt>t</tt> (up to scale)
			def vv2q( f, t ) 
				raise TypeError, "no implicit conversion to ODE::Vector for %s" %
					f.class.name unless f.is_a?( ODE::Vector )
				raise IndexError, "Vector must be a 3rd-order vector" unless
					f.size == 3
				raise TypeError, "no implicit conversion to ODE::Vector for %s" %
					t.class.name unless t.is_a?( ODE::Vector )
				raise IndexError, "Vector must be a 3rd-order vector" unless
					t.size == 3

				f.normalize!
				t.normalize!

				ft = f.gp( t )
				sv2q( 1.0 + ft[3], ft[0..2] ) / (f + t).mag
			end
			alias_method :rotation, :vv2q


			### Construct a new Quaternion from a scalar <tt>scalar</tt> and an
			### ODE::Vector <tt>vector</tt>.
			def sv2q( scalar, vector )
				new( vector, scalar ) # array-concatenation, not vec addition
			end


			### Construct a new Quaternion from the given <tt>roll</tt>,
			### <tt>pitch</tt>, and <tt>yaw</tt>.
			def rpy2q( roll, pitch, yaw )
				new( roll, pitch, yaw )
			end

		end



		### Create and return a new ODE:Quaternion object from the arguments
		### given. The arguments can be in the following forms:
		###	  angle:: angle = Numeric
		###   axis, angle:: axis = 3-element Array/Vector, angle = Numeric
		###   two vectors:: 2 Array/Vector objects
		###   euler angles:: roll, pitch, and yaw as 3 floating-point numbers
		###   axis:: 3-element 
		def initialize( *args )
			case args.length
			when 0
				@elem = self.class.identity.elem

			when 1
				@elem = oneArgInit( *args )

			when 2
				@elem = twoArgInit( *args )

			when 3
				@elem = threeArgInit( *args )

			when 4
				@elem = args.collect {|elem| Float(elem)}

			else
				raise ArgumentError, "wrong number of arguments (%d for 1, 2, or 4)" %
					args.length
			end
		end
		

		#########
		protected
		#########

		### Initialize the quaternion from one argument (angle or vector)
		def oneArgInit( arg )
			case arg

			# Angle
			when Numeric
				@elem = [0.0, 0.0, 0.0, Float(arg)]

			# 3rd- or 4th-order vector
			when ODE::Vector
				case arg.size
				when 3
					@elem = [ arg.elements.collect {|i| Float(i)}, 0.0 ].flatten

				when 4
					@elem = arg.elements.collect {|i| Float(i)}

				else
					raise ArgumentError,
						"Cannot create a %s from a %d-dimensional %s" %
						[ self.class.name, arg.size, arg.class.name ]
				end

			else
				raise TypeError, "wrong type of argument '%s': Expected a %s" %
					[ arg.class.name, "Numeric or ODE::Vector" ]
			end
		end


		### Initialize the quaternion from two arguments (axis + angle or two
		### vectors).
		def twoArgInit( arg1, arg2 )

			# Axis + angle
			if arg2.kind_of?( Numeric ) && arg1.respond_to?( :to_ary )
				args = [ arg1.to_ary, arg2 ].flatten

			# Two vectors
			else
				args.each {|v|
					raise "No implicit conversion to ODE::Vector for #{v.class.name}" unless
						v.respond_to?( :to_ary )
				}

				v1 = ODE::Vector::new( arg1 ).normalize
				v2 = ODE::Vector::new( arg2 ).normalize

				args = [ v1.cross(v2).to_ary, v1.dot(v2) ].flatten
			end

		end


		### Initialize the quaternion from the given <tt>roll</tt>,
		### <tt>pitch</tt>, and <tt>yaw</tt>.
		def threeArgInit( roll, pitch, yaw )
			roll  = Float(roll) / 2
			pitch = Float(pitch) / 2
			yaw   = Float(yaw) / 2

			sr = sin( roll );  cr = cos( roll );
			sp = sin( pitch ); cp = cos( pitch );
			sy = sin( yaw );   cy = cos( yaw );

			return [
				sr * cp * cy - cr * sp * sy,
				cr * sp * cy + sr * cp * sy,
				cr * cp * sy - sr * sp * cy,
				cr * cp * cy + sr * sp * sy
			]
		end



		######
		public
		######

		# Internal array of elements
		attr_accessor :elem


		
		### Element reference operator -- returns the <tt>i</tt>th element of
		### the quaternion.
        def [](i)
            @elem[i]
        end


		### Element assignment operator -- assigns the value <tt>x</tt> to the
		### <tt>i</tt>th element of the vector.
        def []=(i, x)
            @elem[i] = Float(x)
        end


		### Return the 'x' (1st) element of the quaternion.
		def x; self[X]; end

		### Set the 'x' (1st) element of the quaternion to <tt>value</tt>.
		def x=(value); self[X] = Float(value); end

		### Return the 'y' (2nd) element of the quaternion.
		def y; self[Y]; end

		### Set the 'y' (2nd) element of the quaternion to <tt>value</tt>.
		def y=(value); self[Y] = Float(value); end

		### Return the 'z' (3rd) element of the quaternion.
		def z; self[Z]; end

		### Set the 'z' (3rd) element of the quaternion to <tt>value</tt>.
		def z=(value); self[Z] = Float(value); end

		### Return the 'w' (4th) element of the quaternion.
		def w; self[W]; end

		### Set the 'w' (4th) element of the quaternion to <tt>value</tt>.
		def w=(value); self[W] = Float(value); end


		### Return the quaternion's value as the "roll" euler angle (in
		### radians).
		def roll 
			# tan(roll)  =  2(wx + yz) / (w^2 - x^2 - y^2 + z^2)
			x = @elem[X]; y = @elem[Y]; z = @elem[Z]; w = @elem[W]
			return Math::atan( (2 * (w * x + y * z)) / (w**2 - x**2 - y**2 + z**2) )
		end

		### Return the quaternion's value as the "pitch" euler angle (in
		### radians).
		def pitch 
			# sin(pitch) = -2(xz - wy)
			x = @elem[X]; y = @elem[Y]; z = @elem[Z]; w = @elem[W]
			return Math::asin( -2 * (x*z - w*y) )
		end

		### Return the quaternion's value as the "yaw" euler angle (in radians).
		def yaw 
			# tan(yaw) = 2(xy+wz) / (w^2 + x^2 - y^2 - z^2)
			x = @elem[X]; y = @elem[Y]; z = @elem[Z]; w = @elem[W]
			return Math::atan( (2 * (x*y + w*z)) / (w**2 + x**2 - y**2 - z**2) )
		end


		### Return the scalar part of the quaternion as a Float.
		def scalar 
			@elem[3]
		end
		alias_method :s, :scalar


		### Return the dual of the bivector part
		def vec 
			ODE::Vector::new( @elem[0..2] )
		end
		alias_method :v, :vec


		### Return a new quaternion normalized to unit length
		def unit 
			unitq = self.copy
			magnitude = self.abs
			unitq[0] /= magnitude
			unitq[1] /= magnitude
			unitq[2] /= magnitude
			unitq[3] /= magnitude
			return unitq
		end


		### Return the quaternion as a 3x3 ODE::Matrix.
		def to_matrix 
			unitq = self.unit
			
			# For the unit quaternion:
			# Matrix = [ 1 - 2y^2 - 2z^2   2xy - 2wz		2xz + 2wy
			#			 2xy + 2wz		   1 - 2x^2 - 2z^2	2yz - 2wx
			#			 2xz - 2wy		   2yz + 2wx		1 - 2x^2 - 2y^2 ]
			return Matrix[
				[ 1 - 2 * (unitq.y ** 2 - unitq.z ** 2),
				  2 * (unitq.x * unitq.y + unitq.z * unitq.w),
				  2 * (unitq.x * unitq.z - unitq.y * unitq.w),
				  0
				],

				[ 2 * (unitq.x * unitq.y - unitq.z * unitq.w),
				  1 - 2 * (unitq.x ** 2 - unitq.z ** 2),
				  2 * (unitq.y * unitq.z + unitq.x * unitq.w),
				  0
				],

				[ 2 * (unitq.x * unitq.z + unitq.y * unitq.w),
				  2 * (unitq.y * unitq.z - unitq.x * unitq.w),
				  1 - 2 * (unitq.x ** 2 - unitq.y ** 2),
				  0
				],

				[ 0.0, 0.0, 0.0, 1.0 ],
			]
		end


		### Return the receiver as a 4th-order ODE::Vector.
		def to_vector
			return ODE::Vector::new( *@elem )
		end


		### Return the elements of the quaternion as an Array.
		def to_ary 
			return @elem.to_ary
		end


		### Return a distinct copy of the receiver (as opposed to #dup, which
		### only returns a shallow copy).
		def copy
			return self.dup.copy_object( self )
		end


		### Returns the natural exponent of the vector. The result will be a
		### unit quaternion.
		def exp
			mag = self.vec.mag
			Math::exp( self.scalar ) * Quaternion::sv2q( cos(mag), sin(mag) * self.vec/mag )
		end


		### Returns a new ODE::Vector created by transforming the given
		### <tt>vector</tt> by the rotation represented by the unit
		### quaternion. Results are undefined if the quaternion is not a unit
		### quaternion.
		def rotate( vector )
			(self.inverse * self.class.new(vector) * self ).vec 
		end


		### Return the magnitude of the quaternion
		def mag 
			Math::sqrt(self.sqr)
		end
		alias_method :abs, :mag


		### Return the squared magnitude of the quaternion.
		def sqr
			@elem[0] ** 2 + @elem[1] ** 2 + @elem[2] ** 2 + @elem[3] ** 2 
		end
		alias_method :abs2, :sqr


		### Returns the conjugate of the quaternion as a new instance of the
		### receiver.
		def conjugate
			self.class.new( -@elem[0], -@elem[1], -@elem[2], @elem[3] )
		end
		alias_method :conj, :conjugate


		### Normalize the quaternion in place.
		def normalize!
			magnitude = self.mag
			@elem.collect! {|elem| elem / magnitude}
			return self
		end


		### Return a normalized copy of the receiver.
		def normalize
			return self.copy.normalize!
		end
		

		### Return the inverse of the quaternion as new instance of the
		### receiver.
		def inverse
			return self.copy.inverse!
		end


		### Transform the receiving quaternion into its inverse.
		def inverse!
			smag = self.sqr
			@elem = [
				-@elem[0] / smag,
				-@elem[1] / smag,
				-@elem[2] / smag,
				 @elem[3] / smag
			]
		end


		### Return the axis of the quaternion as an ODE::Vector.
		def axis 
			vlen = self.vec.mag
			if vlen > 0.0 
				return self.vec/vlen
			else
				return ODE::Vector::new( 0.0, 0.0, 0.0 )
			end
		end


		### Return the angle of the quaternion in Radians as a Float.
		def angle_rads 
			2.0 * Math.atan2( self.vec.mag, self.scalar ) 
		end


		### Transform the quaternion by rotating it about the given
		### <tt>axis</tt> by <tt>angle</tt> radians.
		def axis_rotation( axis, angle ) 
			raise TypeError, "no implicit conversion of %s to ODE::Vector" unless
				axis.is_a?( ODE::Vector )
			angle = Float(angle)

			if len = axis.abs
				a = axis/len * sin( angle / 2.0 );
				self.class.new( a, cos(angle/2.0) )
			else
				self.class.identity
			end
		end


		### Multiplication operator -- return the receiving quaternion
		### multiplied by the <tt>other</tt> object, which can be a Numeric or
		### another ODE::Quaternion.
		def *( other )
			case other
			when Float, Fixnum
				other = other.to_f
				return self.class.new( @elem[0] * other,
									   @elem[1] * other,
									   @elem[2] * other,
									   @elem[3] * other )

			when ODE::Quaternion
				s = self.scalar * other.scalar - self.vec.dot( other.vec )
				v = self.scalar * other.vec - 
					self.vec.cross( other.vec ) + self.vec * other.scalar

				return self.class.new( v, s )
			else
				raise TypeError,
					"no implicit conversion of %s to %s" %
					[ other.class.name, 'Float, Fixnum, or ODE::Quaternion' ]
			end
		end


		### Division operator -- return the receiving quaternion divided by the
		### given value (a Float or a Fixnum).
		def /( value )
			case value
			when Float, Fixnum
				value = value.to_f
				return self.class.new( @elem[0] / value,
									   @elem[1] / value,
									   @elem[2] / value,
									   @elem[3] / value )
			else
				raise TypeError, "No implicit conversion from %s to %s" %
					[ value.class.name, "Float or Fixnum" ]
			end
		end


		### Addition operator -- add the specified <tt>otherQuat</tt>
		### (ODE::Quaternion object) to the receiver and return the results as a
		### new instance of the receiver.
		def +( otherQuat )
			self.class.new( @elem[0] + otherQuat[0],
						    @elem[1] + otherQuat[1],
						    @elem[2] + otherQuat[2],
						    @elem[3] + otherQuat[3] )
		end


		### Subtraction operator -- subtract the specified <tt>otherQuat</tt>
		### (ODE::Quaternion object) from the receiver and return the results as
		### a new instance of the receiver.
		def -( otherQuat )
			self.class.new( @elem[0] - otherQuat[0],
						    @elem[1] - otherQuat[1],
						    @elem[2] - otherQuat[2],
						    @elem[3] - otherQuat[3] )
		end
	end # class Quaternion


	### Rotation class
	class Rotation < Quaternion ; end


end # module ODE
