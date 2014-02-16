CC=gcc
CXX=g++
CFLAGS=-O2 -Wall -DDEBUG_LOG -DASSERT_LOG -DHAVE_BACKTRACE -I.
CPPFLAGS=-std=c++11 -O2 -Wall -DDEBUG_LOG -DASSERT_LOG -DHAVE_BACKTRACE -I.
LDFLAGS=-lz -lpcre -pthread

USE_JEMALLOC=yes

ALLOC_DEP=../deps/jemalloc/lib/libjemalloc.a
ALLOC_LD=$(ALLOC_DEP) -ldl
ALLOC_FLAGS=-DUSE_JEMALLOC -I../deps/jemalloc/include

XLIBS=util.o log.o exception.o string.o buffer.o datetime.o \
	  socket.o epoll.o pregex.o urllib.o httplib.o cookie.o
LIBS=$(XLIBS)
CORES=ioloop.o iostream.o tcpserver.o httputil.o httpserver.o
WEBS=
OBJS=$(LIBS) $(CORES) $(WEBS)

TESTS=log_test exception_test string_test buffer_test \
	  socket_server_test socket_client_test

all: $(LIBS) $(CORES) $(WEBS)

test: $(TESTS)

clean: clean_test
	rm -f *.o

clean_test:
	rm -f *_test*

$(XLIBS): %.o: lib/%.cc $(ALLOC_DEP)
	$(CXX) -c $(CPPFLAGS) $(ALLOC_FLAGS) -o $@ $<

$(CORES): %.o: core/%.cc $(ALLOC_DEP)
	$(CXX) -c $(CPPFLAGS) $(ALLOC_FLAGS) -o $@ $<

$(WEBS): %.o: web/%.cc $(ALLOC_DEP)
	$(CXX) -c $(CPPFLAGS) $(ALLOC_FLAGS) -o $@ $<

$(TESTS): %: test/%.cc $(OBJS)
	$(CXX) $(CPPFLAGS) $(ALLOC_FLAGS) -o $@ $(OBJS) $(LDFLAGS) $(ALLOC_LD) $<

../deps/jemalloc/lib/libjemalloc.a:
	cd ../deps/jemalloc && ./configure --with-jemalloc-prefix=je_ --enable-cc-silence && $(MAKE) lib/libjemalloc.a
