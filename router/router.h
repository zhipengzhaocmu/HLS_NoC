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
	Router object. Include allocator, and buffers. All different configurations
	are coded compactly. 
*/
class router{
public:
	allocator alloc;
#ifdef VOQ
	flit_buffer<flit_t> buffer[N][B];
#else//VC or IQ
	flit_buffer<RT_flit_t> buffer[N][B];
#endif//end VOQ

	bool grant_reg[M][N];
	ap_uint<LOG2VC> selected_vc_reg[N];

#ifdef VC
	#ifdef CREDIT
	vc_t in_flow_control[N];
	fifo_cnt_t credit_cnt[M][VC_NUM];
	#endif//end CREDIT

	#ifdef VL
	bool lock[M][VC_NUM];
	in_port_num_t vl[M][VC_NUM];
	#endif//end VL
#else//IQ or VOQ
	#ifdef CREDIT
	bool in_flow_control[N];
	fifo_cnt_t credit_cnt[M];
	#endif//end CREDIT

	#ifdef VL
	bool lock[M];
	in_port_num_t vl[M];
	#endif//end VL
#endif //end VC

//////// Get the in_flow_control signal ////////

#ifdef VC
	void get_in_flow_control(vc_t in_flow_control[N]){
#else//IQ or VOQ
	void get_in_flow_control(bool in_flow_control[N]){
#endif//end VC

	#pragma HLS INLINE
	#pragma HLS LATENCY max=0
	#pragma HLS ARRAY_PARTITION variable=in_flow_control complete dim=1
		uint32_t i,j,q,v;

#ifdef VC
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
	#ifdef PIPE_LINK
				//there is one cycle delay for the link between two routers
				//for flow control signal, we use almost full instead of full signal
				in_flow_control[i][v] = buffer[i][v].almost_full();
	#else
				in_flow_control[i][v] = buffer[i][v].full();
	#endif//end PIPE_LINK
			}
		}
#else//IQ or VOQ
		//initialize the temp
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			in_flow_control[i] = 0;
		}
		//assign the temp
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(q=0;q<B;q++){
			#pragma HLS UNROLL
	#ifdef PIPE_LINK
				if(buffer[i][q].almost_full()){
	#else
				if(buffer[i][q].full()){
	#endif//end PIPE_LINK
					in_flow_control[i] = 1;
				}
			}
		}
#endif//end VC
	}

//////// Get the first element from the buffer ////////
#ifdef VOQ
	void get_first(flit_t first_element[N][B]){
#else
	void get_first(RT_flit_t first_element[N][B]){
#endif
	#pragma HLS INLINE
	#pragma HLS LATENCY max=0
	#pragma HLS ARRAY_PARTITION variable=first_element complete dim=0
		uint32_t i,j,q;
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(q=0;q<B;q++){
			#pragma HLS UNROLL
				first_element[i][q] = buffer[i][q].first();
#ifdef VOQ
				first_element[i][q].valid = !buffer[i][q].empty();
#else
				first_element[i][q].flit.valid = !buffer[i][q].empty();
#endif
			}
		}
	}

