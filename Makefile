CC = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = lz_codec
SRC_DIR = src
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

all: CXXFLAGS += -Ofast
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean debug
