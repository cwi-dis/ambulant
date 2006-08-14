ALL=overall.txt walkthrough.txt objects.txt organisation.txt
ALL_HTML=objects.html overall.html timeline.html walkthrough.html organisation.html	
ALL_LATEX=objects.latex overall.latex walkthrough.latex organisation.latex	

REST=/Users/jack/tmp/docutils/tools
REST2HTML=$(REST)/rst2html.py
REST2LATEX=$(REST)/rst2latex.py
PDFLATEX=/usr/local/teTeX/bin/i386-apple-darwin-current/pdflatex
MAKEINDEX=/usr/local/teTeX/bin/i386-apple-darwin-current/makeindex
PYTHON=python

.SUFFIXES: .txt .html .tex .pdf

all: html pdf
html: $(ALL_HTML)
pdf: ambulantdesign.pdf

clean:
	-rm ambulantdesign.txt ambulantdesign.tex ambulantdesign.aux ambulantdesign.idx
	-rm ambulantdesign.idg ambulantdesign.ilg ambulantdesign.ind ambulantdesign.log
	-rm ambulantdesign.out ambulantdesign.toc ambulantdesign.pdf
	-rm $(ALL_HTML)
	
ambulantdesign.pdf: ambulantdesign.txt

ambulantdesign.txt: head_latex.txt $(ALL) diagrams.txt
	cat head_latex.txt $(ALL) diagrams.txt > ambulantdesign.txt

.txt.html:
	cat head_html.txt $< $*_tail.txt | $(PYTHON) $(REST2HTML) > $@

.txt.tex:
	$(PYTHON) $(REST2LATEX) --use-latex-toc $< $@
	
.tex.pdf:
	$(PDFLATEX) $< 
	$(PDFLATEX) $< 
	$(PDFLATEX) $< 
