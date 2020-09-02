#include <Arduino.h>
#line 1 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\OLEDDisplay.cpp"
#line 87 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\qmu_beacon_locator.ino"
void onQspSuccess(uint8_t receivedChannel);
#line 146 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\qmu_beacon_locator.ino"
void onQspFailure();
#line 150 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\qmu_beacon_locator.ino"
void setup();
#line 186 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\qmu_beacon_locator.ino"
void loop();
#line 0 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\qmu_beacon_locator.ino"
#line 1 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\OLEDDisplay.cpp"
/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 by Daniel Eichhorn
 * Copyright (c) 2016 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Credits for parts of this code go to Mike Rankin. Thank you so much for sharing!
 */

#include "OLEDDisplay.h"

bool OLEDDisplay::init() {
  if (!this->connect()) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Can't establish connection to display\n");
    return false;
  }
  this->buffer = (uint8_t*) malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);
  if(!this->buffer) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create display\n");
    return false;
  }

  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  this->buffer_back = (uint8_t*) malloc(sizeof(uint8_t) * DISPLAY_BUFFER_SIZE);
  if(!this->buffer_back) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][init] Not enough memory to create back buffer\n");
    free(this->buffer);
    return false;
  }
  #endif

  sendInitCommands();
  resetDisplay();

  return true;
}

void OLEDDisplay::end() {
  if (this->buffer) free(this->buffer);
  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  if (this->buffer_back) free(this->buffer_back);
  #endif
}

void OLEDDisplay::resetDisplay(void) {
  clear();
  #ifdef OLEDDISPLAY_DOUBLE_BUFFER
  memset(buffer_back, 1, DISPLAY_BUFFER_SIZE);
  #endif
  display();
}

void OLEDDisplay::setColor(OLEDDISPLAY_COLOR color) {
  this->color = color;
}

void OLEDDisplay::setPixel(int16_t x, int16_t y) {
  if (x >= 0 && x < 128 && y >= 0 && y < 64) {
    switch (color) {
      case WHITE:   buffer[x + (y / 8) * DISPLAY_WIDTH] |=  (1 << (y & 7)); break;
      case BLACK:   buffer[x + (y / 8) * DISPLAY_WIDTH] &= ~(1 << (y & 7)); break;
      case INVERSE: buffer[x + (y / 8) * DISPLAY_WIDTH] ^=  (1 << (y & 7)); break;
    }
  }
}

// Bresenham's algorithm - thx wikipedia and Adafruit_GFX
void OLEDDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      setPixel(y0, x0);
    } else {
      setPixel(x0, y0);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void OLEDDisplay::drawRect(int16_t x, int16_t y, int16_t width, int16_t height) {
  drawHorizontalLine(x, y, width);
  drawVerticalLine(x, y, height);
  drawVerticalLine(x + width - 1, y, height);
  drawHorizontalLine(x, y + height - 1, width);
}

void OLEDDisplay::fillRect(int16_t xMove, int16_t yMove, int16_t width, int16_t height) {
  for (int16_t x = xMove; x < xMove + width; x++) {
    drawVerticalLine(x, yMove, height);
  }
}

void OLEDDisplay::drawCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
	int16_t dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;

		setPixel(x0 + x, y0 + y);     //For the 8 octants
		setPixel(x0 - x, y0 + y);
		setPixel(x0 + x, y0 - y);
		setPixel(x0 - x, y0 - y);
		setPixel(x0 + y, y0 + x);
		setPixel(x0 - y, y0 + x);
		setPixel(x0 + y, y0 - x);
		setPixel(x0 - y, y0 - x);

	} while (x < y);

  setPixel(x0 + radius, y0);
  setPixel(x0, y0 + radius);
  setPixel(x0 - radius, y0);
  setPixel(x0, y0 - radius);
}

