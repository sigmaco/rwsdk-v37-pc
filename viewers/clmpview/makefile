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

DEMO = clmpview

# include common makefile options

ifndef RWGDIR
RWGDIR = $(CURDIR)/../..
endif # RWGDIR

include $(RWGDIR)/shared/makeopt

CSRC =	$(SRC)/main.c				\
        $(SRC)/clmppick.c			\
        $(SRC)/clmpdmrf.c			\
        $(SRC)/clmpmorf.c			\
        $(SRC)/clmpatch.c			\
        $(SRC)/clmprndr.c			\
        $(SRC)/clmpskin.c			\
        $(SRC)/clmphanm.c			\
        $(SRC)/clmpview.c			\
        $(SRC)/clmpcntl.c			\
        $(SRC)/scene.c				\
	$(SRC)/$(RWOS)/events.c			\
	$(COMMON)/camera.c			\
	$(COMMON)/menu.c	

# Add skeleton files

CSRC += $(SKEL)/skeleton.c	\
	$(SKELPS)/$(RWOS).c     \
	$(SKELSRC)

ifeq ($(RWMETRICS), 1)
CSRC += $(SKEL)/vecfont.c   \
        $(SKEL)/metrics.c
endif

RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rtpick.$(L)		\
	$(RWLIBDIR)/$(LP)rpcollis.$(L)		\
	$(RWLIBDIR)/$(LP)rpdmorph.$(L)		\
	$(RWLIBDIR)/$(LP)rtanim.$(L)		\
	$(RWLIBDIR)/$(LP)rphanim.$(L)		\
	$(RWLIBDIR)/$(LP)rplodatm.$(L)		\
	$(RWLIBDIR)/$(LP)rplogo.$(L)		\
	$(RWLIBDIR)/$(LP)rpmatfx.$(L)		\
	$(RWLIBDIR)/$(LP)rppatchskinmatfx.$(L)

ifeq ("$(RWTARGET)", "xbox")
RWLIBS +=					\
		$(RWLIBDIR)/$(LP)rpanisot.$(L)		\
		$(RWLIBDIR)/$(LP)rpnormmapskin.$(L)
endif

RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rpmorph.$(L)		\
	$(RWLIBDIR)/$(LP)rprandom.$(L)		\
	$(RWLIBDIR)/$(LP)rpskinmatfx.$(L)	\
	$(RWLIBDIR)/$(LP)rpworld.$(L)		\
	$(RWLIBDIR)/$(LP)rtbmp.$(L)		\
	$(RWLIBDIR)/$(LP)rtcharse.$(L)		\
	$(RWLIBDIR)/$(LP)rtfsyst.$(L)		\
	$(RWLIBDIR)/$(LP)rtintsec.$(L)		\
	$(RWLIBDIR)/$(LP)rtpng.$(L)		\
	$(RWLIBDIR)/$(LP)rtbezpat.$(L)

ifeq ($(RWSUPPRESSINLINE), 1)
RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rtquat.$(L)
else
ifeq ($(RWDEBUG), 1)
RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rtquat.$(L)
endif
endif

RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rtras.$(L)		\
	$(RWLIBDIR)/$(LP)rwcore.$(L)

ifeq ("$(RWTARGET)", "sky2")
RWLIBS += $(RWLIBDIR)/$(LP)rppds.$(L)
endif # ifeq ("$(RWTARGET)", "sky2")

include $(RWGDIR)/shared/maketarg


