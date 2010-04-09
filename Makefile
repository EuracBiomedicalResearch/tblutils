## Makefile for tblutils

# Flags
CXXFLAGS += -ggdb3 -ansi $(CWARN)
CPPFLAGS += -MD

# Config
TBLTRANSP2_OBJECTS = tbltransp2.o
TBL2EXCEL_OBJECTS = tbl2excel.o
TARGETS = tbltransp2 tbl2excel


# Rules
.SUFFIXES: .cc .o
.PHONY: all clean

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


# Targets
all: $(TARGETS)

tbltransp2: $(TBLTRANSP2_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBLTRANSP2_OBJECTS) $(LDFLAGS) $(LDADD)

tbl2excel: $(TBL2EXCEL_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBL2EXCEL_OBJECTS) $(LDFLAGS) $(LDADD)

clean:
	rm -rf *.o *.d core ii_files $(TARGETS)


# Dependencies
sinclude *.d
