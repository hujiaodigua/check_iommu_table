/*************************************************************************
	> File Name: mm_w.c
	> Author:
	> Mail:
	> Created Time: 2021年08月26日 星期四 15时32分17秒
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/mman.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

#define RTT_BIT(x)  ((x & 0x800) >> 11)
#define PASIDE_BIT(x)  ((x & 0x800) >> 11)
#define PRESENT_BIT(x)  (x & 0x1)
#define HUGEPAGE_BIT(x)  ((x & 0x80) >> 7)

#define REG_MAP_SIZE 0xFFF
#define RTE_MAP_SIZE 0xFFF
#define CTE_MAP_SIZE 0xFFF
#define PASIDTE_MAP_SIZE 0xFFF

#define SL_1st_MAP_SIZE 0xFFF
#define SL_2nd_MAP_SIZE 0xFFF
#define SL_3rd_MAP_SIZE 0xFFF
#define SL_4th_MAP_SIZE 0xFFF

#define FL_1st_MAP_SIZE 0xFFF
#define FL_2nd_MAP_SIZE 0xFFF
#define FL_3rd_MAP_SIZE 0xFFF
#define FL_4th_MAP_SIZE 0xFFF

#define PML4_PAGE_OFFSET(x)  (x & 0xFFF)
#define PML4_1st_OFFSET(x)  ((x & 0xFF8000000000) >> 39) << 3
#define PML4_2nd_OFFSET(x)  ((x & 0x7FC0000000) >> 30) << 3
#define PML4_3rd_OFFSET(x)  ((x & 0x3FE00000) >> 21) << 3
#define PML4_4th_OFFSET(x)  ((x & 0x1FF000) >> 12) << 3

int offset_2_index(int x)
{
        int bits = sizeof(char *);

        if (bits == 4)  // 32bit OS
                return x * 8;
        else if (bits == 8)  // 64 bit OS
                return x * 4;
        else
        {
                printf("pointer size error\n");
                return 0;
        }
}

/*int index_2_offset(int x)
{
        int bits = sizeof(char *);
        if (bits == 4)
                return x / 4;
        else if (bits == 8)
                return x / 4;  // 64bit OS and 32bit OS, inde_2_offset are same between them
        else
        {
                printf("pointer size error\n");
                return 0;
        }
}*/

#define OFFSET_INDEX(val)  offset_2_index(val)
// #define INDEX_OFFSET(val)  index_2_offset(val)
#define INDEX_OFFSET(val)  (val / 4)
#define PASID_INDEX(val)  (val * 2)

