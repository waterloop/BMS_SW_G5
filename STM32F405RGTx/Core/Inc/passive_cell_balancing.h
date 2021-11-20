#define OV_BOUNDS 3.7    // upper bounds of cell voltage -- overvoltage limit
#define UV_BOUNDS 2.7    //  lower bounds of cell voltage -- undervoltage limit
#define TARGET_VOLTAGE 3.6   // target cell voltage

void ovCheck ();
void uvCheck ();

void ovBalance (uint32_t overchargedCells);
void uvBalance (uint32_t overchargedCells);