void OLEDDisplay::drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads) {
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  while (x < y) {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;
    if (quads & 0x1) {
      setPixel(x0 + x, y0 - y);
      setPixel(x0 + y, y0 - x);
    }
    if (quads & 0x2) {
      setPixel(x0 - y, y0 - x);
      setPixel(x0 - x, y0 - y);
    }
    if (quads & 0x4) {
      setPixel(x0 - y, y0 + x);
      setPixel(x0 - x, y0 + y);
    }
    if (quads & 0x8) {
      setPixel(x0 + x, y0 + y);
      setPixel(x0 + y, y0 + x);
    }
  }
  if (quads & 0x1 && quads & 0x8) {
    setPixel(x0 + radius, y0);
  }
  if (quads & 0x4 && quads & 0x8) {
    setPixel(x0, y0 + radius);
  }
  if (quads & 0x2 && quads & 0x4) {
    setPixel(x0 - radius, y0);
  }
  if (quads & 0x1 && quads & 0x2) {
    setPixel(x0, y0 - radius);
  }
}


void OLEDDisplay::fillCircle(int16_t x0, int16_t y0, int16_t radius) {
  int16_t x = 0, y = radius;
	int16_t dp = 1 - radius;
	do {
		if (dp < 0)
			dp = dp + 2 * (++x) + 3;
		else
			dp = dp + 2 * (++x) - 2 * (--y) + 5;

    drawHorizontalLine(x0 - x, y0 - y, 2*x);
    drawHorizontalLine(x0 - x, y0 + y, 2*x);
    drawHorizontalLine(x0 - y, y0 - x, 2*y);
    drawHorizontalLine(x0 - y, y0 + x, 2*y);


	} while (x < y);
  drawHorizontalLine(x0 - radius, y0, 2 * radius);

}

void OLEDDisplay::drawHorizontalLine(int16_t x, int16_t y, int16_t length) {
  if (y < 0 || y >= DISPLAY_HEIGHT) { return; }

  if (x < 0) {
    length += x;
    x = 0;
  }

  if ( (x + length) > DISPLAY_WIDTH) {
    length = (DISPLAY_WIDTH - x);
  }

  if (length <= 0) { return; }

  uint8_t * bufferPtr = buffer;
  bufferPtr += (y >> 3) * DISPLAY_WIDTH;
  bufferPtr += x;

  uint8_t drawBit = 1 << (y & 7);

  switch (color) {
    case WHITE:   while (length--) {
        *bufferPtr++ |= drawBit;
      }; break;
    case BLACK:   drawBit = ~drawBit;   while (length--) {
        *bufferPtr++ &= drawBit;
      }; break;
    case INVERSE: while (length--) {
        *bufferPtr++ ^= drawBit;
      }; break;
  }
}

void OLEDDisplay::drawVerticalLine(int16_t x, int16_t y, int16_t length) {
  if (x < 0 || x >= DISPLAY_WIDTH) return;

  if (y < 0) {
    length += y;
    y = 0;
  }

  if ( (y + length) > DISPLAY_HEIGHT) {
    length = (DISPLAY_HEIGHT - y);
  }

  if (length <= 0) return;


  uint8_t yOffset = y & 7;
  uint8_t drawBit;
  uint8_t *bufferPtr = buffer;

  bufferPtr += (y >> 3) * DISPLAY_WIDTH;
  bufferPtr += x;

  if (yOffset) {
    yOffset = 8 - yOffset;
    drawBit = ~(0xFF >> (yOffset));

    if (length < yOffset) {
      drawBit &= (0xFF >> (yOffset - length));
    }

    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }

    if (length < yOffset) return;

    length -= yOffset;
    bufferPtr += DISPLAY_WIDTH;
  }

  if (length >= 8) {
    switch (color) {
      case WHITE:
      case BLACK:
        drawBit = (color == WHITE) ? 0xFF : 0x00;
        do {
          *bufferPtr = drawBit;
          bufferPtr += DISPLAY_WIDTH;
          length -= 8;
        } while (length >= 8);
        break;
      case INVERSE:
        do {
          *bufferPtr = ~(*bufferPtr);
          bufferPtr += DISPLAY_WIDTH;
          length -= 8;
        } while (length >= 8);
        break;
    }
  }

  if (length > 0) {
    drawBit = (1 << (length & 7)) - 1;
    switch (color) {
      case WHITE:   *bufferPtr |=  drawBit; break;
      case BLACK:   *bufferPtr &= ~drawBit; break;
      case INVERSE: *bufferPtr ^=  drawBit; break;
    }
  }
}