//////// Generate the requests ////////
#ifdef VC
	#ifdef CREDIT
		void gen_req(bool request[N][M], ap_uint<LOG2VC> selected_vc[N], bool EN_outport[M]){
	#else
		void gen_req(vc_t out_flow_control[M], bool request[N][M], ap_uint<LOG2VC> selected_vc[N], bool EN_outport[M]){
		#pragma HLS ARRAY_PARTITION variable=out_flow_control complete dim=1
	#endif//end CREDIT
	
		#pragma HLS ARRAY_PARTITION variable=selected_vc complete dim=1
#else//IQ or VOQ
	#ifdef CREDIT
		void gen_req(bool request[N][M], bool EN_outport[M]){
	#else
		void gen_req(bool out_flow_control[M], bool request[N][M], bool EN_outport[M]){
		#pragma HLS ARRAY_PARTITION variable=out_flow_control complete dim=1
	#endif//end CREDIT
#endif//end VC
		#pragma HLS ARRAY_PARTITION variable=request complete dim=0
		#pragma HLS ARRAY_PARTITION variable=EN_outport complete dim=1
		
		uint32_t i,j,q,v;
#ifdef VOQ
		flit_t first_element[N][B];
#else//IQ or VC
		RT_flit_t first_element[N][B];
#endif//end VOQ

		bool in_valid[N];
		out_port_num_t req_port[N];
		bool valid[N];
		#pragma HLS INLINE
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=first_element complete dim=0
		#pragma HLS ARRAY_PARTITION variable=in_valid complete dim=1
		#pragma HLS ARRAY_PARTITION variable=req_port complete dim=1	
		#pragma HLS ARRAY_PARTITION variable=valid complete dim=1
		
#ifdef VC
		fifo_cnt_t temp_credit_cnt[M][VC_NUM];
		#pragma HLS ARRAY_PARTITION variable=temp_credit_cnt complete dim=0
#else//IQ or VOQ
		fifo_cnt_t temp_credit_cnt[M];
		#pragma HLS ARRAY_PARTITION variable=temp_credit_cnt complete dim=1
#endif//end VC

		//virtual link. initialize the temp virtual link variables
#ifdef VL
	#ifdef VC
		bool temp_lock[M][VC_NUM];
		in_port_num_t temp_vl[M][VC_NUM];
		#pragma HLS ARRAY_PARTITION variable=temp_lock complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_vl complete dim=0

		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				temp_lock[j][v] = lock[j][v];
				temp_vl[j][v] = vl[j][v];
			}
		}
	#else
		bool temp_lock[M];
		in_port_num_t temp_vl[M];
		#pragma HLS ARRAY_PARTITION variable=temp_lock complete dim=1
		#pragma HLS ARRAY_PARTITION variable=temp_vl complete dim=1

		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			temp_lock[j] = lock[j];
			temp_vl[j] = vl[j];
		}
	#endif//end VC
#endif//end VL


		//read from credit_cnt to temp_credit_cnt
#ifdef CREDIT
	#ifdef VC
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				temp_credit_cnt[j][v]=credit_cnt[j][v];
			}
		}
	#else//IQ or VOQ
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			temp_credit_cnt[j]=credit_cnt[j];
		}
	#endif//end VC
#endif//end CREDIT

		//get first element
		get_first(first_element);

		//generate alloc request
#ifdef VOQ
		for(i=0;i<N;i++){
		#pragma HLS UNROLL			
			for(j=0;j<M;j++){
			#pragma HLS UNROLL
				request[i][j] = 0;
	#ifdef VL
				if(!out_flow_control[j] && (!temp_lock[j]||(temp_lock[j] && temp_vl[j] == i)) && EN_outport[j]){
	#else
				if(!out_flow_control[j] && EN_outport[j]){
	#endif//end VL
					request[i][j] = first_element[i][j].valid;
				}
			}
		}
#else//IQ or VC

		//initialize the array
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			//valid signal for each flit buffer
			in_valid[i] = 0;
			//request out_port number for each flit buffer
			req_port[i] = 0;
	#ifdef VC
			selected_vc[i] = 0;
	#endif
		}

		for(i=0;i<N;i++){
		#pragma HLS UNROLL
	#ifdef VC
			for(v=0;v<VC_NUM;v++){
			//lower VC has higher priority, target outport is not full
			#pragma HLS UNROLL
				j = first_element[i][v].port_num;//target outport
		#ifdef CREDIT
				valid[i] = first_element[i][v].flit.valid &&
						(temp_credit_cnt[j][v]<FIFO_DEPTH) && EN_outport[j];
		#else
				valid[i] = first_element[i][v].flit.valid &&
						!out_flow_control[j][v] && EN_outport[j];
		#endif//end CREDIT

		#ifdef VL
				valid[i] = valid[i] && (!temp_lock[j][v]||(temp_lock[j][v] && temp_vl[j][v] == i));
		#endif//end VL

				if(valid[i]){
					in_valid[i] = 1;
					req_port[i] = first_element[i][v].port_num;
					selected_vc[i] = v;
					break;
				}
			}
	#else//IQ
			j = first_element[i][0].port_num;//target outport
		#ifdef CREDIT
			valid[i] = first_element[i][0].flit.valid && (temp_credit_cnt[j]<FIFO_DEPTH) && EN_outport[j];
		#else//PEEK
			valid[i] = first_element[i][0].flit.valid && !out_flow_control[j] && EN_outport[j];
		#endif//end CREDIT

		#ifdef VL
			valid[i] = valid[i] && (!temp_lock[j]||(temp_lock[j] && temp_vl[j] == i));
		#endif//end VL

			if(valid[i]){
				in_valid[i] = 1;
				req_port[i] = first_element[i][0].port_num;
			}
	#endif//end VC
		}

		//generate request matrix based on in_valid and req_port
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			if(in_valid[i]){
				for(j=0;j<M;j++){
				#pragma HLS UNROLL
					if((req_port[i] == j)){
						request[i][j] = 1;
					}
					else{
						request[i][j] = 0;
					}
				}
			}
			else{
				for(j=0;j<M;j++){
				#pragma HLS UNROLL
					request[i][j] = 0;
				}
			}
		}
