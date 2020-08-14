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
	Separable allocator object. Performs allocation in two sets of arbitration. 
	The request is a N*M boolean matrix, where N is number of inports, and M is number of
	outports. The grant is a M*N boolean matrix, which has at most one '1' for each row 
	and each column. For example, if grant(2,1) is '1', that means outport2 is granted 
	a request from input_buffer1. For more information, please refer to 19.3 Separable 
	Allocators(p367) of "PRINCIPLES AND OF INTERCONNECTION NETWORKS" by WILLIAM JAMES 
	DALLY & BRIAN TOWLES. 
*/
//Separable allocator 
#ifdef PIPE_ALLOC
	#ifdef SEPIF
class allocator{
public:
	arbiter <out_port_num_t,M> arb1[N];
	arbiter <in_port_num_t,N> arb2[M];
	bool latch_grant_trans[M][N];

		#ifdef STATIC
	allocator(void){
		uint32_t i,j;
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].priority = i;
		}
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].priority = j;
		}
	}
		#endif//end STATIC

	void allocation(bool request[N][M], bool grant[M][N], out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=request complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1	
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0			
		
		bool temp_grant[N][M];
		bool temp_grant_trans[M][N];
		#pragma HLS ARRAY_PARTITION variable=temp_grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_grant_trans complete dim=0

		uint32_t i,j;
		//Perform second Level first
		//Second Level
		for(j=0;j<M;j++){			
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				temp_grant_trans[j][i] = latch_grant_trans[j][i];
			}
		}
		
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].arbitration(&temp_grant_trans[j][0],&grant[j][0],&new_prior2[j]);
		}
		
		//First level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].arbitration(&request[i][0],&temp_grant[i][0],&new_prior1[i]);
		}

		//matrix transpose
		for(i=0;i<N;i++){		
		#pragma HLS UNROLL
			for(j=0;j<M;j++){
			#pragma HLS UNROLL
				new_grant_trans[j][i] = temp_grant[i][j];
			}
		}

	}
	
	void update(out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0			
		uint32_t i,j;
		
		//First level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].update(new_prior1[i]);
		}	

		//Second level
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].update(new_prior2[j]);
		}	

		//Update latch_grant_trans
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				latch_grant_trans[j][i] = new_grant_trans[j][i];
			}
		}		
	}
};
	#endif//end SEPIF

	#ifdef SEPOF
class allocator{
public:
	arbiter <out_port_num_t,M> arb1[N];
	arbiter <in_port_num_t,N> arb2[M];
	bool latch_grant_trans[M][N];

		#ifdef STATIC
	allocator(void){
		uint32_t i,j;
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].priority = i;
		}
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].priority = j;
		}
	}
		#endif//end STATIC

	void allocation(bool request[N][M], bool grant[N][M], out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=request complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1	
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0	
		
		bool request_trans[M][N];
		bool temp_grant[M][N];
		bool temp_grant_trans[N][M];
		bool grant_trans[N][M];
		#pragma HLS ARRAY_PARTITION variable=request_trans complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_grant_trans complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant_trans complete dim=0
		
		uint32_t i,j;

		//Perform second Level first
		//matrix transpose
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(j=0;j<M;j++){
			#pragma HLS UNROLL
				temp_grant_trans[i][j] = latch_grant_trans[j][i];
			}
		}
		
		//Second Level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].arbitration(&temp_grant_trans[i][0],&grant_trans[i][0],&new_prior1[i]);
		}

		//matrix transpose
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				grant[j][i] = grant_trans[i][j];
			}
		}
		
		//matrix transpose
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				request_trans[j][i] = request[i][j];
			}
		}

		//First level
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].arbitration(&request_trans[j][0],&new_grant_trans[j][0],&new_prior2[j]);
		}


	}
	
	void update(out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0			
		uint32_t i,j;
		
		//First level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].update(new_prior1[i]);
		}	

		//Second level
		for(j=0;j<N;j++){
		#pragma HLS UNROLL
			arb2[j].update(new_prior2[j]);
		}	

		//Update latch_grant_trans
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				latch_grant_trans[j][i] = new_grant_trans[j][i];
			}
		}			
	}	
};
	#endif//SEPOF
