# --- directories
SDKDIR		= C:/packages/PSPSN/0.9.0/SDK/PSP
GCCDIR		= C:/packages/PSPSN/0.9.0/SDK/PSP
EAGLHOME	= C:/packages/EAGLPSP/4.08.07-fusion-03
REALCORE	= C:/packages/realcore/5.24.00-fusion-03
REALGRAPH	= C:/packages/realgraph/5.23.00-fusion-02
SN_HOME		= C:/packages/PSPSN/0.9.0/SDK/PSP
PSPHOME		= C:/packages/PSP/0.2.3/SDK/sce/psp

# --- includes
INCDIR		= -I$(SDKDIR)/include -I$(REALCORE)/include -I$(REALGRAPH)/include -I$(EAGLHOME)/include -I$(EAGLHOME)/include/cmn -I$(EAGLHOME)/include/psp -I$(SN_HOME)/include -I$(PSPHOME)/include

# --- libraries
##LIBDIR		= -L$(SDKDIR)/lib -L$(GCCDIR)/lib -L$(REALCORE)/lib/psp-gcc  -L$(REALGRAPH)/lib/psp-gcc  -L$(EAGLHOME)/lib.psp  -L$(PSPHOME)/lib

# --- definitions
DEFINES		= -DDEBUG -DPSP -D_DEBUG
INCLUDES	= $(INCDIR)
##LIBRARIES	= -lgmo_model -lgim_picture -lgum -lgu -lemu -lm -lg -lc -lstdc++ -lsupc++ 

# --- cc
#CC			= psp-gcc
#CXX		= psp-g++
CC			= pspsnc
CXX			= pspsnc

# --- ld
##LD			= psp-ld

# --- ar
#AR			= psp-ar
AR			= snarl

# --- flags
#CFLAGS		= -c -g -O0 -Wall -std=gnu9x $(INCLUDES) $(DEFINES)
CFLAGS		= -c -g -O0  -w $(INCLUDES) $(DEFINES)
CXXFLAGS	= -c -g -O0  -w $(INCLUDES) $(DEFINES)
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

