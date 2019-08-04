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
#include "HcmsDisplay.h"
#include <hal/DirectOutputPin.h>

static constexpr uint8_t HCMS_SLEEP_BITMASK   = (0b01000000);

static constexpr uint8_t HCMS_CURRENT_BITMASK = (0b00110000);
static constexpr uint8_t HCMS_CURRENT_31      = (0b00100000);
static constexpr uint8_t HCMS_CURRENT_50      = (0b00010000);
static constexpr uint8_t HCMS_CURRENT_73      = (0b00000000);
static constexpr uint8_t HCMS_CURRENT_100     = (0b00110000);
static constexpr uint8_t HCMS_CURRENT_VALUES[] = {HCMS_CURRENT_31, HCMS_CURRENT_50, HCMS_CURRENT_73, HCMS_CURRENT_100};
static constexpr uint8_t HCMS_CURRENT_MAXVALUE = 3;

static constexpr uint8_t HCMS_BRIGHTNESS_BITMASK = (0b00001111);
static constexpr uint8_t HCMS_BRIGHTNESS_00  = (0b00000000);
static constexpr uint8_t HCMS_BRIGHTNESS_02  = (0b00000001);
static constexpr uint8_t HCMS_BRIGHTNESS_03  = (0b00000010);
static constexpr uint8_t HCMS_BRIGHTNESS_05  = (0b00000011);
static constexpr uint8_t HCMS_BRIGHTNESS_07  = (0b00000100);
static constexpr uint8_t HCMS_BRIGHTNESS_08  = (0b00000101);
static constexpr uint8_t HCMS_BRIGHTNESS_12  = (0b00000110);
static constexpr uint8_t HCMS_BRIGHTNESS_15  = (0b00000111);
static constexpr uint8_t HCMS_BRIGHTNESS_18  = (0b00001000);
static constexpr uint8_t HCMS_BRIGHTNESS_23  = (0b00001001);
static constexpr uint8_t HCMS_BRIGHTNESS_30  = (0b00001010);
static constexpr uint8_t HCMS_BRIGHTNESS_37  = (0b00001011);
static constexpr uint8_t HCMS_BRIGHTNESS_47  = (0b00001100);
static constexpr uint8_t HCMS_BRIGHTNESS_60  = (0b00001101);
static constexpr uint8_t HCMS_BRIGHTNESS_80  = (0b00001110);
static constexpr uint8_t HCMS_BRIGHTNESS_100 = (0b00001111);
static constexpr uint8_t HCMS_BRIGHTNESS_VALUES[] = {
        HCMS_BRIGHTNESS_00, HCMS_BRIGHTNESS_02, HCMS_BRIGHTNESS_03, HCMS_BRIGHTNESS_05,
        HCMS_BRIGHTNESS_07, HCMS_BRIGHTNESS_08, HCMS_BRIGHTNESS_12, HCMS_BRIGHTNESS_15,
        HCMS_BRIGHTNESS_18, HCMS_BRIGHTNESS_23, HCMS_BRIGHTNESS_30, HCMS_BRIGHTNESS_37,
        HCMS_BRIGHTNESS_47, HCMS_BRIGHTNESS_60, HCMS_BRIGHTNESS_80, HCMS_BRIGHTNESS_100
};
static constexpr uint8_t HCMS_BRIGHTNESS_MAXVALUE = 15;

HcmsDisplay::HcmsDisplay(OutputPin& dataPin, OutputPin& registerSelectPin, OutputPin& clockPin,
                         OutputPin& chipEnablePin, OutputPin& resetPin, uint8_t displayCount, uint8_t displayLength,
                         const char * const font) :
         _dataPin(dataPin), _registerSelectPin(registerSelectPin), _clockPin(clockPin),
         _chipEnablePin(chipEnablePin), _resetPin(resetPin), _displayCount(displayCount),
         _displayLength(displayLength), _font(font)
{
}

