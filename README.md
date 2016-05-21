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

## zlib
http://zlib.net/

1. Download ``zlib-1.2.8.tar.xz`` and unzip and move directories to somewhere you like that is not include space in path.

## lib
http://www.bzip.org/downloads.html