NAME=hw02
BIB=
EXTRAFILES=
IMAGES=
LATEXFLAGS=-shell-escape
BUILDDIR=build
PDFLATEX=xelatex
INKSCAPE=inkscape

export TEXINPUTS=$(BUILDDIR):
IMAGES_PDF=$(patsubst %,$(BUILDDIR)/%,$(IMAGES:%.svg=%.pdf))

.PHONY: all single bib clean

all: $(BUILDDIR)/$(NAME).pdf

single: $(NAME).tex $(EXTRAFILES)
	@mkdir -p build
	@echo "Compiling $<"
	@texfot --tee=/dev/null --quiet $(PDFLATEX) -output-directory="$(BUILDDIR)" \
		$(LATEXFLAGS) $(NAME).tex

bib: $(NAME).tex $(BIB)
	@echo "Running biber"
	@biber -q --output_directory "$(BUILDDIR)" $(NAME).bcf

$(BUILDDIR)/%.pdf: %.svg
	@mkdir -p build
	@echo "Creating $@"
	@$(INKSCAPE) --export-pdf="$@" "$<" 2>/dev/null


ifeq ($(BIB),)
build_cmd=$(MAKE) --no-print-directory single && \
	$(MAKE) --no-print-directory single
else
build_cmd=$(MAKE) --no-print-directory single && \
	$(MAKE) --no-print-directory bib && \
	$(MAKE) --no-print-directory single
endif

$(BUILDDIR)/$(NAME).pdf: $(NAME).tex $(IMAGES_PDF) $(BIB) $(EXTRAFILES)
	@$(build_cmd)

clean:
	rm -rf "$(BUILDDIR)"
