top_builddir = ../../..
include $(top_builddir)/makeinclude

SRC = \
	Applet.cxx \
	LCD.cxx \
	QP4.cxx \
	PCA9557.cxx \
	AD5542.cxx \
	AD7734.cxx \
	FS.cxx \
	SysTick.cxx

DEP = $(SRC:%.cxx=%.dep)
ASM = $(SRC:%.cxx=%.s)
OBJ = $(SRC:%.cxx=%.o)

ifeq ($(MAKECMDGOALS), obj)
	include $(DEP)
endif

dep: $(DEP)

obj: $(OBJ)

lib: libsys.a

libsys.a: $(OBJ)
	$(AR) $@ $(OBJ)

clean:
	$(RM) $(DEP) $(OBJ) $(ASM) libsys.a *~

.PHONY: dep obj lib clean
