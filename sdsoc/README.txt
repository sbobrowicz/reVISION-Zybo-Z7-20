Instructions
=====================
To install this reVISION platform and run the sample programs, simply follow the steps outlined in section 7 of Xilinx's reVISION Getting Started Guide 2017.2:

http://www.wiki.xilinx.com/reVISION+Getting+Started+Guide+2017.2#x7%20Tool%20Flow%20Tutorials

It is important to remember to export the SYSROOT variable prior to launching SDx so that it contains the location of zybo_z7_20/sw/sysroot/


Included Samples
=====================
File I/O:    
    *Bilateral Filter
    *Harris Filter
    *Dense Optical Flow
    *Stereo Vision (Depth Detection)
    *Warp Transformation


Further Information
=====================
For more information on using this platform and the included xfopencv libraries, please see the Xilinx OpenCV User Guide (ug1233):

https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_4/ug1233-xilinx-opencv-user-guide.pdf

NOTE: Although this platform is built for SDx 2017.2, it includes the 2017.4 version of the xfopencv libraries. Therefore the user
      should refer to the version of UG1233 that targets 2017.4, but be wary that any information regarding SDSoC found there
      may be slightly off.