int walk_page_structure_entry(unsigned long long int SLPTPTR_val,
		              unsigned long long int input_va_val, int fd)
{
	int bit0_11 = PML4_PAGE_OFFSET(input_va_val);   // page offset
	int bit12_20 = PML4_4th_OFFSET(input_va_val);  // forth offset
	int bit21_29 = PML4_3rd_OFFSET(input_va_val);  // third offset
	int bit30_39 = PML4_2nd_OFFSET(input_va_val);  // second offset
	int bit39_47 = PML4_1st_OFFSET(input_va_val);  // first offset
	unsigned long long int SLPTPTR_1level;
        unsigned long long int SLPTPTR_2level;
        unsigned long long int SLPTPTR_3level;
        unsigned long long int SLPTPTR_4level;

	unsigned int *slp_addr_va = NULL;
        unsigned int *slp_addr_2nd_va = NULL;
        unsigned int *slp_addr_3rd_va = NULL;
        unsigned int *slp_addr_4th_va = NULL;
	int *start;

	slp_addr_va = (unsigned int *)(0x7AA66000);

	printf("input va: %#llx\n", input_va_val);
	printf("page offset bit0_11: %#x\n", bit0_11);
	printf("4th offset bit12_20: %#x\n", bit12_20);
	printf("3rd offset bit21_29: %#x\n", bit21_29);
	printf("2nd offset bit30_39: %#x\n", bit30_39);
	printf("1st offset bit39_47: %#x\n", bit39_47);

	start = (int *)mmap(slp_addr_va, SL_1st_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_val);
	if (start < 0)
        {
		printf("slp_addr_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("===start walk second pml4 table structure===\n");
	printf("1st offset: %#x, pointer val: 0x%08x%08x\n", bit39_47,
			slp_addr_va[1 + INDEX_OFFSET(bit39_47)], slp_addr_va[0 + INDEX_OFFSET(bit39_47)]);

        SLPTPTR_1level = (unsigned long long int)slp_addr_va[1 + INDEX_OFFSET(bit39_47)] << 32
                        | slp_addr_va[0 + INDEX_OFFSET(bit39_47)];

        if (munmap(slp_addr_va, SL_1st_MAP_SIZE) == -1)
                printf("slp_addr_va munmap error\n");

        if (PRESENT_BIT(SLPTPTR_1level) == 0)
                goto entry_not_present;

        SLPTPTR_1level >>= 12;
        SLPTPTR_1level <<= 12;
        printf("SLPTPTR_1level=0x%llx\n", SLPTPTR_1level);

        if (SLPTPTR_1level == 0)
                goto addr_0_err;

        slp_addr_2nd_va = (unsigned int *)(0x8AA66000);

        start = (int *)mmap(slp_addr_2nd_va, SL_2nd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_1level);
	if (start < 0)
        {
		printf("slp_addr_2nd_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("2nd offset: %#x, pointer val: 0x%08x%08x\n", bit30_39,
              slp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)], slp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)]);

        SLPTPTR_2level = (unsigned long long int)slp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)] << 32
                        | slp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)];

        if (munmap(slp_addr_2nd_va, SL_2nd_MAP_SIZE) == -1)
                printf("slp_addr_2nd_va munmap error\n");

        if (HUGEPAGE_BIT(SLPTPTR_2level) == 1)
                goto GB_HUGEPAGE;
        if (PRESENT_BIT(SLPTPTR_2level) == 0)
                goto entry_not_present;

        SLPTPTR_2level >>= 12;
        SLPTPTR_2level <<= 12;
        printf("SLPTPTR_2level=0x%llx\n", SLPTPTR_2level);

        if (SLPTPTR_2level == 0)
                goto addr_0_err;

        slp_addr_3rd_va = (unsigned int *)(0x9AA66000);
        start = (int *)mmap(slp_addr_3rd_va, SL_3rd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_2level);
        if (start < 0)
        {
                printf("slp_addr_3rd_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("3rd offset: %#x, pointer val: 0x%08x%08x\n", bit21_29,
              slp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)], slp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)]);
        SLPTPTR_3level = (unsigned long long int)slp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)] << 32
                        | slp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)];

        if (munmap(slp_addr_3rd_va, SL_3rd_MAP_SIZE) == -1)
                printf("slp_addr_3rd_va munmap error\n");

        if (HUGEPAGE_BIT(SLPTPTR_3level) == 1)
                goto MB_HUGEPAGE;
        if (PRESENT_BIT(SLPTPTR_3level) == 0)
                goto entry_not_present;

        SLPTPTR_3level >>= 12;
        SLPTPTR_3level <<= 12;
        printf("SLPTPTR_3level=0x%llx\n", SLPTPTR_3level);

        if (SLPTPTR_3level == 0)
                goto addr_0_err;

        slp_addr_4th_va = (unsigned int *)(0xaAA66000);
        start = (int *)mmap(slp_addr_4th_va, SL_4th_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_3level);
        if (start < 0)
        {
                printf("slp_addr_4th_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("4th offset: %#x, pointer val: 0x%08x%08x\n", bit12_20,
              slp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)], slp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)]);
        SLPTPTR_4level = (unsigned long long int)slp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)] << 32
                        | slp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)];

        if (munmap(slp_addr_4th_va, SL_4th_MAP_SIZE) == -1)
                printf("slp_addr_4th_va munmap errpr\n");

        if (PRESENT_BIT(SLPTPTR_4level) == 0)
                goto entry_not_present;

        SLPTPTR_4level >>= 12;
        SLPTPTR_4level <<=12;
        printf("SLPTPTR_4level=0x%llx\n", SLPTPTR_4level);

        if (SLPTPTR_4level == 0)
                goto addr_0_err;


        return 0;

