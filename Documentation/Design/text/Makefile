ALL=objects.html auxobjects.html overall.html timeline.html datasource.html \
	document.html event_processor.html player.html region.html renderer.html \
	playable.html
	

REST2HTML=/Users/jack/tmp/docutils/tools/html.py
PYTHON=python

.SUFFIXES: .txt .html

all: $(ALL)

.txt.html:
	$(PYTHON) $(REST2HTML) $< $@
