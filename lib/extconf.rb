#!/usr/bin/ruby -w

require 'find'
require 'rbconfig'

include Config

libs = []
Find::find( "." ) {|f|
	next if f =~ /extconf/
	libs << f if f =~ /\.rb$/
}

vars = {
	:RUBY		=> "#{CONFIG['bindir']}/#{CONFIG['ruby_install_name']}",
	:PREFIX		=> "#{CONFIG['prefix']}",
	:SITELIBDIR	=> "#{CONFIG['sitelibdir']}",
	:ODELIBDIR	=> "$(SITELIBDIR)/ode",
	:INSTALL	=> "$(RUBY) -r ftools -e 'File::install(ARGV[0], ARGV[1], 0644, true)'",
	:MAKEDIRS	=> "$(RUBY) -r ftools -e 'File::makedirs(*ARGV)'",
}
	

File::open( "Makefile", "w" ) {|makefile|
	vars.each do |pair|
		makefile.puts "%-35s = %s" % pair
	end

	makefile.print "\nlibs = \\\n\t"
	makefile.puts libs.join( "\\\n\t" ) + "\n"

	makefile.puts <<-"EOF".gsub(/^\t\t/, '')
		all: $(libs)

		install: $(libs) $(ODELIBDIR)
			@for file in $(libs); do \\
				$(INSTALL) $$file $(ODELIBDIR); \\
			done

		site-install: install

		$(ODELIBDIR):
			$(MAKEDIRS) $@

		clean:

		distclean:
			@rm -f Makefile

	EOF
}
