#
#
#	MAKE file for JMODEM, the 'C' version.
#	Created 01-FEB-1990		Richard B. Johnson
#                                       405 Broughton Drive
#                                       Beverly, Massachusetts 01915
#                                       BBS (508) 922-3166
#	Execute MAKE JMODEM
#
#       With Microsoft V5.1 compiler, one MUST use the medium or large
#       model for compiling because a bug in _calloc() causes buffers to
#       overlap, resulting is data corruption if you use the small model!
#

jmodem_a.obj:		jmodem.h jmodem jmodem_a.c
			CL /AS /c /DNOENV /W3 /Fljmodem_a.cod jmodem_a.c

jmodem_b.obj:		jmodem.h jmodem jmodem_b.c
			CL /AS  /c /W3 /Fljmodem_b.cod jmodem_b.c

jmodem_c.obj:		jmodem.h jmodem jmodem_c.c
			CL /AS  /c /W3 /Fljmodem_c.cod jmodem_c.c

jmodem_d.obj:		jmodem.h jmodem jmodem_d.c
			CL /AS  /c /W3 /Fljmodem_d.cod jmodem_d.c

jmodem_e.obj:		jmodem.h uart.h jmodem jmodem_e.c
			CL /AS  /c /W3 /Fljmodem_e.cod jmodem_e.c

jmodem_f.obj:		jmodem.h jmodem screen.h jmodem_f.c
			CL /AS  /c /W3 /Fljmodem_f.cod jmodem_f.c

jmodem.exe:		jmodem.h jmodem \
                        jmodem_a.obj jmodem_b.obj jmodem_c.obj\
                        jmodem_d.obj jmodem_e.obj jmodem_f.obj
			LINK/NOE/MAP jmodem_a+ \
				jmodem_b+ \
				jmodem_c+ \
				jmodem_d+ \
				jmodem_e+ \
				jmodem_f, \
				jmodem.exe;

