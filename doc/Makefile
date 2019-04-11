############################################################
# Makefile for building documentation
############################################################

SampleFlow-files      = $(shell echo ../include/sampleflow/*.h \
                            ../include/sampleflow/*/*.h)

default: SampleFlow.tag

# Build the documentation
SampleFlow.tag: $(SampleFlow-files) Makefile
	@if test ! -x "`which doxygen`" ; then \
	  echo "-----------------------------------------" ; \
	  echo "Can't find 'doxygen'." ; \
	  echo "-----------------------------------------" ; \
	  false ; \
	 fi
	@echo "=====SampleFlow=================== Generating documentation"
	@echo '@INCLUDE = options.dox'                                > SampleFlow.dox
	@echo 'DOT_IMAGE_FORMAT = png'                               >> SampleFlow.dox
	@echo 'IMAGE_PATH = images'                                  >> SampleFlow.dox
	@echo 'PROJECT_NAME = "SampleFlow"'                          >> SampleFlow.dox
	@echo 'INPUT = $(SampleFlow-files)'                          >> SampleFlow.dox
	@echo 'HTML_OUTPUT      = doxygen'                           >> SampleFlow.dox
	@echo 'GENERATE_TAGFILE = $@'                                >> SampleFlow.dox
	@doxygen SampleFlow.dox || echo "Running doxygen to generate documentation failed."


clean:
	-rm -f SampleFlow.dox SampleFlow.tag
	-rm -rf doxygen
