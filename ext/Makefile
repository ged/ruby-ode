SHELL = /bin/sh

#### Start of system configuration section. ####

srcdir = .
topdir = $(rubylibdir)/$(arch)
hdrdir = $(rubylibdir)/$(arch)
VPATH = $(srcdir)

CC = gcc

CFLAGS   = -fPIC -g -O2 -fPIC  -DdDOUBLE -DBUILD_MODE=debug -DPLATFORM=linux
CPPFLAGS = -I$(hdrdir) -I$(srcdir) -I$(prefix)/include -DHAVE_ODE_ODE_H  
CXXFLAGS = $(CFLAGS)
DLDFLAGS =  -L$(exec_prefix)/lib 
LDSHARED = gcc -shared 
LIBPATH = 

RUBY_INSTALL_NAME = ruby
RUBY_SO_NAME = 
arch = i686-linux
ruby_version = 1.7

prefix = $(DESTDIR)/usr
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib
rubylibdir = $(libdir)/ruby/$(ruby_version)
archdir = $(rubylibdir)/$(arch)
sitedir = $(prefix)/lib/ruby/site_ruby
sitelibdir = $(sitedir)/$(ruby_version)
sitearchdir = $(sitelibdir)/$(arch)
target_prefix = 

#### End of system configuration section. ####

LOCAL_LIBS =  
LIBS = -L. -l$(RUBY_INSTALL_NAME) -lode -lc
OBJS = ode.o rotation.o jointGroup.o position.o body.o mass.o joints.o world.o

TARGET = ode
DLLIB = $(TARGET).so

RUBY = ruby
RM = $(RUBY) -rftools -e "File::rm_f(*ARGV.map{|x|Dir[x]}.flatten.uniq)"

EXEEXT = 

all:		$(DLLIB)

clean:;		@$(RM) *.o *.so *.sl *.a $(DLLIB)
		@$(RM) $(TARGET).lib $(TARGET).exp $(TARGET).ilk *.pdb $(CLEANFILES)

distclean:	clean
		@$(RM) Makefile extconf.h conftest.* mkmf.log
		@$(RM) core ruby$(EXEEXT) *~ $(DISTCLEANFILES)

realclean:	distclean

install:	$(archdir)$(target_prefix)/$(DLLIB)

site-install:	$(sitearchdir)$(target_prefix)/$(DLLIB)

$(archdir)$(target_prefix)/$(DLLIB): $(DLLIB)
	@$(RUBY) -r ftools -e 'File::makedirs(*ARGV)' $(rubylibdir) $(archdir)$(target_prefix)
	@$(RUBY) -r ftools -e 'File::install(ARGV[0], ARGV[1], 0555, true)' $(DLLIB) $(archdir)$(target_prefix)/$(DLLIB)

$(sitearchdir)$(target_prefix)/$(DLLIB): $(DLLIB)
	@$(RUBY) -r ftools -e 'File::makedirs(*ARGV)' $(libdir) $(sitearchdir)$(target_prefix)
	@$(RUBY) -r ftools -e 'File::install(ARGV[0], ARGV[1], 0555, true)' $(DLLIB) $(sitearchdir)$(target_prefix)/$(DLLIB)


.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<
.cpp.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<
.cxx.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<
.C.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<
.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<
$(DLLIB): $(OBJS)
	$(LDSHARED) $(DLDFLAGS) -o $(DLLIB) $(OBJS) $(LIBS) $(LOCAL_LIBS)

depend:
	$(CC) $(CFLAGS) $(CPPFLAGS) -MM *.c > depend

