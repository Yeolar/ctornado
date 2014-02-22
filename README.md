CTornado
========

A C++ fork of Facebook Tornado web framework.

Required C++11.

Build
-----

Just use:

    $ make

To build with clang compiler instead of gcc, use:

    $ make USE_CLANG=yes

Allocator
---------

By default CTornado compiles and links against jemalloc.

To force a libc malloc() build use:

    $ make FORCE_LIBC_MALLOC=yes

