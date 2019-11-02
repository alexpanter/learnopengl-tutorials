# don't print "(entering|leaving) directory" messages
MAKEFLAGS += --no-print-directory

CLANG=clang++-6.0
GCC=g++
LINK_OPENGL=-lGLEW -lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor -lstdc++fs

STD=-std=c++17
FLAGS=-Werror

EXE=heightmap
MAINFILE=$(EXE).cpp

heightmap: $(MAINFILE)
	@$(CLANG) $(STD) $(FLAGS) $< -o $@ $(LINK_OPENGL) $(LINK_SOIL) $(LINK_SDL2)

test: clean $(EXE)
	./$(EXE)

rebuild: clean heightmap

run:
	@test ! -f $(EXE) && make $(EXE)
	./$(EXE)

clean:
	rm -rf *.o $(EXE)
