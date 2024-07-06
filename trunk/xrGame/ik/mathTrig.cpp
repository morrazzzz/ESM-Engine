 
/*
  This source code is a part of IKAN.
  Copyright (c) 2000 University of Pennsylvania
  Center for Human Modeling and Simulation
  All Rights Reserved.

  IN NO EVENT SHALL THE UNIVERSITY OF PENNSYLVANIA BE LIABLE TO ANY
  PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
  DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS
  SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF PENNSYLVANIA
  HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies. For for-profit purposes, please
  contact University of Pennsylvania
 (http://hms.upenn.edu/software/ik/ik.html) for the software license
  agreement.


  THE UNIVERSITY OF PENNSYLVANIA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
  BASIS, AND THE UNIVERSITY OF PENNSYLVANIA HAS NO OBLIGATION
  TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.

 */
#include "StdAfx.h"

#include "mathTrig.h"

//
// Solve a*cos(theta) + b*sin(theta) = c
// Either one or two solutions. Return the answer in radians
//

int solve_trig_math(float a, float b, float c, float theta[2])
{
    float temp  = (a*a+b*b-c*c);

    if (temp < 0.0)
    {
	// temp is practically zero
 
	if (_abs(temp / (_abs(a*a) + _abs(b*b) + _abs(c*c))) < 1e-6)
	{
	    // printf("Special case\n");
	    theta[0] = (float) (2*atan(-b/(-a-c)));
	    return 1;
	}
	else
	    return 0;
    }

    temp = (float)atan2((float)_sqrt(temp), (float)c);//.(float) c
    int num = temp * temp > 1e-6f ? 2 : 1;

    // Calculate answer in radians
    theta[0] = (float) atan2(b,a);
    if (num == 2)
    {
        theta[1] = theta[0] - temp;
        theta[0] += temp;

	//theta[0] = angle_normalize_signed(theta[0]);
	//theta[1] = angle_normalize_signed(theta[1]);
    }
    return num;
}
