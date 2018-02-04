/*
 * main.c
 *
 *  Created on: Nov 8, 2017
 *      Author: digilent
 */
#include <stdio.h>
#include "platform.h"
#include "filter2d/filter2d_int.h"
#include "filter2d/filter2d_xf.h"
#include "common/xf_common.h"
#include "sds_lib.h"
#include "display_ctrl/display_ctrl.h"
#include "uio/uio_ll.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char **argv)
{
//FBDEV stuff
    int tty_fd, mem_fd;

    uint32_t frameAddr;
    uint32_t stride;
    uint32_t hActiveIn, vActiveIn;

    int gpio_fd, dma_fd, vtc_fd, sw_fd;
    uint8_t *gpioMem, *dmaMem, *vtcMem, *swMem;

    XF_TNAME(XF_8UC4,XF_NPPC1) *fb_buf[DISPLAY_NUM_FRAMES];
    uint32_t fb_addr[DISPLAY_NUM_FRAMES];

    filter2d_data f2d_data = {0};

    DisplayCtrl dispCtrl;
    const drm_mode_modeinfo *dispMode;

    /*
     * Video input UIO initialization
     */
	gpio_fd = open(UIO_GPIO_PATH, O_RDWR);
	vtc_fd = open(UIO_VTC_PATH, O_RDWR);
	gpioMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, gpio_fd, (off_t)0);
	vtcMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vtc_fd, (off_t)0);
    /*
     *Check if HDMI input clock locked
     */
	UioWrite32(gpioMem, 0, 1); //
	UioWrite32(gpioMem, 4, 0); //set HPD as output and assert it
	UioWrite32(gpioMem, 12, 1); //set clock locked as input
	int isLocked = 0;
	for(int i = 0; i < INPUT_DETECT_TIMEOUT && !isLocked; i++)
	{
		sleep(1);
		isLocked = UioRead32(gpioMem, 8);
	}
	if (!isLocked)
	{
		printf("HDMI input clock not detected, attach HDMI source");
		close(gpio_fd);
		close(vtc_fd);
		return -1;
	}

    /*
     *Enable VTC and wait for it to lock
     */
	UioWrite32(vtcMem, 0, 8); //Enable detector
	isLocked = 0;
	for(int i = 0; i < INPUT_DETECT_TIMEOUT && !isLocked; i++)
	{
		sleep(1);
		isLocked = UioRead32(vtcMem, 0x24) & 1;
	}
	if (!isLocked) //check if locked
	{
		printf("VTC not locked, attach a stable HDMI source");
		close(gpio_fd);
		close(vtc_fd);
		return -1;
	}
	hActiveIn = UioRead32(vtcMem,0x20);
	vActiveIn = (hActiveIn & 0x1FFF0000) >> 16;
	hActiveIn = hActiveIn & 0x1FFF;
	printf("Input Resolution: %dx%d\n",hActiveIn,vActiveIn);



    /*
     * mmap input framebuffers
     * TODO: Properly close /dev/mem handle on exit
     */
	mem_fd = open("/dev/mem", O_RDWR);
    if(mem_fd == -1) {
	   fprintf(stderr, "Could not open /dev/mem\n");
	   return -1;
    }
    fb_addr[0] = FB0_ADDR;
    fb_addr[1] = FB1_ADDR;
    fb_addr[2] = FB2_ADDR;
    for (int i = 0; i < DISPLAY_NUM_FRAMES; i++)
    {
    	fb_buf[i] = (XF_TNAME(XF_8UC4,XF_NPPC1) *) sds_mmap((void *) fb_addr[i], hActiveIn*DISPLAY_BPP*vActiveIn, NULL);
    }

	/*
     * Display setup
     */
	if (DisplayInitialize(&dispCtrl, UIO_DISP_VTC_PATH, UIO_DISP_VDMA_PATH,UIO_DISP_CLK_PATH, (uint8_t **) fb_addr,hActiveIn*DISPLAY_BPP) != DISPLAY_SUCCESS)
	{
		fprintf(stderr, "Display Initialization Failed\n");
		return -1;
	}
	dispMode = DisplayFindMode(hActiveIn,vActiveIn);
	if (dispMode == NULL)
	{
		fprintf(stderr, "Could not find output mode that matches dimensions of \n" \
						"input mode. Add \"%dx%d\" input mode to drm_modes.h\n", hActiveIn, vActiveIn);
		return -1;
	}
	if (DisplaySetMode(&dispCtrl,dispMode) != DISPLAY_SUCCESS)
	{
		fprintf(stderr, "Display Set Mode Failed\n");
		return -1;
	}
	if (DisplayStart(&dispCtrl) != DISPLAY_SUCCESS)
	{
		fprintf(stderr, "Display Start Failed\n");
		return -1;
	}
	printf("Output Resolution: %s\n",dispMode->name);


    /*
     *Start Input Video VDMA
     */
	dma_fd = open(UIO_DMA_PATH, O_RDWR);
	dmaMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, (off_t)0);
	UioWrite32(dmaMem, 0x30, 3); //Enable S2MM channel with Circular park
	UioWrite32(dmaMem, 0xAC, fb_addr[1]); //Set the frame address to physical frame of display
	UioWrite32(dmaMem, 0x28, 0); //Set Park frame to frame 0
	UioWrite32(dmaMem, 0xA4, hActiveIn * DISPLAY_BPP); //Set horizontal active size in bytes
	UioWrite32(dmaMem, 0xA8, 0x1000000 | dispCtrl.stride); //Set stride and preserve frame delay
	UioWrite32(dmaMem, 0xA0, vActiveIn); //Set the vertical active size, starting a transfer
	close(dma_fd);

	f2d_data.src = new xf::Mat<XF_8UC4, MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> (MAX_HEIGHT, MAX_WIDTH);  //(vActiveIn, hActiveIn); Use this line instead in future
	f2d_data.dst = new xf::Mat<XF_8UC4, MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> (MAX_HEIGHT, MAX_WIDTH);  //(vActiveIn, hActiveIn);
	f2d_data.img_y = new xf::Mat<XF_8UC1, MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> (MAX_HEIGHT, MAX_WIDTH);  //(vActiveIn, hActiveIn);
	f2d_data.img_u = new xf::Mat<XF_8UC1, MAX_HEIGHT/4, MAX_WIDTH, XF_NPPC1> (MAX_HEIGHT/4, MAX_WIDTH); //(vActiveIn/4, hActiveIn);
	f2d_data.img_v = new xf::Mat<XF_8UC1, MAX_HEIGHT/4, MAX_WIDTH, XF_NPPC1> (MAX_HEIGHT/4, MAX_WIDTH);  //(vActiveIn/4, hActiveIn);
	f2d_data.img_yf = new xf::Mat<XF_8UC1, MAX_HEIGHT, MAX_WIDTH, XF_NPPC1> (MAX_HEIGHT, MAX_WIDTH);  //(vActiveIn, hActiveIn);

	f2d_data.src->data = (XF_TNAME(XF_8UC4,XF_NPPC1) *)  fb_buf[1];
	f2d_data.dst->data = (XF_TNAME(XF_8UC4,XF_NPPC1) *) fb_buf[0];

	/*
	 * Set Switches and buttons as input
	 */
    sw_fd = open(UIO_SW_PATH, O_RDWR);
	swMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, sw_fd, (off_t)0);
	UioWrite32(swMem, 4, 0xF); //set all 4 switches as input
	UioWrite32(swMem, 12, 0xF); //set all 4 buttons as input

	printf("Running Loop...\n");
	int i = 0;
	while (1)
	{
		uint32_t sw = UioRead32(swMem, 0);
		uint32_t btn = UioRead32(swMem, 8);
		if (btn & 1)
		{
			//Currently untested and/or broken. Don't push the push button.
			//printf("Calling cv filter\n");
			//filter2d_cv((uint32_t *) fb_buf[0],(uint32_t *) drm.fbMem[drm.current_fb],vActiveIn, hActiveIn, drm.create_dumb[drm.current_fb].pitch, coeff_identity);
		}
		else
		{
			switch (sw)
			{
			case 0 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_off);
				break;
			case 2 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_blur);
				break;
			case 3 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_edge);
				break;
			case 4 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_edge_h);
				break;
			case 5 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_edge_v);
				break;
			case 6 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_emboss);
				break;
			case 7 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_gradient_h);
				break;
			case 8 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_gradient_v);
				break;
			case 9 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_sharpen);
				break;
			case 10 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_sobel_h);
				break;
			case 11 :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_sobel_v);
				break;
			default :
				filter2d_xf(&f2d_data, dispCtrl.stride, coeff_identity);
			}
		}
	}

    return 0;
}

void exit_clean(struct drm_cntrl * drm)
{
	return;
}
