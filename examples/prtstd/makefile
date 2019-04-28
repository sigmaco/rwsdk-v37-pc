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
# Copyright (c) 2000, 2001 Criterion Software Ltd.
# All Rights Reserved.
#
# RenderWare is a trademark of Canon Inc.
#
###########################################################################

# Define the projects to be built

DEMO = prtstd

# include common makefile options

ifndef RWGDIR
RWGDIR = $(CURDIR)/../..
endif # RWGDIR

include $(RWGDIR)/shared/makeopt

CSRC =	$(SRC)/main.c		\
	$(SRC)/prtstd.c		\
	$(SRC)/$(RWOS)/events.c	\
	$(COMMON)/camera.c	\
	$(COMMON)/menu.c	\

# Add skeleton files

CSRC += $(SKEL)/skeleton.c	\
	$(SKELPS)/$(RWOS).c     \
	$(SKELSRC)

ifeq ($(RWMETRICS), 1)
CSRC += $(SKEL)/vecfont.c   \
        $(SKEL)/metrics.c
endif

ifeq ("$(RWTARGET)", "sky2")
RWLIBS += $(RWLIBDIR)/$(LP)rppds.$(L)
endif # ifeq ("$(RWTARGET)", "sky2")

RWLIBS += \
	$(RWLIBDIR)/$(LP)rtcharse.$(L) \
	$(RWLIBDIR)/$(LP)rtfsyst.$(L)	\
	$(RWLIBDIR)/$(LP)rtbmp.$(L)	\
	$(RWLIBDIR)/$(LP)rtpng.$(L)	\
	$(RWLIBDIR)/$(LP)rpprtstd.$(L)	\
	$(RWLIBDIR)/$(LP)rpptank.$(L)	\
	$(RWLIBDIR)/$(LP)rpworld.$(L)	\
	$(RWLIBDIR)/$(LP)rwcore.$(L)

include $(RWGDIR)/shared/maketarg


