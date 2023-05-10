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
#define CTP_MAP_SIZE 0xFF

int walk_structure_entry(int rtt_val, unsigned rta_pointer_val, int fd)
{
        unsigned *ctp_addr_va = (unsigned int *)(0x25AA66000);
        int *start;

        start = (int *)mmap(ctp_addr_va, CTP_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, rta_pointer_val);

        if (rtt_val == 0)
        {

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

        reg_start_addr_va = (unsigned int *)(0x24AA66000);  // user-space start va, 这是指定映射到user-space va上的起始地址

        file_device = open("/dev/mem", O_RDWR);
        if (file_device < 0)
        {
                printf("cannot open /dev/mem\n");
                return -1;
        }

        start = (int *)mmap(reg_start_addr_va, REG_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, file_device, 0xfed91000);
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

        ret = walk_structure_entry(RTT, RTA, file_device);
        if (ret == -2)
                goto unmap;

unmap:
        ret = munmap(reg_start_addr_va, REG_MAP_SIZE);
        if (ret == -1)
                printf("munmap error");

        close(file_device);
        return 0;
}
