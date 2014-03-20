BINARY=fasp
GCC=g++
CXX=$(GCC)
CXXFLAGS=-Wall -std=c++11 -g3 -O0
LINK=$(GCC)
LINKFLAGS=-lm -lglpk

SRCS = \
 Atom.cpp \
 Component.cpp \
 DependencyGraph.cpp \
 GodelTnorm.cpp \
 LukasiewiczTnorm.cpp \
 Parser.cpp \
 ProductTnorm.cpp \
 Program.cpp \
 Rule.cpp \
 Tnorm.cpp \
 main.cpp \
 options.cpp \
 

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

all: $(BINARY) 
	
%.d: %.cpp
	$(CXX) $(CXXFLAGS) -M $< >$@
	
$(BINARY): $(OBJS)
	$(LINK) $(LINKFLAGS) $(LIBS) $(OBJS) -o $(BINARY)

static: $(OBJS)
	$(LINK) $(LINKFLAGS) $(LIBS) $(OBJS) -static -o $(BINARY)

########## Clean

clean:
	rm -f *.o a.out
	rm -f gmon.out

distclean: clean
	rm -f $(BINARY)
	rm -f *.d

-include $(DEPS)

