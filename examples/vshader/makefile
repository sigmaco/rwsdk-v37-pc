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

DEMO = vshader

# include common makefile options

ifndef RWGDIR
RWGDIR = $(CURDIR)/../..
endif # RWGDIR

include $(RWGDIR)/shared/makeopt

unsupported:
	@echo =====================================
	@echo $(DEMO) is unsupported on $(RWTARGET)
	@echo =====================================

ifneq ("$(RWTARGET)", "xbox")
ifneq ("$(RWTARGET)", "d3d8")
ifneq ("$(RWTARGET)", "d3d9")

override TARGET := unsupported
override VALIDATE := unsupported
override OBJ := 

endif # ifneq ("$(RWTARGET)", "d3d9")
endif # ifneq ("$(RWTARGET)", "d3d8")
endif # ifneq ("$(RWTARGET)", "xbox")


# add new rules for vshader headers
ifeq ("$(RWTARGET)", "xbox")

src/rainbow.h : src/rainbow.vsh
	xsasm -h /I src src/rainbow.vsh

endif # ifeq ("$(RWTARGET)", "xbox")


ifeq ("$(RWTARGET)", "xbox")

$(SRC)/vshader.c: src/rainbow.h

CSRC =	$(SRC)/main.c		\
	$(SRC)/vshader.c	\
	$(SRC)/$(RWOS)/events.c	\
	$(COMMON)/camera.c	\
	$(COMMON)/menu.c

endif # ifeq ("$(RWTARGET)", "xbox")


ifeq ("$(RWTARGET)", "d3d8")

CSRC =	$(SRC)/main.c		\
	$(SRC)/vshaderD3D8.c	\
	$(SRC)/$(RWOS)/events.c	\
	$(COMMON)/camera.c	\
	$(COMMON)/menu.c

ifeq ($(RWDEBUG), 1)

LFLAGS +=				\
	$(DXSDK)/lib/d3dx8dt.lib	\
	gdi32.lib			\
	uuid.lib

else # ifeq ($(RWDEBUG), 1)

LFLAGS +=				\
	$(DXSDK)/lib/d3dx8.lib		\
	advapi32.lib

endif # ifeq ($(RWDEBUG), 1)

endif # ifeq ("$(RWTARGET)", "d3d8")


ifeq ("$(RWTARGET)", "d3d9")

CSRC =	$(SRC)/main.c		\
	$(SRC)/vshaderD3D9.c	\
	$(SRC)/$(RWOS)/events.c	\
	$(COMMON)/camera.c	\
	$(COMMON)/menu.c

endif # ifeq ("$(RWTARGET)", "d3d9")


# Add skeleton files

CSRC += $(SKEL)/skeleton.c	\
	$(SKELPS)/$(RWOS).c     \
	$(SKELSRC)

ifeq ($(RWMETRICS), 1)
CSRC += $(SKEL)/vecfont.c   \
        $(SKEL)/metrics.c 

endif

RWLIBS += $(RWLIBDIR)/$(LP)rtcharse.$(L) \
	$(RWLIBDIR)/$(LP)rtfsyst.$(L)	\
	$(RWLIBDIR)/$(LP)rtbmp.$(L)	\
	$(RWLIBDIR)/$(LP)rtpng.$(L)	\
	$(RWLIBDIR)/$(LP)rpworld.$(L)	\
	$(RWLIBDIR)/$(LP)rwcore.$(L)

include $(RWGDIR)/shared/maketarg

ifeq ("$(RWTARGET)", "d3d8")
LFLAGS += /NODEFAULTLIB:libci

ifeq ($(RWDEBUG),1)
LFLAGS += /NODEFAULTLIB:libcpd
else
LFLAGS += /NODEFAULTLIB:libcp
endif

endif # d3d8

