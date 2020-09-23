//------------------------------------------------------------------------------
// Include the IRremote library header
//
#include <IRremote.h>

static const char *key2sym(uint8_t key) {
  switch (key) {
      // sed -e 's,^,case 0x,' -e 's/=/: return "/' -e 's/$/";/' < keys.txt > keyswitch.h
#include "keyswitch.h"
  }
}

// sed -e 's,\(.*\)=\(.*\),#define IR_KC_\2 0x\1,' < keys.txt > keys.h
#include "keys.h"

//------------------------------------------------------------------------------
// Tell IRremote which Arduino pin is connected to the IR Receiver (TSOP4838)
//
#if defined(ESP32)
int IR_RECEIVE_PIN = 15;
#else
int IR_RECEIVE_PIN = 2;
#endif
IRrecv irrecv(IR_RECEIVE_PIN);

//+=============================================================================
// Configure the Arduino
//
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);   // Status message will be sent to PC at 9600 baud
#if defined(__AVR_ATmega32U4__)
  while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__));

  irrecv.enableIRIn();  // Start the receiver

  Serial.print(F("Ready to receive IR signals at pin "));
  Serial.println(IR_RECEIVE_PIN);
}

//+=============================================================================
// Display IR code
//
void ircode(decode_results *results) {
  // Panasonic has an Address
  if (results->decode_type == PANASONIC) {
    Serial.print(results->address, HEX);
    Serial.print(":");
  }
  // Print Code
  Serial.print(results->value, HEX);
}

