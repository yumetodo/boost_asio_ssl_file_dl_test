# boost.asio File Download via SSL test

test code for my study.

# Compiler require

- Visual Studio 2015 Update 1 or later
- some compiler that able to compile C++14 code

# library require
- boost1.61.0  
    - boost.asio
    - boost.optional
    - boost.iostreams.filter.zlib
- OpenSSL v1.0.2h
- zlib

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
http://www.boost.org/users/history/version_1_61_0.html
download boost_1_61_0.7z( or zip) and unzip somewhere.

### zlib
http://zlib.net/

1. Download ``zlib-1.2.8.tar.xz`` and unzip somewhere.

### libbzip2
http://www.bzip.org/downloads.html

### check directory tree

```
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