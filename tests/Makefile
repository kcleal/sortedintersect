CXXFLAGS=	-g -Wall -O3 -std=c++17
#CFLAGS=		-g -Wall -O2 -Wc++-compat
LIBS=		-lz
EXE=		run-cpp-libs
INCLUDE= -I../sortedintersect -I. -I./3rd-party
ifneq ($(asan),)
	CFLAGS+=-fsanitize=address
	CXXFLAGS+=-fsanitize=address
	LIBS+=-fsanitize=address
endif

all:$(EXE)



run-cpp-libs: bench.cpp
		$(CXX) $(CXXFLAGS) $(INCLUDE) $< $(LIBS) -o $@


clean:
		rm -fr *.o a.out *.dSYM $(EXE)