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

#define STYLE2

void switch_comb (VData I1, VData I2,
			   VData *Odd, VData *Even,
			   bool *acpt1, bool *acpt2) {

#pragma HLS INTERFACE ap_ctrl_none port=return

#ifdef STYLE1
	//Source code from the report.
  *acpt1=*acpt2=(*Odd).v=(*Even).v=false;

  if (I1.v && (I1.d%2))
  	  { (*Odd).v=true; (*Odd).d=I1.d; *acpt1=true; }
  else if (I2.v && (I2.d%2))
  	  { (*Odd).v=true; (*Odd).d=I2.d; *acpt2=true; }

  if (I1.v && !(I1.d%2))
  	  { (*Even).v=true; (*Even).d=I1.d; *acpt1=true; }
  else if (I2.v && !(I2.d%2))
  	  { (*Even).v=true; (*Even).d=I2.d; *acpt2=true; }

#endif

#ifdef STYLE2
  //Avoid multiple access to the pointer. Then we can get rid of the associated ovld signals.
#pragma HLS INTERFACE ap_none port=acpt2
#pragma HLS INTERFACE ap_none port=acpt1
#pragma HLS INTERFACE ap_none port=Even
#pragma HLS INTERFACE ap_none port=Odd
  bool temp_acpt1;
  bool temp_acpt2;

  VData temp_Odd;
  VData temp_Even;

  temp_acpt1=temp_acpt2=temp_Odd.v=temp_Even.v=false;

  if (I1.v && (I1.d%2))
  	  { temp_Odd.v=true; temp_Odd.d=I1.d; temp_acpt1=true; }
  else if (I2.v && (I2.d%2))
  	  { temp_Odd.v=true; temp_Odd.d=I2.d; temp_acpt2=true; }

  if (I1.v && !(I1.d%2))
  	  { temp_Even.v=true; temp_Even.d=I1.d; temp_acpt1=true; }
  else if (I2.v && !(I2.d%2))
  	  { temp_Even.v=true; temp_Even.d=I2.d; temp_acpt2=true; }

  *Odd = temp_Odd;
  *Even = temp_Even;
  *acpt1 = temp_acpt1;
  *acpt2 = temp_acpt2;
#endif
}
