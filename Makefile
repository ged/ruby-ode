SUBDIRS = src

all:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} all; \
	done;

clean:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} clean; \
	done;
	@-rm -f mkmf.log src/mkmf.log 2> /dev/null
	@-rm -f src/depend 2> /dev/null

distclean:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} distclean; \
	done;
	@-rm -f mkmf.log src/mkmf.log 2> /dev/null
	@-rm -f src/depend 2> /dev/null
	@rm Makefile

realclean:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} realclean; \
	done;
	@-rm -f mkmf.log src/mkmf.log 2> /dev/null
	@-rm -f src/depend 2> /dev/null
	@rm Makefile

install:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} install; \
	done;

depend:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} depend; \
	done;

site-install:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} site-install; \
	done;

unknown:
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $${subdir} unknown; \
	done;

%.html: %.rd
	rd2 $< > ${<:%.rd=%.html}

HTML = ode.html

html: $(HTML)

test: $(DLLIB)
	ruby test.rb

