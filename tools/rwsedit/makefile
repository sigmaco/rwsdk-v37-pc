###########################################################################
#
# File     : Makefile
#
# Abstract : Command line makefile used to build a RenderWare demo program.
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
# Copyright (c) 2000 Criterion Software Ltd.
# All Rights Reserved.
#
# RenderWare is a trademark of Canon Inc.
#
###########################################################################

ifndef RWTOOLSHAREDDIR
RWTOOLSHAREDDIR = $(CURDIR)/../shared
endif

include $(RWTOOLSHAREDDIR)/makeopt

# Define the projects to be built

DEMO = rwsedit

CSRC =	$(SRC)/main.c

CFLAGS += -DRENDERWARE

# Add RWS common files

CSRC += $(RWTOOLSHAREDSRC)/shared.c

RWLIBS += \
	$(RWLIBDIR)/$(LP)rwcore.$(L) \
	$(RWLIBDIR)/$(LP)rtfsyst.$(L) \
	$(RWLIBDIR)/$(LP)rttoc.$(L)

include $(RWTOOLSHAREDDIR)/maketarg

#installation actions
install: $(RWBINDIR) demo
	$(CP) $(PROJS) $(RWBINDIR)/$(DEMO)$(EXE)
