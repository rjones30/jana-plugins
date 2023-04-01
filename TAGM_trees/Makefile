#
# Standard Makefile for JANA plugins
#

plugin := $(notdir $(shell pwd))
src := $(shell ls *.cc)
hdr := $(shell ls *.h)

CPPFLAGS += -I $(JANA_HOME)/include
CPPFLAGS += -I $(HALLD_RECON_HOME)/$(BMS_OSNAME)/include
CPPFLAGS += -I $(shell root-config --incdir)

CXXFLAGS += -g -O0 -fPIC -std=c++11

all: $(plugin).so

$(plugin).so: $(src) $(hdr)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Wl,--export-dynamic -Wl,-soname,$@ -shared -o $@ $(src)
	mv $@ $(HALLD_RECON_HOME)/$(BMS_OSNAME)/plugins/
