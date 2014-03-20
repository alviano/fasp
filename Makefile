#### BUILD modalities
# use 
#   $ make BUILD=release 
# to compile different binaries
BUILD = release
cxxflags.debug = \
 -Wall -Wextra -std=c++11
linkflags.debug = \
 
cxxflags.trace = \
 -Wall -Wextra -std=c++11 -DTRACE_ON
linkflags.trace = \
 
cxxflags.release = \
 -Wall -Wextra -std=c++11 -DNDEBUG -O3
linkflags.release = \
 
cxxflags.gprof = \
 -Wall -Wextra -std=c++11 -DNDEBUG -O3 -g -pg
linkflags.gprof = \
  -g -pg
cxxflags.stats = \
 -Wall -Wextra -std=c++11 -DNDEBUG -DSTATS_ON -O3
linkflags.stats = \
 
####

SOURCE_DIR = src
BUILD_DIR = build/$(BUILD)

BINARY = $(BUILD_DIR)/fasp
GCC = g++
CXX = $(GCC)
CXXFLAGS = $(cxxflags.$(BUILD))
LINK = $(GCC)
LINKFLAGS = -lm -lglpk $(linkflags.$(BUILD))

SRCS = $(shell find $(SOURCE_DIR) -name '*.cpp')

OBJS = $(patsubst $(SOURCE_DIR)%.cpp,$(BUILD_DIR)%.o, $(SRCS))
DEPS = $(patsubst $(SOURCE_DIR)%.cpp,$(BUILD_DIR)%.d, $(SRCS))

all: $(BINARY)

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MM -MT '$(@:.d=.o)' $< -MF $@
	
$(BINARY): $(OBJS) $(DEPS)
	$(LINK) $(LINKFLAGS) $(LIBS) $(OBJS) -o $(BINARY)

static: $(OBJS) $(DEPS)
	$(LINK) $(LINKFLAGS) $(LIBS) $(OBJS) -static -o $(BINARY)

run: $(BINARY)
	./$(BINARY)


########## Clean

clean-dep:
	rm -f $(DEPS)
clean: clean-dep
	rm -f $(OBJS)

distclean: clean
	rm -fr $(BUILD_DIR)

-include $(DEPS)
