/*
 * SPI slave on Teensy 3.5 to read analag values from 16 8-bit multiplexer
 * and comunicate with a master on Teensy 3.2
 * The data is send in arrays of 16-bit-integers.
 */
 

#include <t3spi.h>

T3SPI SPI_SLAVE;

#define DEFAULT_DELAY 15

// length of data arrays
#define DATA_LENGTH     1
#define NUM_MUX         16
#define NUM_BLOCKS      (NUM_MUX*8)
#define NUM_PACKAGES    ((NUM_BLOCKS + 2) / 3)

/* used for SPI communication */
volatile uint16_t dataIn[DATA_LENGTH] = {};
volatile uint16_t dataOut[DATA_LENGTH] = {};

//  double-byte packages to be send via spi
uint16_t packages[NUM_PACKAGES];

// address pins mulitplexer
int addA = 24;
int addB = 25;
int addC = 26;

// pins connected to mux for analog read
int mux[NUM_MUX] = {A1,A2,A3,A4,A5,A6,A7,A8,A9,A0,A22,A21,A20,A19,A18,A17};

// data read from analog ports
int puffer[NUM_BLOCKS];

// mapping the blocknumber to the right pin
int pinmap[NUM_BLOCKS] =   {   
                                  65,80,17,64,33,18,34,51,3,2,85,55,7,24,87,108,              // row 0
                                  113,83,49,96,1,16,114,19,50,5,21,54,23,39,103,71,           // row 1
                                  81,48,115,67,99,0,98,82,66,53,37,86,43,119,88,27,           // row 2    
                                  97,32,94,112,35,101,38,69,117,91,56,104,72,8,40,58,         // row 3
                                  62,30,13,31,126,22,111,92,6,44,11,28,26,12,59,120,          // row 4
                                  110,47,45,125,29,63,36,52,100,107,106,75,9,89,105,124,      // row 5
                                  109,60,95,15,79,127,68,116,84,20,57,122,73,25,123,76,       // row 6
                                  78,61,93,77,46,14,102,4,70,118,10,41,121,74,42,90           // row 7
                              };

                              
// blocks with id of its kind
int blocks[NUM_BLOCKS];

void setup() {  
    Serial.begin(9600);

    Serial.print("Slave - Setup...");
    
    pinMode(addA, OUTPUT);
    pinMode(addB, OUTPUT);
    pinMode(addC, OUTPUT);
    
    // --- spi setup begin ---
    
    //Begin SPI in SLAVE (SCK pin, MOSI pin, MISO pin, CS pin)
    SPI_SLAVE.begin_SLAVE(SCK, MOSI, MISO, CS0);
    
    //Set the CTAR0_SLAVE0 (Frame Size, SPI Mode)
    SPI_SLAVE.setCTAR_SLAVE(16, SPI_MODE0);
    
    //Enable the SPI0 Interrupt
    NVIC_ENABLE_IRQ(IRQ_SPI0);
    
    // --- spi setup end ---
    
    Serial.println(" done");
}

// interrupt routine
void spi0_isr(void) {
    SPI_SLAVE.rx16(dataIn, DATA_LENGTH); // if data from master is needed to determine the transmitting data

    dataOut[0] = packages[*dataIn];
  
    SPI_SLAVE.rxtx16(dataIn, dataOut, DATA_LENGTH);
}

void loop() {
    // put your main code here, to run repeatedly:
    
    // printSPI(); // only for debugging
    
    fieldRead();
    
    //printRead(); // only for debugging
    //showRead(); // only for debugging
    //showBlocks(); // only for debugging
    
    calculateMatrix();
    
    //printMatrix(); // only for debugging
    
    calculatePackages();
    
    // printPackages(); // only for debugging
    
    //Reset the packet count
    SPI_SLAVE.packetCT=0;

    delay(DEFAULT_DELAY);
}

/*
 * Reads the data from the blocks as an analog value depending on the ohm-value of
 * the resistor used in said blocks.
 */
void fieldRead()
{
    for(int address=0; address<8; address++)
    {
        // address pins counting up binary
        if ((address & 1) == 1) {digitalWrite(addA,HIGH);} else {digitalWrite(addA,LOW);}
        if ((address & 2) == 2) {digitalWrite(addB,HIGH);} else {digitalWrite(addB,LOW);}
        if ((address & 4) == 4) {digitalWrite(addC,HIGH);} else {digitalWrite(addC,LOW);}
        // time for multiplexer to switch
        delayMicroseconds(1);
        // now read the analog-in from every multiplexer
        for(int i=0; i<NUM_MUX; i++)
        {
            puffer[address*NUM_MUX+i] = analogRead(mux[i]);
        }
    }
}

/*
 * Calculates an absolut value for each block to determain the block type.
 */
void calculateMatrix()
{
    for(int i = 0; i < NUM_BLOCKS; i++)
    {
        // cleanup waste but only the current one in case of isr
        blocks[i] = 0;
        if(puffer[pinmap[i]] >= 100 && puffer[pinmap[i]] <= 160) blocks[i]= 1;
        else if(puffer[pinmap[i]] >= 250 && puffer[pinmap[i]] <= 310) blocks[i] = 2;
        else if(puffer[pinmap[i]] >= 450 && puffer[pinmap[i]] <= 510) blocks[i] = 3;
        else if(puffer[pinmap[i]] >= 340 && puffer[pinmap[i]] <= 400) blocks[i] = 4;
    }
}

/*
 * Calculates all packages needed to transmit all gathered data.
 */
void calculatePackages()
{
    uint16_t buildPackage = 0;
    for(int id = 0; id < NUM_PACKAGES; id++)
    {
        buildPackage = 0;
        buildPackage |= 0x8000;
        buildPackage |= (id << 9);
        if (id*3 + 0 < NUM_BLOCKS) buildPackage |= (blocks[id*3 + 0] << 6);
        if (id*3 + 1 < NUM_BLOCKS) buildPackage |= (blocks[id*3 + 1] << 3);
        if (id*3 + 2 < NUM_BLOCKS) buildPackage |= (blocks[id*3 + 2] << 0);
        packages[id] = buildPackage;
    }
}


/*
 *    debugging methods
 */
void printPackages()
{
    for (uint8_t i = 0; i < NUM_PACKAGES; i++)
    {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(packages[i], BIN);
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
        delayMicroseconds(50);
    }
    Serial.println();
}


void printRead()
{
    for(int i=0; i<NUM_BLOCKS; i++)
    {
        Serial.print("B");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(puffer[i]);
        Serial.print("  ");
    }
    Serial.println();
}

void printSPI()
{
  for(int i=0; i<DATA_LENGTH; i++)
  {
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

void showRead()
{
    for(int i=0; i<NUM_BLOCKS; i++)
    {
        if(puffer[i] > 100)
        {
            Serial.print(i);
            Serial.print("  -  ");
            Serial.println(puffer[i]);
        }
    }
}

void showBlocks()
{
    for(int i=0; i<NUM_BLOCKS; i++)
    {
        if(blocks[i] != 0)
        {
            Serial.print(i);
            Serial.print("  -  ");
            Serial.print(blocks[i]);
            Serial.print("  -  ");
            Serial.println(puffer[pinmap[i]]);
        }
    }
}
