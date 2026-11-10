// ***************************************************************************
// Color modes
// ***************************************************************************

int dipInterval = 10;
int darkTime = 250;
unsigned long currentDipTime;
unsigned long dipStartTime;
unsigned long currentMillis;
int ledState = LOW;
unsigned long previousMillis = 0;
int led = 5;
int interval = 2000;
int twitch = 50;
int dipCount = 0;
int analogLevel = 100;
boolean timeToDip = false;
int ledStates[NUMLEDS];


void hsb2rgbAN1(uint16_t index, uint8_t sat, uint8_t bright, uint8_t myled) {
    // Source: https://adafruit.com
    uint8_t temp[5], n = (index >> 8) % 3;
    
    temp[0] = temp[3] = (uint8_t)(((sat ^ 255) * bright) / 255);
    temp[1] = temp[4] = (uint8_t)((((index & 255) * sat / 255) + (sat ^ 255)) * bright / 255);
    temp[2] =           (uint8_t)(((((index & 255) ^ 255) * sat / 255) + (sat ^ 255)) * bright / 255);

    if(strip) strip->setPixelColor(myled, temp[n + 2], temp[n + 1], temp[n]);
}


void updateLed(int target_led, int brightness, boolean triggerShow) {
	ledStates[target_led] = brightness;
	
	for (int i = 0; i < NUMLEDS; i++)
	{
		uint16_t index = (i % 3 == 0) ? 400 : random(0, 767);
		hsb2rgbAN1(index, 200, ledStates[i], i);
	}
  
	if (triggerShow && strip) {
    strip->show();
  }
}


void tv() {
	if (exit_func) {
		exit_func = false;
		return;
	}
	
	if (timeToDip == false)
	{
		currentMillis = millis();
		if (currentMillis - previousMillis > interval) 
		{
			previousMillis = currentMillis;
			interval = random(750, 4001);
			twitch = random(40, 100);
			dipCount++;
		}
    
		if (currentMillis - previousMillis < twitch)
		{
      if (strip) {
        led = random(0, strip->numPixels());
        analogLevel = random(50, 255);
        ledState = (ledState == LOW) ? HIGH : LOW;
        
        updateLed(led, (ledState == HIGH) ? 255 : 0, true);
        
        if (dipCount > dipInterval)
        { 
          DEBUG_PRINTLN(F("TV: Sprožam zatemnitev (Dip)"));
          timeToDip = true;
          dipCount = 0;
          dipStartTime = millis();
          darkTime = random(50, 150);
          dipInterval = random(5, 250);
        }
      }
		} 
	}
	else
	{
		currentDipTime = millis();
		if (currentDipTime - dipStartTime < darkTime)
		{
      if (strip) {
        for (int i = 0; i < strip->numPixels(); i++)
        {
          updateLed(i, 0, false); 
        }
        strip->show();
      }
		}
		else
		{
			timeToDip = false;
		}
	}
}


//******************************************************************************************
//                     PALETTE ANIMATION FUNCTIONS
//******************************************************************************************

void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  if (!strip) return;
  
  for(int i = 0; i < WS2812FXStripSettings.stripSize; i++) {
    strip->setColor(main_color.red, main_color.green, main_color.blue);
    
    if (anim_direction == "forward") { colorIndex += 3; }
    if (anim_direction == "back")    { colorIndex -= 3; }
  }
}
