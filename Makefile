
CC		= ${CROSS_COMPILE}gcc
STRIP   = ${CROSS_COMPILE}strip
RM      = rm

ROOT_DIR	= $(shell pwd)
BIN_DIR		= $(ROOT_DIR)/my-demo
PROJ_INC	= $(ROOT_DIR)/usr/local/include

CFLAGS		= -Wall -O
CFLAGS		+= -g -I$(PROJ_INC)

export CC STRIP RM BIN_DIR CFLAGS

SUB_DIRS	= src

all: $(SUB_DIRS)

$(SUB_DIRS): ECHO
	make -C $@
	
ECHO:
	@echo "==========> building..."

.PHONY : clean
clean:
	-$(RM) -f $(BIN_DIR)/*
