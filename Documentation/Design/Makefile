ALL=index.html
	

REST2HTML=/Users/jack/tmp/docutils/tools/html.py
PYTHON=python

.SUFFIXES: .txt .html

all: $(ALL) text-recursive

.txt.html:
	$(PYTHON) $(REST2HTML) $< $@

text-recursive:
	(cd text ; $(MAKE))