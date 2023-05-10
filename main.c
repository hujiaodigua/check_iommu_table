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

#define OFFSET_INDEX(x)  (x * 4)

int walk_structure_entry(int rtt_val, unsigned long int rta_pointer_val, int fd, int bus_n, int dev_n, int func_n)
{
        unsigned int *rte_addr_va = NULL;
	unsigned int *cte_addr_va = NULL;
        int *start;
	int devfn = dev_n << 3 | func_n;

	rte_addr_va = (unsigned int *)(0x25AA66000);
	cte_addr_va = (unsigned int *)(0x26AA66000);

	unsigned long int CTP;

        if (rtt_val == 0)
        {
		// root entry
		start = (int *)mmap(rte_addr_va, RTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, rta_pointer_val);
        	if (start < 0)
        	{
                	printf("mmap failed in %s\n", __func__);
                	return -1;
        	}
		printf("root entry bus %d [63-0]:0x%x%x\n",
				bus_n, rte_addr_va[1 + OFFSET_INDEX(bus_n)], rte_addr_va[0 + OFFSET_INDEX(bus_n)]);
		printf("root entry bus %d [127-64]:0x%x%x\n",
				bus_n, rte_addr_va[3 + OFFSET_INDEX(bus_n)], rte_addr_va[2 + OFFSET_INDEX(bus_n)]);
		CTP = (unsigned long int)rte_addr_va[1 + OFFSET_INDEX(bus_n)] << 32 | rte_addr_va[0 + OFFSET_INDEX(bus_n)];
		CTP >>= 12;
		CTP <<= 12;
		printf("CTP=0x%lx\n", CTP);
		
		// context entry
		start = (int *)mmap(cte_addr_va, RTE_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CTP);
        	if (start < 0)
        	{
                	printf("mmap failed in %s\n", __func__);
                	return -1;
        	}
		printf("conext entry dev 0x%x func %d [63-0]:0x%x%x\n",
				dev_n, func_n, cte_addr_va[1 + OFFSET_INDEX(devfn)], cte_addr_va[0 + OFFSET_INDEX(devfn)]);
		printf("conext entry dev 0x%x func %d [127-64]:0x%x%x\n",
				dev_n, func_n, cte_addr_va[3 + OFFSET_INDEX(devfn)], cte_addr_va[2 + OFFSET_INDEX(devfn)]);

	}
        else if (rtt_val == 1)
        {

        }
        else
        {
                return -2;
        }
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

        file_device = open("/dev/zxmem", O_RDWR);
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
        printf("CAP_REG value:0x%x%x\n", reg_start_addr_va[3], reg_start_addr_va[2]);  // index 1 means 32bit offset = 4h
        printf("ECAP_REG value:0x%x%x\n", reg_start_addr_va[5], reg_start_addr_va[4]);
        printf("GCMD_REG value:%#x\n", reg_start_addr_va[6]);
        printf("GSTS_REG value:%#x\n", reg_start_addr_va[7]);
        printf("RTADDR_REG value: 0x%x%x\n", reg_start_addr_va[9], reg_start_addr_va[8]);


        RTA = (unsigned long)reg_start_addr_va[9] << 32 | reg_start_addr_va[8];
        RTT = RTT_BIT(reg_start_addr_va[8]);
        RTA >>= 12;  // clear control bit
        RTA <<= 12;
        printf("RTT=%x, RTA=0x%lx\n", RTT, RTA);

        ret = walk_structure_entry(RTT, RTA, file_device, bus_num, dev_num, func_num);
        if (ret == -2)
                goto unmap;

unmap:
        ret = munmap(reg_start_addr_va, REG_MAP_SIZE);
        if (ret == -1)
                printf("munmap error");

        close(file_device);
        return 0;
}
