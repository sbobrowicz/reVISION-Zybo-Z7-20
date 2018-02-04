/************************************************************************/
/*																		*/
/*	display_ctrl.c	--	Digilent Display Controller Driver				*/
/*																		*/
/************************************************************************/
/*	Author: Sam Bobrowicz												*/
/*	Copyright 2014, Digilent Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*																		*/
/*		This module provides an easy to use API for controlling a    	*/
/*		Display attached to a Digilent system board via VGA or HDMI. 	*/
/*		run-time resolution setting and seamless framebuffer-swapping 	*/
/*		for tear-free animation. 										*/
/*																		*/
/*		To use this driver, you must have a Xilinx Video Timing 		*/
/* 		Controller core (vtc), Xilinx axi_vdma core, a Digilent 		*/
/*		axi_dynclk core, a Xilinx AXI Stream to Video core, and either  */
/*		a Digilent RGB2VGA or RGB2DVI core all present in your design.  */
/*		See the Video in or Display out reference projects for your     */
/*		system board to see how they need to be connected. Digilent     */
/*		reference projects and IP cores can be found at 				*/
/*		www.github.com/Digilent.			 							*/
/*																		*/
/*		The following steps should be followed to use this driver:		*/
/*		1) Create a DisplayCtrl object and pass a pointer to it to 		*/
/*		   DisplayInitialize.											*/
/*		2) Call DisplaySetMode to set the desired mode					*/
/*		3) Call DisplayStart to begin outputting data to the display	*/
/*		4) To create a seamless animation, draw the next image to a		*/
/*		   framebuffer currently not being displayed. Then call 		*/
/*		   DisplayChangeFrame to begin displaying that frame.			*/
/*		   Repeat as needed, only ever modifying inactive frames.		*/
/*		5) To change the resolution, call DisplaySetMode, followed by	*/
/*		   DisplayStart again.											*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/* 																		*/
/*		2/20/2014(SamB): Created										*/
/*		11/25/2015(SamB): Changed from axi_dispctrl to Xilinx cores		*/
/*						  Separated Clock functions into dynclk library */
/*																		*/
/************************************************************************/
/*
 * TODO: It would be nice to remove the need for users above this to access
 *       members of the DisplayCtrl struct manually. This can be done by
 *       implementing get/set functions for things like video mode, state,
 *       etc.
 */


/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

/*
 * Uncomment for Debugging messages over UART
 */
//#define DEBUG

#include "display_ctrl.h"
#include "dynclk/dynclk.h"
#include "uio/uio_ll.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

/***	DisplayStop(DisplayCtrl *dispPtr)
**
**	Parameters:
**		dispPtr - Pointer to the initialized DisplayCtrl struct
**
**	Return Value: int
**		XST_SUCCESS if successful.
**		XST_DMA_ERROR if an error was detected on the DMA channel. The
**			Display is still successfully stopped, and the error is
**			cleared so that subsequent DisplayStart calls will be
**			successful. This typically indicates insufficient bandwidth
**			on the AXI Memory-Map Interconnect (VDMA<->DDR)
**
**	Description:
**		Halts output to the display
**
*/
int DisplayStop(DisplayCtrl *dispPtr)
{
	int vtc_fd, vdma_fd;
	uint8_t *vtcMem;
	uint8_t *vdmaMem;
	uint32_t regVal;

	/*
	 * If already stopped, do nothing
	 */
	if (dispPtr->state == DISPLAY_STOPPED)
	{
		return DISPLAY_SUCCESS;
	}

	/*
	 * Disable the disp_ctrl core, and wait for the current frame to finish (the core cannot stop
	 * mid-frame)
	 */
	vtc_fd = open(dispPtr->vtcUioFile, O_RDWR);
	vtcMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vtc_fd, (off_t)0);
	regVal = 0x01ffff03; //Set the signal sources in control register, enable core and register update, disable generator
	UioWrite32(vtcMem, 0x00, regVal);
	close(vtc_fd);

	/*
	 * Stop the VDMA core
	 */
	vdma_fd = open(dispPtr->vdmaUioFile, O_RDWR);
	vdmaMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vdma_fd, (off_t)0);
	UioWrite32(vdmaMem, 0x00, 0); //Disable VDMA
	while(!(UioRead32(vdmaMem, 0x04) & 0x1)); //Wait until VDMA halts
	close(vdma_fd);

	/*
	 * Update Struct state
	 */
	dispPtr->state = DISPLAY_STOPPED;

	//TODO: consider stopping the clock here, perhaps after a check to see if the VTC is finished

	/*
	 * NOTE: May need to clear errors here by writing 0xFFFFFFFF to 0x04
	 */

	return DISPLAY_SUCCESS;
}
/* ------------------------------------------------------------ */

