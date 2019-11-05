/*
 * SPI master on Teensy 3.2 to comunicate with a slave on Teensy 3.5
 * data is send in arrays of 16-bit-integers,
 * read digital values from button presses
 * and send all data via serial communication
 */
 
#include <t3spi.h>


T3SPI SPI_MASTER;

/* defines for debugging */
#define PRINT_BUTTON_PRESSES    1
#define PRINT_BLOCK_DATA        1

/* defines default delay in ms after sending either a playing field or button presses */
#define DEFAULT_DELAY           15
/* defines length of data per transmission in units of 16 bits */
#define DATA_LENGTH             1
/* defines the number of muxes used in the current build */
#define NUM_MUX                 16
/* defines the number of used buttons (if changed, buttonPorts must be adjusted) */
#define NUM_BUTTONS             3
/* defines numbers of bytes per serial transmission */
#define NUM_PSERIAL             (NUM_BLOCKS/2)+2
/* defines the number of blocks on the board */
#define NUM_BLOCKS              (NUM_MUX*8)
/* defines the number of packages required to spi transmit all blocks */
#define NUM_PACKAGES            ((NUM_BLOCKS + 2) / 3)

/* used for SPI communication */
volatile uint16_t dataIn[DATA_LENGTH] = {};
volatile uint16_t dataOut[DATA_LENGTH] = {};

// blocks with id of its kind
uint8_t blocks[NUM_BLOCKS];

// true iff button is pressed
bool buttons[NUM_BUTTONS];

// byte packages to be send via serial-connection
byte packages[NUM_PSERIAL];

/* 
 *  Those arrays hold the pins, where buttons are connected to the board and store, if a pin
 * is constantly pressed, respectivly
 */
const int buttonPorts[NUM_BUTTONS] = {A2, A3, A4};
uint8_t constantlyPressed[NUM_BUTTONS];

/*
 * Sets up buttons as well as the SPI bus for communication with Teensy 3.5
 */
void setup() {

    Serial.begin(9600);

    Serial.print("Master - Setup...");
     
    // init buttons
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        constantlyPressed[i] = 0;
        pinMode(buttonPorts[i], INPUT_PULLUP);
        attachInterrupt(buttonPorts[i], ISR_ButtonPress, FALLING);
    }
  
    // --- spi setup begin ---
    
    // begin SPI in MASTER (SCK pin, MOSI pin, MISO pin, CS pin, Active State)
    SPI_MASTER.begin_MASTER(SCK, MOSI, MISO, CS1, CS_ActiveLOW);
    
    // set the CTAR (CTARn, Frame Size, SPI Mode, Shift Mode, Clock Divider)
    SPI_MASTER.setCTAR(CTAR_0, 16, SPI_MODE0, LSB_FIRST, SPI_CLOCK_DIV2);
    
    // wait for Slave
    delay(3000);
      
    // --- spi setup end ---

    Serial.println(" done");
}


/*
 * Routine: 
 *      1. Check, wether buttons are still pressed and reset the buttons which are not pressed.
 *      2. Loop over the number of packages that are needed to get the block data from the whole
 *          controller. Then directly transmit said data as 2 bytes to PC via the serial port. 
 *      3. After running through the loop, wait a certain amount of time to allow button presses 
 *          to be transmitted to the PC.
 */
void loop() {
    resetConstantlyPressedButtons(); 

    readBlocks();

    //printMatrix();  // only for debugging
    //showBlocks();  // only for debugging

    delay(DEFAULT_DELAY);
    
    //printSPI();  // only for debugging

    calculatePackages();

    //printPackagesBIN();  // onyl for debugging

    sendPackages();
  
    // reset the packet count
    SPI_MASTER.packetCT = 0;
}

/*
 * This function gets triggered, if any button is pressed. It then generates the corresponding 
 * bitstring, which then is send to the PC via the serial connection. Wait a certain amount of
 * time to allow the PC to read and to process the button data properly.
 */
void ISR_ButtonPress()
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        if ((digitalRead(buttonPorts[i]) == LOW && !constantlyPressed[i])){
            constantlyPressed[i] = 1;
            buttons[i] = true;
        }
    }
}

/* 
 * Resets buttons, which are not pressed. 
 */
void resetConstantlyPressedButtons()
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        if (digitalRead(buttonPorts[i]) == HIGH) constantlyPressed[i] = 0;
    }
}

// read block types from double-byte packages reciaved via spi
void readBlocks()
{
    uint8_t id = 0;
    uint8_t sec = 0;
    uint8_t count = 0;
    for (uint8_t i = 0; i < NUM_PACKAGES; i++)
    {   
        count = 0;
        dataOut[0] = i;
        do
        {
            SPI_MASTER.txrx16(dataOut, dataIn, DATA_LENGTH, CTAR_0, CS1);
            sec = (*dataIn&0x8000) >> 15;
            id = (*dataIn&0x7E00) >> 9;
            count++;
            
        } while ((i != id || sec != 1) && count < 15);

        if (count >= 15)
        {
            blocks[i*3] = 0;
            blocks[i*3+1] = 0;
            blocks[i*3+2] = 0;
        }

        blocks[i*3] = (*dataIn&0x01C0) >> 6;
        blocks[i*3+1] = (*dataIn&0x0038) >> 3;
        blocks[i*3+2] = (*dataIn&0x0007) >> 0;
    }
} 

// calculate byte packages to be send via serial connection
void calculatePackages()
{
    byte calc = 0;
    
    packages[0] = 0xFF;

    for(int i=0; i<NUM_BLOCKS/2; i++)
    {
        calc = 0;
        
        calc |= (blocks[i*2] << 4);
        calc |= (blocks[i*2+1] << 0);
        
        packages[i+1] = calc;
    }

    calc = 0;
    
    calc |= 0xC0;
    
    if(buttons[0])
    {
        calc |= (0x3 << 4);
        buttons[0] = false;
    }
    if(buttons[1])
    {
        calc |= (0x3 << 2);
        buttons[1] = false;
    }
    if(buttons[2])
    {
        calc |= (0x3);
        buttons[2] = false;
    }

    packages[NUM_PSERIAL-1] = calc;
}

// send byte packages via serial connection
void sendPackages()
{
    for(int i=0; i<NUM_PSERIAL; i++)
    {
      Serial.write(packages[i]);
    }
}


/*
 *    debugging methods
 */
void printSPI() {
    for(int i = 0; i < DATA_LENGTH; i++) {
        Serial.print("dataIn[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.print(dataIn[i]);
        Serial.print("   dataOut[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(dataOut[i]);
        Serial.flush();
    }
}

void printMatrix()
{
    for(int i=0; i<NUM_BLOCKS; i++)
    {
        Serial.print("B");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(blocks[i]);
        Serial.print("  ");
        //delayMicroseconds(50);
    }
    Serial.println();
}

void printPackagesBIN()
{
    Serial.println();
    for(int i=0; i<NUM_PSERIAL; i++)
    {
        Serial.println(packages[i],BIN);
        delayMicroseconds(50);
    }
    Serial.println();
    Serial.println("------------------------------");
}

void showBlocks()
{
    for(int i=0; i<NUM_BLOCKS; i++)
    {
        if(blocks[i] != 0)
        {
            Serial.print(i);
            Serial.print("  -  ");
            Serial.println(blocks[i]);
        }
    }
}
