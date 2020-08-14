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

import math
import os
import subprocess

def is_power2(x):
	return x !=0 and ((x & (x - 1)) == 0)

def bit_len(x):
	return (x-1).bit_length()

#check the connect_parameters to diffentiate IQ and VC
router_type = 'VC'
with open('./build/connect_parameters.v','r') as f:
	for line in f:
		if ("`define USE_IQ_ROUTER True" in line):
			router_type = 'IQ'
			
#get the router type and flow_control type based on file name	
if (router_type == 'IQ'):
	batcmd = 'ls ./build/ | grep "IQRouterCore"'
else:
	batcmd = 'ls ./build/ | grep "RouterCore"'
type = subprocess.check_output(batcmd, shell=True)	
#print type

#paraphrase the type
router_type = 'VC'
flow_control_type = 'CREDIT'
if (type == 'mkRouterCore.v\n'):
	router_type = 'VC'
	flow_control_type = 'CREDIT'
elif (type == 'mkRouterCoreSimple.v\n'):
	router_type = 'VC'
	flow_control_type = 'PEEK'
elif (type == 'mkIQRouterCore.v\n'):
	router_type = 'IQ'
	flow_control_type = 'CREDIT'
elif (type == 'mkIQRouterCoreSimple.v\n'):
	router_type = 'IQ'
	flow_control_type = 'PEEK'
elif (type == 'mkVOQRouterCoreSimple.v\n'):
	router_type = 'VOQ'
	flow_control_type = 'PEEK'
	
#print router_type
#print flow_control_type	

