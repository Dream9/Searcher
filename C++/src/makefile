ALL:target

TOPDIR=$(shell pwd)
SUBDIR=utils collector index query

target:$(SUBDIR)
	cd ./lib && ar crs libdream-search.a *.o
	@echo all done!

$(SUBDIR):ECHO
	$(MAKE) -C $@

ECHO:
	@echo $(SUBDIR)

.PHONY:clean

clean:
	@for dir in $(SUBDIR);\
		do $(MAKE) -C $$dir clean || exit 1;\
		done
	rm -f *.o
	cd ./lib && rm -f *.o *.a
	@echo all clear
