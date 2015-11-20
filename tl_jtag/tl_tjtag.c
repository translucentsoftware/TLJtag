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

#include "tl_tjtag.h"

#pragma mark - Compiler Directives

// Force the compiler to inline
#define TL_INLINE_STATIC static __inline__
#define TL_INLINE __inline__

#if (__has_attribute(always_inline) && __has_attribute(noinline)) \
|| (defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))))

#define TL_ALWAYS_INLINE_STATIC  __attribute__((always_inline)) TL_INLINE_STATIC
#define TL_ALWAYS_INLINE __attribute__((always_inline))

#define TL_NOINLINE __attribute__((noinline))

#else

#warning "No Ability To Force Inlining On This Compiler"

#define TL_ALWAYS_INLINE_STATIC TL_INLINE_STATIC
#define TL_ALWAYS_INLINE_EXTERN TL_INLINE_EXTERN

#define TL_NOINLINE

#endif //__GNUC__


#pragma mark - Globals

int Arduino_FD = -1;
#define iSSetup() (bool)(Arduino_FD >= 0)

static const char * TL_TJTAG__PORT = "TL_TJTAG__PORT";

#pragma mark - Arduino Interfacing Codes
// Arduino Response Codes
static const unsigned char R_SEND_SUCCESS = 0x4B;
static const unsigned char R_RESET_SUCCESS = 0x42;
// static const unsigned char R_INVALID = 0x7F; // Not needed currently

// Arduino Command Codes
static unsigned char LShiftP = 5;
static unsigned char Arduino_Reset = 0x0;
#define Arduino_Send (0x1 << LShiftP)
#define Arduino_Read (0x2 << LShiftP)
#define Arduino_Cable_select (0x3 << LShiftP)

static speed_t BAUDRATE = B9600;
static const int Arduino_Wait = 5; // The number of wait cycles for reading
// The wait we should expect the for the Arduino
// It does not seem to like to exceed 9600 baud
// This wait gives the Arduino enough time to receive the byte,
// process it and send something back. Doesn't seem to be able to be
// lower as at 30 causes errors after a short while
#define Arduino_Delay (int)((1000000 / BAUDRATE) * 40)

// The underlying cable type, on the Arduino it defaults to
// the 4 pins TDI TDO TMS TCK
static ArduinoCableType CurrCableType = Xilinx_Cable_Type;

#define isWigglerCable ((CurrCableType & Wiggler_Cable_Type))
#define isXilinxCable ((CurrCableType & Xilinx_Cable_Type))


#pragma mark - String Functions
typedef struct {
    char * string;
    size_t length;
} TLCString_t;

typedef TLCString_t * TLCString;

TL_ALWAYS_INLINE_STATIC TLCString TLCStringCreate(const char *buffer) {
    TLCString string = NULL;
    if(!buffer) return NULL;
    // Be cautious about not flitting wildly through memory
    size_t length = strnlen(buffer, 512);
    if(length) {
        string = calloc(sizeof(TLCString_t), 1);
        if(string && !(string->string = calloc(length, 1))) {
            free(string);
        } else {
            strncpy(string->string, buffer, length);
            string->length = length;
        }
    }
    
    return string;
}

TL_ALWAYS_INLINE_STATIC size_t TLCStringLength(TLCString string) { return string ? string->length : 0; }
TL_ALWAYS_INLINE_STATIC char * TLCStringChar(TLCString string) { return string ? string->string : NULL; }
TL_ALWAYS_INLINE_STATIC void TLCStringFree(TLCString string) { if(string) { if(string->string) { free(string->string); } free(string); }}

int read_until(int fd, unsigned char *out_buffer, unsigned int max_len, const unsigned char stop, unsigned int timeout) {
    unsigned char buffer[1];
    ssize_t bytesread = 0;
    
    if(fd < 0) return -1;
    
    int i = 0;
    
    do {
        bytesread = read(fd, buffer, 1);
        if (-1 == bytesread) return -1;;
        
        if(0 == bytesread) {
            usleep(Arduino_Delay);
            timeout--;
            if(timeout == 0) return -2;
            continue;
        }
        
        out_buffer[i] = buffer[0];
        
        i++;
        
    } while (buffer[0] != stop && i < max_len && timeout > 0);
    
    
    
    return 0;
}

#pragma mark - Private Functions

// Delay to give the Arduino adequate processing time
TL_ALWAYS_INLINE_STATIC void WaitForArduino(void)
{
    usleep(Arduino_Delay);
}

int setup_arduino_port(TLCString file)
{
    int fd = -1;
    struct termios tty;
    memset(&tty, 0, sizeof(struct termios));
    
    if (TLCStringLength(file) <= 0) {
        goto OUT;
    }
    
    fd = open( TLCStringChar(file), O_RDWR | O_NONBLOCK );
    
    if(fd < 0) {
        printf("Failed to open file: %s", TLCStringChar(file));
        exit(EXIT_FAILURE);
    }
    
    // Clear non-blocking flag
    //  fcntl(fd, F_SETFL, 0);
    
    cfsetispeed(&tty, BAUDRATE);
    cfsetospeed(&tty, BAUDRATE);
    
    // 8N1
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    // no flow control
    tty.c_cflag &= ~CRTSCTS;
    
    //tty &= ~HUPCL; // disable hang-up-on-close to avoid reset
    
    tty.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
    
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    tty.c_oflag &= ~OPOST; // make raw
    
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;
    
    tcsetattr( fd,TCSANOW,&tty );
    tcsetattr(fd, TCSAFLUSH, &tty);
    tcflush(fd, TCSAFLUSH);
    
OUT:
    return fd;
    
}

