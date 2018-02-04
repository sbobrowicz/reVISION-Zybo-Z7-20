# Zybo Z7-20 reVISION Platform
Created for SDx 2017.2 and Petalinux 2017.2

## Downloading and Using the Platform

Please click on the releases tab in github to download the latest release for your version
of SDx. A README.txt is included with the release that describes how to use the reVISION 
platform.

## Platform Sources

This repository is used by Digilent to version control all the sources used to create the 
platform. It contains the Vivado IPI project, an SDSoC platform generator project and a 
Petalinux project. The SDSoC platform generator project also contains the xfopencv source
and some custom samples. These parts all come together to form a reVISION project.

Advanced users who wish to make modifications to this reVISION platform or use it as a
reference are welcome to. Digilent does not guarantee success, but if you need help please
feel free to reach out on https://forum.digilentinc.com

## Included Documentation

This document contains a procedure for building all the sources and generating the output
platform, however there are several other useful documents in this repo.

#####sdsoc/Platform_creation_notes.txt
   This document contains some notes and observations recorded during the platform creation
   process. It is for internal Digilent use, but may contain some useful info for other platform
   creators.
       
#####linux/petalinux_notes.txt
   This document describes the modifications that needed to be made to the standard Petalinux project
   for this board in order to work with reVISION and SDSoC.

#####linux/README.md
   The standard README that ships with Digilent Petalinux projects. It contains useful information for
   using and building the included Petalinux project.
  
## Known Issues

1. Typically the processing system IP core will infer a BUFG on the FCLK signals. For some reason, this is occuring for FCLK 0 only.
   FCLK2 seems to be getting a BUFG added during implementation, so it doesn't cause any issues for that net, but FCLK 1 was being
   routed as a normal signal (not on the global clock network). This caused insanely long build times and failure to meet timing. The
   current work around is to manually insert a BUFG on FCLK1 using a util_ds_buf IP core.

2. Audio is not functional

3. Currently we have not been able to get the Zybo Z7-20 to boot with a rootfs this large in initramfs mode. Our work around is to
   use an SD rootfs loaded on the second partition of the SD card. This allows for larger file system space, persistent changes, and
   more available system memory, but requires additional steps to prepare the SD card. Since the rootfs is not altered by SDx, a user 
   of this platform should only have to flash the SD card once. 

---------

#Building the Platform
WARNING*** You must have obtained this package using "git clone --recursive https://github.com/Digilent/reVISION-Zybo-Z7-20.git" or else these instructions will not work.

###Prerequisites
1. Host computer running Ubuntu 16.04.3 LTS 

2. Xilinx SDx 2017.2 installed to /opt/Xilinx/SDx/

3. Petalinux 2017.2 installed to /opt/pkg/petalinux/

###Procedure     
1. Open a terminal and run "source /opt/Xilinx/SDx/2017.2/settings64.sh"

2. cd into the Zybo-Z7-20-base-sdsoc folder where you cloned this package.

3. If you have built the Vivado IPI project at least once and it has not changed since the last build you can skip this step. Otherwise do the following:
    
    1. run "vivado" at the terminal to open Vivado.
    
    2. Open the .xpr file found in the proj folder if you have already generated the project, otherwise open the TCL console in Vivado and cd into the proj folder. Then run "source create_project.tcl". If you get IP locked errors, this is likely because you did not use "git clone --recursive" to download this package and therefore did not obtain the submodules.
  
    3. Generate the bitstream.
    
    4. Open the Implemented Project and then click File->Export->Hardware. Include the bitstream and save it to the hw_handoff folder.
   
    5. Close Vivado
    
    6. Copy the new hdf file in hw_handoff to sdsoc/prebuilt, renaming it so that it replaces the .hdf already found in that folder.
    
    7. Copy the bitstream from proj/zybo_z7_20.runs/impl_1/zybo_z7_20_wrapper.bit to sdsoc/prebuilt and rename it to bitstream.bit to replace the existing file there.
    
    8. open proj/zybo_z7_20.xpr in a text editor and remove the line that references "BoardPartRepoPaths" if it exists. Save and close the .xpr.

4. If changes have been made to the petalinux project, or it has not been built yet since cloning this repo, the following must be done:
   
    1. Follow the instructions in the linux/README.md file to build the project if it has not already been built.
    
    2. Replace sdsoc/linux/boot/u-boot.elf with linux/Zybo-Z7-20/images/linux/u-boot.elf
    
    3. Replace sdsoc/linux/image/image.ub with linux/Zybo-Z7-20/images/linux/image.ub

5. cd into the sdsoc folder

6. run "make" to generate the platform in the sdsoc/output folder.

    6. If make fails with the following error then delete everything in ~/.Xilinx/Vivado (you can leave your init.tcl or Vivado_init.tcl if you have one) and rerun make:

    ```
    ERROR: [Common 17-170] Unknown option '-origin_dir_override', please type 'write_project_tcl -help' for usage info.
    ```

7. Run the following commands to add the rootfs SD image and sysroot to the new platform:

    ```
    mkdir output/zybo_z7_20/sw/sd_image
    mkdir output/zybo_z7_20/sw/sysroot
    cp -f linux/Zybo-Z7-20/images/linux/rootfs.ext4 output/zybo_z7_20/sw/sd_image/
    cd output/zybo_z7_20/sw/sd_image
    mkdir mnt
    sudo mount -t ext4 -o loop ./rootfs.ext4 ./mnt
    cp -RLp ./mnt/. ../sysroot
    sudo umount ./mnt
    rm -rf ./mnt
    ```
    
8. Copy sdsoc/README.txt to sdsoc/output/zybo_z7_20/ 

9. The platform is now ready to use!

10. If you will be releasing this platform, right click the sdsoc/output/zybo_z7_20 folder and click Compress... to compress it as a Zip file. Name the Zip file as follows:

    ```
    reVISION-Zybo-Z7-20-v20XX.X-Y
    ```
    
   Where 20XX.X is the SDx version and Y is the release number. Then push a release commit and post the package as a github release.


 