void OLEDDisplay::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
  uint16_t radius = height / 2;
  uint16_t xRadius = x + radius;
  uint16_t yRadius = y + radius;
  uint16_t doubleRadius = 2 * radius;
  uint16_t innerRadius = radius - 2;

  setColor(WHITE);
  drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
  drawHorizontalLine(xRadius, y, width - doubleRadius + 1);
  drawHorizontalLine(xRadius, y + height, width - doubleRadius + 1);
  drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

  uint16_t maxProgressWidth = (width - doubleRadius - 1) * progress / 100;

  fillCircle(xRadius, yRadius, innerRadius);
  fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
  fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);
}

void OLEDDisplay::drawFastImage(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *image) {
  drawInternal(xMove, yMove, width, height, image, 0, 0);
}

void OLEDDisplay::drawXbm(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *xbm) {
  int16_t widthInXbm = (width + 7) / 8;
  uint8_t data;

  for(int16_t y = 0; y < height; y++) {
    for(int16_t x = 0; x < width; x++ ) {
      if (x & 7) {
        data >>= 1; // Move a bit
      } else {  // Read new data every 8 bit
        data = pgm_read_byte(xbm + (x / 8) + y * widthInXbm);
      }
      // if there is a bit draw it
      if (data & 0x01) {
        setPixel(xMove + x, yMove + y);
      }
    }
  }
}

void OLEDDisplay::drawStringInternal(int16_t xMove, int16_t yMove, char* text, uint16_t textLength, uint16_t textWidth) {
  uint8_t textHeight       = pgm_read_byte(fontData + HEIGHT_POS);
  uint8_t firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t sizeOfJumpTable = pgm_read_byte(fontData + CHAR_NUM_POS)  * JUMPTABLE_BYTES;

  uint8_t cursorX         = 0;
  uint8_t cursorY         = 0;

  switch (textAlignment) {
    case TEXT_ALIGN_CENTER_BOTH:
      yMove -= textHeight >> 1;
    // Fallthrough
    case TEXT_ALIGN_CENTER:
      xMove -= textWidth >> 1; // divide by 2
      break;
    case TEXT_ALIGN_RIGHT:
      xMove -= textWidth;
      break;
  }

  // Don't draw anything if it is not on the screen.
  if (xMove + textWidth  < 0 || xMove > DISPLAY_WIDTH ) {return;}
  if (yMove + textHeight < 0 || yMove > DISPLAY_HEIGHT) {return;}

  for (uint16_t j = 0; j < textLength; j++) {
    int16_t xPos = xMove + cursorX;
    int16_t yPos = yMove + cursorY;

    byte code = text[j];
    if (code >= firstChar) {
      byte charCode = code - firstChar;

      // 4 Bytes per char code
      byte msbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES );                  // MSB  \ JumpAddress
      byte lsbJumpToChar    = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_LSB);   // LSB /
      byte charByteSize     = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_SIZE);  // Size
      byte currentCharWidth = pgm_read_byte( fontData + JUMPTABLE_START + charCode * JUMPTABLE_BYTES + JUMPTABLE_WIDTH); // Width

      // Test if the char is drawable
      if (!(msbJumpToChar == 255 && lsbJumpToChar == 255)) {
        // Get the position of the char data
        uint16_t charDataPosition = JUMPTABLE_START + sizeOfJumpTable + ((msbJumpToChar << 8) + lsbJumpToChar);
        drawInternal(xPos, yPos, currentCharWidth, textHeight, fontData, charDataPosition, charByteSize);
      }

      cursorX += currentCharWidth;
    }
  }
}


void OLEDDisplay::drawString(int16_t xMove, int16_t yMove, String strUser) {
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  // char* text must be freed!
  char* text = utf8ascii(strUser);

  uint16_t yOffset = 0;
  // If the string should be centered vertically too
  // we need to now how heigh the string is.
  if (textAlignment == TEXT_ALIGN_CENTER_BOTH) {
    uint16_t lb = 0;
    // Find number of linebreaks in text
    for (uint16_t i=0;text[i] != 0; i++) {
      lb += (text[i] == 10);
    }
    // Calculate center
    yOffset = (lb * lineHeight) / 2;
  }

  uint16_t line = 0;
  char* textPart = strtok(text,"\n");
  while (textPart != NULL) {
    uint16_t length = strlen(textPart);
    drawStringInternal(xMove, yMove - yOffset + (line++) * lineHeight, textPart, length, getStringWidth(textPart, length));
    textPart = strtok(NULL, "\n");
  }
  free(text);
}

