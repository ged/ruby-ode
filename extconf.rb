#!/usr/bin/ruby
#
#	extconf.rb - Extension config script for the Ruby ODE binding
#
#	Author: Michael Granger (with lots of code borrowed from the bdb Ruby
#				extension's extconf.rb)
#
#	Copyright (c) 2001-2003, The FaerieMUD Consortium.
#
#   This work is licensed under the Creative Commons Attribution License. To
#   view a copy of this license, visit
#   http://creativecommons.org/licenses/by/1.0 or send a letter to Creative
#   Commons, 559 Nathan Abbott Way, Stanford, California 94305, USA.

require 'mkmf'

if ( (RUBY_VERSION.split(%r{\.}).collect {|n| n.to_i} <=> [1,7,3]) < 0 )
	$stderr.puts "This extension requires Ruby 1.7.3 or later. You can try to\n" +
		"  compile it anyway, but it isn't likely to work."
end

def rule(target, clean = nil)
   wr = "#{target}:
\t@for subdir in $(SUBDIRS); do \\
\t\t$(MAKE) -C $${subdir} #{target}; \\
\tdone;
"
   if clean != nil
     # wr << "\t@-rm tmp/* tests/tmp/* 2> /dev/null\n"
	  wr << "\t@-rm -f mkmf.log ext/mkmf.log 2> /dev/null\n"
	  wr << "\t@-rm -f ext/depend 2> /dev/null\n"
      wr << "\t@rm Makefile\n" if clean
   end
   wr
end

subdirs = Dir["*"].select do |subdir|
	next if subdir =~ /misc/
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

.PHONY: docs localdocs test debugtest

docs:
	rdoc --all --inline-source --main README --fmt html --op docs/html \
	--title 'Ruby ODE Binding' \
	`ruby -I. -r ./utils -e 'puts UtilityFunctions::findRdocableFiles("docs/CATALOG").join(" ")' 2>/dev/null`

localdocs: docs/makedocs.rb
	docs/makedocs.rb -v

docs/makedocs.rb:
	$(error Local docs require the RDoc extensions in the CVS version of this module)


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
