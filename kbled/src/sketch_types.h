enum ColorMode
{
  CM_RAINBOW = 0,
  CM_WHITE,
  CM_NUM_MODES
};

struct note_t
{
  int index;
  int brightness;
};

enum PedalStatus
{
  PS_DOWN,
  PS_UP,
  PS_NONE
};