void OLEDDisplay::drawStringMaxWidth(int16_t xMove, int16_t yMove, uint16_t maxLineWidth, String strUser) {
  uint16_t firstChar  = pgm_read_byte(fontData + FIRST_CHAR_POS);
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);

  char* text = utf8ascii(strUser);

  uint16_t length = strlen(text);
  uint16_t lastDrawnPos = 0;
  uint16_t lineNumber = 0;
  uint16_t strWidth = 0;

  uint16_t preferredBreakpoint = 0;
  uint16_t widthAtBreakpoint = 0;

  for (uint16_t i = 0; i < length; i++) {
    strWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[i] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);

    // Always try to break on a space or dash
    if (text[i] == ' ' || text[i]== '-') {
      preferredBreakpoint = i;
      widthAtBreakpoint = strWidth;
    }

    if (strWidth >= maxLineWidth) {
      if (preferredBreakpoint == 0) {
        preferredBreakpoint = i;
        widthAtBreakpoint = strWidth;
      }
      drawStringInternal(xMove, yMove + (lineNumber++) * lineHeight , &text[lastDrawnPos], preferredBreakpoint - lastDrawnPos, widthAtBreakpoint);
      lastDrawnPos = preferredBreakpoint + 1;
      // It is possible that we did not draw all letters to i so we need
      // to account for the width of the chars from `i - preferredBreakpoint`
      // by calculating the width we did not draw yet.
      strWidth = strWidth - widthAtBreakpoint;
      preferredBreakpoint = 0;
    }
  }

  // Draw last part if needed
  if (lastDrawnPos < length) {
    drawStringInternal(xMove, yMove + lineNumber * lineHeight , &text[lastDrawnPos], length - lastDrawnPos, getStringWidth(&text[lastDrawnPos], length - lastDrawnPos));
  }

  free(text);
}

uint16_t OLEDDisplay::getStringWidth(const char* text, uint16_t length) {
  uint16_t firstChar        = pgm_read_byte(fontData + FIRST_CHAR_POS);

  uint16_t stringWidth = 0;
  uint16_t maxWidth = 0;

  while (length--) {
    stringWidth += pgm_read_byte(fontData + JUMPTABLE_START + (text[length] - firstChar) * JUMPTABLE_BYTES + JUMPTABLE_WIDTH);
    if (text[length] == 10) {
      maxWidth = max(maxWidth, stringWidth);
      stringWidth = 0;
    }
  }

  return max(maxWidth, stringWidth);
}

uint16_t OLEDDisplay::getStringWidth(String strUser) {
  char* text = utf8ascii(strUser);
  uint16_t length = strlen(text);
  uint16_t width = getStringWidth(text, length);
  free(text);
  return width;
}

void OLEDDisplay::setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT textAlignment) {
  this->textAlignment = textAlignment;
}

void OLEDDisplay::setFont(const char *fontData) {
  this->fontData = fontData;
}

void OLEDDisplay::displayOn(void) {
  sendCommand(DISPLAYON);
}

void OLEDDisplay::displayOff(void) {
  sendCommand(DISPLAYOFF);
}

void OLEDDisplay::invertDisplay(void) {
  sendCommand(INVERTDISPLAY);
}

void OLEDDisplay::normalDisplay(void) {
  sendCommand(NORMALDISPLAY);
}

void OLEDDisplay::setContrast(char contrast) {
  sendCommand(SETCONTRAST);
  sendCommand(contrast);
}

void OLEDDisplay::flipScreenVertically() {
  sendCommand(SEGREMAP | 0x01);
  sendCommand(COMSCANDEC);           //Rotate screen 180 Deg
}

void OLEDDisplay::clear(void) {
  memset(buffer, 0, DISPLAY_BUFFER_SIZE);
}

