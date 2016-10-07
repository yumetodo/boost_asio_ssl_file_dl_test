# Makefile for test.out

TARGET 	= boost_asio_ssl_file_dl_test.out
SRCS 	= ./boost_asio_ssl_file_dl_test/downloader.cpp ./boost_asio_ssl_file_dl_test/Source.cpp

# 基本コマンド
RM 		?= rm
CXX 	?= g++
CC		?= gcc

# デバッグ時とリリース時の微調整
CXX_DEBUG_FLAGS		=	-g -O0 -Wall -Wextra
CXX_RELEASE_FLAGS	=	-O2 -Wall -Wextra

# 基本オプション
CPPFLAGS = -std=c++1z -lssl -lcrypto -lz
ifeq ($(OS),Windows_NT)
	CPPFLAGS += -DBOOST_USE_WINDOWS_H -lboost_system-mt -lboost_iostreams-mt -lboost_thread-mt -lWs2_32
else
	CPPFLAGS += -lboost_system -lboost_iostreams -lpthread
endif

CONFIGURATION = unknown
PLATFORM = $(shell uname)

# make
# debug
.PHONY: Debug
Debug: CXXFLAGS+=$(CXX_DEBUG_FLAGS)
Debug: CONFIGURATION=Debug
Debug: all
# release
.PHONY: Release
Release: CXXFLAGS+=$(CXX_RELEASE_FLAGS)
Release: CONFIGURATION=Release
Release: all

.PHONY: all
all: $(TARGET)
$(TARGET): $(SRCS)
	$(CXX) --version;
	$(CXX) $^ -o $@ $(CXXFLAGS) $(CPPFLAGS)
	mkdir -p bin/$(CXX)/$(PLATFORM)/$(CONFIGURATION) && cp $(TARGET) bin/$(CXX)/$(PLATFORM)/$(CONFIGURATION)/$(TARGET)

# make clean
.PHONY: clean
clean:
	$(RM) -f *.out
