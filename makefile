CXX = clang++
CXXFLAGS = -Wall -std=c++20 -I/opt/homebrew/include/SDL
LDFLAGS = -L/opt/homebrew/lib
LIBS = $(shell /opt/homebrew/bin/sdl-config --libs) -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lphysfs -liconv
TARGET = Release/stg
SOURCES = main.cpp game.cpp MainMenu.cpp PlayerBullet.cpp player.cpp timer.cpp UI.cpp GameBackground.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJECTS) $(TARGET)