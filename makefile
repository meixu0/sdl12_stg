CXX ?= clang++
CC ?= clang
CXXFLAGS = -Wall -std=c++98 -Isrc -g
CFLAGS = -Wall -Isrc -g

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

SRC_DIR = src
TARGET = stg$(EXT)

SOURCES = main.cpp game.cpp MainMenu.cpp PlayerBullet.cpp player.cpp timer.cpp UI.cpp GameBackground.cpp cJSON.cpp cJSON_Utils.cpp LevelManager.cpp Enemy.cpp EnemyBulletManager.cpp ItemManager.cpp InfoArea.cpp StartMenu.cpp OptionMenu.cpp Settlement.cpp ResultDisplay.cpp MusicRoom.cpp DifficultyMenu.cpp EnemyScManager.cpp PlayerBomb.cpp MusicRoom.cpp
SOURCES := $(addprefix $(SRC_DIR)/, $(SOURCES))

OBJECTS = $(SOURCES:.cpp=.o)
OBJECTS := $(OBJECTS:.c=.o)

.PHONY: all clean

all: $(TARGET)
	@echo "Build complete: $(TARGET)"
	@rm -f $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
