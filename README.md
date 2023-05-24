a user space tool in developing for check VT-D iommu table  
coding style ugly at preset.  

sudo ./check-table 0xfff90100 0xfed91000 0x2 0x84 0x0 0x0   
sudo ./check-table [input guest addr] [DMAR addr] [pasid index] [bus num] [dev num] [func num]  

be careful the difference of OFFSET\_INDEX between 5.15.55 and 4.19.196  
#define OFFSET_INDEX(val)  (val * 4)  // gcc 7.5 and kernel-5.15.55  
#define OFFSET_INDEX(val)  (val * 8) // gcc 8.3 and kernel-4.19.196 32bit/64bit  