addr_0_err:
        printf("this level addr pointer is 0!!\n");
        return -2;

entry_not_present:
        printf("this entry not present");
        return -3;

GB_HUGEPAGE:
        printf("Used 1GB Huge Page\n");
        return 0;

MB_HUGEPAGE:
        printf("Used 2MB Huge Page\n");
        return 0;
}

int walk_first_page_structure_entry(unsigned long long int FLPTPTR_val,
		              unsigned long long int input_va_val, int fd)
{
	int bit0_11 = PML4_PAGE_OFFSET(input_va_val);   // page offset
	int bit12_20 = PML4_4th_OFFSET(input_va_val);  // forth offset
	int bit21_29 = PML4_3rd_OFFSET(input_va_val);  // third offset
	int bit30_39 = PML4_2nd_OFFSET(input_va_val);  // second offset
	int bit39_47 = PML4_1st_OFFSET(input_va_val);  // first offset
	unsigned long long int FLPTPTR_1level;
        unsigned long long int FLPTPTR_2level;
        unsigned long long int FLPTPTR_3level;
        unsigned long long int FLPTPTR_4level;

	unsigned int *flp_addr_va = NULL;
        unsigned int *flp_addr_2nd_va = NULL;
        unsigned int *flp_addr_3rd_va = NULL;
        unsigned int *flp_addr_4th_va = NULL;
	int *start;

	flp_addr_va = (unsigned int *)(0x7AA66000);

	printf("input va: %#llx\n", input_va_val);
	printf("page offset bit0_11: %#x\n", bit0_11);
	printf("4th offset bit12_20: %#x\n", bit12_20);
	printf("3rd offset bit21_29: %#x\n", bit21_29);
	printf("2nd offset bit30_39: %#x\n", bit30_39);
	printf("1st offset bit39_47: %#x\n", bit39_47);

	start = (int *)mmap(flp_addr_va, FL_1st_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_val);
	if (start < 0)
        {
		printf("flp_addr_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("===start walk first pml4 table structure===\n");
	printf("1st offset: %#x, pointer val: 0x%08x%08x\n", bit39_47,
			flp_addr_va[1 + INDEX_OFFSET(bit39_47)], flp_addr_va[0 + INDEX_OFFSET(bit39_47)]);

        FLPTPTR_1level = (unsigned long long int)flp_addr_va[1 + INDEX_OFFSET(bit39_47)] << 32
                        | flp_addr_va[0 + INDEX_OFFSET(bit39_47)];

        if (munmap(flp_addr_va, FL_1st_MAP_SIZE) == -1)
                printf("flp_addr_va munmap error\n");

        if (PRESENT_BIT(FLPTPTR_1level) == 0)
                goto entry_not_present;

        FLPTPTR_1level >>= 12;
        FLPTPTR_1level <<= 12;
        printf("FLPTPTR_1level=0x%llx\n", FLPTPTR_1level);

        if (FLPTPTR_1level == 0)
                goto addr_0_err;

        flp_addr_2nd_va = (unsigned int *)(0x8AA66000);

        start = (int *)mmap(flp_addr_2nd_va, FL_2nd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_1level);
	if (start < 0)
        {
		printf("flp_addr_2nd_va mmap failed in %s\n", __func__);
		return -1;
        }

        printf("2nd offset: %#x, pointer val: 0x%08x%08x\n", bit30_39,
              flp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)], flp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)]);

        FLPTPTR_2level = (unsigned long long int)flp_addr_2nd_va[1 + INDEX_OFFSET(bit30_39)] << 32
                        | flp_addr_2nd_va[0 + INDEX_OFFSET(bit30_39)];

        if (munmap(flp_addr_2nd_va, FL_2nd_MAP_SIZE) == -1)
                printf("flp_addr_2nd_va munmap error\n");

        if (HUGEPAGE_BIT(FLPTPTR_2level) == 1)
                goto GB_HUGEPAGE;
        if (PRESENT_BIT(FLPTPTR_2level) == 0)
                goto entry_not_present;

        FLPTPTR_2level >>= 12;
        FLPTPTR_2level <<= 12;
        printf("FLPTPTR_2level=0x%llx\n", FLPTPTR_2level);

        if (FLPTPTR_2level == 0)
                goto addr_0_err;

        flp_addr_3rd_va = (unsigned int *)(0x9AA66000);
        start = (int *)mmap(flp_addr_3rd_va, FL_3rd_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_2level);
        if (start < 0)
        {
                printf("flp_addr_3rd_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("3rd offset: %#x, pointer val: 0x%08x%08x\n", bit21_29,
              flp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)], flp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)]);
        FLPTPTR_3level = (unsigned long long int)flp_addr_3rd_va[1 + INDEX_OFFSET(bit21_29)] << 32
                        | flp_addr_3rd_va[0 + INDEX_OFFSET(bit21_29)];

        if (munmap(flp_addr_3rd_va, FL_3rd_MAP_SIZE) == -1)
                printf("flp_addr_3rd_va munmap error\n");

        if (HUGEPAGE_BIT(FLPTPTR_3level) == 1)
                goto MB_HUGEPAGE;
        if (PRESENT_BIT(FLPTPTR_3level) == 0)
                goto entry_not_present;

        FLPTPTR_3level >>= 12;
        FLPTPTR_3level <<= 12;
        printf("FLPTPTR_3level=0x%llx\n", FLPTPTR_3level);

        if (FLPTPTR_3level == 0)
                goto addr_0_err;

        flp_addr_4th_va = (unsigned int *)(0xaAA66000);
        start = (int *)mmap(flp_addr_4th_va, FL_4th_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, FLPTPTR_3level);
        if (start < 0)
        {
                printf("flp_addr_4th_va mmap failed in %s\n", __func__);
                return -1;
        }

        printf("4th offset: %#x, pointer val: 0x%08x%08x\n", bit12_20,
              flp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)], flp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)]);
        FLPTPTR_4level = (unsigned long long int)flp_addr_4th_va[1 + INDEX_OFFSET(bit12_20)] << 32
                        | flp_addr_4th_va[0 + INDEX_OFFSET(bit12_20)];

        if (munmap(flp_addr_4th_va, FL_4th_MAP_SIZE) == -1)
                printf("flp_addr_4th_va munmap errpr\n");

        if (PRESENT_BIT(FLPTPTR_4level) == 0)
                goto entry_not_present;

        FLPTPTR_4level >>= 12;
        FLPTPTR_4level <<=12;
        printf("FLPTPTR_4level=0x%llx\n", FLPTPTR_4level);

        if (FLPTPTR_4level == 0)
                goto addr_0_err;

        return 0;

