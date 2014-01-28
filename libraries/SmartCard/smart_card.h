#ifndef SMART_CARD_H__
#define SMART_CARD_H__

enum SmartCardType
{
  SMART_CARD_IS24C16A,
  SMART_CARD_NUM_TYPES
};

class SmartCard
{
public:
  /**
   * Common initialization routing for all smart card usage.
   */
  static void init();
  
  SmartCard();

  void write(uint8_t byte);
  uint8_t write(const uint8_t* bytes, uint16_t size);

  uint8_t read();
  uint16_t read(uint8_t* bytes, uint16_t size);

  void seek(uint16_t addy);
  
private:
  uint8_t block_num, subaddress;
  
  void _increment_address() {
      // Increment the address.
    ++subaddress;
    if (subaddress == 0) {
      block_num = (block_num + 1) % 8;
    }
  }
};


#endif
