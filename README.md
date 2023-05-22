a user space tool in developing for check VT-D iommu table  
coding style ugly at preset.  

sudo ./check-table 0xfff90100 0xfe48c000 0x2 0x84 0x0
sudo ./check-table [input guest addr] [DMAR addr] [pasid index] [bus num] [dev num] [func num]

