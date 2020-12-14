## Astrology Research Framework

## Flags, Includes
#-- one letter vars can be refered without parens
C=gcc -std=gnu11 -g -O3 -Wall

F=$(shell pkg-config --cflags gtk+-3.0)
I=$(shell pkg-config --cflags --libs gtk+-3.0) -lm
T=-g -DTEST -lmcheck

## Swiss Ephemeris
swe_TAR=swe_unix_src_2.00.00.tar.gz
swe_URL=https://www.astro.com/ftp/swisseph/$(swe_TAR)

## Objects and things
OBJs=convert.o astro.o stringify.o serialize.o draw.o
Hs=arf.h arfc.h
SE=swe/libswe.a

## Scaffolding
BUILD_NUMBER: ar arfant
	echo $$(($$(cat BUILD_NUMBER) + 1)) > BUILD_NUMBER

clean:
	rm -rf *.o *.test ar arfant

cleanall: clean
	make -C swe clean

.PHONY: clean cleanall check libswe

## EXECUTABLES
# NOTE TO SELF: linking order is important, most basic files go LAST
arfant: arfant.o $(SE) $(OBJs)
	@echo cc -o $@
	@ $C -o $@ arfant.o $(OBJs) $(SE) $I

ar: ar.o $(SE) $(OBJs)
	@ echo cc -o $@
	@ $C -o $@ ar.o $(OBJs) $(SE) $I

## OBJECTS
ar.o arfant.o: %.o: %.c $(Hs)
	@echo cc $<
	@ $C $F -c $<

$(OBJs): %.o: %.c %.test $(Hs)
	@./$(basename $< ).test
	@echo cc $<
	@ $C $F -c $<

# Swiss Ephemeris
$(swe_TAR):
	wget $(swe_URL)

swe: $(swe_TAR)
	tar xaf $(swe_TAR) src/
	mv src swe
	make -C swe clean

libswe: $(SE) swe
$(SE):
	make -C swe libswe.a

## Tests
check: convert.test astro.test serialize.test stringify.test draw.test
	-@./convert.test
	-@./stringify.test
	-@./astro.test
	-@./serialize.test
	-@./draw.test

convert.test: convert.c $(Hs)
	@ echo cc -o $@
	@ $C $F $T -o $@ $< $(SE) $I

stringify.test: stringify.c convert.o $(Hs)
	@ echo cc -o $@
	@ $C $F $T -o $@ $< convert.o $(SE) $I

astro.test: astro.c convert.o stringify.o $(Hs)
	@ echo cc -o $@
	@ $C $F $T -o $@ $< convert.o stringify.o $(SE) $I

serialize.test: serialize.c stringify.o convert.o $(Hs)
	@ echo cc -o $@
	@ $C $F $T -o $@ $< stringify.o convert.o $(SE) $I

draw.test: draw.c stringify.o convert.o $(Hs)
	@ echo cc -o $@
	@ $C $F $T -o $@ $< stringify.o convert.o $(SE) $I
