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
	Buffer object based on ap_shift_reg. 
*/
template <typename T>
class flit_buffer{
public:
	ap_shift_reg <T, FIFO_DEPTH> Sreg;

	fifo_cnt_t cnt;

	bool empty(){
		return (cnt == 0);
	}

#ifdef PIPE_LINK
	bool almost_full(){
		return (cnt >= FIFO_DEPTH-2);
	}
#endif//end PIPE_LINK

	bool full(){
		return (cnt == FIFO_DEPTH);
	}

	T first(){
#pragma HLS INLINE
		T ret;
		fifo_cnt_t temp_cnt;
		fifo_cnt_t addr;

		if(empty())
			temp_cnt = 1;
		else
			temp_cnt = cnt;
		addr = temp_cnt-1;
		return Sreg.read(addr);
	}

	void update(T in, bool read_req, bool write_req){
#pragma HLS INLINE
		fifo_cnt_t temp_cnt;
		fifo_cnt_t addr;
		bool read_grant,write_grant;

		read_grant = !empty() && read_req;
		write_grant = !full() && write_req;

		if(full() && write_req){
			printf("WARNING: Write full!\n");
		}

		if(empty() && read_req){
			//printf("WARNING: Read empty!\n");
		}

		if(empty())
			temp_cnt = 1;
		else
			temp_cnt = cnt;
		addr = temp_cnt-1;
		if(write_grant)
			Sreg.shift(in, addr);

		//update cnt
		if(read_grant && !write_grant){
			cnt = cnt -1;
		}
		else if(write_grant && !read_grant){
			cnt = cnt + 1;
		}
	}
};
