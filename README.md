a user space tool in developing for check VT-D iommu table  
coding style ugly at preset.  

sudo ./check-table 0xfff90100 0xfed91000 0x2 0x84 0x0 0x0   
sudo ./check-table [input guest addr] [DMAR addr] [pasid index] [bus num] [dev num] [func num]  

be careful the difference of OFFSET\_INDEX between 5.15.55 and 4.19.196  
#define OFFSET_INDEX(val)  (val * 4)  // gcc 64bit, under 64bit OS  
#define OFFSET_INDEX(val)  (val * 8) // gcc 32bit, under 32bit OS  

*support VT-D 2.5 spec*  
*set CONFIG_STRICT_DEVMEM=n CONFIG_X86_PAT=n and CONFIG_EXPERT=y in config file to reduce the addr limition and the permissions*  

For kernel with IOMMU Debugfs(like 5.15, /sys/kernel/debug/iommu/intel), it's an assistance for checktable  
  
For kernel without IOMMU Debugfs(like 4.19), it's a helpful tool