void OLEDDisplay::drawLogBuffer(uint16_t xMove, uint16_t yMove) {
  uint16_t lineHeight = pgm_read_byte(fontData + HEIGHT_POS);
  // Always align left
  setTextAlignment(TEXT_ALIGN_LEFT);

  // State values
  uint16_t length   = 0;
  uint16_t line     = 0;
  uint16_t lastPos  = 0;

  for (uint16_t i=0;i<this->logBufferFilled;i++){
    // Everytime we have a \n print
    if (this->logBuffer[i] == 10) {
      length++;
      // Draw string on line `line` from lastPos to length
      // Passing 0 as the lenght because we are in TEXT_ALIGN_LEFT
      drawStringInternal(xMove, yMove + (line++) * lineHeight, &this->logBuffer[lastPos], length, 0);
      // Remember last pos
      lastPos = i;
      // Reset length
      length = 0;
    } else {
      // Count chars until next linebreak
      length++;
    }
  }
  // Draw the remaining string
  if (length > 0) {
    drawStringInternal(xMove, yMove + line * lineHeight, &this->logBuffer[lastPos], length, 0);
  }
}

bool OLEDDisplay::setLogBuffer(uint16_t lines, uint16_t chars){
  if (logBuffer != NULL) free(logBuffer);
  uint16_t size = lines * chars;
  if (size > 0) {
    this->logBufferLine     = 0;      // Lines printed
    this->logBufferMaxLines = lines;  // Lines max printable
    this->logBufferSize     = size;   // Total number of characters the buffer can hold
    this->logBuffer         = (char *) malloc(size * sizeof(uint8_t));
    if(!this->logBuffer) {
      DEBUG_OLEDDISPLAY("[OLEDDISPLAY][setLogBuffer] Not enough memory to create log buffer\n");
      return false;
    }
  }
  return true;
}

size_t OLEDDisplay::write(uint8_t c) {
  if (this->logBufferSize > 0) {
    // Don't waste space on \r\n line endings, dropping \r
    if (c == 13) return 1;

    bool maxLineNotReached = this->logBufferLine < this->logBufferMaxLines;
    bool bufferNotFull = this->logBufferFilled < this->logBufferSize;

    // Can we write to the buffer?
    if (bufferNotFull && maxLineNotReached) {
      this->logBuffer[logBufferFilled] = utf8ascii(c);
      this->logBufferFilled++;
      // Keep track of lines written
      if (c == 10) this->logBufferLine++;
    } else {
      // Max line number is reached
      if (!maxLineNotReached) this->logBufferLine--;

      // Find the end of the first line
      uint16_t firstLineEnd = 0;
      for (uint16_t i=0;i<this->logBufferFilled;i++) {
        if (this->logBuffer[i] == 10){
          // Include last char too
          firstLineEnd = i + 1;
          break;
        }
      }
      // If there was a line ending
      if (firstLineEnd > 0) {
        // Calculate the new logBufferFilled value
        this->logBufferFilled = logBufferFilled - firstLineEnd;
        // Now we move the lines infront of the buffer
        memcpy(this->logBuffer, &this->logBuffer[firstLineEnd], logBufferFilled);
      } else {
        // Let's reuse the buffer if it was full
        if (!bufferNotFull) {
          this->logBufferFilled = 0;
        }// else {
        //  Nothing to do here
        //}
      }
      write(c);
    }
  }
  // We are always writing all uint8_t to the buffer
  return 1;
}

size_t OLEDDisplay::write(const char* str) {
  if (str == NULL) return 0;
  size_t length = strlen(str);
  for (size_t i = 0; i < length; i++) {
    write(str[i]);
  }
  return length;
}

// Private functions
void OLEDDisplay::sendInitCommands(void) {
  sendCommand(DISPLAYOFF);
  sendCommand(SETDISPLAYCLOCKDIV);
  sendCommand(0xF0); // Increase speed of the display max ~96Hz
  sendCommand(SETMULTIPLEX);
  sendCommand(0x3F);
  sendCommand(SETDISPLAYOFFSET);
  sendCommand(0x00);
  sendCommand(SETSTARTLINE);
  sendCommand(CHARGEPUMP);
  sendCommand(0x14);
  sendCommand(MEMORYMODE);
  sendCommand(0x00);
  sendCommand(SEGREMAP);
  sendCommand(COMSCANINC);
  sendCommand(SETCOMPINS);
  sendCommand(0x12);
  sendCommand(SETCONTRAST);
  sendCommand(0xCF);
  sendCommand(SETPRECHARGE);
  sendCommand(0xF1);
  sendCommand(DISPLAYALLON_RESUME);
  sendCommand(NORMALDISPLAY);
  sendCommand(0x2e);            // stop scroll
  sendCommand(DISPLAYON);
}

