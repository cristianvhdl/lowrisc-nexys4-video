open_vcd /local/scratch/$::env(USER)/lowrisc-chip.vcd
log_vcd -level 10
start_vcd
#run 200 us
run -all
stop_vcd
quit
