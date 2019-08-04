//
// Created by craig on 8/4/2019.
//

#include "HcmsBrightness.h"

HcmsBrightness::HcmsBrightness(unsigned int address, HcmsDisplay &display) :
    _display(display), _address(address)
{}

void HcmsBrightness::onDcsBiosWrite(unsigned int address, unsigned int value) {
    if (address == _address) {
        _display.setBrightness(map(value, 0, 65535, 0, 15));
    }
}
