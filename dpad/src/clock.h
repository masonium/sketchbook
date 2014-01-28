#ifndef DS1307_CLOCK_H__
#define DS1307_CLOCK_H__

struct ds1307_time
{
  union
  {
    uint8_t datetime[8];
    struct
    {
      uint8_t seconds, minutes, hours;
      uint8_t day_of_week;
      uint8_t day, month, year;
      uint8_t register_;
    };
  };
};

#define DS1307_ADDRESS      0x68

#define DS1307_TIME_LENGTH  8

#define CLOCK_SET_TIME      0x1
#define CLOCK_SET_DATE      0x2

#define CLOCK_WEEKDAY_MONDAY    1
#define CLOCK_WEEKDAY_TUESDAY   2
#define CLOCK_WEEKDAY_WEDNESDAY 3
#define CLOCK_WEEKDAY_THURSDAY  4
#define CLOCK_WEEKDAY_FRIDAY    5
#define CLOCK_WEEKDAY_SATURDAY  6
#define CLOCK_WEEKDAY_SUNDAY    7

typedef void (*printChar)(char);

class DS1307_Clock
{
public:
DS1307_Clock();

void begin();

/*
 * Starts the clock oscillation. When the clock is disable, the time won't
 * change. 
 */
void enable();

/* 
 * Stops the clock oscillation.
 **/
void disable();
  
void get_time(ds1307_time* t);

/*
 * Set the time stored on the chip. This function automatically activates the
 * clock, unless disable is specifically set.
 */
void set_time(ds1307_time t,  bool enable=true); 

static void print_time(ds1307_time _t);

private:
static uint8_t bcd_to_int(uint8_t);
static uint8_t int_to_bcd(uint8_t);

static uint8_t write_bytes(uint8_t* bytes, uint8_t len);
static uint8_t write_bytes(ds1307_time);
static uint8_t read_bytes(uint8_t* bytes, uint8_t len);
};


#endif
