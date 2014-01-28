#include <Wire.h>
#include <Arduino.h>
#include "clock.h"

DS1307_Clock::DS1307_Clock()
{
}

// int myputc( char c, FILE * f ) {
//   Serial.write( c );
// }

void DS1307_Clock::begin()
{
  Wire.begin();
  //fdevopen(&myputc, 0);
}

void DS1307_Clock::enable()
{


}

void DS1307_Clock::disable()
{

}

void DS1307_Clock::set_time(ds1307_time t, bool enable)
{
  // Convert all of the bytes to bcd.
  for (uint8_t i = 0; i < 7; ++i)
    t.datetime[i] = int_to_bcd(t.datetime[i]);
  t.datetime[7] = 0;
  
  // Set to enable or disable
  if (enable)
    t.seconds &= 0x7f;
  else
    t.seconds |= 0x80;

  write_bytes(t.datetime, 7);
}

uint8_t DS1307_Clock::write_bytes(uint8_t* bytes, uint8_t len)
{
  uint8_t lenWritten = 0;
  Wire.beginTransmission( DS1307_ADDRESS );
  Wire.write((uint8_t)0);
  lenWritten = Wire.write(bytes, len);
  Wire.endTransmission();

  Serial.print("Wrote "); Serial.print(lenWritten);
  Serial.println(" bytes to RTC (in class).");
  return lenWritten;
}

uint8_t DS1307_Clock::read_bytes(uint8_t* bytes, uint8_t len)
{
  Wire.beginTransmission( DS1307_ADDRESS );
  uint8_t x = 0;
  Wire.write(&x, 1);
  Wire.endTransmission();

  Wire.requestFrom((uint8_t)DS1307_ADDRESS, len);

  uint8_t bytes_read = 0;
  for (uint8_t i = 0; i < len; ++i)
  {
    if (Wire.available())
    {
      bytes[i] = Wire.read();
      ++bytes_read;
    }
  }

  return bytes_read;
}

void DS1307_Clock::get_time(ds1307_time* t)
{
  uint8_t bytes_read = read_bytes((uint8_t*)t, 7);
  Serial.print(bytes_read);
  Serial.println(F(" bytes read."));
  
  t->datetime[0] = bcd_to_int(t->datetime[0] & 0x7f);
  for (uint8_t i = 1; i < 7; ++i)
  {
    Serial.print(t->datetime[i]);
    t->datetime[i] = bcd_to_int(t->datetime[i]);
    Serial.print("  ");
    Serial.println(t->datetime[i]);
  }
}


uint8_t DS1307_Clock::int_to_bcd(uint8_t t)
{
  return (t % 10) + ((t / 10) << 4);
}

uint8_t DS1307_Clock::bcd_to_int(uint8_t t)
{
  return (t & 0xF) + 10 * (t >> 4);
}

void print_2d(uint8_t num)
{
  if (num < 10)
  {
    Serial.print("0");
    Serial.print(num);
  }
  else
  {
    Serial.print(num);
  }
}

void DS1307_Clock::print_time(ds1307_time _t)
{
  switch (_t.day_of_week)
  {
  case CLOCK_WEEKDAY_MONDAY:
    Serial.print(F("Monday, "));
    break;
  case CLOCK_WEEKDAY_TUESDAY:
    Serial.print(F("Tuesday, "));
    break;
  case CLOCK_WEEKDAY_WEDNESDAY:
    Serial.print(F("Wednesday, "));
    break;
  case CLOCK_WEEKDAY_THURSDAY:
    Serial.print(F("Thursday, "));
    break;
  case CLOCK_WEEKDAY_FRIDAY:
    Serial.print(F("Friday, "));
    break;
  case CLOCK_WEEKDAY_SATURDAY:
    Serial.print(F("Saturday, "));
    break;
  case CLOCK_WEEKDAY_SUNDAY:
    Serial.print(F("Sunday, "));
    break;
  }

  Serial.print(20);
  print_2d(_t.year);
  Serial.print("/");
  print_2d(_t.month);
  Serial.print("/");
  print_2d(_t.day);
  Serial.print("   ");
  print_2d(_t.hours);
  Serial.print(":");
  print_2d(_t.minutes);
  Serial.print(":");
  print_2d(_t.seconds);
  Serial.println("");
  
  // printf("%02d/%02d/%02d   %02d:%02d:%02d", _t.year, _t.month, _t.day,
  //        _t.hours, _t.minutes, _t.seconds);
}
