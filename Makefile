## Makefile for gm_snp2x

# Flags
CXXFLAGS += -ggdb3 -ansi $(CWARN)
CPPFLAGS += -MD

# Config
TBLTRANSP2_OBJECTS = tbltransp2.o
TARGETS = tbltransp2


# Rules
.SUFFIXES: .cc .o
.PHONY: all clean

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


# Targets
all: $(TARGETS)

tbltransp2: $(TBLTRANSP2_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBLTRANSP2_OBJECTS) $(LDFLAGS) $(LDADD)

clean:
	rm -rf *.o *.d core ii_files $(TARGETS)


# Dependencies
sinclude *.d
