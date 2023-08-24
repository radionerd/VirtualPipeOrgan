# This file contains the rules to generate virtual pipe organ enclosure drawings for laser cutting.
# Type 'make' at the command line to generate and view the dxf files
all:	test.dxf musicStand.dxf tray.dxf
	librecad    test.dxf
#	librecad bracket.dxf
#	librecad musicStand.dxf
#	librecad keySupports.dxf
#	librecad tray172.dxf
#	librecad tray308.dxf
#	librecad tray444.dxf
#	librecad cutting.dxf
tray.dxf: enclosure
	./enclosure 172 > tray172.dxf
	./enclosure 308 > tray308.dxf
	./enclosure 444 > tray444.dxf
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
