EXE=vcfUtil
TOOLBASE = VcfExecutable ReplaceReference HyunVcfFile VcfExample
SRCONLY = Main.cpp
HDRONLY = Logger.h

DATE=$(shell date)
USER=$(shell whoami)
USER_COMPILE_VARS = -DDATE="\"${DATE}\"" -DVERSION="\"${VERSION}\"" -DUSER="\"${USER}\"" -lcrypto

COMPILE_ANY_CHANGE = VcfExecutable

PARENT_MAKE = Makefile.src
include ../Makefile.inc
