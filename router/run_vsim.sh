#!/bin/sh
rm -r work
rm vsim.wlf

vlib work
vlog *.v
#GUI full debug
#vsim tb -voptargs="+acc"

#Fast
vsim -c -do "run -all" CONNECT_testbench_sample
