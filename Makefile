## Makefile for tblutils

# Flags
CXXFLAGS += -ggdb3 -ansi $(CWARN)
CPPFLAGS += -MD

# Paths
DESTDIR :=
PREFIX := /usr/local

# Config
tbltransp2_OBJECTS = tbltransp2.o shared.o
tblmerge2_OBJECTS = tblmerge2.o shared.o
tblcut_OBJECTS = tblcut.o shared.o
tbl2excel_OBJECTS = tbl2excel.o shared.o
BUILT_TARGETS = tbltransp2 tblmerge2 tblcut tbl2excel
TARGETS = tblabelize tblcsort tblfilter tblmerge tblnorm tbltransp \
	tblunlabelize tbl2excel-helper tbl2tbl tblsubsplit tblsubmerge \
	tbltomatrix $(BUILT_TARGETS)


# Rules
.SUFFIXES:
.SECONDEXPANSION:
.PHONY: all clean install

all_OBJECTS := $(foreach T,$(BUILT_TARGETS),$($(T)_OBJECTS))
all_DEPS := $(all_OBJECTS:.o=.d)
all: $(TARGETS)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(TARGETS): %: $$($$@_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $($@_OBJECTS) $(LDFLAGS) $($@_LDADD)

clean:
	rm -f $(all_OBJECTS) $(all_DEPS) $(BUILT_TARGETS)

install: $(TARGETS)
	install -p -t $(DESTDIR)$(PREFIX)/bin/ $(TARGETS)


# Dependencies
sinclude *.d
