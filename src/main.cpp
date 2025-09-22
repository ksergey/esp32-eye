// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <AnimatedGIF.h>
#include <Arduino_GFX_Library.h>

#include "darthvader.h"
#include "hyperspace.h"
#include "nostromo.h"
#include "x_wing.h"
#include "eye.h"

#define GIF_IMAGE eye

Arduino_DataBus* bus = new Arduino_ESP32SPI(GC9A01_SPI_CONFIG_DC /* DC */, GC9A01_SPI_CONFIG_CS /* CS */,
    GC9A01_SPI_BUS_SCLK /* SCK */, GC9A01_SPI_BUS_MOSI /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX* gfx = new Arduino_GC9A01(bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */);

AnimatedGIF gif;

#define BRIGHT_SHIFT 3

void setup() {
#ifdef DEV_DEVICE_INIT
  DEV_DEVICE_INIT();
#endif

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  // while(!Serial);
  Serial.println("Arduino_GFX Hello World example");

  // Init Display
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(RGB565_BLACK);
  gfx->invertDisplay(true);

#ifdef DISPLAY_BCKL
  pinMode(DISPLAY_BCKL, OUTPUT);
  digitalWrite(DISPLAY_BCKL, HIGH);
#endif

  gif.begin(BIG_ENDIAN_PIXELS);
}

void gif_draw(GIFDRAW* pDraw) {
  uint8_t* s;
  uint16_t *d, *usPalette, usTemp[320];
  int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth + pDraw->iX > gfx->width()) {
    iWidth = gfx->width() - pDraw->iX;
  }
  usPalette = pDraw->pPalette;
  y = pDraw->iY + pDraw->y; // current line
  if (y >= gfx->height() || pDraw->iX >= gfx->width() || iWidth < 1) {
    return;
  }
  s = pDraw->pPixels;
  if (pDraw->ucDisposalMethod == 2) // restore to background color
  {
    for (x = 0; x < iWidth; x++) {
      if (s[x] == pDraw->ucTransparent) {
        s[x] = pDraw->ucBackground;
      }
    }
    pDraw->ucHasTransparency = 0;
  }

  // Apply the new pixels to the main image
  if (pDraw->ucHasTransparency) // if transparency used
  {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int x, iCount;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0; // count non-transparent pixels
    while (x < iWidth) {
      c = ucTransparent - 1;
      d = usTemp;
      while (c != ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent) // done, stop
        {
          s--; // back up to treat it like transparent
        } else // opaque
        {
          *d++ = usPalette[c];
          iCount++;
        }
      } // while looking for opaque pixels
      if (iCount) // any opaque pixels?
      {
        gfx->draw16bitBeRGBBitmap(pDraw->iX + x, y, usTemp, iCount, 1);
        x += iCount;
        iCount = 0;
      }
      // no, look for a run of transparent pixels
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent) {
          iCount++;
        } else {
          s--;
        }
      }
      if (iCount) {
        x += iCount; // skip these
        iCount = 0;
      }
    }
  } else {
    s = pDraw->pPixels;
    // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
    for (x = 0; x < iWidth; x++) {
      usTemp[x] = usPalette[*s++];
    }
    gfx->draw16bitBeRGBBitmap(pDraw->iX, y, usTemp, iWidth, 1);
  }
}

void loop() {
  auto rc = gif.open((uint8_t*)GIF_IMAGE, sizeof(GIF_IMAGE), gif_draw);
  if (rc) {
    while (rc) {
      rc = gif.playFrame(true, nullptr);
    }
    gif.close();
  }
}
