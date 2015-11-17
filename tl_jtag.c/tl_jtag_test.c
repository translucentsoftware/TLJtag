//
//  tl_jtag_test.c
//  tl_jtag
//
//  Created by Josh Gibson on 11/15/15.
//  Copyright © 2015 Translucent Software. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include "tl_jtag.h"

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