//+=============================================================================
// Display encoding type
//
void encoding(decode_results *results) {
  switch (results->decode_type) {
    default:
    case UNKNOWN:
      Serial.print("UNKNOWN");
      break;
    case NEC:
      Serial.print("NEC");
      break;
    case SONY:
      Serial.print("SONY");
      break;
    case RC5:
      Serial.print("RC5");
      break;
    case RC6:
      Serial.print("RC6");
      break;
    case DISH:
      Serial.print("DISH");
      break;
    case SHARP:
      Serial.print("SHARP");
      break;
    case SHARP_ALT:
      Serial.print("SHARP_ALT");
      break;
    case JVC:
      Serial.print("JVC");
      break;
    case SANYO:
      Serial.print("SANYO");
      break;
    case MITSUBISHI:
      Serial.print("MITSUBISHI");
      break;
    case SAMSUNG:
      Serial.print("SAMSUNG");
      break;
    case LG:
      Serial.print("LG");
      break;
    case WHYNTER:
      Serial.print("WHYNTER");
      break;
    case AIWA_RC_T501:
      Serial.print("AIWA_RC_T501");
      break;
    case PANASONIC:
      Serial.print("PANASONIC");
      break;
    case DENON:
      Serial.print("Denon");
      break;
    case BOSEWAVE:
      Serial.print("BOSEWAVE");
      break;
    case 250:
      Serial.print("KEYBOARD");
      break;
    case 251:
      Serial.print("JOYSTICK");
      break;
  }
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpInfo(decode_results *results) {
  // Check if the buffer overflowed
  if (results->overflow) {
    Serial.println("IR code too long. Edit IRremoteInt.h and increase RAW_BUFFER_LENGTH");
    return;
  }

  // Show Encoding standard
  Serial.print("Encoding  : ");
  encoding(results);
  Serial.println("");

  // Show Code & length
  Serial.print("Code      : 0x");
  ircode(results);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpRaw(decode_results *results) {
  // Print Raw data
  Serial.print("Timing[");
  Serial.print(results->rawlen - 1, DEC);
  Serial.println("]: ");

  for (unsigned int i = 1; i < results->rawlen; i++) {
    unsigned long x = results->rawbuf[i] * MICROS_PER_TICK;
    if (!(i & 1)) {  // even
      Serial.print("-");
      if (x < 1000)
        Serial.print(" ");
      if (x < 100)
        Serial.print(" ");
      Serial.print(x, DEC);
    } else {  // odd
      Serial.print("     ");
      Serial.print("+");
      if (x < 1000)
        Serial.print(" ");
      if (x < 100)
        Serial.print(" ");
      Serial.print(x, DEC);
      if (i < results->rawlen - 1)
        Serial.print(", "); //',' not needed for last one
    }
    if (!(i % 8))
      Serial.println("");
  }
  Serial.println("");                    // Newline
}

//+=============================================================================
// Dump out the decode_results structure.
//
void dumpCode(decode_results *results) {
  // Start declaration
  Serial.print("unsigned int  ");          // variable type
  Serial.print("rawData[");                // array name
  Serial.print(results->rawlen - 1, DEC);  // array size
  Serial.print("] = {");                   // Start declaration

  // Dump data
  for (unsigned int i = 1; i < results->rawlen; i++) {
    Serial.print(results->rawbuf[i] * MICROS_PER_TICK, DEC);
    if (i < results->rawlen - 1)
      Serial.print(","); // ',' not needed on last one
    if (!(i & 1))
      Serial.print(" ");
  }

  // End declaration
  Serial.print("};");  //

  // Comment
  Serial.print("  // ");
  encoding(results);
  Serial.print(" ");
  ircode(results);

  // Newline
  Serial.println("");

  // Now dump "known" codes
  if (results->decode_type != UNKNOWN) {

    // Some protocols have an address
    if (results->decode_type == PANASONIC) {
      Serial.print("unsigned int  addr = 0x");
      Serial.print(results->address, HEX);
      Serial.println(";");
    }

    // All protocols have data
    Serial.print("unsigned int  data = 0x");
    Serial.print(results->value, HEX);
    Serial.println(";");
  }


  if (results->decode_type == 250) {
    Serial.print("Keyboard: ");
    dumpKeyb(results->value);
    Serial.println(";");
  }
  if (results->decode_type == 251) {
    Serial.print("Joystick: ");
    dumpJoy(results->value);
    Serial.println(";");
  }

}

// Note the byte order
struct keyb_event {
  uint8_t header;
  union {
    struct {
      uint8_t modifier;
      uint8_t code;
    } key_ev;
    struct {
      uint8_t x: 6;
      uint8_t y: 6;
      uint8_t pad: 4;
    } joy_ev;
  } u;
  uint8_t chksum: 4;
  uint8_t pad: 4;
};


static void dumpKeyb(uint32_t data) {
  int chksum = 2;
  for (uint32_t mask = 0x1, i = 0; i < 24; i++, mask <<= 1) {
    if (data & mask) chksum++;
  }

  struct keyb_event p;
  *((uint32_t*)(&p)) = data;
  Serial.print("chk:");
  Serial.print(p.chksum, HEX);
  Serial.print(chksum == p.chksum ? " OK" : " ERROR");
  Serial.print(",header:");
  Serial.print(p.header, HEX);
  if (p.header & 0x02) {
    Serial.print("+KEY");
    if (p.header & 0x80) {
      Serial.print("+RELEASE");
    }
    if (p.header & 0x40) {
      Serial.print("+REPEAT");
    }
    Serial.print(",mods:");
    Serial.print(p.u.key_ev.modifier, HEX);
    if (p.u.key_ev.modifier & 0x01) {
      Serial.print("+SHIFT");
    }
    if (p.u.key_ev.modifier & 0x02) {
      Serial.print("+ALT");
    }
    if (p.u.key_ev.modifier & 0x04) {
      Serial.print("+CTRL");
    }
    if (p.u.key_ev.modifier & 0x08) {
      Serial.print("+WIN");
    }
    Serial.print(",code:");
    Serial.print(p.u.key_ev.code, HEX);
    Serial.print(" ");
    Serial.print(key2sym(p.u.key_ev.code));
  } else {

    if (p.header & 0x20) {
      Serial.print("+BUTTON1");
    }
    if (p.header & 0x40) {
      Serial.print("+BUTTON2");
    }
    Serial.print(" x:");
    Serial.print(p.u.joy_ev.x, HEX);
    Serial.print(",y:");
    Serial.print(p.u.joy_ev.y, HEX);
  }
}

static void dumpJoy(uint32_t data) {
  Serial.print(" x:");
  Serial.print((data >> 8) & 0x7f, HEX);
  Serial.print(",y:");
  Serial.print(data & 0x7f, HEX);
  if (data & 0x80) {
    Serial.print(" +BUTTON1");
  }
  if (data & 0x8000) {
    Serial.print(" +BUTTON2");
  }
}


//+=============================================================================
// Dump out the raw data as Pronto Hex.
//
void dumpPronto(decode_results *results) {
  Serial.print("Pronto Hex: ");
  irrecv.dumpPronto(Serial, results);
  Serial.println();
}

#define KEYB_HZ            38
#define KEYB_BITS          28
#define KEYB_SUM_BITS    2
#define KEYB_HDR_MARK    1000
#define KEYB_HDR_SPACE   500
#define KEYB_BIT_MARK     500
#define KEYB_00_SPACE  450
#define KEYB_01_SPACE  650
#define KEYB_10_SPACE  900
#define KEYB_11_SPACE  1150

bool decodeKeyb(decode_results *results) {
  uint32_t data = 0;
  unsigned int offset = 1;
  DBG_PRINTLN("Attempting keyboard decode");

  // Check SIZE
  if (irparams.rawlen < KEYB_BITS / 2 + 3) {
    return false;
  }

  // Check HDR Mark/Space
  if (!MATCH_MARK(results->rawbuf[offset], KEYB_HDR_MARK)) {
    return false;
  }
  offset++;

  if (!MATCH_SPACE(results->rawbuf[offset], KEYB_HDR_SPACE)) {
    return false;
  }
  offset++;

  for (int i = 0; i < KEYB_BITS; i += 2) {
    int us = results->rawbuf[offset] * MICROS_PER_TICK;
    if (!MATCH_MARK(results->rawbuf[offset], KEYB_BIT_MARK)) {
      DBG_PRINT("ERROR ");
      DBG_PRINT(offset);
      DBG_PRINT(",");
      DBG_PRINTLN(us);
      return false;
    }
    offset++;
    us = results->rawbuf[offset] * MICROS_PER_TICK;
    if (us >= KEYB_00_SPACE - 100 && us <= KEYB_00_SPACE + 100) {
      data = (data >> 2);
      //data = (data << 2);
    } else if (us >= KEYB_01_SPACE - 100 && us <= KEYB_01_SPACE + 100) {
      data = (data >> 2) | 0x40000000;
      //  data = (data << 2) | 1;
    } else if (us >= KEYB_10_SPACE - 100 && us <= KEYB_10_SPACE + 100) {
      data = (data >> 2) | 0x80000000;
      // data = (data << 2) | 2;
    } else if (us >= KEYB_11_SPACE - 100 && us <= KEYB_11_SPACE + 100) {
      data = (data >> 2) | 0xC0000000;
      //data = (data << 2) | 3;
    } else {
      DBG_PRINT("ERROR ");
      DBG_PRINT(offset);
      DBG_PRINT(",");
      DBG_PRINTLN(us);
      return false;
    }
    offset++;
  }

  results->bits = (offset - 1) / 2;
  results->bits = KEYB_BITS;
  data >>= 4;

  int chksum = 2;
  for (uint32_t mask = 0x1, i = 0; i < 24; i++, mask <<= 1) {
    if (data & mask) chksum++;
  }
  if (chksum != data >> 24) {
    DBG_PRINTLN("CHECKSUM ERROR");
    return false;
  }

  results->value = data;
  results->decode_type = 250;
  return true;
}


#define JOY_BITS          16  // The number of bits in the command
#define JOY_HDR_MARK     1200  // The length of the Header:Mark
#define JOY_T1          600  // Manchester 600us - 1200us

bool decodeJoy(decode_results *results) {
  unsigned long data = 0;  // Somewhere to build our code
  int offset = 1;  // Skip the Gap reading
  DBG_PRINTLN("Attempting joystick decode");

  if (irparams.rawlen < JOY_BITS + 1) {
    return false;
  }

  // Check initial Mark match
  if (!MATCH_MARK(results->rawbuf[offset], JOY_HDR_MARK)) {
    return false;
  }
  offset++;

  // Read the bits in
  bool skip = true;
  for (int i = 0; i < JOY_BITS; ) {
    int us = results->rawbuf[offset] * MICROS_PER_TICK;
    bool mark = (offset & 1);
    offset++;
    if (offset > irparams.rawlen) {
      DBG_PRINT("ERROR ");
      DBG_PRINT(offset);
      DBG_PRINT(",");
      DBG_PRINT(us);
      DBG_PRINT(",");
      DBG_PRINTLN(irparams.rawlen);
      return false;
    }
    if (us >= JOY_T1 - 100 && us < JOY_T1 + 100) {
      if (skip) {
        skip = false;
      } else {
        data = (data << 1);
        if (mark) {
          data = data | 1;
        }
        i++;
        skip = true;
      }

    } else if (us >= JOY_T1 * 2 - 100 && us < JOY_T1 * 2 + 100) {
      data = (data << 1);
      if (mark) {
        data = data | 1;
      }
      i++;
      skip = true;

    } else {
      DBG_PRINT("ERROR ");
      DBG_PRINT(offset);
      DBG_PRINT(",");
      DBG_PRINTLN(us);
      return false;
    }
  }

  // Success
  results->bits = JOY_BITS;
  results->value = data;
  results->decode_type = 251;
  return true;
}


//+=============================================================================
// The repeating section of the code
//
void loop() {
  decode_results results;        // Somewhere to store the results

  if (irrecv.decode(&results)) {  // Grab an IR code
    decodeKeyb(&results);
    decodeJoy(&results);
    dumpInfo(&results);           // Output the results
    dumpRaw(&results);            // Output the results in RAW format
    dumpPronto(&results);
    dumpCode(&results);           // Output the results as source code
    Serial.println("");           // Blank line between entries
    //decode_results(&results);

    irrecv.resume();              // Prepare for the next value
  }
}