#endif//VOQ
	}


//////// Routing logic, output the flits and deq signal ////////
#ifdef CREDIT
	#ifdef VC
	void routing(flit_t out_flit[M], b_t deq[N], vc_t out_send[M], bool EN_outport[M], 
		out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N], 
		ap_uint<LOG2VC> new_selected_vc_reg[N], bool new_grant_reg[M][N], bool new_lock[M][VC_NUM],
		in_port_num_t new_vl[M][VC_NUM]){			
	#else//IQ or VOQ
	void routing(flit_t out_flit[M], b_t deq[N], bool out_send[M], bool EN_outport[M],
		out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N], 
		ap_uint<LOG2VC> new_selected_vc_reg[N], bool new_grant_reg[M][N], bool new_lock[M],
		in_port_num_t new_vl[M]){
	#endif//end VC
	#pragma HLS ARRAY_PARTITION variable=out_send complete dim=1
#else//PEEK
	#ifdef VC
	void routing(flit_t out_flit[M], b_t deq[N], vc_t out_flow_control[M], bool EN_outport[M], 
		out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N], 
		ap_uint<LOG2VC> new_selected_vc_reg[N], bool new_grant_reg[M][N], bool new_lock[M][VC_NUM],
		in_port_num_t new_vl[M][VC_NUM]){
	#else//IQ or VOQ
	void routing(flit_t out_flit[M], b_t deq[N], bool out_flow_control[M], bool EN_outport[M],
		out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N], 
		ap_uint<LOG2VC> new_selected_vc_reg[N], bool new_grant_reg[M][N], bool new_lock[M],
		in_port_num_t new_vl[M]){
	#endif//end VC
	
	vc_t out_send[M];//unused, just for compatibility
	#pragma HLS ARRAY_PARTITION variable=out_flow_control complete dim=1
#endif//end CREDIT

	#pragma HLS INLINE
	#pragma HLS LATENCY max=0
	#pragma HLS DATA_PACK variable=out_flit
	
	#pragma HLS ARRAY_PARTITION variable=out_flit complete dim=1
	#pragma HLS ARRAY_PARTITION variable=deq complete dim=1
	#pragma HLS ARRAY_PARTITION variable=EN_outport complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_selected_vc_reg complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_grant_reg complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_lock complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_vl complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_selected_vc_reg complete dim=1

		uint32_t i,j;
		vc_t v;
		flit_t invalid_flit = {0,0,0,0,0};
#ifdef VOQ
		flit_t first_element[N][B];
#else//IQ or VC
		RT_flit_t first_element[N][B];
#endif//end VOQ

		in_port_num_t grant_port[M];
		b_t temp_deq[N];
		flit_t temp_out_flit[M];
		bool request[N][M];
		bool grant[M][N];
		bool out_valid[M];
		ap_uint<LOG2VC> selected_vc[N];
		#pragma HLS ARRAY_PARTITION variable=first_element complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant_port complete dim=1
		#pragma HLS ARRAY_PARTITION variable=temp_deq complete dim=1
		#pragma HLS ARRAY_PARTITION variable=temp_out_flit complete dim=1
		#pragma HLS ARRAY_PARTITION variable=request complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=out_valid complete dim=1
		#pragma HLS ARRAY_PARTITION variable=selected_vc complete dim=1

#ifdef VC
		bool has_credits[M][VC_NUM];
		#pragma HLS ARRAY_PARTITION variable=has_credits complete dim=0
#else//IQ or VOQ
		bool has_credits[M];
		#pragma HLS ARRAY_PARTITION variable=has_credits complete dim=1
#endif//end VC

#ifdef VL
	#ifdef VC
		bool temp_lock[M][VC_NUM];
		in_port_num_t temp_vl[M][VC_NUM];
		#pragma HLS ARRAY_PARTITION variable=temp_lock complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_vl complete dim=0

		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				temp_lock[j][v] = lock[j][v];
				temp_vl[j][v] = vl[j][v];
			}
		}
	#else//IQ or VOQ
		bool temp_lock[M];
		in_port_num_t temp_vl[M];
		#pragma HLS ARRAY_PARTITION variable=temp_lock complete dim=1
		#pragma HLS ARRAY_PARTITION variable=temp_vl complete dim=1

		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			temp_lock[j] = lock[j];
			temp_vl[j] = vl[j];
		}
	#endif//end VC
