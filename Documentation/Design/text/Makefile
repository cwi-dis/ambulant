ALL=objects.html overall.html timeline.html walkthrough.html organisation.html	

REST2HTML=/Users/jack/tmp/docutils/tools/html.py
PYTHON=python

.SUFFIXES: .txt .html

all: $(ALL)

.txt.html:
	$(PYTHON) $(REST2HTML) $< $@
