all:
	cd src && $(MAKE) $@

clean:
	cd src && $(MAKE) $@
	cd deps/jemalloc && $(MAKE) distclean
