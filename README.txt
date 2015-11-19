Translucent Software (Josh Gibson) patches to JTMOD and tjtag
------------------------------------------------

These patches were done to create a usable Arduino jtag adapter for OS X.
JTMOD was designed for linux and Windows but OS X required some significant changes
in order for it to properly communicate with the Arduino and not have any communication errors.

It is based heavily off the source code of JTMOD (https://github.com/zoobab/tjtag-arduino).
The serial communication portion was completely rewritten but the Arduino sketch only 
had slight modifications for clarity and cable selection.

The JTMOD source tree this was based off has been included.

------------------------------------------------

- OS X only
This patch only works on OS X but still contains the modifications in tjtag from JTMOD.
tjtag has also been patched in places to make it compatibile with OS X.

------------------------------------------------

arduiggler is JTMODs Arduino sketch and has been patched to provide the ability
to use different cable types. Wiggler is the preferred choice.
------------------------------------------------

The cable hookups are as follows:

	JTAG	   - Wiggler -	     Arduino		JTAG	   - Xilinx -	    Arduino
	1  - nTRST ———————————-——————> 6 		1  - nTRST ——————————————————> X
	3  - TDI ————————————————————> 5		3  - TDI ————————————————————> 2
	5  - TDO ————————————————————> 7		5  - TDO ————————————————————> 5
	7  - TMS ————————————————————> 3		7  - TMS ————————————————————> 4
	9  - TCK ————————————————————> 4		9  - TCK ————————————————————> 3
	11 - nSRST ——————————————————> 2		11 - nSRST ——————————————————> X

Note: 1 is ALWAYS the square pad and is the top-left orientation. Don’t forget the 1K Ohm resisters!

The Arduino pins need to be connected with 1K Ohm resisters on 5v models as JTAG pins are 3v.
The ground pins 4 - 10 (Right side) need to be hooked up to the Arduino ground. 

------------------------------------------------

tl_tjtag is the OS X patched version of tjtag and accepts all of the commands same commands as tjtag

The environment variable TL_TJTAG__PORT needs to be set to the Arduino port, it will be something
like /dev/tty.usbserial*

Example:
	~> set TL_TJTAG_PORT=/dev/tty.usbserial-A603UE5O
	~> ./tl_tjtag -probeonly /wiggler

------------------------------------------------

tl_tjtag_tests is a test application that will test turing on the pins. So you can connect LEDs to each
		pin and validate that it is working.


------------------------------------------------

As with the previous version this patch also runs very slow! It most likely is the latency experienced with
sending to, waiting for and finally receiving back from the Arduino.

