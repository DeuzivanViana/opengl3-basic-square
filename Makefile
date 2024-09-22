.PHONY: clean

BIN_DIR = .bin
SRC_DIR = src

TARGET = app.elf
COMPILER = g++

SOURCE_FILES := \
	$(wildcard $(SRC_DIR)/*.cpp)\
	$(wildcard $(SRC_DIR)/*/*.cpp)

OBJECT_FILES := $(SOURCE_FILES:%.cpp=%.o)

CFLAGS = -pedantic -Iinclude $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image -lGL -lGLEW

RM = rm -f
MKDIR = mkdir -p

%.o: %.cpp
	@$(MKDIR) $(dir $@)
	@$(COMPILER) -c -o $@ $< $(CFLAGS)
	$(info [$(COMPILER)] Compilling $< -> $@...)

$(TARGET): $(OBJECT_FILES)
	@$(COMPILER) -o $@ $^ $(LDFLAGS)
	$(info Generating $(TARGET) executable...)

$(BIN_DIR):
	@$(MKDIR) $(BIN_DIR)
	$(info Generate bin directory...)

clean:
	@$(RM) $(TARGET) $(OBJECT_FILES)
	@$(RM) -rf $(BIN_DIR)
	$(info Cleaning...)