void inline OLEDDisplay::drawInternal(int16_t xMove, int16_t yMove, int16_t width, int16_t height, const char *data, uint16_t offset, uint16_t bytesInData) {
  if (width < 0 || height < 0) return;
  if (yMove + height < 0 || yMove > DISPLAY_HEIGHT)  return;
  if (xMove + width  < 0 || xMove > DISPLAY_WIDTH)   return;

  uint8_t  rasterHeight = 1 + ((height - 1) >> 3); // fast ceil(height / 8.0)
  int8_t   yOffset      = yMove & 7;

  bytesInData = bytesInData == 0 ? width * rasterHeight : bytesInData;

  int16_t initYMove   = yMove;
  int8_t  initYOffset = yOffset;


  for (uint16_t i = 0; i < bytesInData; i++) {

    // Reset if next horizontal drawing phase is started.
    if ( i % rasterHeight == 0) {
      yMove   = initYMove;
      yOffset = initYOffset;
    }

    byte currentByte = pgm_read_byte(data + offset + i);

    int16_t xPos = xMove + (i / rasterHeight);
    int16_t yPos = ((yMove >> 3) + (i % rasterHeight)) * DISPLAY_WIDTH;

    int16_t yScreenPos = yMove + yOffset;
    int16_t dataPos    = xPos  + yPos;

    if (dataPos >=  0  && dataPos < DISPLAY_BUFFER_SIZE &&
        xPos    >=  0  && xPos    < DISPLAY_WIDTH ) {

      if (yOffset >= 0) {
        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte << yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte << yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte << yOffset; break;
        }
        if (dataPos < (DISPLAY_BUFFER_SIZE - DISPLAY_WIDTH)) {
          switch (this->color) {
            case WHITE:   buffer[dataPos + DISPLAY_WIDTH] |= currentByte >> (8 - yOffset); break;
            case BLACK:   buffer[dataPos + DISPLAY_WIDTH] &= ~(currentByte >> (8 - yOffset)); break;
            case INVERSE: buffer[dataPos + DISPLAY_WIDTH] ^= currentByte >> (8 - yOffset); break;
          }
        }
      } else {
        // Make new offset position
        yOffset = -yOffset;

        switch (this->color) {
          case WHITE:   buffer[dataPos] |= currentByte >> yOffset; break;
          case BLACK:   buffer[dataPos] &= ~(currentByte >> yOffset); break;
          case INVERSE: buffer[dataPos] ^= currentByte >> yOffset; break;
        }

        // Prepare for next iteration by moving one block up
        yMove -= 8;

        // and setting the new yOffset
        yOffset = 8 - yOffset;
      }

      yield();
    }
  }
}

// Code form http://playground.arduino.cc/Main/Utf8ascii
uint8_t OLEDDisplay::utf8ascii(byte ascii) {
  static uint8_t LASTCHAR;

  if ( ascii < 128 ) { // Standard ASCII-set 0..0x7F handling
    LASTCHAR = 0;
    return ascii;
  }

  uint8_t last = LASTCHAR;   // get last char
  LASTCHAR = ascii;

  switch (last) {    // conversion depnding on first UTF8-character
    case 0xC2: return  (ascii);  break;
    case 0xC3: return  (ascii | 0xC0);  break;
    case 0x82: if (ascii == 0xAC) return (0x80);    // special case Euro-symbol
  }

  return  0; // otherwise: return zero, if character has to be ignored
}

