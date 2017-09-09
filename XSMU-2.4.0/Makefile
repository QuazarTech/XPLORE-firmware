top_builddir = .
include ${top_builddir}/makeinclude

all:
	@echo "Please specify target: ihx, doc, burn, or clean."

ihx:
	${MAKE} -C code all

doc:
	doxygen doxy.conf

burn:
	$(AVRDUDE) -p m644 -P /dev/ttyUSB0 \
		-c stk500v2 -b 19200 -U flash:w:code/app/src/app.ihx

clean:
	${MAKE} -C code clean
	rm -f *~

.PHONY: all bin doc burn clean

