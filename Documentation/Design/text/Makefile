ALL=objects.html auxobjects.html overall.html timelines.html

REST2HTML=/Users/jack/tmp/docutils/tools/html.py
PYTHON=python

.SUFFIXES: .txt .html

all: $(ALL)

.txt.html:
	$(PYTHON) $(REST2HTML) $< $@
