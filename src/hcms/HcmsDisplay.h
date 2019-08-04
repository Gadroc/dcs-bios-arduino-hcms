/*
	Copyright 2019 Craig Courtney

    This file is part of DcsBios-Firmware.

    DcsBios-Firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DcsBios-Firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DcsBios-Firmware.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DCSBIOS_HCMS_DISPLAY_H
#define DCSBIOS_HCMS_DISPLAY_H

#include <Arduino.h>
#include <hal/OutputPin.h>
#include <dcs/ExportStreamListener.h>

#define HCSM_DISPLAY_MAX_LENGTH 32

class HcmsDisplay : public Print, ExportStreamListener {
private:
    const uint8_t _displayCount;       // Number of displays daisy chained
    const uint8_t _displayLength;      // Total number of characters across all daisy chained displays

    const char * const _font;

    uint8_t _position;
    uint8_t _displayData[HCSM_DISPLAY_MAX_LENGTH * 5];
    bool    _displayDataDirty;
    uint8_t _controlWord0;
    bool    _controlWordDirty;

    OutputPin& _dataPin;
    OutputPin& _registerSelectPin;
    OutputPin& _clockPin;
    OutputPin& _chipEnablePin;
    OutputPin& _resetPin;

    // Shifts out a byte to the display
    void shiftOut(uint8_t data);

    // Writes out the control register
    void writeControlRegister(uint8_t data);

    void writePixelMap();

    // Listen for sync to update display
    void onDcsBiosFrameSync() override;

public:
    HcmsDisplay(
            OutputPin& dataPin, OutputPin& registerSelectPin, OutputPin& clockPin,
            OutputPin& chipEnablePin, OutputPin& resetPin,
            uint8_t displayCount, uint8_t displayLength, const char * font);

    HcmsDisplay(
            uint8_t dataPin, uint8_t registerSelectPin, uint8_t clockPin,
            uint8_t chipEnablePin, uint8_t resetPin,
            uint8_t displayCount, uint8_t displayLength, const char * font);

    // Initialize the displays
    void begin();

    // Gets the current position for writes to the display data
    uint8_t getPosition();

    // Sets the current position for writes to the display data
    void setPosition(uint8_t position);

    // Writes a character at the current position
    size_t write(uint8_t) override;

    // Clears all data from the display
    void clear();

    void setSleep(bool sleep);
    bool isSleep();

    void setPeakCurrent(uint8_t limit);

    void setBrightness(uint8_t brightness);

    // Sends all changes to the display
    void update();
};

#endif //DCSBIOS_HCMS_DISPLAY_H
