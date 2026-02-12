#pragma once

#include "board.hpp"

class BLDCMotorController {

public:
    void config();

    void enableDriver();
    void disableDriver();

    

private:
    void controlTask();

private:

};
