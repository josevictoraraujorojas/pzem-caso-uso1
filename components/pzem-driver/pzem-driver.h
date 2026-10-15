#ifndef PZEM_DRIVER_H
#define PZEM_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

void initialize_pzem(uint8_t txPin, uint8_t rxPin);
bool updateValues(uint8_t addr);

float getVoltage();
float getCurrent();
float getPower();
float getEnergy();
float getFrequency();
float getPF();
uint16_t getAlarm();

#endif