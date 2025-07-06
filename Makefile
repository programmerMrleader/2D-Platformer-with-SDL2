CXX = x86_64-w64-mingw32-c++


# Compiler flags
CXXFLAGS = -lstdc++ -Wall -Wextra -ISDL2/x86_64-w64-mingw32/include

# Linker flags - IMPORTANT ORDER: SDL2main must come first!
LDFLAGS = -lSDL2 -lSDL2main -lSDL2_image -LSDL2/x86_64-w64-mingw32/lib -lstdc++
# Required Windows libraries

#WIN_LIBS = -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 \
#           -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid

SRCDIR = src
OBJDIR = obj

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TARGET = platform-game.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS)  -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
