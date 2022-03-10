#pragma once

#include "util.hpp"

class BistThread {
    public:
        static void initialize();

    protected:
        static RTOSThread thread_;
        static void runBist(void* args);

        /*
            prompt  -> string to prompt user for input
            buff    -> buffer to receive input
            len     -> length of buffer, this value will be changed to
                       the length of the user inputted string on return
        */
        static void _sinput(uint8_t* prompt, uint8_t* buff, uint32_t* len);
        static void _print(uint8_t* str);

        // print measurements
        static void _p_measurements();
};

