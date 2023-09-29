# This file contains the rules to generate virtual pipe organ enclosure drawings for laser cutting.
# Type 'make' at the command line to generate and view the dxf files
all:	test.dxf musicStand.dxf tray.dxf cutting.dxf
#	librecad    test.dxf
#	librecad bracket.dxf
#	librecad musicStand.dxf
#	librecad keySupports.dxf
#	librecad trayIII.dxf
#	librecad trayII.dxf
#	librecad trayI.dxf
	librecad cutting.dxf
tray.dxf: enclosure
	./enclosure 172 > trayIII.dxf
	./enclosure 317 > trayII.dxf # was 308
	./enclosure 462 > trayI.dxf # was 444
#                                             
test.dxf:	enclosure Makefile
	./enclosure 0 > test.dxf
bracket.dxf:	enclosure Makefile
	./enclosure 1 > bracket.dxf
musicStand.dxf: enclosure Makefile
	./enclosure 2 > musicStand.dxf
keySupports.dxf: enclosure Makefile
	./enclosure 3 > keySupports.dxf
cutting.dxf:	 enclosure Makefile
	./enclosure 4 > cutting.dxf
enclosure:	enclosure.c Makefile
	gcc -Wall -Werror enclosure.c -o enclosure -lm
clean:
	rm enclosure bracket.dxf keySupports.dxf test.dxf tray*dxf \#*dxf musicStand.dxf cutting.dxf
