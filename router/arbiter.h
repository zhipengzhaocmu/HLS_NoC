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
	Arbiter object. Grant 1 request from SIZE requests based on priority. 
*/
template <typename port_num_t, int SIZE>
#ifdef STATIC
//Static arbiter. The priority is fixed. 
class arbiter{
public:
	//No need to update the priority of STATIC arbiter.
	//However, here we still use "new_prior" and "update" 
	//to make the interface compatible with Non-Static 
	//arbiter. The passing value is alway the initialized
	//priority.
	port_num_t priority;

	void arbitration(bool request[SIZE], bool grant[SIZE], port_num_t *new_prior){
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=request complete dim=1
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=1
		int temp,i;
		bool temp_grant[SIZE];
		#pragma HLS ARRAY_PARTITION variable=temp_grant complete dim=1
		
		//initialize
		for(i = 0; i<SIZE; i++){
		#pragma HLS UNROLL
			temp_grant[i] = 0;
		}

		//arbitration
		for(i = 0; i<SIZE; i++){
		#pragma HLS UNROLL
			temp = priority - i -1;
			if(temp < 0 )
				temp = temp + SIZE;
			if(request[temp]){
				temp_grant[temp] = 1;
				break;
			}
		}

		//output
		for(i = 0; i<SIZE; i++){
		#pragma HLS UNROLL
			grant[i] = temp_grant[i];
		}

		//alwasy keep the initialized priority
		*new_prior = priority;
	}
	
	void update(port_num_t new_prior){
		priority = new_prior;
	}
};
#endif//end STATIC

#ifdef ROUNDROBIN
//RoundRobin Arbiter. After a round of arbitration, the priority changes. 
//Please refer to 18.4.2 Round-Robin Arbiter Chapter(p355) of "PRINCIPLES 
//AND OF INTERCONNECTION NETWORKS" by WILLIAM JAMES DALLY & BRIAN TOWLES. 
class arbiter{
public:
	port_num_t priority;

	void arbitration(bool request[SIZE], bool grant[SIZE], port_num_t *new_prior){
		#pragma HLS LATENCY max=0
		#pragma HLS ARRAY_PARTITION variable=request complete dim=1
		#pragma HLS ARRAY_PARTITION variable=grant complete dim=1
		uint32_t temp,i;
		bool temp_grant[SIZE];
		#pragma HLS ARRAY_PARTITION variable=temp_grant complete dim=1
		port_num_t temp_prior;

		//initialize
		for(i = 0; i<SIZE; i++){
		#pragma HLS UNROLL
			temp_grant[i] = 0;
		}
		//if there is no request, keep the original priority
		temp_prior = priority;
		
		//arbitration
		for(i = 0; i<N; i++){
		#pragma HLS UNROLL
			temp = priority+i;
			if(temp >= N)
				temp -= N;
			if(request[temp]){
				temp_prior = temp + 1;
				temp_grant[temp] = 1;
				break;
			}
		}

		//output
		for(i = 0; i<SIZE; i++){
		#pragma HLS UNROLL
			grant[i] = temp_grant[i];
		}
		
		*new_prior = temp_prior;
	}
	
	void update(port_num_t new_prior){
		priority = new_prior;
	}
};
#endif//end ROUNDROBIN
