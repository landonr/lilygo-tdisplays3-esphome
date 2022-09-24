#include <SPI.h>
#include <TFT_eSPI.h>
#include "esphome.h"
#include "Free_Fonts.h"

#ifndef DISPLAYCONTROLLER
#define DISPLAYCONTROLLER

class DisplayController : public Component
{
	double barSize = 0;
	public:
	std::string time = "init";
	void setup() override {
		tft.init();
		tft.fillScreen(TFT_BLACK);
		tft.setRotation(3);
		tft.setFreeFont(FSB18);
	}

	void drawString(std::string text) {
	  	tft.setTextColor(TFT_RED, TFT_BLACK);
		tft.drawString(text.c_str(), 8, 64, GFXFF);
	}

	void drawBar() {
		tft.fillRect(0, 4, barSize, 8, TFT_RED);
		barSize+=1;
		if(barSize > 320) {
			tft.fillRect(0, 4, barSize, 8, TFT_BLACK);
			barSize = 0;
		}
	}

	void clear() {
		tft.fillScreen(TFT_BLACK);
	}

	void loop() override {
	}

	private:
	TFT_eSPI tft = TFT_eSPI();
};
DisplayController * displayControllerComponent = new DisplayController();

#endif