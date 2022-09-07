CXX=g++

SRCDIR=.
BUILDDIR=build

STATIC=main.cpp disp.cpp prog.cpp glyph.cpp util.cpp
OBJ_STATIC=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(STATIC))

HDR=col.h layout.h

LDFLAGS+=-lGLEW -lGL
LDFLAGS+=-lSDL2
LDFLAGS+=-lpng

.PHONY: all
all: mk_build mk_o omni_spray_glyph

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.h
	$(CXX) -c $< -o $@ $(LDFLAGS)

$(BUILDDIR)/main.o: $(SRCDIR)/main.cpp
	$(CXX) -c $< -o $@ $(LDFLAGS)

omni_spray_glyph: $(OBJ_STATIC) $(HDR)
	$(CXX) $(OBJ_STATIC) $(LDFLAGS) -o $@

.PHONY: mk_build
mk_build:
	mkdir -p $(BUILDDIR)

.PHONY: mk_o
mk_o:
	mkdir -p o

.PHONY: clean
clean:
	rm $(BUILDDIR)/*.o omni_spray_glyph