#endif//end VL

		get_first(first_element);

#ifdef VC
	#ifdef CREDIT
		gen_req(request,selected_vc,EN_outport);
		//Get the credits
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				has_credits[j][v] = credit_cnt[j][v] < FIFO_DEPTH;
			}
		}
	#else//PEEK
		gen_req(out_flow_control,request,selected_vc,EN_outport);
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				has_credits[j][v] = !out_flow_control[j][v];
			}
		}
	#endif//end CREDIT
#else//IQ or VOQ
	#ifdef CREDIT
		gen_req(request,EN_outport);
		//Get the credits
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			if(credit_cnt[j] < FIFO_DEPTH){
				has_credits[j] = 1;
			}
			else{
				has_credits[j] = 0;
			}
		}
	#else//PEEK
		gen_req(out_flow_control,request,EN_outport);
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			has_credits[j] = !out_flow_control[j];
		}
	#endif//end CREDIT
#endif//end VC

#ifndef PIPE_CORE
		alloc.allocation(request,grant,new_prior1,new_prior2,new_grant_trans);	
#endif
		//initialize deq & output flit
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			temp_deq[i] = 0;
		}

		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			out_send[j] = 0;
			temp_out_flit[j] = invalid_flit;
			grant_port[j] = 0;
		}

		//If there is no data in buffer, out_flit an invalid flit
		//The deq would not change the counter in flit_buffer
		//If there is data in buffer, out_flit the first flit
		//Use the deq to update the counter in flit_buffer
		
		//for each outport
		for (j=0;j<M;j++){
		#pragma HLS UNROLL
			//for each inport
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				//the outport is valid and granted
#ifdef PIPE_CORE
				if(grant_reg[j][i] && EN_outport[j]){
#else
				if(grant[j][i] && EN_outport[j]){
#endif//end PIPE_CORE

					//for outport j, the out_flit should come from inport i
					grant_port[j] = i;
					
#ifdef VOQ
				//Double check credits and target, necessary for speculative pipeline core or alloc
	#ifdef PIPE_CORE
		#ifdef VL
					if(has_credits[j]
						&& (!temp_lock[j]||(temp_lock[j] && temp_vl[j] == i))
						&& first_element[i][j].valid){
		#else
					if(has_credits[j]
							&& first_element[i][j].valid){
		#endif//end VL
	#endif//end PIPE_CORE
	#ifdef PIPE_ALLOC
		#ifdef VL
					if(has_credits[j]
						&& (!temp_lock[j]||(temp_lock[j] && temp_vl[j] == i))
						&& first_element[i][j].valid){
		#else
					if(has_credits[j]
							&& first_element[i][j].valid){
		#endif//end VL
	#endif//end PIPE_ALLOC
						out_send[j] = 1;
						//make a deq
						temp_deq[i][j] = 1;
						//read the data
						temp_out_flit[j] = first_element[i][j];
	#ifdef PIPE_CORE
					}
	#endif//end PIPE_CORE
	#ifdef PIPE_ALLOC
					}
	#endif//end PIPE_ALLOC
#else//IQ or VC
	#ifdef VC
					//get the selected virtual channel
		#ifdef PIPE_CORE
					v = selected_vc_reg[i];
		#else
			#ifdef PIPE_ALLOC
					v = selected_vc_reg[i];
			#else
					v = selected_vc[i];
			#endif//end PIPE_ALLOC
		#endif//end PIPE_CORE
		
					//Double check credits and target, necessary for speculative pipeline core or alloc
		#ifdef PIPE_CORE
			#ifdef VL
					//Double check lock as well
					if(has_credits[j][v] && first_element[i][v].port_num == j
							&& (!temp_lock[j][v]||(temp_lock[j][v] && temp_vl[j][v] == i))
							&& first_element[i][v].flit.valid){
			#else
					if(has_credits[j][v] && first_element[i][v].port_num == j
							&& first_element[i][v].flit.valid){
			#endif//end VL
		#endif//end PIPE_CORE
		#ifdef PIPE_ALLOC
			#ifdef VL
					//Double check lock as well
					if(has_credits[j][v] && first_element[i][v].port_num == j
							&& (!temp_lock[j][v]||(temp_lock[j][v] && temp_vl[j][v] == i))
							&& first_element[i][v].flit.valid){
			#else
					if(has_credits[j][v] && first_element[i][v].port_num == j
							&& first_element[i][v].flit.valid){
			#endif//end VL
		#endif//end PIPE_ALLOC
						out_send[j][v] = 1;
						//Make a deq
						temp_deq[i][v] = 1;
						//Read the data
						temp_out_flit[j] = first_element[i][v].flit;
		#ifdef PIPE_CORE
					}
		#endif//end PIPE_CORE
		#ifdef PIPE_ALLOC
					}
		#endif//end PIPE_ALLOC
	#else//IQ
					//Double check credits and target, necessary for speculative pipeline core or alloc
		#ifdef PIPE_CORE
			#ifdef VL
					//Double check lock as well
					if(has_credits[j] && first_element[i][0].port_num == j
							&& (!temp_lock[j]||(temp_lock[j] && temp_vl[j] == i))
							&& first_element[i][0].flit.valid){
			#else
					if(has_credits[j] && first_element[i][0].port_num == j
							&& first_element[i][0].flit.valid){
			#endif//end VL
		#endif//end PIPE_CORE
		#ifdef PIPE_ALLOC
			#ifdef VL
					//Double check lock as well
					if(has_credits[j] && first_element[i][0].port_num == j
							&& (!temp_lock[j]||(temp_lock[j] && temp_vl[j] == i))
							&& first_element[i][0].flit.valid){
			#else
					if(has_credits[j] && first_element[i][0].port_num == j
							&& first_element[i][0].flit.valid){
			#endif//end VL
		#endif//end PIPE_ALLOC
						out_send[j] = 1;
						//Make a deq
						temp_deq[i] = 1;
						//Read the data
						temp_out_flit[j] = first_element[i][0].flit;
		#ifdef PIPE_CORE
					}
		#endif//end PIPE_CORE
		#ifdef PIPE_ALLOC
					}
		#endif//end PIPE_ALLOC
	#endif//VC
#endif//VOQ
				}//end grant & EN_outport
			}//end i
		}//end j

#ifdef PIPE_CORE
		alloc.allocation(request,new_grant_reg,new_prior1,new_prior2,new_grant_trans);	
#endif
		
		//output the selected_vc
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			new_selected_vc_reg[i] = selected_vc[i];
		}

		//generate virtual link variables
#ifdef VL
	#ifdef VC
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				if(temp_out_flit[j].valid && !temp_out_flit[j].tail && temp_out_flit[j].vc==v){
					temp_lock[j][v] = 1;
					temp_vl[j][v] = grant_port[j];
				}
				else{
					if(temp_out_flit[j].valid && temp_out_flit[j].tail && temp_out_flit[j].vc==v){
						temp_lock[j][v] = 0;
						temp_vl[j][v] = 0;
					}
				}
			}
		}

		//output the lock and vl
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				new_lock[j][v] = temp_lock[j][v];
				new_vl[j][v] = temp_vl[j][v];
			}
		}
	#else//IQ or VOQ
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			if(temp_out_flit[j].valid && !temp_out_flit[j].tail){
				temp_lock[j] = 1;
				temp_vl[j] = grant_port[j];
			}
			else{
				if(temp_out_flit[j].valid && temp_out_flit[j].tail){
					temp_lock[j] = 0;
					temp_vl[j] = 0;
				}
			}
		}

		//output the lock and vl
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			new_lock[j] = temp_lock[j];
			new_vl[j] = temp_vl[j];
		}
	#endif//end VC