HcmsDisplay::HcmsDisplay(uint8_t dataPin, uint8_t registerSelectPin, uint8_t clockPin, uint8_t chipEnablePin,
                         uint8_t resetPin, uint8_t displayCount, uint8_t displayLength, const char * const font) :
        _dataPin(*(new DirectOutputPin(dataPin))),
        _registerSelectPin(*(new DirectOutputPin(registerSelectPin))),
        _clockPin(*(new DirectOutputPin(clockPin))),
        _chipEnablePin(*(new DirectOutputPin(chipEnablePin))),
        _resetPin(*(new DirectOutputPin(resetPin))), _displayCount(displayCount),
        _displayLength(displayLength), _font(font)
{
}

void HcmsDisplay::begin() {
    // Rest the display && setup pin states
    _resetPin.clear();
    _chipEnablePin.set();
    _clockPin.clear();
    delay(10);
    _resetPin.set();

    // Setup control register to chip defaults
    setSleep(false);
    setPeakCurrent(2);
    setBrightness(15);
    clear();

    _controlWordDirty = false;
    _displayDataDirty = false;
}

void HcmsDisplay::shiftOut(uint8_t data) {
    _dataPin.setState((data & 0b10000000) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b01000000) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b00100000) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b00010000) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b00001000) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b00000100) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b00000010) != 0); _clockPin.set(); _clockPin.clear();
    _dataPin.setState((data & 0b00000001) != 0); _clockPin.set(); _clockPin.clear();
}

void HcmsDisplay::writeControlRegister(uint8_t data) {
    for (uint8_t i = 0; i<_displayCount; i++) {
        _registerSelectPin.set();
        _chipEnablePin.clear();
        shiftOut(data);
        _chipEnablePin.set();
    }
}

void HcmsDisplay::writePixelMap() {
    _registerSelectPin.clear();
    _chipEnablePin.clear();
    for (uint8_t i = 0; i < (_displayLength * 5); i++) {
        shiftOut(_displayData[i]);
    }
    _chipEnablePin.set();
}

void HcmsDisplay::setSleep(bool sleep) {
    if (isSleep() != sleep) {
        if (sleep) {
            _controlWord0 &= ~HCMS_SLEEP_BITMASK;
        } else {
            _controlWord0 |= HCMS_SLEEP_BITMASK;
        }
        _controlWordDirty = true;
    }
}

bool HcmsDisplay::isSleep() {
    return (_controlWord0 & HCMS_SLEEP_BITMASK) == 0;
}

void HcmsDisplay::setPeakCurrent(uint8_t limit) {
    if (limit > HCMS_CURRENT_MAXVALUE) {
        limit = HCMS_CURRENT_MAXVALUE;
    }

    _controlWord0 &= ~HCMS_CURRENT_BITMASK;
    _controlWord0 |= HCMS_CURRENT_VALUES[limit];
    _controlWordDirty = true;
}

void HcmsDisplay::setBrightness(uint8_t brightness) {
    if (brightness > HCMS_BRIGHTNESS_MAXVALUE) {
        brightness = HCMS_BRIGHTNESS_MAXVALUE;
    }

    _controlWord0 &= ~HCMS_BRIGHTNESS_BITMASK;
    _controlWord0 |= HCMS_BRIGHTNESS_VALUES[brightness];
    _controlWordDirty = true;
}

void HcmsDisplay::clear() {
    memset(_displayData, 0, HCSM_DISPLAY_MAX_LENGTH * 5);
    _displayDataDirty = true;
    _position = 0;
}

uint8_t HcmsDisplay::getPosition() {
    return _position;
}

void HcmsDisplay::setPosition(uint8_t position) {
    _position = (position >= _displayLength) ? (_displayLength - 1) : position;
}

size_t HcmsDisplay::write(uint8_t character) {
    if (_position < _displayLength) {
        void * displayDataAddress = &_displayData[_position * 5];
        if ((character < 0x20) || (character > 0x7e)) { character = 0x7e; }
        const void * fontDataAddress = &_font[(character - 0x20) * 5];
        memcpy_P(displayDataAddress, fontDataAddress, 5);
        _position++;
        _displayDataDirty = true;
    }
    return 1;
}

void HcmsDisplay::onDcsBiosFrameSync() {
    update();
}

void HcmsDisplay::update() {
    if (_controlWordDirty) {
        writeControlRegister(_controlWord0);
        _controlWordDirty = false;
    }

    if (_displayDataDirty) {
        writePixelMap();
        _displayDataDirty = false;
    }
}
