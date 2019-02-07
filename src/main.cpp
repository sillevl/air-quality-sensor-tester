#include "Simple-LoRaWAN.h"
#include "LoRaMessage.h"
#include "AirQualitySensor.h"
#include "NovaSDS011.h"

using namespace SimpleLoRaWAN;

uint8_t devEui[8] = { 0x00, 0x9D, 0xF8, 0x5F, 0xB7, 0xEA, 0x2F, 0xCE };
uint8_t appEui[8] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x01, 0x78, 0x46 };
uint8_t appKey[16] = { 0x4F, 0x15, 0x94, 0x63, 0xB9, 0xDF, 0x09, 0x70, 0x77, 0xE3, 0x93, 0x6B, 0x20, 0xFB, 0x0B, 0x39 };

Serial pc(USBTX, USBRX, 115200);
Serial novaSerial(A4,A5);

Node node(devEui, appEui, appKey);
AirQualitySensor omron(D4, D5);
NovaSDS011 nova(&novaSerial);
// TODO: Sharp sensor

uint8_t data[32];

struct PmValues {
  float pm10;
  float pm2_5;
};

int main(void)
{
  pc.baud(115200);
  pc.printf("\r\n*** staring airquality sensor tester ***\r\n");
  
  while(true) {
    pc.printf("\r\nStarting measurement...\r\n");

    omron.start();
    while(omron.isMeasuring()){
        wait_ms(100);
    }

    nova.read();

    PmValues omronValues = { (float) omron.getPM10(), (float) omron.getPM2_5() };
    PmValues novaValues = { nova.getPM10(), nova.getPM2_5() };
    PmValues sharpValues = { 0.0, 0.0 };


    pc.printf("measuring ready\r\n");
    pc.printf("|   Sensor    |   PM10   |   PM2.5  |\r\n");
    pc.printf("|-------------|----------|----------|\r\n");
    pc.printf("| Omron       | %8.1f | %8.1f |\r\n", omronValues.pm10, omronValues.pm2_5);
    pc.printf("| Nova        | %8.1f | %8.1f |\r\n", novaValues.pm10, novaValues.pm2_5);
    pc.printf("| Sharp       | %8.1f | %8.1f |\r\n", sharpValues.pm10, sharpValues.pm2_5);

    // int pm10 = sensor.getPM10();
    // int pm2_5 = sensor.getPM2_5();

    LoRaMessage message;

    message
      .addUint16(omronValues.pm10)
      .addUint16(omronValues.pm2_5)
      .addUint16(novaValues.pm10)
      .addUint16(novaValues.pm2_5)
      .addUint16(sharpValues.pm10)
      .addUint16(sharpValues.pm2_5);

    
    // data[0] = (pm10 >> 8) & 0xFF;
    // data[1] = (pm10 >> 0) & 0xFF;
    // data[2] = (pm2_5 >> 8) & 0xFF;
    // data[3] = (pm2_5 >> 0) & 0xFF;

    node.send(message.getMessage(), message.getLength());
  }
}