CXX=g++
CXXFLAGS=-std=c++11 -g
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system
TARGET=3d_tetris
SCRIPTS=scripts/build_obj_files.sh

.PHONY: all clean

all: run_obj_script $(TARGET)

$(TARGET): $(TARGET).cpp
	@$(CXX) $< $(CXXFLAGS) $(LDFLAGS) -o $@

run_obj_script: obj/*.obj

obj/*.obj: $(SCRIPTS)
	@bash $(SCRIPTS)

clean:
	@rm -f $(TARGET)
	@rm -f obj/*.obj

