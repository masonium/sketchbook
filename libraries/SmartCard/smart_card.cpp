#include <Wire.h>
#include <Arduino.h>
#include "smart_card.h"

#define SMART_CARD_ADDRESS( block ) (0x50 | (block) )

//#define PRINT_DEBUG

void SmartCard::init()
{
  Wire.begin();
}

SmartCard::SmartCard() : block_num(0), subaddress(0)
{
  seek(0);
}

void SmartCard::seek(uint16_t full_address)
{
  subaddress = full_address & 0xff;
  block_num = (full_address >> 8) & 0x8;

#ifdef PRINT_DEBUG
  Serial.print("Seeking  "); Serial.print(block_num);
  Serial.print(":"); Serial.println(subaddress);
#endif PRINT_DEBUG
  
  // Wire.beginTransmission( SMART_CARD_ADDRESS( block_num ) );
  // Wire.write( subaddress );
  // Wire.endTransmission();
}

void SmartCard::write( uint8_t byte )
{
#ifdef PRINT_DEBUG
  Serial.print("Writing "); Serial.print(block_num);
  Serial.print(":"); Serial.println(subaddress);
#endif PRINT_DEBUG
  
  // Assume we are at the write location internally.
  Wire.beginTransmission( SMART_CARD_ADDRESS( block_num ) );
  Wire.write( subaddress );
  Wire.write( byte );
  Wire.endTransmission();

  _increment_address();
}

  
uint8_t SmartCard::read(  )
{
#ifdef PRINT_DEBUG
  Serial.print("Reading "); Serial.print(block_num);
  Serial.print(":"); Serial.println(subaddress);
#endif PRINT_DEBUG
  Wire.beginTransmission( SMART_CARD_ADDRESS( block_num ) );
  Wire.write( subaddress );
  Wire.endTransmission(false);

  Wire.requestFrom( SMART_CARD_ADDRESS( block_num ), 1 );

  uint8_t byte = Wire.read();

  _increment_address();
  return byte;
}
