#ifndef __UTYPES_H__
#define __UTYPES_H__

#include "soes/cc.h"
#include "RUR_ECATBoard_Base/board/EtherCATConverter.hpp"

/* Object dictionary storage */

typedef struct

{
    /* Inputs */
    struct
    {
        uint8_t Button1;
    } Buttons;

    /* Outputs */
    struct
    {
        uint8_t LED1;
        uint8_t LED2;
        uint8_t LED3;
    } LEDs;

    /* Parameters */
    struct
    {
        uint32_t Multiplier;
    } Parameters;

    /* Manufacturer specific data */

    /* Dynamic TX PDO:s */
    board::ethercat_converter::Outputs outputs;

    /* Dynamic RX PDO:s */
    board::ethercat_converter::Inputs inputs;

    /* Sync Managers */

} _Objects;

extern _Objects Obj;

#endif /* __UTYPES_H__ */
