const char* const BuiltInPalettesNames[] PROGMEM = {
  "Neon Rage",
  "Deep Anger",
  "Chill Out",
  "Sunset",
  "Cyberpunk",
  "Forest",
  "Synthwave",
  "Toxic Waste",
  "Ice Kingdom",
  "Volcano",
  "Miami Vice",
  "Matrix Code",
  "Candy Shop",
  "Deep Ocean",
  "Halloween",
  "Aurora",
  "Gold Rush",
  "Arcade",
  "Space Warp",
  "Zen Garden"
};

const uint32_t BuiltInPalettes[][3] PROGMEM = {
  {0xFF0055, 0x00FFCC, 0x9900FF}, // 0: Neon Rage
  {0x110000, 0xFF0000, 0x330000}, // 1: Deep Anger
  {0x0055FF, 0x00FF88, 0x001133}, // 2: Chill Out
  {0xFF4500, 0xFF8C00, 0xFFD700}, // 3: Sunset
  {0x00F0FF, 0xFF007F, 0x7B00FF}, // 4: Cyberpunk
  {0x004B23, 0x38B000, 0x007200}, // 5: Forest
  {0xF000FF, 0x0018FF, 0xFF00B4}, // 6: Synthwave (Retro vijolična, globoka modra in roza)
  {0x00FF00, 0x7FFF00, 0x1A3300}, // 7: Toxic Waste (Strupeno zelena, neonska lipa in temno ozadje)
  {0xE0FFFF, 0x00FFFF, 0x4682B4}, // 8: Ice Kingdom (Ledeno bela, svetlo modra in jeklena)
  {0xFF0000, 0xFF7F00, 0x4A0000}, // 9: Volcano (Žareča rdeča, oranžna lava in ugašajoč pepel)
  {0xFF1493, 0x00EEEE, 0xFFC0CB}, // 10: Miami Vice (Hot pink, turkizna in pastelno roza)
  {0x00FF00, 0x003300, 0x00AF00}, // 11: Matrix Code (Klasična hekerska matrika odtenkov zelene)
  {0xFF69B4, 0xBA55D3, 0x00FFFF}, // 12: Candy Shop (Sladka roza, prelivna orhideja in mint turkizna)
  {0x000080, 0x008080, 0x000033}, // 13: Deep Ocean (Mornarsko modra, teal in temačna globina)
  {0xFF8C00, 0x4B0082, 0x000000}, // 14: Halloween (Bučna oranžna, čarovniško vijolična in črna)
  {0x00FF88, 0x00FFCC, 0x6A0DAD}, // 15: Aurora (Severni sij - mentol zelena, svetel turkiz in mistična vijolična)
  {0xD4AF37, 0x8B6508, 0xFFDF00}, // 16: Gold Rush (Luksuzna zlata, temno zlata in svetleč sijaj)
  {0xFF0000, 0x0000FF, 0xFFFF00}, // 17: Arcade (Primarni retro kontrasti - čista rdeča, modra in rumena)
  {0x4B0082, 0xFF00FF, 0x000011}, // 18: Space Warp (Globoki vesoljski indigo, magenta zvezda in kozmični prah)
  {0x556B2F, 0x8FBC8F, 0xD2B48C}  // 19: Zen Garden (Oljčno zelena, blagi morski žajbelj in bambusov pesek)
};

const uint8_t BUILTIN_PALETTE_COUNT = sizeof(BuiltInPalettes) / sizeof(BuiltInPalettes[0]);
