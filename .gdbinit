source ~/.gdbinit

target extended-remote :3333

file build/main.elf
load

b main
b bms_entry
# b STM32F405RGTx/Core/Src/rtos_threads/measurements_thread.c:89
b StartStateMachine
# b HAL_CAN_RxFifo0MsgPendingCallback
# b coulomb_counting_thread_fn

