//
//  tl_tjtag.h
//  tl_tjtag
//
//  Created by Josh Gibson on 11/15/15.
//  Copyright © 2015 Translucent Software. All rights reserved.
//

#ifndef tl_tjtag_h
#define tl_tjtag_h

#ifdef __APPLE__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>

typedef enum {
    Xilinx_Cable_Type = 1,
    Wiggler_Cable_Type = 2
} ArduinoCableType;

/*
 * All of these function will exit if they detect errors coming 
 * from the Arduino connection.
 */

// Sets up the interface to talk to the Arduino
// Defaults to using Xilinx cable
bool tljtag_setup(void);

// Sets the cable type, wiggler seems to be a neccesity
// as the Xilinx style doesn't seem to work
bool tljtag_setup_cable_type(int wiggler);

// The currently selected cable type
ArduinoCableType currentCableType(void);

// Set the cable type used for the jtag connection
// Returns true if is was changed
bool tljtag_set_cable(ArduinoCableType type);

// Send a byte through the Arduino to the jtag
// Returns true on success and false otherwise
bool tljtag_send_byte(unsigned char byte);

// Receive a byte from the Arduino
// This will timeout after a short time if no
// traffic is being sent from the Arduino
char tljtag_receive_byte(void);

// Shutdown the connection to the Arduino
void tljtag_shutdown(void);

#else
#error "This implementation has only been tested on Macintosh!!"
#endif /* __APPLE__ */
#endif /* tl_tjtag_h */
