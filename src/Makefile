CC=gcc
CXX=g++
CFLAGS=-O2 -Wall -DDEBUG_LOG -DASSERT_LOG -DHAVE_BACKTRACE -I.
CPPFLAGS=-std=c++11 -O2 -Wall -DDEBUG_LOG -DASSERT_LOG -DHAVE_BACKTRACE -I.
LDFLAGS=-lz -lpcre

XLIBS=util.o log.o exception.o string.o buffer.o datetime.o \
	  socket.o epoll.o pregex.o urllib.o httplib.o cookie.o
LIBS=$(XLIBS)
CORES=ioloop.o iostream.o tcpserver.o httputil.o httpserver.o
WEBS=

TESTS=log_test

all: $(LIBS) $(CORES) $(WEBS)

test: $(TESTS)

clean:
	rm -f *.o

clean_test:
	rm -f *_test*

$(XLIBS): %.o: lib/%.cc
	$(CXX) -c -o $@ $(CPPFLAGS) $<

$(CORES): %.o: core/%.cc
	$(CXX) -c -o $@ $(CPPFLAGS) $<

$(WEBS): %.o: web/%.cc
	$(CXX) -c -o $@ $(CPPFLAGS) $<

$(TESTS): %: test/%.cc $(LIBS) $(CORES) $(WEBS)
	$(CXX) -o $@ $(CPPFLAGS) $(LDFLAGS) $< $(LIBS) $(CORES) $(WEBS)

