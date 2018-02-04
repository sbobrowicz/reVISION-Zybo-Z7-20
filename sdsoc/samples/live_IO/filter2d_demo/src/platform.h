/*
 * platform.h
 *
 *  Created on: Nov 17, 2017
 *      Author: digilent
 */

#ifndef SRC_PLATFORM_H_
#define SRC_PLATFORM_H_

#define UIO_SW_PATH "/dev/uio2"
#define UIO_GPIO_PATH "/dev/uio3"
#define UIO_DMA_PATH "/dev/uio4"
#define UIO_VTC_PATH "/dev/uio7"

#define UIO_DISP_CLK_PATH "/dev/uio0"
#define UIO_DISP_VDMA_PATH "/dev/uio5"
#define UIO_DISP_VTC_PATH "/dev/uio8"

#define FB0_ADDR 0x10000000
#define FB1_ADDR 0x10800000
#define FB2_ADDR 0x11000000
#define FB3_ADDR 0x11800000

/*
 * Input frame stride=2048x4
 */
#define INPUT_FB_MAX_W 1280
#define INPUT_FB_MAX_H 720
#define INPUT_FB_BPP DISPLAY_BPP
#define INPUT_FB_STRIDE (INPUT_FB_MAX_W*INPUT_FB_BPP)
#define INPUT_FB_SIZE (INPUT_FB_STRIDE*INPUT_FB_MAX_H)

#define NUM_INPUT_FB 1

#define MAX_HEIGHT INPUT_FB_MAX_H
#define MAX_WIDTH INPUT_FB_MAX_W
#define MAX_STRIDE INPUT_FB_STRIDE

#define KSIZE 3
#define SHIFT 0

#define INPUT_DETECT_TIMEOUT 5

#endif /* SRC_PLATFORM_H_ */
