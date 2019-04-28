###########################################################################
#
# File     : Makefile
#
# Abstract : Command line makefile used to build a RenderWare tool program.
#            This is a GNU Make makefile. A copy of GNU Make is required in
#	     order to use this file.
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
# Copyright (c) 2001 Criterion Software Ltd.
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

DEMO = rwstouch

CFLAGS += -DRENDERWARE -DRENDERWARE_ALLPLUGINS

CXXFLAGS += $(CFLAGS)
CXXFLAGS += -GX

CSRC =	$(SRC)/main.cpp

# Add RWS common files

CSRC += $(RWTOOLSHAREDSRC)/shared.c

RWLIBS += \
	$(RWLIBDIR)/$(LP)rwcore.$(L)

RWLIBS +=    $(RWLIBDIR)/$(LP)rt2d.$(L) \
    $(RWLIBDIR)/$(LP)rt2danim.$(L) \
    $(RWLIBDIR)/$(LP)rpspline.$(L) \
    $(RWLIBDIR)/$(LP)rpanisot.$(L) \
    $(RWLIBDIR)/$(LP)rwcore.$(L) \
    $(RWLIBDIR)/$(LP)rtfsyst.$(L) \
    $(RWLIBDIR)/$(LP)rptoon.$(L) \
    $(RWLIBDIR)/$(LP)rtintsec.$(L) \
    $(RWLIBDIR)/$(LP)rpcollis.$(L) \
    $(RWLIBDIR)/$(LP)rpptank.$(L) \
    $(RWLIBDIR)/$(LP)rpmatfx.$(L) \
    $(RWLIBDIR)/$(LP)rppvs.$(L) \
    $(RWLIBDIR)/$(LP)rtanim.$(L) \
    $(RWLIBDIR)/$(LP)rpusrdat.$(L) \
    $(RWLIBDIR)/$(LP)rpskinmatfx.$(L) \
    $(RWLIBDIR)/$(LP)rphanim.$(L) \
    $(RWLIBDIR)/$(LP)rpmorph.$(L) \
    $(RWLIBDIR)/$(LP)rpdmorph.$(L) \
    $(RWLIBDIR)/$(LP)rppatch.$(L) \
    $(RWLIBDIR)/$(LP)rpltmap.$(L) \
    $(RWLIBDIR)/$(LP)rpprtstd.$(L) \
    $(RWLIBDIR)/$(LP)rpprtadv.$(L) \
    $(RWLIBDIR)/$(LP)rpworld.$(L) \
    $(RWLIBDIR)/$(LP)rtpitexd.$(L) \
    $(RWLIBDIR)/$(LP)rpuvanim.$(L) \
    $(RWLIBDIR)/$(LP)rtdict.$(L) \
    $(RWLIBDIR)/$(LP)rpadc.$(L) \
    $(RWLIBDIR)/$(LP)rtcmpkey.$(L) \
    $(RWLIBDIR)/$(LP)rpuvanim.$(L) \
    $(RWLIBDIR)/$(LP)rpnormmapskin.$(L)

ifeq ($(RWSUPPRESSINLINE), 1)
RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rtquat.$(L)
else
ifeq ($(RWDEBUG), 1)
RWLIBS +=					\
	$(RWLIBDIR)/$(LP)rtquat.$(L)
endif
endif

ifneq ("$(RWTARGET)", "nullsky")
RWLIBS += $(RWLIBDIR)/$(LP)rpmipkl.$(L)
endif

SYSLIBS =

LFLAGS =                                        \
        -incremental:no                         \
        -libpath:$(RWLIBDIR)                    \
        -machine:I386                           \
        -nologo                                 \
        -subsystem:console                      \
        -warn:3                                 \
        -NODEFAULTLIB:OLDNAMES.$(L)

SYSLIBS += user32.$(L)

include $(RWTOOLSHAREDDIR)/maketarg

#installation actions
install: $(RWBINDIR) demo
	$(CP) $(PROJS) $(RWBINDIR)/$(DEMO)$(EXE)
