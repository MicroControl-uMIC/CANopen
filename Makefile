#=============================================================================#
# Makefile for project: µMIC.200 CANopen example                              #
# Target              : µMIC.200 with Cortex-A8 CPU                           #
#-----------------------------------------------------------------------------#
#
#
#=============================================================================#

#-----------------------------------------------------------------------------#
# Target name                                                                 #
#                                                                             #
#-----------------------------------------------------------------------------#
TARGET     = umic_canopen
TARGET_HW  = mMIC.200


#-----------------------------------------------------------------------------#
# Debug code generation                                                       #
#                                                                             #
#-----------------------------------------------------------------------------#
DEBUG      = 1

#-----------------------------------------------------------------------------#
# object file selection                                                       #
# select objects that should be compiled & linked here                        #
#-----------------------------------------------------------------------------#
APP_OBJS   = 1

#-----------------------------------------------------------------------------#
# Path setup (source and object directory)                                    #
#                                                                             #
#-----------------------------------------------------------------------------#
PWD = $(shell pwd)

#---------------------------------------------------------------
# PRJ_DIR: absolute or relative path to project root directory
#
PRJ_DIR  = $(PWD)

#---------------------------------------------------------------
# APP_DIR: path to application directory
#
APP_DIR  = $(PRJ_DIR)

#---------------------------------------------------------------
# LIB_DIR: path to common library directory 
#
LIB_DIR = 

#----------------------------------------------------------
# Object directory
#
OBJ_DIR  = $(PRJ_DIR)


#-----------------------------------------------------------------------------#
# Compiler settings (GNU-ARM compiler)                                        #
#                                                                             #
#-----------------------------------------------------------------------------#
CROSS = /usr/bin/
AS =$(CROSS)as
AR =$(CROSS)ar
CC =$(CROSS)gcc
GG =$(CROSS)g++
OC =$(CROSS)objcopy
OD =$(CROSS)objdump
NM =$(CROSS)nm
SZ =$(CROSS)size
FE =$(CROSS)fromelf


#---------------------------------------------------------------
# Include directory for header files
#
INC_DIR   = -I $(APP_DIR)

#---------------------------------------------------------------
# Set VPATH to the same value like include paths
# but without '-I'
VPATH =$(INC_DIR:-I= )
ALL_CFILES = $(wildcard $(PRJ_DIR)/source/*.c $(PRJ_DIR)/source/*.h)

#---------------------------------------------------------------
# Warning level
#
WARN  = -Wall
WARN += -Wextra 
WARN += -std=c99


#---------------------------------------------------------------
# TARGET CPU & FPU
# CPU : CPU architecture for -mcpu, possible values
#       cortex-a8
# FPU : defines FPU support for -mfloat-abi, possible values
#       soft
#       softfp
#       hard
#
CPU = -mcpu=cortex-a8
FPU = -mfloat-abi=hard -mfpu=neon-fp-armv8


#---------------------------------------------------------------
# Check for debug option flag
# -g[level]      : Request debugging information and also use 
#                  level to specify how much information
# -fomit-frame-pointer : Don't keep the frame pointer in a 
#                        register for functions that don't need one
#                        Enabled at levels '-O', '-O2', '-O3', '-Os'
# we have to set -O optimization for debug, so we can use 
# OpenOCD debugger without problems
ifeq ($(DEBUG),1)
	GDB_FLAG  = -g3
	OPTIMIZE  = -O0
	DEBUG_MSG = DEBUG
else
	GDB_FLAG  =
	OPTIMIZE  = -O0
	DEBUG_MSG = RELEASE 
endif

#---------------------------------------------------------------
# Specific user/application symbol definition
#
MC_FLAG += -DDEBUG=$(DEBUG)
MC_FLAG += -D_GNU_SOURCE

#-----------------------------------------------------------------------------#
# GCC compiler and linker settings                                            #
#                                                                             #
#-----------------------------------------------------------------------------#

#--------------------------------------------------------------------
# Compiler FLAGS
#
CFLAGS  = $(CPU) $(FPU)
CFLAGS += $(OPTIMIZE) $(GDB_FLAG) $(WARN) $(INC_DIR) -c $(MC_FLAG)

#--------------------------------------------------------------------
# Linker FLAGS
#
LFLAGS  = $(INC_DIR)


LIBS    = -lQCANopenMaster -lrt -pthread


#-----------------------------------------------------------------------------#
# List of source files that need to be compiled                               #
#                                                                             #
#-----------------------------------------------------------------------------#


#--------------------------------------------------------------------
# Application object files 
#
#--------------------------------------------------------------------
APP_SRC =	canopen_event.c   \
			   canopen_main.c



#--------------------------------------------------------------------
# generate list of all required object files
#
#--------------------------------------------------------------------
ifeq ($(APP_OBJS),1) 
TARGET_OBJS   = $(patsubst %.c,$(OBJ_DIR)/%.o, $(APP_SRC))
TARGET_OBJS   += $(patsubst %.cpp,$(OBJ_DIR)/%.o, $(APP_SRC2))
endif 

#-----------------------------------------------------------------------------#
# Rules                                                                       #
#                                                                             #
#-----------------------------------------------------------------------------#
all: msg_compiling msg_compiler $(TARGET_OBJS)
	@echo 'Invoking linker: '$(CC)
	$(CC) $(LFLAGS) -o $(OBJ_DIR)/$(TARGET) $(TARGET_OBJS) $(LIBS)

clean:
	@mkdir -p $(OBJ_DIR)
	@rm -f $(OBJ_DIR)/*.a
	@rm -f $(OBJ_DIR)/*.o
	@rm -f $(OBJ_DIR)/*.d
	@rm -f $(OBJ_DIR)/$(TARGET)
	@rm -f $(OBJ_DIR)/*.elf
	
run: 
	@$(OBJ_DIR)/$(TARGET)

size:
	$(SZ) $(OBJ_DIR)/$(TARGET)

show:
	@echo Print some information about used Makefile
	@echo TARGET_OBJS:
	@echo $(TARGET_OBJS)
	@echo INC_DIR:
	@echo $(INC_DIR) 
	@echo VPATH:
	@echo $(VPATH)
	@echo ALL_CFILES:
	@echo $(ALL_CFILES)

msg_compiling: 
	@echo -----------------------------------------------------------------------------
	@echo 'Build target: '$(TARGET) 
	@echo -----------------------------------------------------------------------------
	@echo 
	
msg_compiler:
	@echo 'Invoking compiler: '$(CC)


#-----------------------------------------------------------------------------#
# Dependencies                                                                #
#                                                                             #
#-----------------------------------------------------------------------------#
#--- standard c files -------------------------------------
$(OBJ_DIR)/%.o : %.c
	@echo - Compiling : $(<F)
	@$(CC) $(CFLAGS) $< -o $@ -MMD

#--- standard cpp files -------------------------------------
$(OBJ_DIR)/%.o : %.cpp
	@echo - Compiling : $(<F)
	@$(GG) $(CFLAGS) $< -o $@ -MMD

#--- include source file dependencies ---------------------
-include $(patsubst %.o,%.d, $(TARGET_OBJS))
