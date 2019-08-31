all: a.exe a-msg-only.exe

a.exe: main.cxx
	x86_64-w64-mingw32-g++ -o $@ $< -mconsole -static $(CXXFLAGS) $(LDFLAGS) -UMESSAGE_ONLY

# Doesn't work under Wine.
#a-msg-only.exe: main.cxx
#	x86_64-w64-mingw32-g++ -o $@ $< -mconsole -static $(CXXFLAGS) $(LDFLAGS) -DMESSAGE_ONLY
