#############################################
#  FaustLive documentation
#  UserManual.pdf is installed with FaustLive and embedded in the application
#  (help window): rebuild it when UserManual.tex or ../version.txt changes,
#  and commit it. faust-quick-reference.pdf comes from the Faust repository
#  (documentation/faust-quick-reference.pdf).
#############################################

PDFLATEX ?= pdflatex

all: UserManual.pdf

# twice, for the table of contents
UserManual.pdf: UserManual.tex ../version.txt
	$(PDFLATEX) -interaction=nonstopmode -halt-on-error UserManual.tex
	$(PDFLATEX) -interaction=nonstopmode -halt-on-error UserManual.tex

clean:
	rm -f UserManual.aux UserManual.log UserManual.toc UserManual.out
