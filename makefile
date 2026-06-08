CXX ?= clang++
CC ?= clang
CXXFLAGS = -Wall -std=c++20
CFLAGS = -Wall

EXT =

LIBS_BASE = -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx -lphysfs

ifeq ($(OS), Windows_NT)
    EXT = .exe
    CXX ?= g++
    CC ?= gcc
    CXXFLAGS += $(shell sdl-config --cflags)
    CFLAGS += $(shell sdl-config --cflags)
    LDFLAGS = -mwindows
    LIBS = -lmingw32 -lSDLmain $(shell sdl-config --libs) $(LIBS_BASE)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)
        BREW_PREFIX := $(shell brew --prefix)
        CXXFLAGS += -I$(BREW_PREFIX)/include/SDL -I$(BREW_PREFIX)/include
        CFLAGS += -I$(BREW_PREFIX)/include/SDL -I$(BREW_PREFIX)/include
        LDFLAGS = -L$(BREW_PREFIX)/lib
        LIBS = $(shell $(BREW_PREFIX)/bin/sdl-config --libs) $(LIBS_BASE) -liconv
    else
        CXXFLAGS += $(shell sdl-config --cflags)
        CFLAGS += $(shell sdl-config --cflags)
        LDFLAGS = 
        LIBS = $(shell sdl-config --libs) $(LIBS_BASE)
    endif
endif

TARGET_DIR = Release
TARGET = $(TARGET_DIR)/stg$(EXT)

SOURCES = main.cpp game.cpp MainMenu.cpp PlayerBullet.cpp player.cpp timer.cpp UI.cpp GameBackground.cpp cJSON.cpp cJSON_Utils.cpp LevelManager.cpp Enemy.cpp EnemyBulletManager.cpp

OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

.PHONY: all clean create_dir bundle

all: create_dir $(TARGET) bundle

create_dir:
	@mkdir -p $(TARGET_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)

bundle:
	@echo "Bundling resources..."
	@cp -r res $(TARGET_DIR)/res 2>/dev/null || true
	@cp -r level $(TARGET_DIR)/level 2>/dev/null || true

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
