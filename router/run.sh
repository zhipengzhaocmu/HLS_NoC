#!/usr/bin/env bash

#	Copyright (c) <2017> <Zhipeng Zhao, Carnegie Mellon University>

#	Permission is hereby granted, free of charge, to any person obtaining a copy
#	of this software and associated documentation files (the "Software"), to deal
#	in the Software without restriction, including without limitation the rights
#	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#	copies of the Software, and to permit persons to whom the Software is
#	furnished to do so, subject to the following conditions:

#	The above copyright notice and this permission notice shall be included in all
#	copies or substantial portions of the Software.

#	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#	SOFTWARE.
	
#update parameters
./update_para.py

#get the config
router_type=$(grep "router_type" config | awk -F ': ' '{print $2}')
flow_control_type=$(grep "flow_control_type" config | awk -F ': ' '{print $2}')

#echo "router_type:" $router_type
#echo "flow_control_type:" $flow_control_type

#copy network files into build_hls
if [ -e "build_hls" ];then
	rm -rf build_hls	
fi
mkdir build_hls
cp ./build/*.hex ./build_hls/
cp ./build/mkNetwork* ./build_hls/
cp ./build/RegFileLoadSyn.v ./build_hls/
cp ./build/run* ./build_hls/
cp ./build/testbench_sample* ./build_hls/
cp ./build/connect_parameters.v ./build_hls/

#copy wrapper into build_hls
if [ $router_type = "IQ" ]; then
	if [ $flow_control_type = "PEEK" ];then
		./gen_iqsimple_wrapper.py
		cp mkIQRouterCoreSimple.v ./build_hls/
	else
		./gen_iq_wrapper.py
		cp mkIQRouterCore.v ./build_hls/
	fi
elif [ $router_type = "VC" ]; then
	if [ $flow_control_type = "PEEK" ];then
		./gen_vcsimple_wrapper.py
		cp mkRouterCoreSimple.v ./build_hls/
	else
		./gen_vc_wrapper.py
		cp mkRouterCore.v ./build_hls/
	fi
elif [ $router_type = "VOQ" ]; then
	./gen_voq_wrapper.py
	cp mkVOQRouterCoreSimple.v ./build_hls/
fi

#run vivado-hls C synthesis
if [ -e "hls_project" ];then
	rm -rf hls_project
fi
vivado_hls -f vivado_hls.tcl

#copy generated verilog code into build_hls
cp ./hls_project/solution1/syn/verilog/* ./build_hls/

#run sim
cd build_hls
./run_vcs.sh


