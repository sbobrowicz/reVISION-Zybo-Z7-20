# 1 "/home/digilent/work/git/Zybo-Z7-20-base-sdsoc/linux/Zybo-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/digilent/work/git/Zybo-Z7-20-base-sdsoc/linux/Zybo-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts"







/dts-v1/;
/include/ "zynq-7000.dtsi"
/include/ "pl.dtsi"
/include/ "pcw.dtsi"
/ {
 chosen {
  bootargs = "earlycon";
  stdout-path = "serial0:115200n8";
 };
 aliases {
  ethernet0 = &gem0;
  serial0 = &uart1;
  spi0 = &qspi;
 };
 memory {
  device_type = "memory";
  reg = <0x0 0x40000000>;
 };
 cpus {
 };
};
# 1 "/home/digilent/work/git/Zybo-Z7-20-base-sdsoc/linux/Zybo-Z7-20/build/tmp/work/plnx_arm-xilinx-linux-gnueabi/device-tree-generation/xilinx+gitAUTOINC+43551819a1-r0/system-user.dtsi" 1
/include/ "system-conf.dtsi"
# 1 "/home/digilent/work/git/Zybo-Z7-20-base-sdsoc/linux/Zybo-Z7-20/build/tmp/work-shared/plnx_arm/kernel-source/include/dt-bindings/gpio/gpio.h" 1
# 3 "/home/digilent/work/git/Zybo-Z7-20-base-sdsoc/linux/Zybo-Z7-20/build/tmp/work/plnx_arm-xilinx-linux-gnueabi/device-tree-generation/xilinx+gitAUTOINC+43551819a1-r0/system-user.dtsi" 2

/ {
 model = "Zynq Zybo Z7 Development Board";
 compatible = "digilent,zynq-zyboz7", "xlnx,zynq-7000";

 chosen {
  bootargs = "console=ttyPS0,115200 earlyprintk uio_pdrv_genirq.of_id=generic-uio quiet root=/dev/mmcblk0p2 rw rootwait";
 };

 usb_phy0: usb_phy@0 {
  compatible = "ulpi-phy";
  #phy-cells = <0>;
  reg = <0xe0002000 0x1000>;
  view-port = <0x0170>;
  drv-vbus;
 };

 reserved-memory {
  #address-cells = <1>;
  #size-cells = <1>;
  ranges;

  framebuffer_reserved: framebuffer@10000000 {
   reg = <0x10000000 0x2000000>;
  };
 };
};

&gem0 {
 phy-handle = <&ethernet_phy>;



 ethernet_phy: ethernet-phy@1 {
  reg = <1>;
  device_type = "ethernet-phy";
 };
};

&usb0 {
 usb-phy = <&usb_phy0>;
};
&sdhci0 {
 u-boot,dm-pre-reloc;
};
&uart1 {
 u-boot,dm-pre-reloc;
};

&qspi {
 u-boot,dm-pre-reloc;
};
&flash0 {
 compatible = "micron,m25p80", "s25fl128s", "spi-flash";
};



&amba_pl {
 encoder_0: digilent_encoder {
  compatible = "digilent,drm-encoder";
  digilent,fmax = <150000>;
  digilent,edid-i2c = <&i2c0>;
 };

 xilinx_drm {
  compatible = "xlnx,drm";
  xlnx,vtc = <&v_tc_out>;
  xlnx,connector-type = "HDMIA";
  xlnx,encoder-slave = <&encoder_0>;
  clocks = <&axi_dynclk_0>;
  planes {
   xlnx,pixel-format = "xrgb8888";
   plane0 {
    dmas = <&axi_vdma_1 0>;
    dma-names = "dma0";
   };
  };
 };
};
&axi_dynclk_0 {
 compatible = "digilent,axi-dynclk";
 #clock-cells = <0>;
 clocks = <&clkc 15>;
};
&axi_vdma_1 {
 dma-ranges = <0x00000000 0x00000000 0x40000000>;
};
&v_tc_out {
 compatible = "xlnx,v-tc-5.01.a";
};

&v_tc_in {
 compatible = "generic-uio";
};
&axi_vdma_0 {
 compatible = "generic-uio";
 /delete-node/ dma-channel@43000030;
};
&axi_gpio_video {
 compatible = "generic-uio";
};
&pwm_rgb {
 compatible = "generic-uio";
};


&amba_pl {
 i2s_clk: i2s_clk {
  #clock-cells = <0>;
  compatible = "fixed-clock";
  clock-frequency = <12000000>;
  clock-output-names = "i2s_clk";
 };

 sound {
  compatible = "simple-audio-card";
  simple-audio-card,name = "Zybo-Z7-Sound-Card";
  simple-audio-card,format = "i2s";
  simple-audio-card,bitclock-master = <&dailink0_master>;
  simple-audio-card,frame-master = <&dailink0_master>;
  simple-audio-card,widgets =
   "Microphone", "Microphone Jack",
   "Headphone", "Headphone Jack",
   "Line", "Line In Jack";
  simple-audio-card,routing =
   "MICIN", "Microphone Jack",
   "Headphone Jack", "LHPOUT",
   "Headphone Jack", "RHPOUT",
   "LLINEIN", "Line In Jack",
   "RLINEIN", "Line In Jack";
  dailink0_master: simple-audio-card,cpu {
   clocks = <&i2s_clk>;
   sound-dai = <&axi_i2s_adi_0>;
  };
  simple-audio-card,codec {
   clocks = <&i2s_clk>;
   sound-dai = <&ssm2603>;
  };
 };
};
&axi_i2s_adi_0 {
 #sound-dai-cells = <0>;
 compatible = "adi,axi-i2s-1.00.a";
 clocks = <&clkc 15>, <&i2s_clk>;
 clock-names = "axi", "ref";
 dmas = <&dmac_s 0 &dmac_s 1>;
 dma-names = "tx", "rx";
};
&i2c1 {
 ssm2603: ssm2603@1a{
  #sound-dai-cells = <0>;
  compatible = "adi,ssm2603";
  reg = <0x1a>;
 };
};

&amba {
 xlnk {
  compatible = "xlnx,xlnk-1.0";
 };
};

&axi_gpio_led {
 compatible = "generic-uio";
};


&axi_gpio_sw_btn {
 compatible = "generic-uio";
};
# 29 "/home/digilent/work/git/Zybo-Z7-20-base-sdsoc/linux/Zybo-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts" 2