addr_0_err:
        printf("this level addr pointer is 0!!\n");
        return -2;

entry_not_present:
        printf("this entry is not present\n");
        return -3;

GB_HUGEPAGE:
        printf("Used 1GB Huge Page\n");
        return 0;

MB_HUGEPAGE:
        printf("Used 2MB Huge Page\n");
        return 0;
}

int walk_structure_entry(int rtt_val, unsigned long long int rta_pointer_val,
                         int fd, int bus_n, int dev_n, int func_n,
                         unsigned long long int input_va, int input_pasid)
{
        unsigned int *rte_addr_va = NULL;
	unsigned int *cte_addr_va = NULL;
        unsigned int *pasidte_addr_va = NULL;
        int *start;
	int devfn = dev_n << 3 | func_n;
        int ret;

        rte_addr_va = (unsigned int *)(0x5AA66000);
	cte_addr_va = (unsigned int *)(0x6AA66000);
        pasidte_addr_va = (unsigned int *)(0x1AA66000);  // too higher userspace va addr will cause error in 32bit OS

	unsigned long long int CTP;
	unsigned long long int SLPTPTR;
        unsigned long long int PASIDPTR;
        unsigned long long int FLPTPTR;

        if (rtt_val == 1)
        {
                printf("===Used Extended Root Entry===\n");
		// root entry
		start = (int *)mmap(rte_addr_va, RTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, rta_pointer_val);
        	if (start < 0)
        	{
                	printf("rte_addr_va mmap failed in %s\n", __func__);
                	return -1;
        	}

		printf("root entry bus %#x [63-0]:0x%08x%08x\n",
				bus_n, rte_addr_va[1 + INDEX_OFFSET(bus_n * 0x10)], rte_addr_va[0 + INDEX_OFFSET(bus_n * 0x10)]);
		printf("root entry bus %#x [127-64]:0x%08x%08x\n",
				bus_n, rte_addr_va[3 + INDEX_OFFSET(bus_n * 0x10)], rte_addr_va[2 + INDEX_OFFSET(bus_n * 0x10)]);

                if (dev_n >= 0 && dev_n <= 15)
                {
		        CTP = (unsigned long long int)rte_addr_va[1 + INDEX_OFFSET(bus_n * 0x10)] << 32
                                | rte_addr_va[0 + INDEX_OFFSET(bus_n * 0x10)];  // OFFSET 0x840 means bus 0x84

                        if (munmap(rte_addr_va, RTE_MAP_SIZE) == -1)
		                printf("rte_addr_va munmap error\n");

                        if (PRESENT_BIT(CTP) == 0)
                        {
                                printf("lower extend root entry not present\n");
                                return -3;
                        }

		        CTP >>= 12;
		        CTP <<= 12;
                        printf("dev_n:%d Use Lower CTP=0x%llx\n", dev_n, CTP);
                }
                if (dev_n >= 16 && dev_n <= 31)
                {
		        CTP = (unsigned long long int)rte_addr_va[3 + INDEX_OFFSET(bus_n * 0x10)] << 32
                                | rte_addr_va[2 + INDEX_OFFSET(bus_n * 0x10)];

                        if (munmap(rte_addr_va, RTE_MAP_SIZE) == -1)
		                printf("rte_addr_va munmap error\n");

                        if (PRESENT_BIT(CTP) == 0)
                        {
                                printf("upper extend root entry not present\n");
                                return -3;
                        }

		        CTP >>= 12;
		        CTP <<= 12;
                        printf("dev_n:%d Use Upper CTP=0x%llx\n", dev_n, CTP);
                }

	        if (munmap(rte_addr_va, RTE_MAP_SIZE) == -1)
		        printf("rte_addr_va munmap error\n");



		// extended context entry
                printf("===Used Extended Context Entry===\n");
		start = (int *)mmap(cte_addr_va, CTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CTP);
        	if (start < 0)
        	{
                	printf("cte_addr_va mmap failed in %s\n", __func__);
                	return -1;
        	}
		printf("conext entry dev 0x%x func %d [63-0]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[1 + OFFSET_INDEX(devfn)], cte_addr_va[0 + OFFSET_INDEX(devfn)]);
		printf("conext entry dev 0x%x func %d [127-64]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[3 + OFFSET_INDEX(devfn)], cte_addr_va[2 + OFFSET_INDEX(devfn)]);
		printf("conext entry dev 0x%x func %d [191-128]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[5 + OFFSET_INDEX(devfn)], cte_addr_va[4 + OFFSET_INDEX(devfn)]);
		printf("conext entry dev 0x%x func %d [255-192]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[7 + OFFSET_INDEX(devfn)], cte_addr_va[6 + OFFSET_INDEX(devfn)]);

		SLPTPTR = (unsigned long long int)cte_addr_va[1 + OFFSET_INDEX(devfn)] << 32 | cte_addr_va[0 + OFFSET_INDEX(devfn)];


                if (PRESENT_BIT(SLPTPTR) == 0)
                {
	                if (munmap(cte_addr_va, CTE_MAP_SIZE) == -1)
		                printf("cte_addr_va munmap error\n");
                        printf("extend context entry not present\n");
                        return -3;
                }

		SLPTPTR >>= 12;
		SLPTPTR <<= 12;
		printf("SLPTPTR=0x%llx\n", SLPTPTR);
		ret = walk_page_structure_entry(SLPTPTR, input_va, fd);

                if (PASIDE_BIT(cte_addr_va[0 + OFFSET_INDEX(devfn)]) == 1)
                {
                        printf("PASID enable\n");
                        PASIDPTR = (unsigned long long int)cte_addr_va[5 + OFFSET_INDEX(devfn)]  << 32 |
                                   cte_addr_va[4 + OFFSET_INDEX(devfn)];


                        PASIDPTR >>= 12;
                        PASIDPTR <<= 12;

                        if (input_pasid == 0)
                                goto pasid_zero;

                        printf("PASIDPTR=0x%llx\n", PASIDPTR);
                        printf("===Used PASID Entry===\n");

                        if (input_pasid != 0)
                        {
                                start = (int *)mmap(pasidte_addr_va, PASIDTE_MAP_SIZE,
                                                    PROT_READ | PROT_WRITE, MAP_SHARED, fd, PASIDPTR);
                                if (start < 0)
                                {
                	                printf("pasidte_addrva mmap failed in %s\n", __func__);
                	                return -1;
                                }
                                printf("pasid entry pasid:%#x, [63-0]:0x%08x%08x\n",
                                        input_pasid, pasidte_addr_va[1 + PASID_INDEX(input_pasid)],
                                        pasidte_addr_va[0 + PASID_INDEX(input_pasid)]);
                        }
                        FLPTPTR = (unsigned long long int)pasidte_addr_va[1 + PASID_INDEX(input_pasid)] << 32 |
                                  pasidte_addr_va[0 + PASID_INDEX(input_pasid)];

                        if (munmap(pasidte_addr_va, PASIDTE_MAP_SIZE) == -1)
                                printf("pasidte_addr_va munmap error\n");

                        FLPTPTR >>= 12;
                        FLPTPTR <<= 12;
                        printf("FLPTPTR=0x%llx\n", FLPTPTR);
                        ret = walk_first_page_structure_entry(FLPTPTR, input_va, fd);
                }

	        if (munmap(cte_addr_va, CTE_MAP_SIZE) == -1)
		        printf("cte_addr_va munmap error\n");
        }
        else if (rtt_val == 0)
        {
                printf("===Used Root Entry===\n");
		// root entry
		start = (int *)mmap(rte_addr_va, RTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, rta_pointer_val);
        	if (start < 0)
        	{
                	printf("rte_addr_va mmap failed in %s\n", __func__);
                	return -1;
        	}

		printf("root entry bus %#x [63-0]:0x%08x%08x\n",
				bus_n, rte_addr_va[1 + INDEX_OFFSET(bus_n * 0x10)], rte_addr_va[0 + INDEX_OFFSET(bus_n * 0x10)]);
		printf("root entry bus %#x [127-64]:0x%08x%08x\n",
				bus_n, rte_addr_va[3 + INDEX_OFFSET(bus_n * 0x10)], rte_addr_va[2 + INDEX_OFFSET(bus_n * 0x10)]);

		CTP = (unsigned long long int)rte_addr_va[1 + INDEX_OFFSET(bus_n * 0x10)] << 32
                        | rte_addr_va[0 + INDEX_OFFSET(bus_n * 0x10)];  // OFFSET 0x840 means bus 0x84

                if (munmap(rte_addr_va, RTE_MAP_SIZE) == -1)
		        printf("rte_addr_va munmap error\n");

                if (PRESENT_BIT(CTP) == 0)
                {
                        printf("root entry not present\n");
                        return -3;
                }

		CTP >>= 12;
		CTP <<= 12;
                printf("dev_n:%d CTP=0x%llx\n", dev_n, CTP);

		// context entry
                printf("===Used Context Entry===\n");
		start = (int *)mmap(cte_addr_va, CTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CTP);
        	if (start < 0)
        	{
                	printf("cte_addr_va mmap failed in %s\n", __func__);
                	return -1;
        	}
		printf("conext entry dev 0x%x func %d [63-0]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[1 + OFFSET_INDEX(devfn)], cte_addr_va[0 + OFFSET_INDEX(devfn)]);
		printf("conext entry dev 0x%x func %d [127-64]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[3 + OFFSET_INDEX(devfn)], cte_addr_va[2 + OFFSET_INDEX(devfn)]);

		SLPTPTR = (unsigned long long int)cte_addr_va[1 + OFFSET_INDEX(devfn)] << 32 | cte_addr_va[0 + OFFSET_INDEX(devfn)];

                if (munmap(cte_addr_va, CTE_MAP_SIZE) == -1)
		        printf("cte_addr_va munmap error\n");

                if (PRESENT_BIT(SLPTPTR) == 0)
                {
                        printf("context entry not present\n");
                }

		SLPTPTR >>= 12;
		SLPTPTR <<= 12;
		printf("SLPTPTR=0x%llx\n", SLPTPTR);
		walk_page_structure_entry(SLPTPTR, input_va, fd);
        }
        else
        {
                return -2;
        }


        return ret;

pasid_zero:
        printf("input pasid is zero\n");
        return 0;
}

int main(int argc, char* argv[], char* envp[])
{
        int *start;
        int file_device;
        unsigned int *reg_start_addr_va = NULL;
        unsigned long long int input_guest_addr;
        unsigned long long int input_DMAR_addr;
        int input_pasid_val = 0;  // pasid 0 is reserved

        unsigned long long int RTA;
        int RTT;
        int ret = 0;

	int bus_num = 0x0;
	int dev_num = 0x0;
	int func_num = 0x0;

        char *ptr;
        if (argv[1] != NULL)
                input_guest_addr = strtoll(argv[1], &ptr, 16);

        char *ptr_dmar;
        if (argv[2] != NULL)
                input_DMAR_addr = strtoll(argv[2], &ptr_dmar, 16);

        char *ptr_pasid;
        if (argv[3] != NULL)
                input_pasid_val = strtoll(argv[3], &ptr_pasid, 16);

        char *ptr_bus_num;
        if (argv[4] != NULL)
                bus_num = strtoll(argv[4], &ptr_bus_num, 16);

        char *ptr_dev_num;
        if (argv[5] != NULL)
                dev_num = strtoll(argv[5], &ptr_dev_num, 16);

        char *ptr_func_num;
        if (argv[6] != NULL)
                func_num = strtoll(argv[6], &ptr_func_num, 16);

        reg_start_addr_va = (unsigned int *)(0x4AA66000);  // user-space start va, 这是指定映射到user-space va上的起始地址

        // file_device = open("/dev/zxmem", O_RDWR);
        file_device = open("/dev/mem", O_RDWR);
        if (file_device < 0)
        {
                printf("cannot open /dev/mem\n");
                return -1;
        }

        // DMAR0 0xfe48c000
        start = (int *)mmap(reg_start_addr_va, REG_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, file_device, input_DMAR_addr);
        // printf("start=%llx\n", start);
        if (start < 0)
        {
                printf("reg_start_addr_va mmap failed\n");
                return -1;
        }

        printf("VER_REG value:%#x\n", reg_start_addr_va[0]);   // offset 0h
        printf("CAP_REG value:0x%08x%08x\n", reg_start_addr_va[3], reg_start_addr_va[2]);  // index 1 means 32bit offset = 4h
        printf("ECAP_REG value:0x%08x%08x\n", reg_start_addr_va[5], reg_start_addr_va[4]);
        printf("GCMD_REG value:%#x\n", reg_start_addr_va[6]);
        printf("GSTS_REG value:%#x\n", reg_start_addr_va[7]);
        printf("RTADDR_REG value: 0x%08x%08x\n", reg_start_addr_va[9], reg_start_addr_va[8]);

        RTA = (unsigned long long)reg_start_addr_va[9] << 32 | reg_start_addr_va[8];
        RTT = RTT_BIT(reg_start_addr_va[8]);
        RTA >>= 12;  // clear control bit
        RTA <<= 12;
        printf("RTT=%#x, RTA=0x%llx\n", RTT, RTA);

        ret = walk_structure_entry(RTT, RTA, file_device, bus_num,
                                   dev_num, func_num, input_guest_addr, input_pasid_val);  // can input_va 0x0fff90100  for test
        if (ret == -2)
                goto unmap;

unmap:
        ret = munmap(reg_start_addr_va, REG_MAP_SIZE);
        if (ret == -1)
                printf("reg_start_addr_va munmap error\n");

        close(file_device);
        return 0;
}
