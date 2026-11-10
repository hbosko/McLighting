#include "palette_editor_HTML.h"

uint8_t paletteIdx = 0;
bool isCustomPaletteActive = false;

#define MAX_CUSTOM_PALETTES 20

uint32_t myCustomPalettes[MAX_CUSTOM_PALETTES][3];
char myCustomPaletteNames[MAX_CUSTOM_PALETTES][32];
uint8_t actual_custom_palette_count = 0;


uint32_t getCustomPaletteColor(int pIdx, int colorIdx) {
  if (pIdx < 0 || pIdx >= actual_custom_palette_count) return 0x000000;
  if (colorIdx < 0 || colorIdx > 2) return 0x000000;
  
  return myCustomPalettes[pIdx][colorIdx];
}
