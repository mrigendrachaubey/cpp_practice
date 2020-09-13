main: libshared.so derived.o main.o
	$(CXX) -o main  derived.o main.o -L. -lshared

derived.o: derived.cpp
	$(CXX) -c derived.cpp

libshared.so: shared.cpp
	$(CXX) -fPIC -c shared.cpp -o shared.o
	$(CXX) -shared  -Wl,-soname,libshared.so -o libshared.so shared.o

clean:
	rm main *.o *.so
