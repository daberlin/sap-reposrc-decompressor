#CXX     = g++
CXXFLAGS = -O2 -Wall -Wno-maybe-uninitialized
#RM      = rm -f

all: clean
	$(CXX) -o sap-reposrc-decompressor $(CXXFLAGS) sap-*.cpp lib/*.cpp

clean:
	$(RM) sap-reposrc-decompressor tests/ZDBTEST.*.txt

test:
	cd tests
	../sap-reposrc-decompressor ZDBTEST.bin ZDBTEST.ascii.txt
	../sap-reposrc-decompressor ZDBTEST.bin ZDBTEST.utf16.txt -u
	md5sum -c CHECKSUM.md5

.PHONY: clean test
