CC=gcc
DOCC=doxygen
#CFLAGS=-g -Wall
CFLAGS=-O3 -Wall

REFDIR=.
SRCDIR=$(REFDIR)/src
BINDIR=$(REFDIR)/bin
DOCDIR=$(REFDIR)/doc

CSOURCE=$(wildcard $(SRCDIR)/*.c)

all: binary doc 


$(BINDIR)/applyPatch: $(SRCDIR)/applyPatch.c
	$(CC) $(CFLAGS)  $^ -o $@

$(BINDIR)/computePatchOpt: $(SRCDIR)/computePatchOpt.c
	$(CC) $(CFLAGS) -std=c99 $^ -o $@

$(DOCDIR)/index.html: $(SRCDIR)/Doxyfile $(CSOURCE) 
	$(DOCC) $(SRCDIR)/Doxyfile

binary: $(BINDIR)/applyPatch $(BINDIR)/computePatchOpt

doc: $(DOCDIR)/index.html

clean:
	rm -rf $(DOCDIR) $(BINDIR)/*


.PHONY: all doc binary clean
