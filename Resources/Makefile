
.PHONY: application.qrc

all: application.qrc 

help:
	@echo "============ FaustLive tools ============"
	@echo "Available targets are:"
	@echo "  all (default) : generate application.qrc"
	@echo "  clean         : remove output files"

# The Faust libraries are flattened into ':/Libs', like the Faust 'install' script
# does in 'faust --dspdir' (cp *.lib old/*.lib dx7/*.lib), so that for instance
# stdfaust.lib's library("dx7.lib") resolves to Libs/dx7/dx7.lib.
# doc.lib is excluded: it only serves to generate the libraries documentation
# and cannot be imported once flattened (it imports "dx7/*.lib").
LIBSUBDIRS := old dx7
LIBEXCLUDE := Libs/doc.lib

application.qrc: application.qrc.base
	cat application.qrc.base > $@
	ls Libs/*.lib | grep -v -x -F $(addprefix -e ,$(LIBEXCLUDE)) | sed -e 's/^/    <file>/' | sed -e  's/\.lib/.lib<\/file>/' >> $@
	for d in $(LIBSUBDIRS); do \
		ls Libs/$$d/*.lib | sed -e 's#^Libs/[^/]*/\(.*\)$$#    <file alias="Libs/\1">&</file>#' >> $@; \
	done
	echo '</qresource>' >> $@
	echo '</RCC>' >> $@

	
clean:
	rm -f application.qrc

