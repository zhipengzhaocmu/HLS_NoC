#!/usr/bin/env python

'''
Copyright (c) <2017> <Zhipeng Zhao, Carnegie Mellon University>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''

#create mkVOQRouterCoreSimple.v, a wrapper for router_top module.
with open('./config','r') as f:
	for line in f:
		if ("num_in_ports: " in line):
			x = line[:-1]
			a,b = x.split(" ")
			in_ports = int(b)
			#print in_ports
		if ("num_out_ports: " in line):
			x = line[:-1]
			a,b = x.split(" ")
			out_ports = int(b)
			#print out_ports
		if ("flit_width: " in line):
			x = line[:-1]
			a,b = x.split(" ")
			flit_width = int(b)
			#print flit_width
		if ("log2m: " in line):
			x = line[:-1]
			a,b = x.split(" ")
			log2m = int(b)
			#print log2m
RT_w = flit_width + log2m - 1;
flit_w = flit_width - 1;
			
output_file = 'mkVOQRouterCoreSimple.v'
with open(output_file,'w') as f:

	f.write('module mkVOQRouterCoreSimple(CLK,\n')
	f.write('\t' + 'RST_N,\n')
	f.write('\n')
	
	#for in_ports
	for i in range(0,in_ports):
		f.write('\t'+'in_ports_'+`i`+'_putRoutedFlit_flit_in,\n')
		f.write('\t'+'EN_in_ports_'+`i`+'_putRoutedFlit,\n')	
		f.write('\n')
		f.write('\t'+'EN_in_ports_'+`i`+'_getNonFullVCs,\n')
		f.write('\t'+'in_ports_'+`i`+'_getNonFullVCs,\n')	
		f.write('\n')	
		
	#for out_ports
	for i in range(0,out_ports):
		f.write('\t'+'EN_out_ports_'+`i`+'_getFlit,\n')
		f.write('\t'+'out_ports_'+`i`+'_getFlit,\n')	
		f.write('\n')
		f.write('\t'+'out_ports_'+`i`+'_putNonFullVCs_nonFullVCs,\n')
		if (i == out_ports-1):
			f.write('\t'+'EN_out_ports_'+`i`+'_putNonFullVCs);\n')	
		else:
			f.write('\t'+'EN_out_ports_'+`i`+'_putNonFullVCs,\n')	
		f.write('\n')	

	f.write('\t' + 'input  CLK;\n')		
	f.write('\t' + 'input  RST_N;\n')		
	f.write('\n')		

	#for in_ports
	for i in range(0,in_ports):
		f.write('\t'+'// action method in_ports_'+`i`+'_putRoutedFlit\n')
		f.write('\t'+'input  ['+`RT_w`+' : 0] in_ports_'+`i`+'_putRoutedFlit_flit_in;\n')	
		f.write('\t'+'input  EN_in_ports_'+`i`+'_putRoutedFlit;\n')
		f.write('\n')			
		f.write('\t'+'// actionvalue method in_ports_'+`i`+'_getNonFullVCs\n')
		f.write('\t'+'input  EN_in_ports_'+`i`+'_getNonFullVCs;\n')	
		f.write('\t'+'output in_ports_'+`i`+'_getNonFullVCs;\n')	
		f.write('\n')

	#for out_ports
	for i in range(0,out_ports):
		f.write('\t'+'// actionvalue method out_ports_'+`i`+'_getFlit\n')
		f.write('\t'+'input  EN_out_ports_'+`i`+'_getFlit;\n')	
		f.write('\t'+'output ['+`flit_w`+' : 0] out_ports_'+`i`+'_getFlit;\n')
		f.write('\n')			
		f.write('\t'+'// action method out_ports_'+`i`+'_putNonFullVCs\n')
		f.write('\t'+'input  out_ports_'+`i`+'_putNonFullVCs_nonFullVCs;\n')	
		f.write('\t'+'input  EN_out_ports_'+`i`+'_putNonFullVCs;\n')	
		f.write('\n')		

	#wrapper signals
	f.write('\t'+'// sub-module signals\n')
	for i in range(0,in_ports):	
		f.write('\t'+'wire full_'+`i`+';\n')
	f.write('\n')	
	
	for i in range(0,in_ports):	
		f.write('\t'+'assign in_ports_'+`i`+'_getNonFullVCs = !full_'+`i`+';\n')
	f.write('\n')		
	
	#instantiate router_top
	f.write('router_top net_routers_router_core(\n')
	f.write('\t'+'.ap_clk(CLK),\n')	
	f.write('\t'+'.ap_rst(!RST_N),\n')	
	
	#for ports
	for i in range(0,in_ports):
		f.write('\t'+'.in_flit_'+`i`+'(in_ports_'+`i`+'_putRoutedFlit_flit_in),\n')	
	for i in range(0,in_ports):
		f.write('\t'+'.in_flow_control_'+`i`+'(full_'+`i`+'),\n')	
	for i in range(0,out_ports):
		f.write('\t'+'.out_flit_'+`i`+'(out_ports_'+`i`+'_getFlit),\n')		
	for i in range(0,out_ports):
		f.write('\t'+'.out_flow_control_'+`i`+'(!out_ports_'+`i`+'_putNonFullVCs_nonFullVCs),\n')	
	for i in range(0,in_ports):
		f.write('\t'+'.EN_inport_'+`i`+'(EN_in_ports_'+`i`+'_getNonFullVCs),\n')	
	for i in range(0,out_ports):
		if (i == out_ports-1):
			f.write('\t'+'.EN_outport_'+`i`+'(EN_out_ports_'+`i`+'_getFlit));\n')
		else:
			f.write('\t'+'.EN_outport_'+`i`+'(EN_out_ports_'+`i`+'_getFlit),\n')
	
	f.write('endmodule  // mkVOQRouterCoreSimple\n')