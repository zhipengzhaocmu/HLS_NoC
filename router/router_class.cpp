/*
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
*/

/*
	Top level function for router instance. To be synthesized, the top level has to be a function.
*/
#include "para.h"
#include "arbiter.h"
#include "alloc.h"
#include "flit_buffer.h"
#include "router.h"

#ifdef VC
void router_top(RT_flit_t in_flit[N], vc_t in_flow_control[N], flit_t out_flit[M], 
	vc_t out_flow_control[M], bool EN_inport[N], bool EN_outport[M]){
#else
void router_top(RT_flit_t in_flit[N], bool in_flow_control[N], flit_t out_flit[M], 
	bool out_flow_control[M], bool EN_inport[N], bool EN_outport[M]){

#endif //end VC

	//remove the unused control signals
	#pragma HLS INTERFACE ap_none port=in_flow_control
	#pragma HLS INTERFACE ap_none port=out_flit
	#pragma HLS INTERFACE ap_ctrl_none port=return
	
	//Pack the struct members into one wide signal
	#pragma HLS DATA_PACK variable=out_flit
	#pragma HLS DATA_PACK variable=in_flit
	
	//Partition the interfaces
	#pragma HLS ARRAY_PARTITION variable=in_flit complete dim=1
	#pragma HLS ARRAY_PARTITION variable=in_flow_control complete dim=1
	#pragma HLS ARRAY_PARTITION variable=out_flit complete dim=1
	#pragma HLS ARRAY_PARTITION variable=out_flow_control complete dim=1
	
	//Force all the logic to be executed within one cycle
	#pragma HLS LATENCY max=0
	
	//Instantiate router
	static router my_router;

	uint32_t i,j,q,v;
#ifdef VC
	vc_t temp_in_flow_control[N];
	vc_t out_send[M];
	bool new_lock[M][VC_NUM];
	in_port_num_t new_vl[M][VC_NUM];
#else
	bool temp_in_flow_control[N];
	bool out_send[M];
	bool new_lock[M];
	in_port_num_t new_vl[M];
#endif

	b_t deq[N];
	flit_t temp_out_flit[M];
	out_port_num_t new_prior1[N];
	in_port_num_t new_prior2[M];	
	bool new_grant_trans[M][N];
	ap_uint<LOG2VC> new_selected_vc_reg[N];
	bool new_grant_reg[M][N];
	
	#pragma HLS ARRAY_PARTITION variable=temp_in_flow_control complete dim=1
	#pragma HLS ARRAY_PARTITION variable=deq complete dim=1
	#pragma HLS ARRAY_PARTITION variable=out_send complete dim=1
	#pragma HLS ARRAY_PARTITION variable=temp_out_flit complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_lock complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_vl complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_selected_vc_reg complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_grant_reg complete dim=0
	
#ifdef PEEK
	my_router.get_in_flow_control(temp_in_flow_control);
#endif

#ifdef CREDIT
	my_router.routing(temp_out_flit,deq,out_send,EN_outport,new_prior1,new_prior2,new_grant_trans,
		new_selected_vc_reg,new_grant_reg,new_lock,new_vl);
	for(i=0;i<N;i++){
	#pragma HLS UNROLL
	#ifdef VC
		for(v=0;v<VC_NUM;v++){
		#pragma HLS UNROLL
			temp_in_flow_control[i][v] = deq[i][v];
		}
	#else//IQ or VOQ
		temp_in_flow_control[i] = deq[i];
	#endif //end VC
	}
#endif//end CREDIT

#ifdef PEEK
	my_router.routing(temp_out_flit,deq,out_flow_control,EN_outport,new_prior1,new_prior2,
		new_grant_trans,new_selected_vc_reg,new_grant_reg,new_lock,new_vl);
#endif

#ifdef CREDIT
	my_router.update(in_flit,deq,out_flow_control,out_send,EN_inport,new_prior1,new_prior2,
		new_grant_trans,new_selected_vc_reg,new_grant_reg,new_lock,new_vl);	
#else//PEEK
	my_router.update(in_flit,deq,EN_inport,new_prior1,new_prior2,
		new_grant_trans,new_selected_vc_reg,new_grant_reg,new_lock,new_vl);
#endif//end CREDIT

	//output
	for(i=0;i<N;i++){
	#pragma HLS UNROLL
		in_flow_control[i] = temp_in_flow_control[i];
	}

	for(j=0;j<M;j++){
	#pragma HLS UNROLL
		out_flit[j] = temp_out_flit[j];
	}
}
