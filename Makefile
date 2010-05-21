## Makefile for tblutils

# Flags
CXXFLAGS += -ggdb3 -ansi $(CWARN)
CPPFLAGS += -MD

# Paths
DESTDIR :=
PREFIX := /usr/local

# Config
TBLTRANSP2_OBJECTS = tbltransp2.o shared.o
TBLMERGE2_OBJECTS = tblmerge2.o shared.o
TBLCUT_OBJECTS = tblcut.o shared.o
TBL2EXCEL_OBJECTS = tbl2excel.o shared.o
BUILT = tbltransp2 tblmerge2 tblcut tbl2excel
TARGETS = tblabelize tblcsort tblfilter tblmerge tblnorm tbltransp \
	tblunlabelize tbl2excel-helper $(BUILT)


# Rules
.SUFFIXES: .cc .o
.PHONY: all clean install

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<


# Targets
all: $(TARGETS)

tbltransp2: $(TBLTRANSP2_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBLTRANSP2_OBJECTS) $(LDFLAGS) $(LDADD)

tblmerge2: $(TBLMERGE2_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBLMERGE2_OBJECTS) $(LDFLAGS) $(LDADD)

tblcut: $(TBLCUT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBLCUT_OBJECTS) $(LDFLAGS) $(LDADD)

tbl2excel: $(TBL2EXCEL_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(TBL2EXCEL_OBJECTS) $(LDFLAGS) $(LDADD)

clean:
	rm -rf *.o *.d core ii_files $(BUILT)

install: $(TARGETS)
	install -p -t $(DESTDIR)$(PREFIX)/bin/ $(TARGETS)


# Dependencies
sinclude *.d