#else//non PIPE_ALLOC

	#ifdef SEPIF
class allocator{
public:
	arbiter <out_port_num_t,M> arb1[N];
	arbiter <in_port_num_t,N> arb2[M];

		#ifdef STATIC
	//Initialize the priority of static arbiters
	allocator(void){
		uint32_t i,j;
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].priority = i;
		}
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].priority = j;
		}
	}
		#endif//end STATIC

	//new_grant_trans is dead signal, just to keep the interface compatible
	void allocation(bool request[N][M], bool grant[M][N], out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=request complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1	
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0
		
		bool temp_grant[N][M];
		bool temp_grant_trans[M][N];
		#pragma HLS ARRAY_PARTITION variable=temp_grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_grant_trans complete dim=0

		uint32_t i,j;

		//First level arbiter
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].arbitration(&request[i][0],&temp_grant[i][0],&new_prior1[i]);
		}

		//matrix transpose
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				temp_grant_trans[j][i] = temp_grant[i][j];
			}
		}

		//Second level arbiter
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].arbitration(&temp_grant_trans[j][0],&grant[j][0],&new_prior2[j]);
		}
	}
	
	//new_grant_trans is dead signal, just to keep the interface compatible
	void update(out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0
		uint32_t i,j;
		
		//First level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].update(new_prior1[i]);
		}	

		//Second level
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].update(new_prior2[j]);
		}			
	}
};
	#endif//end SEPIF

	#ifdef SEPOF
class allocator{
public:
	arbiter <out_port_num_t,M> arb1[N];
	arbiter <in_port_num_t,N> arb2[M];
	//arbiter arb1[M];
	//arbiter arb2[N];

		#ifdef STATIC
	allocator(void){
		uint32_t i,j;
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].priority = i;
		}
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].priority = j;
		}
	}
		#endif//end STATIC

	//new_grant_trans is dead signal, just to keep the interface compatible	
	void allocation(bool request[N][M], bool grant[M][N], out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=request complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1	
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=0
		
		bool request_trans[M][N];
		bool temp_grant[M][N];
		bool temp_grant_trans[N][M];
		bool grant_trans[N][M];
		#pragma HLS ARRAY_PARTITION variable=request_trans complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_grant complete dim=0
		#pragma HLS ARRAY_PARTITION variable=temp_grant_trans complete dim=0
		#pragma HLS ARRAY_PARTITION variable=grant_trans complete dim=0

		uint32_t i,j;

		//matrix transpose
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			for(i=0;i<N;i++){
			#pragma HLS UNROLL
				request_trans[j][i] = request[i][j];
			}
		}

		//First level; arbitration on out_port first
		for(j=0;j<M;j++){
		#pragma HLS UNROLL
			arb2[j].arbitration(&request_trans[j][0],&temp_grant[j][0],&new_prior2[j]);
		}

		//matrix transpose
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(j=0;j<M;j++){
			#pragma HLS UNROLL
				temp_grant_trans[i][j] = temp_grant[j][i];
			}
		}

		//Second Level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].arbitration(&temp_grant_trans[i][0],&grant_trans[i][0],&new_prior1[i]);
		}
		
		//matrix transpose
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			for(j=0;j<M;j++){
			#pragma HLS UNROLL
				grant[j][i] = grant_trans[i][j];
			}
		}
	}
	
	//new_grant_trans is dead signal, just to keep the interface compatible
	void update(out_port_num_t new_prior1[N], in_port_num_t new_prior2[M], bool new_grant_trans[M][N]){
		#pragma HLS ARRAY_PARTITION variable=new_prior1 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_prior2 complete dim=1
		#pragma HLS ARRAY_PARTITION variable=new_grant_trans complete dim=1
		uint32_t i,j;
		
		//First level
		for(i=0;i<N;i++){
		#pragma HLS UNROLL
			arb1[i].update(new_prior1[i]);
		}	

		//Second level
		for(j=0;j<N;j++){
		#pragma HLS UNROLL
			arb2[j].update(new_prior2[j]);
		}			
	}	
};
	#endif//end SEPOF

#endif//end PIPE_ALLOC
