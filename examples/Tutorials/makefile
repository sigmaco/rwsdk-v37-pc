
all:

# Include target specific make options
# all directories are defined relative to the rwsdk BASEDIR

ifndef BASEDIR
BASEDIR = $(CURDIR)/../../rwsdk
endif

include $(BASEDIR)/makeincl/makeopt
include $(BASEDIR)/makeincl/rwtarget/$(RWTARGET)/maketarg

RWHLPDIR	= ../../rwsdk/autodocs/hlp/tutorials


################################################################################

TUTORIALS	=		\
	tutorial1		\
	tutorial2		\
	tutorial3		\
	tutorial4		\
	tutorial5		\
	tutorial6		\
	tutorial7

DOXYSRC		=		\
	$(CURDIR)/tutorial1/doc	\
	$(CURDIR)/tutorial2/doc	\
	$(CURDIR)/tutorial3/doc	\
	$(CURDIR)/tutorial4/doc	\
	$(CURDIR)/tutorial5/doc	\
	$(CURDIR)/tutorial6/doc

DOXYEXAMPLE	=			\
	$(CURDIR)/tutorial1/src		\
	$(CURDIR)/tutorial1/src/win	\
	$(CURDIR)/tutorial2/src		\
	$(CURDIR)/tutorial2/src/win	\
	$(CURDIR)/tutorial3/src		\
	$(CURDIR)/tutorial3/src/win	\
	$(CURDIR)/tutorial4/src		\
	$(CURDIR)/tutorial4/src/win	\
	$(CURDIR)/tutorial5/src		\
	$(CURDIR)/tutorial5/src/win	\
	$(CURDIR)/tutorial6/src		\
	$(CURDIR)/tutorial6/src/win	\
	$(CURDIR)/tutorial7/src		\
	$(CURDIR)/tutorial7/src/win

IMAGEPATH	=		\
	$(BASEDIR)/doxy/images      \

SRCIMAGE	=		\
	$(CURDIR)/tutorial1/doc/*.png	\
	$(CURDIR)/tutorial2/doc/*.png	\
	$(CURDIR)/tutorial3/doc/*.png	\
	$(CURDIR)/tutorial4/doc/*.png	\
	$(CURDIR)/tutorial5/doc/*.png	\

################################################################################

# Generate the documentation
$(RWHLPDIR):
	$(MD) $@
	
doxygen:
	echo $(DOXYSRC) |	\
	$(BINDIR)/doxycfg	\
	  -b$(BASEDIR)		\
	  -d$(RWHLPDIR)/	\
	  -i"$(IMAGEPATH)"	\
	  -m$(BASEDIR)/doxy/format/html		\
	  -nTutorials		\
	  -o$(RWOS)		\
	  -s"$(DOXYEXAMPLE)"	\
	  -t$(RWTARGET)		\
	> $(RWHLPDIR)/Doxyfile

tutorialdoc:
	doxygen $(RWHLPDIR)/Doxyfile
	-$(CP) $(SRCIMAGE) $(RWHLPDIR)/html
	$(BINDIR)/UNIX2DOS $(RWHLPDIR)/html/index.hhc
	@echo About to run -cmd /C $(BINDIR)/hhc $(RWHLPDIR)/html/index.hhp
	-cmd /C $(BINDIR)/hhc $(RWHLPDIR)/html/index.hhp

doxy: $(RWHLPDIR) doxygen tutorialdoc

doc:
	echo $(DOXYSRC) 1>> $(RWHLPDIR)/$(RWTARGET)/Docfile

clean distclean:
	for powermake in $(TUTORIALS);			\
	do						\
		$(MAKE) -C $(CURDIR)/$$powermake $@	\
		RWOPTIONS=$(RWOPTIONS) ;		\
	done