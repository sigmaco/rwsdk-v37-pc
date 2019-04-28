###########################################################################
#
# File     : Makefile
#
# Abstract : Command line makefile used to build RenderWare demo programs.
#            This is a GNU Make makefile. A copy of GNU Make is required in 
#            order to use this file.
#
###########################################################################
#
# This file is a product of Criterion Software Ltd.
#
# This file is provided as is with no warranties of any kind and is
# provided without any obligation on Criterion Software Ltd. or Canon Inc. 
# to assist in its use or modification.
#
# Criterion Software Ltd. will not, under any circumstances, be liable for 
# any lost revenue or other damages arising from the use of this file.
#
# Copyright (c) 1999 Criterion Software Ltd.
# All Rights Reserved.
#
# RenderWare is a trademark of Canon Inc.
#
###########################################################################

#
# include user custom options file if it exists
#
ifndef RWOPTIONS
RWOPTIONS = $(CURDIR)/options.mak
endif
include $(RWOPTIONS)

#
# Build without logo by default 
#
ifndef RWLOGO
RWLOGO=0
endif

# all directories are defined relative to DEMODIR
ifndef DEMODIR
DEMODIR = $(CURDIR)
# setting BASDEIR=$(CURDIR) enables absolute path names
# which makes a better job of debug symbols for profiling. debugging etc.
# see also http://www.fsf.org/software/make/make.html
endif

ifndef DEMOS
EXCLUDED =						\
	$(DEMODIR)/Tutorials				\
	$(DEMODIR)/makefile				\
	$(wildcard $(DEMODIR)/*.*)

DEMOS = $(filter-out $(EXCLUDED), $(wildcard $(DEMODIR)/*))
endif

all: all-recursive

message:
	@echo DEMOS are $(DEMOS)

clean: clean-recursive
distclean: distclean-recursive clean
doscheck: doscheck-recursive

all-recursive clean-recursive doc-recursive header-recursive distclean-recursive doscheck-recursive:
	@cd $(DEMODIR); \
	for demo in $(DEMOS); do \
		echo ; \
		echo Attempting to process demo [$$demo] ; \
		echo ------------------------------------ ; \
		$(MAKE) -C $$demo $(subst -recursive,,$@) \
		RWOPTIONS=$(RWOPTIONS) RWLOGO=$(RWLOGO); \
	done

msg_9600:
	@echo '---------------------------------'
	@echo 'Simple checks for ISO-9600 CD format conformance'
	@echo 'See http://www.ecma.ch/ecma1/stand/ecma%2D119.htm'
	@echo '  10 Levels of interchange'
	@echo '  This Standard specifies three nested levels of interchange.'
	@echo '  10.1 Level 1'
	@echo '  At Level 1 the following restrictions shall apply:'
	@echo '  - each file shall consist of only one File Section;'
	@echo '  - a File Name shall not contain more than '
	@echo '    eight d-characters or eight d1-characters;'
	@echo '  - a File Name Extension shall not contain more than '
	@echo '    three d-characters or three d1-characters;'
	@echo '  - a Directory Identifier shall not contain more than '
	@echo '    eight d-characters or eight d1-characters.'

long_dir:
	@echo '---------------------------------'
	@echo Searching for directory identifiers exceeding 8 characters
	cygfind . -type d -name "?????????*" -print

long_suf:
	@echo '---------------------------------'
	@echo Searching for file name extensions exceeding 3 characters
	cygfind . -type f -name "?*\.????*" -print

long_pre:
	@echo '---------------------------------'
	@echo Searching for file names exceeding 8 characters
	cygfind . -type f -name "?????????*\.?*" -print

iso_9600: msg_9600 long_dir long_suf long_pre 
	@echo '---------------------------------'
