CXX ?= clang++
CC ?= clang
CXXFLAGS = -Wall -std=c++20
CFLAGS = -Wall

UNAME_S := $(shell uname -s)

LIBS = -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lphysfs

ifeq ($(UNAME_S), Darwin)
    # macOS
    BREW_PREFIX := $(shell brew --prefix)
    CXXFLAGS += -I$(BREW_PREFIX)/include/SDL -I$(BREW_PREFIX)/include
    CFLAGS += -I$(BREW_PREFIX)/include/SDL -I$(BREW_PREFIX)/include
    LDFLAGS = -L$(BREW_PREFIX)/lib
    LIBS += $(shell $(BREW_PREFIX)/bin/sdl-config --libs) -liconv
else
    # Linux
    CXXFLAGS += $(shell sdl-config --cflags)
    CFLAGS += $(shell sdl-config --cflags)
    LDFLAGS = 
    LIBS += $(shell sdl-config --libs)
endif

TARGET_DIR = Release
TARGET = $(TARGET_DIR)/stg

SOURCES = main.cpp game.cpp MainMenu.cpp PlayerBullet.cpp player.cpp timer.cpp UI.cpp GameBackground.cpp cJSON.cpp cJSON_Utils.cpp LevelManager.cpp Enemy.cpp EnemyBulletManager.cpp

OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

all: create_dir $(TARGET)

create_dir:
	@mkdir -p $(TARGET_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
