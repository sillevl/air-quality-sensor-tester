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

struct PmValues {
  float pm10;
  float pm2_5;
};

struct Dataset {
  PmValues omron;
  PmValues nova;
  PmValues sharp;
};

void sendMessage(Dataset data)
{
  LoRaMessage message;

  message
    .addUint16(data.omron.pm10)
    .addUint16(data.omron.pm2_5)
    .addUint16(data.nova.pm10)
    .addUint16(data.nova.pm2_5)
    .addUint16(data.sharp.pm10)
    .addUint16(data.sharp.pm2_5);

  node.send(message.getMessage(), message.getLength());
}

void logData(Dataset data)
{
  pc.printf("measuring ready\r\n");
  pc.printf("|   Sensor    |   PM10   |   PM2.5  |\r\n");
  pc.printf("|-------------|----------|----------|\r\n");
  pc.printf("| Omron       | %8.1f | %8.1f |\r\n", data.omron.pm10, data.omron.pm2_5);
  pc.printf("| Nova        | %8.1f | %8.1f |\r\n", data.nova.pm10, data.nova.pm2_5);
  pc.printf("| Sharp       | %8.1f | %8.1f |\r\n", data.sharp.pm10, data.sharp.pm2_5);
}

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

    Dataset dataset = {
      { (float) omron.getPM10(), (float) omron.getPM2_5() },
      { nova.getPM10(), nova.getPM2_5() },
      { 0.0, 0.0 }
    };

    logData(dataset);
    sendMessage(dataset);
  }
}