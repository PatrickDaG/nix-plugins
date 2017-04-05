.POSIX:

all: nix-plugins.nixc

PREFIX=/usr/local
CXX=c++
MKDIR=mkdir
CP=cp
RM=rm
NIX_INCLUDE=/usr/local/include
GC_INCLUDE=/usr/local/include
PLUGINS_CXXFLAGS=-O3 -std=c++14 -fpic -I$(NIX_INCLUDE)/nix -I$(GC_INCLUDE) $(CXXFLAGS)
PLUGINS_LDFLAGS=-shared -undefined dynamic_lookup $(LDFLAGS)
OBJS=initialize.o exec.o

.SUFFIXES:

.SUFFIXES:.cc .o

.cc.o:
	$(CXX) $(PLUGINS_CXXFLAGS) -c $<

nix-plugins.nixc: $(OBJS)
	$(CXX) $(PLUGINS_LDFLAGS) -o nix-plugins.nixc $(OBJS)

install: nix-plugins.nixc
	$(MKDIR) -p -m 755 $(PREFIX)/lib
	$(CP) nix-plugins.nixc $(PREFIX)/lib

clean:
	$(RM) -f $(OBJS) nix-plugins.nixc
