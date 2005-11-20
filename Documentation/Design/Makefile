ALL=index.html
	

REST2HTML=/Users/jack/tmp/docutils/tools/html.py
PYTHON=python

.SUFFIXES: .txt .html

all: html pdf
html: $(ALL) html-recursive
pdf: pdf-recursive

.txt.html:
	$(PYTHON) $(REST2HTML) $< $@

html-recursive:
	(cd text ; $(MAKE) html)

pdf-recursive:
	(cd text ; $(MAKE) pdf)
	