/***	DisplayStart(DisplayCtrl *dispPtr)
**
**	Parameters:
**		dispPtr - Pointer to the initialized DisplayCtrl struct
**
**	Return Value: int
**		XST_SUCCESS if successful, XST_FAILURE otherwise
**
**	Errors:
**
**	Description:
**		Starts the display.
**
*/
int DisplayStart(DisplayCtrl *dispPtr)
{
	ClkConfig clkReg;
	ClkMode clkMode;
	uint32_t regVal;
	int vtc_fd, vdma_fd;
	uint8_t *vtcMem;
	uint8_t *vdmaMem;

	/*
	 * If already started, do nothing
	 */
	if (dispPtr->state == DISPLAY_RUNNING)
	{
		return DISPLAY_SUCCESS;
	}


	/*
	 * Calculate the PLL divider parameters based on the required pixel clock frequency
	 */
	ClkFindParams(((double) dispPtr->vMode.clock) / 1000.0, &clkMode);

	/*
	 * Store the obtained frequency to pxlFreq. It is possible that the PLL was not able to
	 * exactly generate the desired pixel clock, so this may differ from vMode.freq.
	 */
	dispPtr->pxlFreq = clkMode.freq;

	/*
	 * Write to the PLL dynamic configuration registers to configure it with the calculated
	 * parameters.
	 */
	if (!ClkFindReg(&clkReg, &clkMode))
	{
		fprintf(stderr, "Error calculating CLK register values\n\r");
		return DISPLAY_FAILURE;
	}
	ClkWriteReg(&clkReg, dispPtr->clkUioFile);

	/*
	 * Enable the dynamically generated clock
    */
	ClkStop(dispPtr->clkUioFile);
	ClkStart(dispPtr->clkUioFile);

	/*
	 * Initialize VTC by doing the following (see PG016 from Xilinx):
	 *
	 * Open UIO and map register memory
	 * Set regupdate and enable core (set bit2 and bit1 (mask=0x3) of control reg (0x0))
	 * Set Active size reg 0x60 (((VSize << 16) & 0x0x1FFF0000) | (HSize & 0x1FFF))
	 * Set HSize reg 0x70 (HTotal & 0x1FFF)
	 * Set VSize Reg 0x74 (VTotal & 0x1FFF)
	 * Set Hsync reg 0x78 (((HSyncEnd  << 16) & 0x0x1FFF0000) | (HSyncStart & 0x1FFF))
	 * Set F0 Vsync reg 0x80 (((VSyncEnd  << 16) & 0x0x1FFF0000) | (VSyncStart & 0x1FFF))
	 * Set F1 Vsync reg 0x8C (((VSyncEnd  << 16) & 0x0x1FFF0000) | (VSyncStart & 0x1FFF))
	 * Set VSync H offset registers (see notes below)
	 * Set Encoding reg 0x68 (to 0x2)
	 * Set Polarity reg 0x6C
	 * Set Control reg (source and enable) (0x0x01ff_ff07)
	 */
	vtc_fd = open(dispPtr->vtcUioFile, O_RDWR);
	vtcMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vtc_fd, (off_t)0);

	regVal = 0x01ffff03; //Set the signal sources in control register, enable core and register update, disable generator
	UioWrite32(vtcMem, 0x00, regVal);

	regVal = (((dispPtr->vMode.vdisplay  <<  16) & 0x1FFF0000) | (dispPtr->vMode.hdisplay & 0x1FFF));
	UioWrite32(vtcMem, 0x60, regVal);

	regVal = dispPtr->vMode.htotal & 0x1FFF;
	UioWrite32(vtcMem, 0x70, regVal);

	regVal = dispPtr->vMode.vtotal & 0x1FFF;
	UioWrite32(vtcMem, 0x74, regVal);

	regVal = (((dispPtr->vMode.hsync_end  <<  16) & 0x1FFF0000) | (dispPtr->vMode.hsync_start & 0x1FFF));
	UioWrite32(vtcMem, 0x78, regVal);

	regVal = (((dispPtr->vMode.vsync_end  <<  16) & 0x1FFF0000) | (dispPtr->vMode.vsync_start & 0x1FFF));
	UioWrite32(vtcMem, 0x80, regVal);

	regVal = (((dispPtr->vMode.vsync_end  <<  16) & 0x1FFF0000) | (dispPtr->vMode.vsync_start & 0x1FFF));
	UioWrite32(vtcMem, 0x8C, regVal);

	/*
	 * Not sure what these 4 additional Vsync settings are for, but I think they have something to do with Vsync's
	 * relation to Hsync. They should be inconsequential. I set them the same way Xilinx baremetal drivers included
	 * with XSDK do.
	 */
	regVal = (((dispPtr->vMode.hdisplay  <<  16) & 0x1FFF0000) | (dispPtr->vMode.hdisplay & 0x1FFF));
	UioWrite32(vtcMem, 0x7C, regVal);
	UioWrite32(vtcMem, 0x88, regVal);
	regVal = (((dispPtr->vMode.hsync_start  <<  16) & 0x1FFF0000) | (dispPtr->vMode.hsync_start & 0x1FFF));
	UioWrite32(vtcMem, 0x84, regVal);
	UioWrite32(vtcMem, 0x90, regVal);

	regVal = 0x2; //Set the encoder register to standard RGB encoding
	UioWrite32(vtcMem, 0x68, regVal);

	regVal = 0x13; //Start with Active-video and blank signals set to active-high polarity
	if (dispPtr->vMode.flags & DRM_MODE_FLAG_PHSYNC)
	{
		regVal = regVal | (1<<3); //set HSync active high
	}
	if (dispPtr->vMode.flags & DRM_MODE_FLAG_PVSYNC)
	{
		regVal = regVal | (1<<2);
	}
	UioWrite32(vtcMem, 0x6C, regVal);

	regVal = 0x01ffff07; //Set the signal sources in control register, enable core and register update, Enable generator
	UioWrite32(vtcMem, 0x00, regVal);

	close(vtc_fd);

	/*
	 * Configure the VDMA to access a frame with the same dimensions as the
	 * current mode and start the transfer.
	 */
	vdma_fd = open(dispPtr->vdmaUioFile, O_RDWR);
	vdmaMem = (uint8_t *) mmap(0, UIO_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, vdma_fd, (off_t)0);
	UioWrite32(vdmaMem, 0x00, 3); //Enable MM2S channel with Circular park
	UioWrite32(vdmaMem, 0x5C, (uint32_t) (dispPtr->framePtr[dispPtr->curFrame])); //Set the frame address to physical frame of display
	UioWrite32(vdmaMem, 0x28, 0); //Set Park frame to frame 0
	UioWrite32(vdmaMem, 0x54, dispPtr->vMode.hdisplay * DISPLAY_BPP); //Set horizontal active size in bytes
	UioWrite32(vdmaMem, 0x58, 0x01000000 | (dispPtr->stride & 0xFFFF)); //Set stride and preserve frame delay
	UioWrite32(vdmaMem, 0x50, dispPtr->vMode.vdisplay); //Set the vertical active size, starting a transfer
	close(vdma_fd);

	dispPtr->state = DISPLAY_RUNNING;

	return DISPLAY_SUCCESS;
}

