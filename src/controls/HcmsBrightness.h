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
#ifndef DCSBIOS_HCMS_BRIGHTNESS_H
#define DCSBIOS_HCMS_BRIGHTNESS_H

#include <dcs/ExportStreamListener.h>
#include "hcms/HcmsDisplay.h"

class HcmsBrightness : ExportStreamListener {
private:
    const unsigned int _address;
    HcmsDisplay&       _display;

public:
    HcmsBrightness(unsigned int address, HcmsDisplay& display);

    void onDcsBiosWrite(unsigned int address, unsigned int value) override;
};

#endif //DCSBIOS_HCMS_BRIGHTNESS_H
