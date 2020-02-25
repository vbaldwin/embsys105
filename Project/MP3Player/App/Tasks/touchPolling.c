#include "tasks.h"

#include "bsp.h"
#include "print.h"

#include "Buttons.h"
#include "InputCommands.h"

#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>

#define PENRADIUS 3

//Globals
extern OS_FLAG_GRP *rxFlags;       // Event flags for synchronizing mailbox messages
extern OS_EVENT * touch2CmdHandler;
extern INPUT_COMMAND commandPressed[1];

Adafruit_FT6206 touchCtrl = Adafruit_FT6206(); // The touch controller

static long MapTouchToScreen(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static uint8_t checkForButtonPress(const TS_Point p, const size_t index)
{
    if(p.x >= btn_array[index].x && p.x <= (btn_array[index].x + btn_array[index].w)) {
        if(p.y >= btn_array[index].y && p.y <= (btn_array[index].y + btn_array[index].h)) {
            commandPressed[0] = (INPUT_COMMAND)index;
            uint8_t err = OSMboxPostOpt(touch2CmdHandler, commandPressed, OS_POST_OPT_NONE);
            if(OS_ERR_NONE != err) {
                PrintFormattedString("TouchPollingTask: failed to post touch2CmdHandler with error %d\n", (INT32U)err);
            }
            return 1;
        }
    }
    return 0;
}

/************************************************************************************

Runs LCD/Touch demo code

************************************************************************************/
void TouchPollingTask(void* pData)
{
	PrintFormattedString("TouchPollingTask: starting\n");

    PrintFormattedString("Initializing FT6206 touchscreen controller\n");
    HANDLE hI2C1 = Open(PJDF_DEVICE_ID_I2C1, 0);
    PrintFormattedString("I2C1 handle opened\n");
    if(!PJDF_IS_VALID_HANDLE(hI2C1)) {
        PrintFormattedString("##! I2C1 PJDF HANDLE IS INVALID!!\nABORTING!\n\n");
        while(1);
    }

    touchCtrl.setPjdfHandle(hI2C1);
    PrintFormattedString("I2C handle set in touchCtrl\n");
    if (! touchCtrl.begin(40)) {  // pass in 'sensitivity' coefficient
        PrintFormattedString("Couldn't start FT6206 touchscreen controller\n");
        while (1);
    }
    else {
        INT8U err;
        OSFlagPost(rxFlags, 2, OS_FLAG_SET, &err);
        if(OS_ERR_NONE != err) {
            PrintFormattedString("TouchPollingTask: posting to flag group with error code %d\n", (INT32U)err);
        }
    }

    uint32_t output = 1;
    const uint32_t delayTicks = OS_TICKS_PER_SEC / 12;
    TS_Point rawPoint;
    TS_Point p;

    while (1) {
        boolean touched = false;

        if(touchCtrl.touched()) {
            touched = true;
        }
        else {
            output = 1;
        }

        if(!touched) {
            OSTimeDly(delayTicks);
            continue;
        }

        rawPoint = touchCtrl.getPoint();

        if (rawPoint.x == 0 && rawPoint.y == 0)
        {
            continue; // usually spurious, so ignore
        }

        // transform touch orientation to screen orientation.
        p = TS_Point();
        p.x = MapTouchToScreen(rawPoint.x, 0, ILI9341_TFTWIDTH, ILI9341_TFTWIDTH, 0);
        p.y = MapTouchToScreen(rawPoint.y, 0, ILI9341_TFTHEIGHT, ILI9341_TFTHEIGHT, 0);

        while(touchCtrl.touched()) {
            OSTimeDly(delayTicks);
        }

        if(1 == touched && 1 == output) {
            output = 0;

            uint8_t hit = 0;

            size_t i = 0;
            while(0 == hit && i < btn_array_sz) {
                hit = checkForButtonPress(p, i++);
            }
        }
    }
}