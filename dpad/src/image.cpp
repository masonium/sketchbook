#include <Arduino.h>
#include "image.h"

/**
 * Taken from:
 * http://www.cs.rit.edu/~ncs/color/t_convert.html
 */
void HSVtoRGB( float *r, float *g, float *b, float h, float s, float v )
{
  int i;
  float f, p, q, t;
  if( s == 0 ) {
    // achromatic (grey)
    *r = *g = *b = v;
    return;
  }
  h /= 60;			// sector 0 to 5
  i = floor( h );
  f = h - i;			// factorial part of h
  p = v * ( 1 - s );
  q = v * ( 1 - s * f );
  t = v * ( 1 - s * ( 1 - f ) );
  switch( i ) {
  case 0:
    *r = v;
    *g = t;
    *b = p;
    break;
  case 1:
    *r = q;
    *g = v;
    *b = p;
    break;
  case 2:
    *r = p;
    *g = v;
    *b = t;
    break;
  case 3:
    *r = p;
    *g = q;
    *b = v;
    break;
  case 4:
    *r = t;
    *g = p;
    *b = v;
    break;
  default:		// case 5:
    *r = v;
    *g = p;
    *b = q;
    break;
  }
}

void Palette::set_color_hsv(uint8_t index, uint8_t hue, uint8_t saturation, uint8_t value)
{
  float h = float(hue) * 360.0f / 255.0f;
  float s = float(saturation) / 255.0f;
  float v = float(value) / 255.0f;
  float rf, gf, bf;

  HSVtoRGB(&rf, &gf, &bf, h, s, v);

  set_color(index, rf * 255, gf * 255, bf * 255);
}
