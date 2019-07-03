ALL:target

TOPDIR=$(shell pwd)
SUBDIR=utils collector index query

target:$(SUBDIR)
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
	@echo all clear
