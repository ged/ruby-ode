#!/usr/bin/ruby
#
#	extconf.rb - Extension config script for the Ruby ODE binding
#
#	See the INSTALL file for instructions on how to use this script.
#
#	Author: Michael Granger (with lots of code borrowed from the bdb Ruby
#				extension's extconf.rb)
#
#	Copyright (c) 2001, The FaerieMUD Consortium. All rights reserved.
#
#	This program is free software; you can redistribute it and/or modify it
#	under the terms of the GNU Lesser General Public License as published by
#	the Free Software Foundation; either version 2.1 of the License, or (at
#	your option) any later version.
#
#	This library is distributed in the hope that it will be useful, but
#	WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
#	General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public License
#	along with this library (see the file LICENSE.TXT); if not, write to the
#	Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#	02111-1307 USA.
#

require 'mkmf'

def rule(target, clean = nil)
   wr = "#{target}:
\t@for subdir in $(SUBDIRS); do \\
\t\t$(MAKE) -C $${subdir} #{target}; \\
\tdone;
"
   if clean != nil
     # wr << "\t@-rm tmp/* tests/tmp/* 2> /dev/null\n"
	  wr << "\t@-rm -f mkmf.log src/mkmf.log 2> /dev/null\n"
	  wr << "\t@-rm -f src/depend 2> /dev/null\n"
      wr << "\t@rm Makefile\n" if clean
   end
   wr
end

subdirs = Dir["*"].select do |subdir|
   File.file?(subdir + "/extconf.rb")
end

begin
   make = open("Makefile", "w")
   make.print <<-EOF
SUBDIRS = #{subdirs.join(' ')}

#{rule('all')}
#{rule('clean', false)}
#{rule('distclean', true)}
#{rule('realclean', true)}
#{rule('install')}
#{rule('depend')}
#{rule('site-install')}
#{rule('unknown')}
docs:
	ruby docs/makedocs.rb -v

test: all
	ruby test.rb

debugtest: clean all
	ruby -wd test.rb

	EOF
ensure
   make.close
end

subdirs.each do |subdir|
   STDERR.puts("#{$0}: Entering directory `#{subdir}'")
   Dir.chdir(subdir)
   system("#{Config::CONFIG['RUBY_INSTALL_NAME']} extconf.rb " + ARGV.join(" "))
   Dir.chdir("..")
   STDERR.puts("#{$0}: Leaving directory `#{subdir}'")
end
