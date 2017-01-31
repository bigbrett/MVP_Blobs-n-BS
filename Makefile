# Implemented from http://hiltmon.com/blog/2013/07/03/a-simple-c-plus-plus-project-structure/
 
CC := g++
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := src
BUILDDIR := build
TARGET := bin/out
 
SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name "*.$(SRCEXT)")
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -g -Wall
LIB := `pkg-config --libs opencv`
INC := -I include -I/usr/local/Cellar/opencv3/3.2.0/include/opencv -I/usr/local/Cellar/opencv3/3.2.0/include

$(TARGET): $(OBJECTS)
	@echo "Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)
	@echo "Completed"

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo "Building..."
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo "Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)

# Tests
tester:
	$(CC) $(CFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

.PHONY: clean
