FLAGS=-MP -MD -Wall -fvisibility=hidden -I./
CXXCOMMONFLAGS=-fno-rtti -std=c++11\
  -fvisibility-inlines-hidden -Wno-invalid-offsetof\
  -msse -msse2

DEBUGFLAGS=-O0 -DMEMORY_DEBUGGER -g
OPTDEBUGFLAGS=-O2 -DMEMORY_DEBUGGER -g #-fsanitize=address
RELEASEFLAGS=-fomit-frame-pointer -O3 -DNDEBUG -DRELEASE
CXXFLAGS=$(CXXCOMMONFLAGS) $(FLAGS) $(RELEASEFLAGS) #-fsanitize=address

OBJS=boodew.o tests.o

all: tests

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(OBJS:.o=.d)

tests: $(OBJS)
	$(CXX) $(CXXFLAGS) -o tests $(OBJS) $(LIBS)

clean:
	rm -rf tests *.o *.d

