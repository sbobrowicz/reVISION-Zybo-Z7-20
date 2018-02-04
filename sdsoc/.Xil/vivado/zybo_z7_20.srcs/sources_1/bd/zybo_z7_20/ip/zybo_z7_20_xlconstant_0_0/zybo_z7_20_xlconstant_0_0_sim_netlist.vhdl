-- Copyright 1986-2017 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2017.2_sdx (lin64) Build 1972098 Wed Aug 23 11:34:38 MDT 2017
-- Date        : Fri Oct 27 14:06:41 2017
-- Host        : ubuntu running 64-bit unknown
-- Command     : write_vhdl -force -mode funcsim
--               /home/digilent/work/git/Zybo-Z7-20-base-sdsoc/src/bd/zybo_z7_20/ip/zybo_z7_20_xlconstant_0_0/zybo_z7_20_xlconstant_0_0_sim_netlist.vhdl
-- Design      : zybo_z7_20_xlconstant_0_0
-- Purpose     : This VHDL netlist is a functional simulation representation of the design and should not be modified or
--               synthesized. This netlist cannot be used for SDF annotated simulation.
-- Device      : xc7z020clg400-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
library UNISIM;
use UNISIM.VCOMPONENTS.ALL;
entity zybo_z7_20_xlconstant_0_0 is
  port (
    dout : out STD_LOGIC_VECTOR ( 0 to 0 )
  );
  attribute NotValidForBitStream : boolean;
  attribute NotValidForBitStream of zybo_z7_20_xlconstant_0_0 : entity is true;
  attribute CHECK_LICENSE_TYPE : string;
  attribute CHECK_LICENSE_TYPE of zybo_z7_20_xlconstant_0_0 : entity is "zybo_z7_20_xlconstant_0_0,xlconstant_v1_1_3_xlconstant,{}";
  attribute DowngradeIPIdentifiedWarnings : string;
  attribute DowngradeIPIdentifiedWarnings of zybo_z7_20_xlconstant_0_0 : entity is "yes";
  attribute X_CORE_INFO : string;
  attribute X_CORE_INFO of zybo_z7_20_xlconstant_0_0 : entity is "xlconstant_v1_1_3_xlconstant,Vivado 2017.2_sdx";
end zybo_z7_20_xlconstant_0_0;

architecture STRUCTURE of zybo_z7_20_xlconstant_0_0 is
  signal \<const1>\ : STD_LOGIC;
begin
  dout(0) <= \<const1>\;
VCC: unisim.vcomponents.VCC
     port map (
      P => \<const1>\
    );
end STRUCTURE;
