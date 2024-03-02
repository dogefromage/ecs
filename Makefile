# https://wiki.libsdl.org/SDL2/README/emscripten#building-your-app
# https://gist.github.com/wenchy/64db1636845a3da0c4c7

RAYLIB_PATH ?= /home/seb/raylib

CC := emcc
CFLAGS := -Wall -std=c++14 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os -DPLATFORM_WEB
LFLAGS := -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' -s EXPORTED_RUNTIME_METHODS=ccall --preload-file assets
INCS := -I $(RAYLIB_PATH)/src -I $(RAYLIB_PATH)/src/external -I $(RAYLIB_PATH)/examples/shaders
LIBS := -L $(RAYLIB_PATH)/src $(RAYLIB_PATH)/src/web/libraylib.a

TARGET := build/index.html

# $(wildcard *.cpp /xxx/xxx/*.cpp): get all .cpp files from the current directory and dir "/xxx/xxx/"
SRCS := $(wildcard src/*.cpp)
# $(patsubst %.cpp,%.o,$(SRCS)): substitute all ".cpp" file name strings to ".o" file name strings
OBJS := $(patsubst src/%.cpp,obj/%.o,$(SRCS))

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) $(LIBS) -o $@ $^
obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INCS) -c -o $@ $< 
clean:
	rm -rf build/** obj/*.o
	
.PHONY: all clean


# emcc 
# 	-o snake.html main.cpp 
# 	-Wall -std=c++14 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result 
# 	-I. -I C:/raylib/raylib/src -I C:/raylib/raylib/src/external 
# 	-L. -L C:/raylib/raylib/src 
# 	-s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1 
# 	--preload-file Graphics --preload-file Sounds 
# 	--shell-file C:/raylib/raylib/src/shell.html 
# 	C:/raylib/raylib/src/web/libraylib.a 
# 	-DPLATFORM_WEB 
# 	-s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]'-s EXPORTED_RUNTIME_METHODS=ccall



# CFLAGS = -g -Wall 

# default:
# 	emcc $(CFLAGS) -I/home/seb/raylib/src -L/home/seb/raylib/src -o web/index.html src/*.cpp 

# clean:
# 	echo "rm some files"