RM			= rm -f
MKDIR		= mkdir -p

SOURCES		= $(shell find src -type f -iname "*.cpp")
OBJECTS		= $(patsubst src/%.cpp,out/%.o,$(SOURCES))
TARGET		= ./out/sxf

CXX			= g++
CXXFLAGS	= -std=c++17 -Wall -Wextra -Wpedantic
CXXFLAGS	= `pkg-config libavcodec --cflags`
CXXFLAGS	+= `pkg-config libavformat --cflags`
CXXFLAGS	+= `pkg-config libavutil --cflags`
CXXFLAGS	+= `pkg-config libswscale --cflags`

LD			= $(CXX)
LDFLAGS		= `pkg-config libavcodec --libs`
LDFLAGS		+= `pkg-config libavformat --libs`
LDFLAGS		+= `pkg-config libavutil --libs`
LDFLAGS		+= `pkg-config libswscale --libs`

CPPCHECK	= cppcheck
CLANGXX		= clang++
VALGRIND	= valgrind

ifeq ($(origin DEBUG), environment)
	CXXFLAGS += -Og -g -DSXF_DEBUG
else
	CXXFLAGS += -O2
endif

all: sxf

clean:
	$(RM) $(TARGET) $(OBJECTS)

check:
	@$(CPPCHECK) --language=c++ --std=c++17 ./src/main.cpp
	@$(CLANGXX) --analyze -Xclang -analyzer-output=html $(CXXFLAGS) \
		-o ./out/analysis \
		./src/main.cpp \
		$(LDFLAGS)

sxf: $(OBJECTS)
	$(LD) -o $(TARGET) $^ $(LDFLAGS)

out/%.o: src/%.cpp | create_dirs
	@$(CXX) $(CXXFLAGS) -c $^ -o $@ $(LDFLAGS)

create_dirs:
	@$(MKDIR) $(sort $(dir $(OBJECTS)))

test:
	$(VALGRIND) \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		$(TARGET)