/* ------------------------------------------------------------ */

/***	DisplayInitialize(DisplayCtrl *dispPtr, XAxiVdma *vdma, uint16_t vtcId, uint32_t dynClkAddr, uint8_t *framePtr[DISPLAY_NUM_FRAMES], uint32_t stride)
**
**	Parameters:
**		dispPtr - Pointer to the struct that will be initialized
**		vdma - Pointer to initialized VDMA struct
**		vtcId - Device ID of the VTC core as found in xparameters.h
**		dynClkAddr - BASE ADDRESS of the axi_dynclk core
**		framePtr - array of pointers to the framebuffers. The framebuffers must be instantiated above this driver, and there must be 3
**		stride - line stride of the framebuffers. This is the number of bytes between the start of one line and the start of another.
**
**	Return Value: int
**		XST_SUCCESS if successful, XST_FAILURE otherwise
**
**	Errors:
**
**	Description:
**		Initializes the driver struct for use.
**
*/
int DisplayInitialize(DisplayCtrl *dispPtr, const char *vtcUioFile, const char *vdmaUioFile, const char *clkUioFile, uint8_t *framePtr[DISPLAY_NUM_FRAMES], uint32_t stride)
{
	int Status;
	int i;
	ClkConfig clkReg;
	ClkMode clkMode;

	/*
	 * Initialize all the fields in the DisplayCtrl struct
	 */
	dispPtr->curFrame = 0;
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
	{
		dispPtr->framePtr[i] = framePtr[i];
	}
	dispPtr->state = DISPLAY_STOPPED;
	dispPtr->stride = stride;
	dispPtr->vMode = drm_dmt_modes[3]; //Set default mode to 640x480
	dispPtr->vdmaUioFile = vdmaUioFile;
	dispPtr->vtcUioFile = vtcUioFile;
	dispPtr->clkUioFile = clkUioFile;


	ClkFindParams( ((double) dispPtr->vMode.clock) / 1000.0, &clkMode);

	/*
	 * Store the obtained frequency to pxlFreq. It is possible that the PLL was not able to
	 * exactly generate the desired pixel clock, so this may differ from vMode.freq.
	 */
	dispPtr->pxlFreq = clkMode.freq;

	/*
	 * Write to the PLL dynamic configuration registers to configure it with the calculated
	 * parameters.
	 */
	if (!ClkFindReg(&clkReg, &clkMode))
	{
		fprintf(stderr, "Error calculating CLK register values\n\r");
		return DISPLAY_FAILURE;
	}
	ClkWriteReg(&clkReg, dispPtr->clkUioFile);

	/*
	 * Enable the dynamically generated clock
    */
	ClkStart(dispPtr->clkUioFile);

	return DISPLAY_SUCCESS;
}
/* ------------------------------------------------------------ */

