#source ~/.gdbinit

target extended-remote :3333

file build/main.elf
load

b main
b bms_entry
b BistThread::runBist
# b MeasurementsThread::runMeasurements
# b StateMachineThread::runStateMachine
