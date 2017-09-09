top_builddir = ../../..
include $(top_builddir)/makeinclude

SRC = \
	Application.cxx \
	Calibration.cxx \
	CM.cxx \
	CS.cxx \
	Comm.cxx \
	Storage.cxx \
	VM.cxx \
	VM2.cxx \
	VS.cxx \
	LEDDisplay.cxx \
	RM.cxx \
	SystemConfig.cxx

DEP = $(SRC:%.cxx=%.dep)
ASM = $(SRC:%.cxx=%.s)
OBJ = $(SRC:%.cxx=%.o)

ifeq ($(MAKECMDGOALS), obj)
	include $(DEP)
endif

dep: $(DEP)

obj: $(OBJ)

ihx: app.ihx

app.elf: $(OBJ) $(LIBDEPS)
	$(CC) $(LFLAGS) $(OBJ) $(LIBS) -o $@

app.ihx: app.elf
	$(OBJCOPY) -O ihex $< $@
	$(OBJDUMP) -D -m avr app.ihx > app.dis

clean:
	$(RM) $(DEP) $(OBJ) $(ASM) app.elf app.map app.dis *~

.PHONY: dep obj clean ihx