#endif//end VL

		//output the deq
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			deq[i] = temp_deq[i];
		}

		//output the out_flit
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			out_flit[j] = temp_out_flit[j];
		}
	}

//////// Update logic, update the buffer by in_flit and deq signal, update internal states ////////
#ifdef CREDIT
	#ifdef VC
	void update(RT_flit_t in_flit[N], b_t deq[N], vc_t out_flow_control[M], vc_t out_send[M], 
		bool EN_inport[N], out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N], 
		ap_uint<LOG2VC> new_selected_vc_reg[N], bool new_grant_reg[M][N], bool new_lock[M][VC_NUM],
		in_port_num_t new_vl[M][VC_NUM]){
	#else//IQ or VOQ
	void update(RT_flit_t in_flit[N], b_t deq[N], bool out_flow_control[M], bool out_send[M], 
		bool EN_inport[N], out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N], 
		ap_uint<LOG2VC> new_selected_vc_reg[N], bool new_grant_reg[M][N], bool new_lock[M],
		in_port_num_t new_vl[M]){
	#endif//end VC
	#pragma HLS ARRAY_PARTITION variable=out_flow_control complete dim=1
	#pragma HLS ARRAY_PARTITION variable=out_send complete dim=1
#else//PEEK
	#ifdef VC
	void update(RT_flit_t in_flit[N], b_t deq[N], bool EN_inport[N], out_port_num_t new_prior1[N], 
		in_port_num_t new_prior2[M], bool new_grant_trans[M][N], ap_uint<LOG2VC> new_selected_vc_reg[N], 
		bool new_grant_reg[M][N], bool new_lock[M][VC_NUM], in_port_num_t new_vl[M][VC_NUM]){
	#else//IQ or VOQ
	void update(RT_flit_t in_flit[N], b_t deq[N], bool EN_inport[N], out_port_num_t new_prior1[N], 
		in_port_num_t new_prior2[M], bool new_grant_trans[M][N], ap_uint<LOG2VC> new_selected_vc_reg[N], 
		bool new_grant_reg[M][N], bool new_lock[M], in_port_num_t new_vl[M]){	
	#endif//end VC
#endif//end CREDIT

	#pragma HLS INLINE
	#pragma HLS LATENCY max=0
	#pragma HLS DATA_PACK variable=in_flit
	#pragma HLS ARRAY_PARTITION variable=in_flit complete dim=1
	#pragma HLS ARRAY_PARTITION variable=deq complete dim=1
	#pragma HLS ARRAY_PARTITION variable=EN_inport complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_selected_vc_reg complete dim=1
	#pragma HLS ARRAY_PARTITION variable=new_grant_reg complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_lock complete dim=0
	#pragma HLS ARRAY_PARTITION variable=new_vl complete dim=0
	
		uint32_t i,j,q,v;
		b_t enq[N];
		b_t temp_deq[N];
		#pragma HLS ARRAY_PARTITION variable=enq complete dim=1
		#pragma HLS ARRAY_PARTITION variable=temp_deq complete dim=1
#ifdef VC
		fifo_cnt_t temp_credit_cnt[M][VC_NUM];
		#pragma HLS ARRAY_PARTITION variable=temp_credit_cnt complete dim=0
#else//IQ or VOQ
		fifo_cnt_t temp_credit_cnt[M];
		#pragma HLS ARRAY_PARTITION variable=temp_credit_cnt complete dim=1
#endif//end VC

		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			//write req to a fifo when input is valid
			enq[i] = 0;
			if (in_flit[i].flit.valid && EN_inport[i]){
#ifdef VOQ
				for(q=0;q<B;q++){
				#pragma HLS UNROLL
					if(in_flit[i].port_num == q){
						enq[i][q] = 1;
					}
				}
#else//IQ or VC
	#ifdef VC
				for(v=0;v<VC_NUM;v++){
				#pragma HLS UNROLL
					if(in_flit[i].flit.vc == v){
						enq[i][v] = 1;
					}
				}
	#else//IQ
				enq[i] = 1;

	#endif//end VC
#endif//end VOQ
			}
		}

		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			temp_deq[i] = 0;
			if(EN_inport[i]){
				temp_deq[i] = deq[i];
			}
		}

		//update the buffer
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(q=0;q<B;q++){
			#pragma HLS UNROLL
#ifdef VOQ
				buffer[i][q].update(in_flit[i].flit, (bool)temp_deq[i][q], (bool)enq[i][q]);
#else
				buffer[i][q].update(in_flit[i], (bool)temp_deq[i][q], (bool)enq[i][q]);
#endif
			}
		}

		//update the priority of Arbiters
		alloc.update(new_prior1,new_prior2,new_grant_trans);
		
		//update credits