#get the other config from connect_parameters
pipe = 'NO_PIPE'
with open('./build/connect_parameters.v','r') as f:
	for line in f:
		if ("NUM_USER_RECV_PORTS " in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			node = c
			#print node
		if ("NUM_IN_PORTS" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			n = c
			#print n
		if ("NUM_OUT_PORTS" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			m = c
			#print n
		if ("NUM_VCS" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			num_vc = c
			#print num_vc
		if ("ALLOC_TYPE" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			alloc_type = c
			#print alloc_type
		if ("USE_VIRTUAL_LINKS" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			virtual_link = c
			#print virtual_link	
		if ("FLIT_BUFFER_DEPTH" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			flit_buffer_depth = c
			#print flit_buffer_depth			
		if ("FLIT_DATA_WIDTH" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			flit_data_width = c
			#print flit_data_width
		if ("PIPELINE_CORE" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			if (c == 'True' ):
				pipe = 'PIPE_CORE'
			#print pipe
		if ("PIPELINE_ALLOCATOR" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			if (c == 'True' ):
				pipe = 'PIPE_ALLOC'
			#print pipe
		if ("PIPELINE_LINKS" in line):
			x = line[:-1]
			a,b,c = x.split(" ")
			if (c == 'True' ):
				pipe = 'PIPE_LINK'
			#print pipe
			
#paraphrase the allocator
if (alloc_type == 'SepIFRoundRobin'):
	alloc = 'SEPIF'
	arbiter = 'ROUNDROBIN'
elif (alloc_type == 'SepOFRoundRobin'):
	alloc = 'SEPOF'
	arbiter = 'ROUNDROBIN'
elif (alloc_type == 'SepIFStatic'):
	alloc = 'SEPIF'
	arbiter = 'STATIC'
elif (alloc_type == 'SepOFStatic'):
	alloc = 'SEPOF'
	arbiter = 'STATIC'

#print "alloc_type: " + alloc_type
#print "alloc: " + alloc
#print "arbiter: " + arbiter

#calculate the bit length of the fifo cnt
if (is_power2(int(flit_buffer_depth))):
	log2_depth = bit_len(int(flit_buffer_depth)) + 1
else:
	log2_depth = bit_len(int(flit_buffer_depth))
	
#create a temp para.h
input_file = 'para.h'
output_file = open('temp_para.h','w')
row = 0
router_flag = 0
fc_flag = 0
alloc_flag = 0
arbiter_flag = 0
vl_flag = 0
pipeline_flag = 0

with open(input_file,'r') as f:
	for line in f:
		#replace number of Endpoints
		if ("#define NODE " in line):
			mod_line = "#define NODE " + node + "\n"
			output_file.write(mod_line)
			continue
		if ("#define LOG2NODE " in line):
			mod_line = "#define LOG2NODE " + str(bit_len(int(node))) + "\n"
			output_file.write(mod_line)
			continue		

		#replace number of ports
		if ("#define N " in line):
			mod_line = "#define N " + n + "\n"
			output_file.write(mod_line)
			continue
		if ("#define M " in line):
			mod_line = "#define M " + m + "\n"
			output_file.write(mod_line)
			continue			
		if ("#define LOG2N " in line):
			mod_line = "#define LOG2N " + str(bit_len(int(n))) + "\n"
			output_file.write(mod_line)
			continue	
		if ("#define LOG2M " in line):
			mod_line = "#define LOG2M " + str(bit_len(int(m))) + "\n"
			output_file.write(mod_line)
			continue
			
		#replace router type
		if router_flag == 1:
			mod_line = "#define " + router_type + "\n"
			output_file.write(mod_line)
			router_flag = 0
			continue
		if ("Router Type:" in line):
			router_flag = 1

		#replace VC
		if ("#define VC_NUM" in line):
			mod_line = "#define VC_NUM " + num_vc + "\n"
			output_file.write(mod_line)
			continue
		if ("#define LOG2VC" in line):
			if (num_vc == '1'):
				log2_vc = 1
			else:
				log2_vc = bit_len(int(num_vc))
			mod_line = "#define LOG2VC " + str(log2_vc) + "\n"
			output_file.write(mod_line)
			continue

		#replace flow_control_type
		if fc_flag == 1:
			if row == 3:
				mod_line = "\t" + "#define " + flow_control_type + "\n"
				output_file.write(mod_line)
				row = 0
				fc_flag = 0
				continue
			else:
				row = row + 1
		if ("Flow Control Type:" in line):
			fc_flag = 1
		
		#replace flit data width
		if ("#define DATA_WIDTH" in line):
			mod_line = "#define DATA_WIDTH " + flit_data_width + "\n"
			output_file.write(mod_line)
			continue

		#replace flit buffer depth
		if ("#define FIFO_DEPTH" in line):
			mod_line = "#define FIFO_DEPTH " + flit_buffer_depth + "\n"
			output_file.write(mod_line)
			continue

		if ("#define LOG2_FIFO_DEPTH" in line):
			mod_line = "#define LOG2_FIFO_DEPTH " + str(log2_depth) + "\n"
			output_file.write(mod_line)
			continue
	
		#replace alloc
		if alloc_flag == 1:
			mod_line = "#define " + alloc + "\n"
			output_file.write(mod_line)
			alloc_flag = 0
			continue
		if ("Allocator:" in line):
			alloc_flag = 1

		#replace arbiter
		if arbiter_flag == 1:
			mod_line = "#define " + arbiter + "\n"
			output_file.write(mod_line)
			arbiter_flag = 0
			continue
		if ("Arbiter:" in line):
			arbiter_flag = 1

		#replace virtual link
		if vl_flag == 1:
			if (virtual_link == "False"):
				mod_line = "//#define NO_VL\n"
			else:
				mod_line = "#define VL\n"
			output_file.write(mod_line)
			vl_flag = 0
			continue
		if ("Virtual Link" in line):
			vl_flag = 1

		#replace Pipeline
		if pipeline_flag == 1:
			if (pipe == "NO_PIPE"):
				mod_line = "//#define NO_PIPE\n"
			else:
				mod_line = "#define " + pipe + "\n"
			output_file.write(mod_line)
			pipeline_flag = 0
			continue
		if ("Pipeline:" in line):
			pipeline_flag = 1


		output_file.write(line)

output_file.close()

#replace the para.h with the new one
os.remove('para.h')
os.rename('temp_para.h','para.h')

#write necessary parameters to config
flit_width = 2 + log2_vc + bit_len(int(node)) + int(flit_data_width)
with open('config','w') as f:
	f.write("router_type: " + router_type + "\n")
	f.write("flow_control_type: " + flow_control_type + "\n")
	f.write("num_in_ports: " + n + "\n")
	f.write("num_out_ports: " + m + "\n")
	f.write("num_vc: " + num_vc + "\n")
	f.write("flit_width: " + str(flit_width) + "\n")
	f.write("log2m: " + str(bit_len(int(m))) + "\n")