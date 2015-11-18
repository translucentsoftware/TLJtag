//  Written by Josh Gibson - Translucent Software
//  josh@translucentsw.com
// **************************************************************************
//
//  This program is copyright (C) 2015 Translucent Software
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of version 2 the GNU General Public License as published
//  by the Free Software Foundation.
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//  To view a copy of the license go to:
//  http://www.fsf.org/copyleft/gpl.html
//  To receive a copy of the GNU General Public License write the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// **************************************************************************

#include <stdio.h>
#include <unistd.h>
#include "tl_tjtag.h"

int main() {
    
    unsigned char byte = 0;
    tljtag_setup();
    
    printf("Test Sending Bytes to the Arduino\n");
    for (byte = 0; byte < 32; byte++) {
        tljtag_send_byte(byte);
        usleep(500 * 1000);
    }
    
    printf("Test Receiving Bytes from the Arduino\n");
    for (byte = 0; byte < 32; byte++) {
        unsigned char received = tljtag_receive_byte();
        //  printf("Received %c\n", received);
        usleep(500 * 1000);
    }
    
    tljtag_shutdown();
    
}
