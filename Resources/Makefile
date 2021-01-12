
.PHONY: application.qrc

all: application.qrc 

help:
	@echo "============ FaustLive tools ============"
	@echo "Available targets are:"
	@echo "  all (default) : generate application.qrc"
	@echo "  clean         : remove output files"

application.qrc: application.qrc.base
	cat application.qrc.base > $@
	ls Libs/*.lib | sed -e 's/^/    <file>/' | sed -e  's/\.lib/.lib<\/file>/' >> $@
	echo '</qresource>' >> $@
	echo '</RCC>' >> $@

	
clean:
	rm -f application.qrc

