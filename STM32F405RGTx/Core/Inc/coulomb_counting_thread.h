#include "util.h"

class CoulombCountingThread {
    public:
        CoulombCountingThread();

        float getCharge();
    private:
        RTOSThread thread;
        void runCoulombCounting(void* args);

        float prev_current;
};