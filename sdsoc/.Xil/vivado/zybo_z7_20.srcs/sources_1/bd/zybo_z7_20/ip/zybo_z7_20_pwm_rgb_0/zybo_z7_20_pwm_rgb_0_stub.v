// Copyright 1986-2017 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2017.2_sdx (lin64) Build 1972098 Wed Aug 23 11:34:38 MDT 2017
// Date        : Fri Oct 27 14:05:35 2017
// Host        : ubuntu running 64-bit unknown
// Command     : write_verilog -force -mode synth_stub
//               /home/digilent/work/git/Zybo-Z7-20-base-sdsoc/src/bd/zybo_z7_20/ip/zybo_z7_20_pwm_rgb_0/zybo_z7_20_pwm_rgb_0_stub.v
// Design      : zybo_z7_20_pwm_rgb_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "PWM_v2_0,Vivado 2017.2_sdx" *)
module zybo_z7_20_pwm_rgb_0(pwm, pwm_axi_awaddr, pwm_axi_awprot, 
  pwm_axi_awvalid, pwm_axi_awready, pwm_axi_wdata, pwm_axi_wstrb, pwm_axi_wvalid, 
  pwm_axi_wready, pwm_axi_bresp, pwm_axi_bvalid, pwm_axi_bready, pwm_axi_araddr, 
  pwm_axi_arprot, pwm_axi_arvalid, pwm_axi_arready, pwm_axi_rdata, pwm_axi_rresp, 
  pwm_axi_rvalid, pwm_axi_rready, pwm_axi_aclk, pwm_axi_aresetn)
/* synthesis syn_black_box black_box_pad_pin="pwm[5:0],pwm_axi_awaddr[6:0],pwm_axi_awprot[2:0],pwm_axi_awvalid,pwm_axi_awready,pwm_axi_wdata[31:0],pwm_axi_wstrb[3:0],pwm_axi_wvalid,pwm_axi_wready,pwm_axi_bresp[1:0],pwm_axi_bvalid,pwm_axi_bready,pwm_axi_araddr[6:0],pwm_axi_arprot[2:0],pwm_axi_arvalid,pwm_axi_arready,pwm_axi_rdata[31:0],pwm_axi_rresp[1:0],pwm_axi_rvalid,pwm_axi_rready,pwm_axi_aclk,pwm_axi_aresetn" */;
  output [5:0]pwm;
  input [6:0]pwm_axi_awaddr;
  input [2:0]pwm_axi_awprot;
  input pwm_axi_awvalid;
  output pwm_axi_awready;
  input [31:0]pwm_axi_wdata;
  input [3:0]pwm_axi_wstrb;
  input pwm_axi_wvalid;
  output pwm_axi_wready;
  output [1:0]pwm_axi_bresp;
  output pwm_axi_bvalid;
  input pwm_axi_bready;
  input [6:0]pwm_axi_araddr;
  input [2:0]pwm_axi_arprot;
  input pwm_axi_arvalid;
  output pwm_axi_arready;
  output [31:0]pwm_axi_rdata;
  output [1:0]pwm_axi_rresp;
  output pwm_axi_rvalid;
  input pwm_axi_rready;
  input pwm_axi_aclk;
  input pwm_axi_aresetn;
endmodule
