top_builddir = ../../..
include $(top_builddir)/makeinclude

SRC = Filter.cxx

DEP = $(SRC:%.cxx=%.dep)
ASM = $(SRC:%.cxx=%.s)
OBJ = $(SRC:%.cxx=%.o)

ifeq ($(MAKECMDGOALS), obj)
	include $(DEP)
endif

dep: $(DEP)

obj: $(OBJ)

lib: libutils.a

libutils.a: $(OBJ)
	$(AR) $@ $(OBJ)

clean:
	$(RM) $(DEP) $(OBJ) $(ASM) libutils.a *~

.PHONY: dep obj lib clean