// You need to free the char!
char* OLEDDisplay::utf8ascii(String str) {
  uint16_t k = 0;
  uint16_t length = str.length() + 1;

  // Copy the string into a char array
  char* s = (char*) malloc(length * sizeof(char));
  if(!s) {
    DEBUG_OLEDDISPLAY("[OLEDDISPLAY][utf8ascii] Can't allocate another char array. Drop support for UTF-8.\n");
    return (char*) str.c_str();
  }
  str.toCharArray(s, length);

  length--;

  for (uint16_t i=0; i < length; i++) {
    char c = utf8ascii(s[i]);
    if (c!=0) {
      s[k++]=c;
    }
  }

  s[k]=0;

  // This will leak 's' be sure to free it in the calling function.
  return s;
}

#line 1 "c:\\Users\\pspyc\\Documents\\Arduino\\QmuBeaconLocator\\qmu_beacon_locator.ino"
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "SSD1306.h"
#include <QmuTactile.h>
#include <SPI.h>
#include "variables.h"
#include "lora.h"
#include "radio_node.h"
#include "beacons.h"
#include "oled_display.h"
#include "config_node.h"
#include "utils.h"
#include "device_node.h"

#define LORA_TX_POWER 10
#define LORA_BANDWIDTH 500000
#define LORA_SPREADING_FACTOR 7
#define LORA_CODING_RATE 6

//Target
// #define ARDUINO_TTGO_TBEAM_ESP32

#ifdef ARDUINO_TTGO_TBEAM_ESP32
    #define LORA_SS_PIN     18
    #define LORA_RST_PIN    23
    #define LORA_DI0_PIN    26

    #define SPI_SCK_PIN     5
    #define SPI_MISO_PIN    19
    #define SPI_MOSI_PIN    27

    #define PIN_BUTTON_L 38
    // #define PIN_BUTTON_R 0

    #define I2C_SDA_PIN 21
    #define I2C_SCL_PIN 22

    #define GPS_SERIAL_TX_PIN 12
    #define GPS_SERIAL_RX_PIN 34
#elif defined(ARDUINO_ESP32_DEV)
    #define LORA_SS_PIN     18
    #define LORA_RST_PIN    14
    #define LORA_DI0_PIN    26

    #define SPI_SCK_PIN     5
    #define SPI_MISO_PIN    19
    #define SPI_MOSI_PIN    27

    #define PIN_BUTTON_L 4
    #define PIN_BUTTON_R 0

    #define I2C_SDA_PIN 21
    #define I2C_SCL_PIN 22

    #define GPS_SERIAL_TX_PIN 15
    #define GPS_SERIAL_RX_PIN 13
#else
    #error please select hardware
#endif

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
SSD1306  display(0x3c, I2C_SDA_PIN, I2C_SCL_PIN);
OledDisplay oledDisplay(&display);

QmuTactile buttonMain(PIN_BUTTON_L);

#ifdef PIN_BUTTON_R
QmuTactile buttonR(PIN_BUTTON_R);
#endif

RadioNode radioNode;
QspConfiguration_t qsp = {};
Beacons beacons;

ConfigNode configNode;

#define TASK_SERIAL_RATE 500
#define TASK_LORA_READ 2 // We check for new packets only from time to time, no need to do it more often
#define TASK_LORA_TX_MS 200 // Number of ms between positio updates

DeviceNode deviceNode(TASK_LORA_TX_MS);

uint32_t nextSerialTaskTs = 0;
uint32_t nextLoRaReadTaskTs = 0;

void onQspSuccess(uint8_t receivedChannel) {
    //If recide received a valid frame, that means it can start to talk
    radioNode.lastReceivedChannel = receivedChannel;
    
    radioNode.readRssi();
    radioNode.readSnr();

    uint32_t beaconId = qsp.payload[3] << 24;
    beaconId += qsp.payload[2] << 16;
    beaconId += qsp.payload[1] << 8;
    beaconId += qsp.payload[0];
    
    // Serial.print("Beacon="); Serial.println(beaconId);

    Beacon *beacon = beacons.getBeacon(beaconId);

    /*
     * Set common beacon attributes
     */
    beacon->setRssi(radioNode.rssi);
    beacon->setSnr(radioNode.snr);
    beacon->setLastContactMillis(millis());

    if (qsp.frameId == QSP_FRAME_COORDS) {
        long tmp;

        //We have a valid position data
        if (qsp.payload[19] & POSITION_FLAG_POSITION_VALID) {
            tmp = qsp.payload[4];
            tmp += qsp.payload[5] << 8;
            tmp += qsp.payload[6] << 16;
            tmp += qsp.payload[7] << 24;

            beacon->setLat(tmp / 10000000.0d);

            tmp = qsp.payload[8];
            tmp += qsp.payload[9] << 8;
            tmp += qsp.payload[10] << 16;
            tmp += qsp.payload[11] << 24;

            beacon->setLon(tmp / 10000000.0d);
        }

        if (qsp.payload[19] & POSITION_FLAG_ALTITUDE_VALID) {
            tmp = qsp.payload[12];
            tmp += qsp.payload[13] << 8;
            tmp += qsp.payload[14] << 16;
            tmp += qsp.payload[15] << 24;

            beacon->setAlt(tmp / 100.0d);
        }

        beacon->setAction(qsp.payload[18]);

        //TODO fill flags
        //TODO fill course        
    }
}

