# Makefile to compile our PDF
# You should replace the below variables for your own system

LATEX_BIN=/usr/local/texlive/2013/bin/x86_64-darwin/
LATEX=$(LATEX_BIN)/pdflatex
BIBTEX=$(LATEX_BIN)/bibtex

all: final-paper.pdf extended-abstract.pdf

final-paper.pdf: final-paper.tex ref.bib
	$(LATEX) final-paper.tex
	$(BIBTEX) final-paper
	$(LATEX) final-paper.tex
	$(LATEX) final-paper.tex

extended-abstract.pdf: extended-abstract.tex ref.bib
	$(LATEX) extended-abstract.tex
	$(BIBTEX) extended-abstract
	$(LATEX) extended-abstract.tex
	$(LATEX) extended-abstract.tex

.PHONY: clean
clean:
	rm -f *.out *.aux *.log *.pdf *.bbl *.dvi *.aux *.blg
