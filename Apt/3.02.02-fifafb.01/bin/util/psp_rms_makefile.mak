# --- directories
PSPHOME		= C:/packages/PSP/0.2.3/SDK/sce/psp
SDKDIR		= C:/packages/PSP/0.2.3/SDK/sce/psp
GCCDIR		= C:/packages/PSP/0.2.3/SDK/sce/gcc/psp

EAGLHOME	= C:/packages/eaglpsp/4.08.07-fusion-03
REALCORE	= C:/packages/realcore/5.24.00-fusion-03
REALGRAPH	= C:/packages/realgraph/5.23.00-fusion-02

# --- includes
INCDIR		= -I$(SDKDIR)/include -I$(REALCORE)/include -I$(REALGRAPH)/include -I$(EAGLHOME)/include -I$(EAGLHOME)/include/cmn -I$(EAGLHOME)/include/psp -I$(PSPHOME)/include

# --- libraries
##LIBDIR		= -L$(SDKDIR)/lib -L$(GCCDIR)/lib -L$(REALCORE)/lib/psp-gcc  -L$(REALGRAPH)/lib/psp-gcc  -L$(EAGLHOME)/lib.psp  -L$(PSPHOME)/lib

# --- definitions
DEFINES		= -DDEBUG -DPSP -D_DEBUG
INCLUDES	= $(INCDIR)
##LIBRARIES	= -lgmo_model -lgim_picture -lgum -lgu -lemu -lm -lg -lc -lstdc++ -lsupc++ 

# --- cc
CC			= psp-gcc
CXX			= psp-g++

# --- ld
##LD			= psp-ld

# --- ar
AR			= psp-ar

# --- flags
CFLAGS		= -c -g -O0 -Wall -std=gnu9x $(INCLUDES) $(DEFINES)
CXXFLAGS	= -c -g -O0 -Wall $(INCLUDES) $(DEFINES)
##LDFLAGS		= -shared $(LIBDIR) $(LIBRARIES)

# --- files
TARGET		= aptrm.a
OBJS		= GouraudApt.o TextureApt.o

# --- rules
.PHONY: all clean

all:	$(TARGET)

$(TARGET): $(OBJS)
	$(AR) rc $@ $(OBJS) 

.cpp.o:
	$(CXX) -o $@ $(CXXFLAGS) -c $<

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

lib:
	$(AR) rc $(TARGET) $(OBJS) 

# --- operations
clean:
	-rm -f $(TARGET)
	-rm -f $(OBJS)

