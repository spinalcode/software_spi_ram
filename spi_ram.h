//#define BYTEMODE 0
//#define SEQMODE 64
//#define PAGEMODE 128
//#define DUALMODE 0x3B
//#define QUADMODE 0x38
//#define SPIMODE 0xFF

void init_RAM(){
    cs=0;
    spi.write(0x01); // Command
    spi.write(0x64); // SEQ Mode
    cs=1;
}

void sendToRAM(int address, const uint8_t* data, int howLong) {
    
    while (cs == 0) {
        wait_us(1);
    }

    SPI spi(MOSI_pin, MISO_pin, SCLK_pin); // mosi, miso, sclk
    cs.output();
    cs=0;

    spi.write(0x02); // Write command

    // assume SIZEOFRAM == 1024
    uint8_t temp = address >> 16;
    spi.write(temp);
    temp = address >> 8;
    spi.write(temp);
    temp = address & 255;
    spi.write(temp);

    for(int t=0; t<howLong; t++){
        spi.write(data[t]);
    }
    cs.input();

}

void getFromRAM(int address, uint8_t* data, int howLong) {

    while (cs == 0) {
        wait_us(1);
    }

    SPI spi(MOSI_pin, MISO_pin, SCLK_pin); // mosi, miso, sclk
    cs.output();
    cs=0;

    spi.write(0x03); // Read command

    // assume SIZEOFRAM == 1024
    uint8_t temp = address >> 16;
    spi.write(temp);
    temp = address >> 8;
    spi.write(temp);
    temp = address & 255;
    spi.write(temp);

    for(int t=0; t<howLong; t++){
        data[t] = spi.write(0x00); // sending dummy bytes will also read the MISO
    }
    cs.input();
}

