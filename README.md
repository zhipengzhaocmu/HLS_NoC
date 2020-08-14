# Using Vivado-HLS for Structural Design: a NoC Case Study
There have been ample successful examples of applying Xilinx Vivado's “function-to-module” high-level synthesis (HLS) where the subject is algorithmic in nature. In CONNECT-HLS project, we carried out a design study to assess the effectiveness of applying Vivado-HLS in structural design, where precise bit- and cycle- level control is a must. We succeeded in using Vivado-HLS to produce router and NoC modules that are exact cycle- and bit-accurate replacements of our reference CONNECT RTL-based router and NoC modules. The routers and NoCs resulting from HLS and RTL are comparable in resource utilization and critical path delay. Our experience subjectively suggests that HLS is able to simplify the design effort even though much of the structural details had to be provided in the HLS description through a combination of coding discipline and explicit pragmas.

For more details, please see our technical report, poster and source code. The technical report is also avaiable in [arXiv](https://arxiv.org/abs/1710.10290). A poster with the same title was presented at the [2017 International Symposium on Field Programmable Gate Arrays](https://dl.acm.org/doi/10.1145/3020078.3021772). Please contact Zhipeng Zhao at zzhao1@andrew.cmu.edu for any questions or bug reports. We provide the C++ source code for both illustrative examples in technical report and a fully parameterized router. The code is released under MIT License.

# Router Vivado-HLS source code
This project offers Vivado-HLS C++ source code for CONNECT fully parameterized router REPLACEMENTS and supporting scripts.

# Prerequisite tools
Vivado-HLS - Compile the C++ source code to RTL. <br />
VCS simulator - The default RTL simulation tool used by our script. However, one could use other RTL simulation tools. 

## Steps
1. Download this git repo.
2. Generate a NoC from [CONNECT website](http://users.ece.cmu.edu/~mpapamic/connect/).
3. Download the generated NoC and untar it. Move the `build` directory to `HLS_NoC/router`.
4. run `./run.sh`. The router C++ code will be synthesized into verilog by Vivado-HLS. The original CONNECT network level will be kept, while the router module will be replaced by the HLS-generated router module. This test will run the CONNECT-provided bare bones testbench. The working directory is `build_hls`.

## Files Description
**arbiter.h** - Arbiter object. Input and Output is a SIZE-width Vector. Only one of the SIZE requests is granted. <br />
**alloc.h** - Allocator object, containing two stage of arbiters. The Input is a N-M matrix, Output is a M-N matrix. <br />
**flit_buffer.h** - Buffer object. Buffer the incoming flits. <br />
**router.h** - Router object, containing buffers and allocator. Forward the incoming flits to proper output port while resovling their potential conflicts. <br />
**router_class.cpp** - The top-level function to be synthesized into router module. Vivado-HLS required that the top-level must be a function. <br />
**para.h** - Parameters and datatypes. <br />
**update_para.py** - Update `para.h` based on `connect_parameters.v` provided by CONNECT. Create `config` for generating wrappers. <br />
**gen_iq_wrapper.py** - Generate wrapper for IQ Router (router_type: iq; flow_control_type: credit). <br />
**gen_iqsimple_wrapper.py** - Generate wrapper for IQSimple Router (router_type: iq; flow_control_type: peek). <br />
**gen_vc_wrapper.py** - Generate wrapper for VC Router (router_type: vc; flow_control_type: credit). <br />
**gen_vc_simplewrapper.py** - Generate wrapper for VCSimple Router (router_type: vc; flow_control_type: peek). <br />
**gen_voqsimple_wrapper.py** - Generate wrapper for VOQSimple Router (router_type: voq; flow_control_type: peek). <br />
**vivado_hls.tcl** - Tcl script for running Vivado-HLS C-Synthesize. <br />
**run.sh** - The main scripts. 
- Update the `para.h` based on CONNECT parameters. 
- Create working directory `build_hls` and copy necessary verilog code from `build` into it. 
- Generate proper wrapper according to router and flow control type. 
- Run Vivado-HLS and copy HLS-generated router module verilog code into `build_hls`. 
- Run CONNECT-provided bare bones testbench.
