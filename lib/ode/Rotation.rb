#!/usr/bin/ruby
#
# This file contains the ODE::Rotation class. Instances of this class represent
# the orientation of a ODE::Body in an ODE::World simulation.
#
# This class is mostly just an elaboration of the Math3d::Rotation class.
#
# == Synopsis
#
#
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
#  $Id: Rotation.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $
#

require 'math3d'

module ODE

    ### Instances of this class represent the rotation of a ODE::Body in an
    ### ODE::World simulation. This class is mostly just an elaboration of the
    ### Math3d::Rotation class.
    class Rotation < Math3d::Rotation

        ### Class constants
        Version = /([\d\.]+)/.match( %q{$Revision: 1.1 $} )[1]
        Rcsid = %q$Id: Rotation.rb,v 1.1 2002/11/23 23:07:45 deveiant Exp $


		alias :to_matrix :to_mat
		alias :to_quaternion :to_vec4


    end # class Rotation
end # module ODE
