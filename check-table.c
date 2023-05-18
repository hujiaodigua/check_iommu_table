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

#define RTT_BIT(x) ((x & 0x800) >> 0x11)

#define REG_MAP_SIZE 0xFFF
#define RTE_MAP_SIZE 0xFFF
#define CTE_MAP_SIZE 0xFFF
#define SL_1st_MAP_SIZE 0xFFF
#define SL_2nd_MAP_SIZE 0xFFF
#define SL_3rd_MAP_SIZE 0xFFF
#define SL_4th_MAP_SIZE 0xFFF

#define OFFSET_INDEX(x)  (x * 4)
#define INDEX_OFFSET(val)  (val / 4)

#define PML4_PAGE_OFFSET(x)  (x & 0xFFF)
#define PML4_1st_OFFSET(x)  ((x & 0xFF8000000000) >> 39) << 3
#define PML4_2nd_OFFSET(x)  ((x & 0x7FC0000000) >> 30) << 3
#define PML4_3rd_OFFSET(x)  ((x & 0x3FE00000) >> 21) << 3
#define PML4_4th_OFFSET(x)  ((x & 0x1FF000) >> 12) << 3

int walk_second_structure_entry(unsigned long int SLPTPTR_val,
		unsigned long int input_va_val, int fd)
{
	int bit0_11 = PML4_PAGE_OFFSET(input_va_val);   // page offset
	int bit12_20 = PML4_4th_OFFSET(input_va_val);  // forth offset
	int bit21_29 = PML4_3rd_OFFSET(input_va_val);  // third offset
	int bit30_39 = PML4_2nd_OFFSET(input_va_val);  // second offset
	int bit39_47 = PML4_1st_OFFSET(input_va_val);  // first offset

	unsigned int *slp_addr_va = NULL;
	int *start;

	slp_addr_va = (unsigned int *)(0x27AA66000);

	printf("input va: %#lx\n", input_va_val);
	printf("page offset bit0_11: %#x\n", bit0_11);
	printf("4th offset bit12_20: %#x\n", bit12_20);
	printf("3rd offset bit21_29: %#x\n", bit21_29);
	printf("2nd offset bit30_39: %#x\n", bit30_39);
	printf("1st offset bit39_47: %#x\n", bit39_47);

	start = (int *)mmap(slp_addr_va, SL_1st_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SLPTPTR_val);
	if (start < 0)
        {
		printf("mmap failed in %s\n", __func__);
		return -1;
        }

	printf("1st offset: %#x, pointer val: 0x%08x%08x\n", bit39_47,
			slp_addr_va[1 + INDEX_OFFSET(bit39_47)], slp_addr_va[0 + INDEX_OFFSET(bit39_47)]);

	if (munmap(slp_addr_va, SL_1st_MAP_SIZE) == -1)
                printf("munmap error");

	unsigned long int SLPTPTR_2level = ;
	
}

