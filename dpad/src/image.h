#ifndef IMAGE_H__
#define IMAGE_H__

class LPD8806x8;

/**
 * Representation of given as input. The public interface to all color-based
 * methods assume that colors are 8-bit per component. Any conversions are done
 * internally and transparently.
 */
struct color
{
  union
  {
    uint8_t grb[3];
    struct
    {
      uint8_t g, r, b;
    } v;
  };
};

struct Palette
{
  Palette(uint16_t size) : _size(size) {
    _c = (color*)malloc(_size * sizeof(color));
  }

  ~Palette() {
    free(_c);
  }

  /**
   * Set the color specified for the given index.
   */
  void set_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
  
  void set_color(uint8_t index, uint32_t rgb) {
    set_color(index, (rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
  }

  void set_color_hsv(uint8_t index, uint8_t hue, uint8_t saturation, uint8_t value);
  /** 
   * Rotate the first 'num' colors in the palette.
   */
  void cycle_colors(uint16_t num) {
    color c = _c[0];
    for (uint16_t i = 0; i < num - 1; ++i)
      _c[i] = _c[i+1];
    _c[num-1] = c;
  }
    
public:
  color* _c;
  
private:
  friend class LPD8806x8;
  uint16_t _size;

};

class StripImage
{
private:
  uint8_t _c[128];
  uint8_t _size;
  friend class LPD8806x8;

public:
  StripImage() : _size(128) { }
  StripImage(uint8_t size) : _size(size) {  }

  void fill( uint8_t index ) {
    for (uint8_t i = 0; i < _size; ++i) {
      _c[i] = 0;
    }
  }

  void set_color( uint8_t x, uint8_t color_index) {
    _c[x] = color_index;
  }
};
    
class Image
{
public:
  uint8_t _c[8][128];
  friend class LPD8806x8;
  
public:
  Image() {
  }

  void fill( uint8_t index ) {
    for (uint8_t i = 0; i < 8; ++i)
      for (uint8_t j = 0; j < 128; ++j)
        _c[i][j] = index;
  }

  void set_color( uint8_t x, uint8_t y, uint8_t color_index) {
    uint8_t r, c;
    rowcol(x, y, r, c);
    _c[r][c] = color_index;
  }
  uint8_t get_color( uint8_t x, uint8_t y ) const {
    uint8_t r, c;
    rowcol(x, y, r, c);
    return _c[r][c];
  }
  
  uint8_t& operator() (uint8_t x, uint8_t y) {
    uint8_t r, c;
    rowcol(x, y, r, c);
    return _c[r][c];
  }    
  uint8_t operator() (uint8_t x, uint8_t y) const {
    uint8_t s, r;
    rowcol(x, y, s, r);
    return _c[s][r];
  }    

  void rowcol(uint8_t x, uint8_t y, uint8_t& s, uint8_t& r) const {
    bool h = x >= 16;
    s = x % 4 + h * 4;

    uint8_t m;
    if (h)
    {
       m = (x - s - 12) / 4;
    }
    else
    {
      m = (12 + s - x) / 4;
    }

    r = m * 32;
    r += m % 2 == 0 ? y : 31 - y;

  }
};

#endif
