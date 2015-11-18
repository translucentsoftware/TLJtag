/* Arduino Wiggler Cable Simulator */
/* Intended for use with tjtag3-0-1 with JTMOD patch. */

typedef
enum __arduOp
{
    OP_RESET = 0,
    OP_SEND  = 1,
    OP_READ  = 2,
    OP_SETCABLE = 3,
    OP_RSVD  = 4
}
arduOp;                                                                                                                                                      

static int WSRST_N = 2;
static int WTMS = 3;
static int WTCK = 4;
static int WTDI = 5;
static int WRTST_N = 6;
static int WTDO = 7;


static const int TDI = 2;
static const int TCK = 3;
static const int TMS = 4;
static const int TDO = 5;

static const int Xilinx_Cable_Type = 1;
static const int Wiggler_Cable_Type = 2;

static int CABLE_TYPE = Xilinx_Cable_Type;

// Return Codes
static const int R_SEND_SUCCESS = 0x4B;
static const int R_RESET_SUCCESS = 0x42;
static const int R_INVALID = 0x7F;

void setup_cable_state(void)
{
    // Reset all line states
    PORTD = (unsigned char)(B00000000);
        
  if(CABLE_TYPE & Wiggler_Cable_Type) {
         // Pins 0-7 are part of PORTD
        // pins 0 and 1 are RX and TX, respectively
    
        pinMode(WSRST_N, OUTPUT);     // WSRST_N
        pinMode(WTMS, OUTPUT);     // WTMS
        pinMode(WTCK, OUTPUT);     // WTCK
        pinMode(WTDI, OUTPUT);     // WTDI
        pinMode(WRTST_N, OUTPUT);     // WTRST_N
    
        pinMode(WTDO, INPUT);      // WTDO
  } else {
        pinMode(TDI, OUTPUT);    
        pinMode(TCK, OUTPUT);     
        pinMode(TMS, OUTPUT);     
        pinMode(TDO, INPUT);   
  }
}

void setup(void)
{
    setup_cable_state();
    Serial.begin(9600);
    //Serial.begin(115200);

    /*
     * Before starting...
     * Say something invalid so that if the Arduino
     * gets reset in the middle of a run, the tjtag
     * program will know that something's gone wrong.
     *
     * This is invalid because our simple protocol
     * says that bits 6-0 should never be set except
     * when responding to a reset command with 0x42.
     */
    Serial.write(R_INVALID);
}

void loop(void)
{
    // Wait until byte available.
    while ( ! Serial.available() );

    int value = Serial.read();
    unsigned char byte = value;

  //  Serial.print("byte received: ");
 //   Serial.println(byte, HEX);

    unsigned char op = (byte & B01100000) >> 5 ;

 //   Serial.print("opcode interpreted: ");
 //   Serial.println(op, HEX);


    switch ( op )
    {
        case OP_RESET:
            {
                setup_cable_state();
                // Respond that we're ready to go.
                Serial.write(R_RESET_SUCCESS);

                // Clear out any other incoming data.
                Serial.flush();
            }
            break;
            
        case OP_SEND:
            {
             //   Serial.println("  OP_SEND");

                // Set output pins to requested values.
                PORTD = (unsigned char)( (byte & B00011111) << 2 );

                // If needed:  put a Serial.write() here so that
                // we can "wait" for the pins to be set.
                Serial.write(R_SEND_SUCCESS);
            }
            break;

        case OP_READ:
            {
             //   Serial.println("  OP_READ");

                unsigned char readByte = 0;
                if(CABLE_TYPE & Wiggler_Cable_Type) {
                        readByte = digitalRead(WTDO) == HIGH ?     // WTDO
                                                    B10000000 :     // send back a 1 in bit 7
                                                    B00000000;      // send back a 0 in bit 7
                } else {
                     readByte = digitalRead(TDO) == HIGH ?     // TDO
                                                    B00010000 :     // send back a 1 in bit 5
                                                    B00000000;      // send back a 0 in bit 5
                }

                Serial.write(readByte);
            }
            break;

         case OP_SETCABLE:
            {
              if(Wiggler_Cable_Type & byte) {
                CABLE_TYPE = Wiggler_Cable_Type;
              } else {
                CABLE_TYPE = Xilinx_Cable_Type;
              }

              setup_cable_state();
              
              Serial.write(byte);
            }
            break;
        default:
            {
                // BAD OPCODE
                // Send invalid data in response.
                Serial.write(R_INVALID);
            }
    };

    //Serial.println();
    //Serial.println();
}