int walk_structure_entry(int rtt_val, unsigned long int rta_pointer_val,
		int fd, int bus_n, int dev_n, int func_n, unsigned long int input_va)
{
        unsigned int *rte_addr_va = NULL;
	unsigned int *cte_addr_va = NULL;
        int *start;
	int devfn = dev_n << 3 | func_n;

	rte_addr_va = (unsigned int *)(0x25AA66000);
	cte_addr_va = (unsigned int *)(0x26AA66000);

	unsigned long int CTP;
	unsigned long int SLPTPTR;

        if (rtt_val == 0)
        {
		// root entry
		start = (int *)mmap(rte_addr_va, RTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, rta_pointer_val);
        	if (start < 0)
        	{
                	printf("mmap failed in %s\n", __func__);
                	return -1;
        	}
		printf("root entry bus %d [63-0]:0x%08x%08x\n",
				bus_n, rte_addr_va[1 + OFFSET_INDEX(bus_n)], rte_addr_va[0 + OFFSET_INDEX(bus_n)]);
		printf("root entry bus %d [127-64]:0x%08x%08x\n",
				bus_n, rte_addr_va[3 + OFFSET_INDEX(bus_n)], rte_addr_va[2 + OFFSET_INDEX(bus_n)]);
		CTP = (unsigned long int)rte_addr_va[1 + OFFSET_INDEX(bus_n)] << 32 | rte_addr_va[0 + OFFSET_INDEX(bus_n)];
		CTP >>= 12;
		CTP <<= 12;
		printf("CTP=0x%lx\n", CTP);
		
		// context entry
		start = (int *)mmap(cte_addr_va, CTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CTP);
        	if (start < 0)
        	{
                	printf("mmap failed in %s\n", __func__);
                	return -1;
        	}
		printf("conext entry dev 0x%x func %d [63-0]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[1 + OFFSET_INDEX(devfn)], cte_addr_va[0 + OFFSET_INDEX(devfn)]);
		printf("conext entry dev 0x%x func %d [127-64]:0x%08x%08x\n",
				dev_n, func_n, cte_addr_va[3 + OFFSET_INDEX(devfn)], cte_addr_va[2 + OFFSET_INDEX(devfn)]);

		SLPTPTR = (unsigned long int)cte_addr_va[1 + OFFSET_INDEX(devfn)] << 32 | cte_addr_va[0 + OFFSET_INDEX(devfn)];
		SLPTPTR >>= 12;
		SLPTPTR <<= 12;
		printf("SLPTPTR=0x%lx\n", SLPTPTR);
		walk_second_structure_entry(SLPTPTR, input_va, fd);
	}
        else if (rtt_val == 1)
        {

        }
        else
        {
                return -2;
        }

	if (munmap(rte_addr_va, RTE_MAP_SIZE) == -1)
		printf("munmap error");

	if (munmap(cte_addr_va, CTE_MAP_SIZE) == -1)
		printf("munmap error");

        return 0;
}

int main()
{
        int *start;
        int file_device;
        unsigned int *reg_start_addr_va = NULL;

        unsigned long int RTA;
        int RTT;
        int ret = 0;

	int bus_num = 0;
	int dev_num = 0xf;
	int func_num = 0x0;

        reg_start_addr_va = (unsigned int *)(0x24AA66000);  // user-space start va, 这是指定映射到user-space va上的起始地址

        // file_device = open("/dev/zxmem", O_RDWR);
        file_device = open("/dev/mem", O_RDWR);
        if (file_device < 0)
        {
                printf("cannot open /dev/mem\n");
                return -1;
        }

        start = (int *)mmap(reg_start_addr_va, REG_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, file_device, 0xfe40c000);
        // printf("start=%llx\n", start);
        if (start < 0)
        {
                printf("mmap failed\n");
                return -1;
        }

        printf("VER_REG value:%#x\n", reg_start_addr_va[0]);   // offset 0h
        printf("CAP_REG value:0x%08x%08x\n", reg_start_addr_va[3], reg_start_addr_va[2]);  // index 1 means 32bit offset = 4h
        printf("ECAP_REG value:0x%08x%08x\n", reg_start_addr_va[5], reg_start_addr_va[4]);
        printf("GCMD_REG value:%#x\n", reg_start_addr_va[6]);
        printf("GSTS_REG value:%#x\n", reg_start_addr_va[7]);
        printf("RTADDR_REG value: 0x%08x%08x\n", reg_start_addr_va[9], reg_start_addr_va[8]);


        RTA = (unsigned long)reg_start_addr_va[9] << 32 | reg_start_addr_va[8];
        RTT = RTT_BIT(reg_start_addr_va[8]);
        RTA >>= 12;  // clear control bit
        RTA <<= 12;
        printf("RTT=%#x, RTA=0x%lx\n", RTT, RTA);

        ret = walk_structure_entry(RTT, RTA, file_device, bus_num, dev_num, func_num, 0xfe1010100);  // input_va 0xfe101000 only for test
        if (ret == -2)
                goto unmap;

unmap:
        ret = munmap(reg_start_addr_va, REG_MAP_SIZE);
        if (ret == -1)
                printf("munmap error");

        close(file_device);
        return 0;
}
