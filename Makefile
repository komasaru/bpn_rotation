gcc_options = -std=c++17 -Wall -O2 --pedantic-errors

bpn_rotation: bpn_rotation.o bpn.o matrix.o obliquity.o nutation.o delta_t.o time.o file.o
	g++102 $(gcc_options) -o $@ $^

bpn_rotation.o : bpn_rotation.cpp
	g++102 $(gcc_options) -c $<

matrix.o : matrix.cpp
	g++102 $(gcc_options) -c $<

bpn.o : bpn.cpp
	g++102 $(gcc_options) -c $<

obliquity.o : obliquity.cpp
	g++102 $(gcc_options) -c $<

nutation.o : nutation.cpp
	g++102 $(gcc_options) -c $<

delta_t.o : delta_t.cpp
	g++102 $(gcc_options) -c $<

time.o : time.cpp
	g++102 $(gcc_options) -c $<

file.o : file.cpp
	g++102 $(gcc_options) -c $<

run : bpn_rotation
	./bpn_rotation

clean :
	rm -f ./bpn_rotation
	rm -f ./*.o

.PHONY : run clean

