#define screenWidth 400

int sx=0;
int sy=0;

#include <Pokitto.h>

#include "globals.h"
#include "font.h"
#include "buttonhandling.h"


// #define HWSPI

#ifdef HWSPI

#define MISO_pin P1_21
#define MOSI_pin P1_22
#define SCLK_pin P1_20
#define CS_pin P1_5

SPI spi(MOSI_pin, MISO_pin, SCLK_pin); // mosi, miso, sclk
DigitalInOut cs(CS_pin);

#include "spi_ram.h"
//#include "screen.h"

#else

//----------------------------- SPI Stuff -----------------------
#define CLOCK_PIN 1<<20 // hardware clock pin
#define MISO_PIN 1<<21  // hardware MISO
#define MOSI_PIN 1<<22  // hardware MOSI
#define SS_PIN 1<<5    //  SS 

#define SET_CLOCK LPC_GPIO_PORT->SET[1] = CLOCK_PIN
#define CLR_CLOCK LPC_GPIO_PORT->CLR[1] = CLOCK_PIN
#define TOG_CLOCK ((volatile uint32_t *) 0xA0002304)[0] = CLOCK_PIN

#define SET_MOSI LPC_GPIO_PORT->SET[1] = MOSI_PIN
#define CLR_MOSI LPC_GPIO_PORT->CLR[1] = MOSI_PIN

#define GET_MISO (((volatile uint32_t *) 0xA0002104)[0] & MISO_PIN) >> 21

#define SS_HIGH LPC_GPIO_PORT->SET[1] = SS_PIN
#define SS_LOW LPC_GPIO_PORT->CLR[1] = SS_PIN

#define NOP asm volatile ("nop\n")

DigitalInOut cs(P1_5);

int spi_write(int value){

    uint8_t read = 0;
    for (int bit = 7; bit >= 0; --bit){

        if((value >> bit) & 0x01){ // Set MOSI Value
            SET_MOSI;
        }else{
            CLR_MOSI;
        }
        read |= (GET_MISO << bit); // Get MISO Value
        TOG_CLOCK; // Flip the Clock
        TOG_CLOCK; // Flip the Clock
        NOP; // This loop is too fast for the RAM chip
    }
    
    return read;
}

void initRAM(){

    SET_MOSI;
    CLR_CLOCK;
    SS_HIGH;

    // set clock pin to output
    LPC_GPIO_PORT->DIR[1] |= (1<<20);
    // set miso pin to input
    LPC_GPIO_PORT->DIR[1] &= ~(1<<21);
    // set mosi pin to output
    LPC_GPIO_PORT->DIR[1] |= (1<<22);
    // set SS pin to output
    LPC_GPIO_PORT->DIR[1] |= (1<<5);


    SS_LOW;
    spi_write(0x01); // Command
    spi_write(0x64); // SEQ Mode (dec 100)
    SS_HIGH;
}

void sendToRAM(int address, const uint8_t* data, int howLong) {
    
//    while (cs == 0) {
//        wait_us(1);
//    }

//    SPI spi(MOSI_pin, MISO_pin, SCLK_pin); // mosi, miso, sclk
//    cs.output();
//    cs=0;

    SS_LOW;
    spi_write(0x02); // Write command

    // assume SIZEOFRAM == 1024
    uint8_t temp = address >> 16;
    spi_write(temp);
    temp = address >> 8;
    spi_write(temp);
    temp = address & 255;
    spi_write(temp);

    for(int t=0; t<howLong; t++){
        spi_write(data[t]);
    }
//    cs.input();
    SS_HIGH;

}

void getFromRAM(int address, uint8_t* data, int howLong) {

//    while (cs == 0) {
//        wait_us(1);
//    }

//    SPI spi(MOSI_pin, MISO_pin, SCLK_pin); // mosi, miso, sclk
//    cs.output();
//    cs=0;

    SS_LOW;
    spi_write(0x03); // Read command

    // assume SIZEOFRAM == 1024
    uint8_t temp = address >> 16;
    spi_write(temp);
    temp = address >> 8;
    spi_write(temp);
    temp = address & 255;
    spi_write(temp);

    for(int t=0; t<howLong; t++){
        data[t] = spi_write(0x00); // sending dummy bytes will also read the MISO
    }
//    cs.input();
    SS_HIGH;
}

#endif

//---------------------------------------------------------------

#include "screen.h"


char tempText[64];

// print text
void myPrint(char x, char y, const char* text) {
    uint8_t numChars = strlen(text);
    uint8_t x1 = 0;//2+x+28*y;
    for (uint8_t t = 0; t < numChars; t++) {
        uint8_t character = text[t] - 32;
        Pokitto::Display::drawSprite(x+((x1++)*8), y, font88[character]);
    }
}

int main(){
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;
    using PS=Pokitto::Sound;

    PC::begin();
    PD::invisiblecolor = 0;
    PD::adjustCharStep = 1;
    PD::adjustLineStep = 0;
    PD::setFont(font3x5);

    // Setup the spi for 8 bit data, high steady state clock,
    // second edge capture, with a 20MHz clock rate
#ifdef HWSPI

    spi.format(8, 3); // 8bit clock low
    //spi.frequency(64000000); // does nothing at all, frequency stays 1mhz

    //while(1){};

    init_RAM();
#else    

    initRAM();
#endif


    // load a larger than screen image to RAM, this one takes up nearly all of it.
    sendToRAM(0, &background1[0], 400*300);
    Pokitto::Display::load565Palette(background1_pal); // load a palette the same way as any other palette in any other screen mode

    PD::lineFillers[0] = myBGFiller; // A custom filler to draw from SRAM HAT to screen
    
    while( PC::isRunning() ){

        if(!PC::update()) continue;
        updateButtons();

        if(_Left[HELD] && sx>0) sx--;
        if(_Right[HELD] && sx<180) sx++;
        if(_Up[HELD] && sy>0) sy--;
        if(_Down[HELD] && sy<124) sy++;

        sprintf(tempText,"FPS:%d",fpsCount);
        myPrint(0,0,tempText);

        fpsCounter++;

        if(PC::getTime() >= lastMillis+1000){
            PD::clear();
            lastMillis = PC::getTime();
            fpsCount = fpsCounter;
            fpsCounter = 0;
        }

    }
    
    return 0;
}