bool reset_arduino(int fd) {
    bool was_reset = false;
    int tries = 0;
    // Use 8 here because invariable we receive and old invalid number
    // from the reset, usually only needs two characters
    unsigned char buffer[8];
    
    if(fd >=0) {
        ssize_t bytesread = 0;
        
        while (was_reset == false) {
            *buffer = Arduino_Reset;
            if(write(fd, buffer, 1) < 0) {
                printf("Problem writing to the Arduino\n");
                break;
            }
            
            WaitForArduino();
            
            bytesread = read_until(fd, buffer, 8, R_RESET_SUCCESS, Arduino_Wait);
            
            if(bytesread < 0) {
                // During setup the Arduino seems to need extra time
                // to properly read the reset request
                if(tries++ < 5)
                    continue;
            } else
                was_reset = true;
        }
    }
    
    if(!was_reset) {
        printf("Error reseting the Arduino\n");
        exit(EXIT_FAILURE);
    }
    
    
    return was_reset;
}

bool set_arduino_cable(ArduinoCableType cable_type)
{
    bool changed = false;
    
    if(cable_type != Wiggler_Cable_Type && cable_type != Xilinx_Cable_Type)
        return changed;
    
    unsigned char buffer = Arduino_Cable_select | cable_type;
    
    write(Arduino_FD, &buffer, 1);
    
    WaitForArduino();
    
    changed = (read_until(Arduino_FD, &buffer, 1, 0, Arduino_Wait) >= 0) ? true : false;
    
    if(changed && buffer & cable_type) {
        CurrCableType = cable_type;
    }
    
    return changed;
}

// The returned string must be released
TLCString createArduinoFileString(void)
{
    TLCString file = NULL;
    const char *envior = getenv(TL_TJTAG__PORT);
    if(envior) file = TLCStringCreate(envior);
    
    return file;
}


#pragma mark - Public Functions
bool tljtag_setup(void)
{
    bool success = false;
    
    if(iSSetup()) return false;
    
    TLCString filename = createArduinoFileString();
    
    if(filename) {
        Arduino_FD = setup_arduino_port(filename);
        
        if(reset_arduino(Arduino_FD)) {
            success = true;
        }
    } else {
        printf("Valid port not set with TL_TJTAG_PORT!\n");
        exit(EXIT_FAILURE);
    }
    
    TLCStringFree(filename);
    
    return success;
}



bool tljtag_setup_cable_type(int wiggler)
{
    bool success = false;
    success = tljtag_setup();
    
    if(!success) return success;
    
    success = tljtag_set_cable((wiggler ? Wiggler_Cable_Type : Xilinx_Cable_Type));
    
    if(!success) // We have a valid setup but can not change the cable type
        tljtag_shutdown();
    
    return success;
}

ArduinoCableType currentCableType(void)
{
    return CurrCableType;
}

bool tljtag_set_cable(ArduinoCableType type)
{
    // If we are requesting the current type just return true
    if(CurrCableType == type) return true;
    
    // Check that we have a valid type
    if(CurrCableType != type && (type == Wiggler_Cable_Type || type == Xilinx_Cable_Type)) {
        return set_arduino_cable(type);
    }
    
    return false;
}

bool tljtag_send_byte(unsigned char byte)
{
    bool success = false;
    unsigned char buffer[1];
    ssize_t bytesread = 0;
    
    if(!iSSetup()) goto OUT;
    
    // From JTMod
    *buffer = byte & 0x1F;
    *buffer |= 0x20;
    
    write(Arduino_FD, buffer, 1);
    
    WaitForArduino();
    
    bytesread = read_until(Arduino_FD, buffer, 1, R_SEND_SUCCESS, Arduino_Wait);
    
    
    if(bytesread < 0) {
        printf("Invalid Response Code Reading From Arduino %c\n", *buffer);
        exit(EXIT_FAILURE);
    } else {
        success = true;
    }
    
    
OUT:
    return success;
}


char tljtag_receive_byte(void)
{
    unsigned char output = Arduino_Read;
    ssize_t bytesread = 0;
    
    
    output = Arduino_Read;
    write(Arduino_FD, &output, 1);
    
    WaitForArduino();
    
    bytesread = read_until(Arduino_FD, &output, 1, 0, Arduino_Wait);
    
    if(bytesread < 0) {
        printf("Error receiving from Arduino\n");
        exit(EXIT_FAILURE);
    }
    
    output ^= 0x80; // From JTMod
    
    return output;
}

void tljtag_shutdown(void)
{
    if(iSSetup()) {
        reset_arduino(Arduino_FD);
        close(Arduino_FD), Arduino_FD = -1;
    }
}