/*
 * filter2d_xf.h
 *
 *  Created on: Nov 21, 2017
 *      Author: digilent
 */

#ifndef SRC_FILTER2D_FILTER2D_XF_H_
#define SRC_FILTER2D_FILTER2D_XF_H_

#include "platform.h"
#include "common/xf_common.h"

typedef struct filter2d_data{
	xf::Mat<XF_8UC4,  MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> *src;
	xf::Mat<XF_8UC4,  MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> *dst;
	xf::Mat<XF_8UC1, MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> *img_y;
	xf::Mat<XF_8UC1, MAX_HEIGHT/4, MAX_WIDTH, XF_NPPC1> *img_u;
	xf::Mat<XF_8UC1, MAX_HEIGHT/4, MAX_WIDTH, XF_NPPC1> *img_v;
	xf::Mat<XF_8UC1, MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> *img_yf;
} filter2d_data;

void filter2d_xf( filter2d_data *data, int stride, const short int coeff[KSIZE][KSIZE]);

#endif /* SRC_FILTER2D_FILTER2D_XF_H_ */