/***	DisplaySetMode(DisplayCtrl *dispPtr, const VideoMode *newMode)
**
**	Parameters:
**		dispPtr - Pointer to the initialized DisplayCtrl struct
**		newMode - The VideoMode struct describing the new mode.
**
**	Return Value: int
**		XST_SUCCESS if successful, XST_FAILURE otherwise
**
**	Errors:
**
**	Description:
**		Changes the resolution being output to the display. If the display
**		is currently started, it is automatically stopped (DisplayStart must
**		be called again).
**
*/
int DisplaySetMode(DisplayCtrl *dispPtr, const drm_mode_modeinfo *newMode)
{
	int Status;

	/*
	 * If currently running, stop
	 */
	if (dispPtr->state == DISPLAY_RUNNING)
	{
		Status = DisplayStop(dispPtr);
		if (Status != DISPLAY_SUCCESS)
		{
			fprintf(stderr, "Cannot change mode, unable to stop display %d\r\n", Status);
			return Status;
		}
	}

	dispPtr->vMode = *newMode;

	return DISPLAY_SUCCESS;
}
/* ------------------------------------------------------------ */

/***	DisplayChangeFrame(DisplayCtrl *dispPtr, uint32_t frameIndex)
**
**	Parameters:
**		dispPtr - Pointer to the initialized DisplayCtrl struct
**		frameIndex - Index of the framebuffer to change to (must
**				be between 0 and (DISPLAY_NUM_FRAMES - 1))
**
**	Return Value: int
**		XST_SUCCESS if successful, XST_FAILURE otherwise
**
**	Errors:
**
**	Description:
**		Changes the frame currently being displayed.
**
*/

int DisplayChangeFrame(DisplayCtrl *dispPtr, uint32_t frameIndex)
{
	int Status;

	dispPtr->curFrame = frameIndex;
	/*
	 * If currently running, then the DMA needs to be told to start reading from the desired frame
	 * at the end of the current frame
	 */
	if (dispPtr->state == DISPLAY_RUNNING)
	{
		DisplayStop(dispPtr);
		DisplayStart(dispPtr);
	}

	return DISPLAY_SUCCESS;
}

const drm_mode_modeinfo *DisplayFindMode(uint32_t width, uint32_t height)
{
	const drm_mode_modeinfo *newMode;

	newMode = NULL;
    for (int i = 0; i < NUM_STANDARD_MODES; i++)
    {
    	if (width == drm_dmt_modes[i].hdisplay && height == drm_dmt_modes[i].vdisplay)
    		newMode = &(drm_dmt_modes[i]);
    }

	return newMode;
}

/************************************************************************/