#ifdef CREDIT
	#ifdef VC
		//initialize temp_credit_cnt
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				temp_credit_cnt[j][v]=credit_cnt[j][v];
			}
		}

		//assign the temp_credit_cnt
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				if(out_flow_control[j][v]){//the target buffer deq one flit
					if(!out_send[j][v] && (temp_credit_cnt[j][v]>0)){//doesn't send a flit
						temp_credit_cnt[j][v]--;
					}
				}
				else{//the target buffer doesn't deq one flit
					if(out_send[j][v]){//send a flit
						temp_credit_cnt[j][v]++;
					}
				}
			}
		}

		//update credit_cnt
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				credit_cnt[j][v]=temp_credit_cnt[j][v];
			}
		}
	#else//IQ or VOQ
		//initialize temp_credit_cnt
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			temp_credit_cnt[j]=credit_cnt[j];
		}

		//assign the temp_credit_cnt
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			if(out_flow_control[j]){//the target buffer deq one flit
				if(!out_send[j] && (temp_credit_cnt[j]>0)){//doesn't send a flit
					temp_credit_cnt[j]--;
				}
			}
			else{//the target buffer doesn't deq one flit
				if(out_send[j]){//send a flit
					temp_credit_cnt[j]++;
				}
			}
		}

		//update the credit_cnt
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			credit_cnt[j]=temp_credit_cnt[j];
		}
	#endif//end VC
#endif//end CREDIT

		//update the lock and vl
#ifdef VL
	#ifdef VC
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(v=0;v<VC_NUM;v++){
			#pragma HLS UNROLL
				lock[j][v] = new_lock[j][v];
				vl[j][v] = new_vl[j][v];
			}
		}
	#else
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			lock[j] = new_lock[j];
			vl[j] = new_vl[j];
		}
	#endif
#endif//end VL
		
		//update vc_reg and grant_reg
#ifdef PIPE_CORE
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			selected_vc_reg[i] = new_selected_vc_reg[i];
			for(j=0;j<M;j++){
			#pragma HLS UNROLL
				grant_reg[i][j] = new_grant_reg[i][j];
			}
		}
#endif//end PIPE_CORE

		//update vc_reg
#ifdef PIPE_ALLOC
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			selected_vc_reg[i] = new_selected_vc_reg[i];
		}
#endif//end PIPE_ALLOC
	}
};
