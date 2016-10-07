# boost.asio File Download via SSL test

test code for my study.

# Compiler require

- Visual Studio 2015 Update 1 or later
- some compiler that able to compile C++14 code

# library require
- boost1.61.0 or later
    - boost.asio
    - boost.optional
    - boost.iostreams.filter.zlib
    - boost.iostreams.filter.bzip2
- OpenSSL v1.0.2h or later
- zlib
- bzip2

# Enviroment Setup(Windows)

## OpenSSL
https://slproweb.com/products/Win32OpenSSL.html

1. Download ``Win32 OpenSSL v1.0.2h`` and ``Win64 OpenSSL v1.0.2h``
2. run ``Win32OpenSSL-1_0_2h.exe`` and ``Win64OpenSSL-1_0_2h.exe`` and install to openssl directory(ex. ``C\lib\openssl``)
3. Set User Environment Variable ``OPENSSL_ROOT`` to openssl directory(ex. ``C\lib\openssl``)

```
C\lib\openssl
├─OpenSSL-Win32
│  ├─bin
│  │  └─PEM
│  │      ├─demoCA
│  │      │  └─private
│  │      └─set
│  ├─exp
│  ├─include
│  │  └─openssl
│  └─lib
│      ├─MinGW
│      └─VC
│          └─static
└─OpenSSL-Win64
    ├─bin
    │  └─PEM
    │      ├─demoCA
    │      │  └─private
    │      └─set
    ├─exp
    ├─include
    │  └─openssl
    └─lib
        └─VC
            └─static
```

## boost
http://www.boost.org/users/history/version_1_62_0.html
download boost_1_62_0.7z( or zip) and unzip somewhere.

### zlib
http://zlib.net/

1. Download ``zlib-1.2.8.tar.xz`` and unzip somewhere.

### libbzip2
http://www.bzip.org/downloads.html

### check directory tree

```
C:\lib
├─boost_1_61_0
│  ├─boost
│  ├─doc
│  ├─libs
│  ├─more
│  ├─status
│  └─tools
├─bzip2-1.0.6
├─openssl
│  ├─OpenSSL-Win32
│  │  ├─bin
│  │  │  └─PEM
│  │  │      ├─demoCA
│  │  │      │  └─private
│  │  │      └─set
│  │  ├─exp
│  │  ├─include
│  │  │  └─openssl
│  │  └─lib
│  │      ├─MinGW
│  │      └─VC
│  │          └─static
│  └─OpenSSL-Win64
│      ├─bin
│      │  └─PEM
│      │      ├─demoCA
│      │      │  └─private
│      │      └─set
│      ├─exp
│      ├─include
│      │  └─openssl
│      └─lib
│          └─VC
│              └─static
└─zlib-1.2.8
   ├─amiga
   ├─as400
   ├─contrib
   ├─doc
   ├─examples
   ├─msdos
   ├─nintendods
   ├─old
   │  └─os2
   ├─qnx
   ├─test
   ├─watcom
   └─win32
```

Please modify script below when you do different tree.

### boost build
1. Download ``bzip2-1.0.6.tar.gz`` and unzip somewhere.
2. run this script at Boost root directory.

```
bootstrap.bat
b2 toolset=msvc threading=multi variant=debug,release link=static runtime-link=static address-model=32 --stagedir=stage/x86 -j 4 -s BZIP2_SOURCE=C:/lib/bzip2-1.0.6 -s ZLIB_SOURCE=C:/lib/zlib-1.2.8
b2 toolset=msvc threading=multi variant=debug,release link=shared runtime-link=shared address-model=32 --stagedir=stage/x86 -j 4 -s BZIP2_SOURCE=C:/lib/bzip2-1.0.6 -s ZLIB_SOURCE=C:/lib/zlib-1.2.8
b2 toolset=msvc threading=multi variant=debug,release link=static runtime-link=static address-model=64 --stagedir=stage/x64 -j 4 -s BZIP2_SOURCE=C:/lib/bzip2-1.0.6 -s ZLIB_SOURCE=C:/lib/zlib-1.2.8
b2 toolset=msvc threading=multi variant=debug,release link=shared runtime-link=shared address-model=64 --stagedir=stage/x64 -j 4 -s BZIP2_SOURCE=C:/lib/bzip2-1.0.6 -s ZLIB_SOURCE=C:/lib/zlib-1.2.8
````

cf.)
- http://stackoverflow.com/questions/7282645/how-to-build-boost-iostreams-with-gzip-and-bzip2-support-on-windows

# Enviroment Setup(Ubuntu)
## boost

http://www.boost.org/users/history/version_1_62_0.html
download boost_1_62_0.tar.bz2( or .tar.gz) and unzip somewhere.

## bzip2

```sh
$ sudo apt install libbz2-dev
```

## boost build

```sh
$ ./bootstrap.sh
$ ./b2 threading=multi vaddress-model=64 architecture=x86 -j 4 cxxflags="-std=c++14"
$ sudo ./b2 install
$ cd /usr/local/lib
$ sudo ldconfig
```

# Set Environment Variable(VS only)

required environment variables is below.

- ``BOOST_ROOT``: ``C:\lib\boost_1_61_0``
- ``OPENSSL_ROOT`` : ``C:\lib\openssl``
