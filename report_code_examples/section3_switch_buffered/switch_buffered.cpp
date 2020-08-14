/*
	Copyright (c) <2017> <Zhipeng Zhao and James C. Hoe, Carnegie Mellon university>

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

#include "switch.h"
#include "buffer.h"

Path allocate(Path req) {
  Path grnt={false, false, false, false};

  if (req.p1xOd) grnt.p1xOd=true;
  else if (req.p2xOd) grnt.p2xOd=true;

  if (req.p1xEv) grnt.p1xEv=true;
  else if (req.p2xEv) grnt.p2xEv=true;

  return grnt;
}

Path decode(VData I1, VData I2) {
  Path req={false, false, false, false};

  if (I1.v&&(I1.d%2)) req.p1xOd=true;
  else if (I2.v&&(I2.d%2)) req.p2xOd=true;

  if (I1.v&&!(I1.d%2)) req.p1xEv=true;
  else if (I2.v&&!(I2.d%2)) req.p2xEv=true;

  return req;
}

void switch_2stage(VData I1, VData I2,
		      VData *Odd, VData *Even,
		      bool *acpt1, bool *acpt2) {

  static VData L1={false,0};
  static VData L2={false,0};
  static Path Lreq={false,false,false,false};

  Path grnt;

  // stage 2
  grnt=allocate(Lreq);

  if (grnt.p1xOd) *Odd=L1;
  else if (grnt.p2xOd) *Odd=L2;
    else (*Odd).v=false;

  if (grnt.p1xEv) *Even=L1;
  else if (grnt.p2xEv) *Even=L2;
  else (*Even).v=false;

  // stage 1
  if (grnt.p1xOd||grnt.p1xEv) L1.v=false;
  if (I1.v && (!L1.v)) { *acpt1=true; L1=I1; }
  else *acpt1=false;

  if (grnt.p2xOd||grnt.p2xEv) L2.v=false;
  if (I2.v && (!L2.v)) { *acpt2=true; L2=I2; }
  else *acpt2=false;

  Lreq=decode(L1,L2);
}

void switch_buffered(VData I1, VData I2,
		 VData *Odd, VData *Even,
		 bool *acpt1, bool *acpt2) {
#pragma HLS LATENCY max=0

#pragma HLS INTERFACE ap_ctrl_none port=return
#pragma HLS INTERFACE ap_none port=acpt2
#pragma HLS INTERFACE ap_none port=acpt1
#pragma HLS INTERFACE ap_none port=Even
#pragma HLS INTERFACE ap_none port=Odd

  static FIFO<int> F1,F2;
  bool okX1, okX2;

  VData X1, X2;
  X1.v=!F1.empty(); X1.d=F1.front();
  X2.v=!F2.empty(); X2.d=F2.front();

  switch_2stage
    (X1, X2, Odd, Even, &okX1, &okX2);

  if (!F1.full() && I1.v) { *acpt1=true; }
  else *acpt1=false;

  if (!F2.full() && I2.v) { *acpt2=true; }
  else *acpt2=false;

  //update the buffer by providing input_data + read_req + write_req
  F1.next(I1.d,okX1,I1.v);
  F2.next(I2.d,okX2,I2.v);
}
