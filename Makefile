IDIR=include
ODIR=obj
LIBS=-lsqlite3 -lpcrecpp
VER=$(shell cat include/VERSION 2> /dev/null || echo -n 0.0)
CC=g++-4.4
CFLAGS=-I$(IDIR) -O2 -Wall -D_VERSION_=$(VER) -D_GNU_SOURCE -Wno-write-strings -ggdb

_DEPS = lgram.h preprocess.h window.h error.h output.h input.h \
        output.h options.h blocks.h merge_blocks.h process.h regex.h \
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = preprocess.o window.o lgram.o error.o sqlite.o input.o \
       usage.o output.o options.o blocks.o merge_blocks.o \
       process.o regex.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	@./check_libs.sh
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/%.o: %.cpp $(DEPS)
	@./check_libs.sh
	$(CC) $(CFLAGS) -c $< -o $@

lgram: $(OBJ) 
	$(CC) $(CFLAGS) $(OBJ) $(LIBS) -o lgram

.PHONY: clean

clean:
	rm $(ODIR)/*.o lgram