void onQspFailure() {

}

void setup()
{
    Serial.begin(115200);
	SerialGPS.begin(9600, SERIAL_8N1, GPS_SERIAL_RX_PIN, GPS_SERIAL_TX_PIN);

    configNode.begin();
    randomSeed(analogRead(A4));
    configNode.seed();
    configNode.beaconId = configNode.loadBeaconId();

    buttonMain.start();
#ifdef PIN_BUTTON_R
    buttonR.start();
#endif

    qsp.onSuccessCallback = onQspSuccess;
    qsp.onFailureCallback = onQspFailure;

    radioNode.configure(
        LORA_TX_POWER, 
        LORA_BANDWIDTH, 
        LORA_SPREADING_FACTOR, 
        LORA_CODING_RATE
    );

    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, LORA_SS_PIN);
    LoRa.setSPIFrequency(4E6);
    radioNode.init(LORA_SS_PIN, LORA_RST_PIN, LORA_DI0_PIN, NULL);
    radioNode.reset();
    radioNode.canTransmit = true;
    LoRa.receive();

    oledDisplay.init();
    oledDisplay.setPage(OLED_PAGE_BEACON_LIST);
}

void loop()
{
    radioNode.handleTxDoneState(false);

    if (radioNode.radioState != RADIO_STATE_TX && nextLoRaReadTaskTs < millis()) {
        int packetSize = LoRa.parsePacket();
        if (packetSize) {
            radioNode.bytesToRead = packetSize;
            radioNode.readAndDecode(&qsp);
        }

        nextLoRaReadTaskTs = millis() + TASK_LORA_READ;
    }

    /*
     * Watchdog for frame decoding stuck somewhere in the middle of a process
     */
    if (
        qsp.protocolState != QSP_STATE_IDLE &&
        qsp.frameDecodingStartedAt + QSP_MAX_FRAME_DECODE_TIME < millis()
    ) {
        qsp.protocolState = QSP_STATE_IDLE;
    }

    buttonMain.loop();

#ifdef PIN_BUTTON_R
    buttonR.loop();
#endif

    //TODO remove when ready
    if (SerialGPS.available()) {
        while (SerialGPS.available() > 0) {
            gps.encode(SerialGPS.read());
        }
    }

    deviceNode.execute();
    deviceNode.processInputs();

    if (nextSerialTaskTs < millis()) {
        // Serial.println(configNode.beaconId);
        // Serial.println(gps.altitude.meters());
        if (beacons.currentBeaconIndex >= 0) {
            // Beacon *beacon = beacons.get(beacons.currentBeaconIndex);
            // Serial.print("LAT=");  Serial.println(beacon->getLat(), 6);
            // Serial.print("LONG="); Serial.println(beacon->getLon(), 6);
            // Serial.print("ALT=");  Serial.println(beacon->getAlt());
            // Serial.print("RSSI=");  Serial.println(beacon->getRssi());
            // Serial.print("SNR=");  Serial.println(beacon->getSnr());
        }
        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }

    if (beacons.currentBeaconIndex == -1 && beacons.count() > 0) {
        beacons.currentBeaconIndex = 0;
        beacons.currentBeaconId = beacons.get(beacons.currentBeaconIndex)->getId();
    }

    oledDisplay.loop();
}

