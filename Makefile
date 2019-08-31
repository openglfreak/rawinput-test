all: a.exe

a.exe: main.cxx
	x86_64-w64-mingw32-g++ -o a.exe main.cxx -mconsole -static $(CXXFLAGS) $(LDFLAGS)
