#include "TestHarness.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rankbv.h"

TEST(rankbv , saveload)
{
    uint32_t A[14] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,0};
    rankbv_t* rbv = rankbv_create(A,13*32,2);

    FILE* f = fopen("rankbv.test1","w");
    rankbv_save(rbv,f);
    fclose(f);
    f = fopen("rankbv.test1","r");
    rankbv_t* rbvl = rankbv_load(f);
    fclose(f);

    CHECK(rankbv_length(rbv)==rankbv_length(rbvl));
    CHECK(rankbv_ones(rbv)==rankbv_ones(rbvl));
    CHECK(rankbv_spaceusage(rbv)==rankbv_spaceusage(rbvl));

    CHECK(rbv->n == rbvl->n);
    CHECK(rbv->s == rbvl->s);
    CHECK(rbv->ones == rbvl->ones);
    CHECK(rbv->factor == rbvl->factor);

    for (size_t i=0; i<rankbv_length(rbv); i++) {
        CHECK(rankbv_access(rbv,i)==rankbv_access(rbvl,i));
        CHECK(rankbv_rank1(rbv,i)==rankbv_rank1(rbvl,i));
    }

    size_t numones = rankbv_ones(rbv);
    size_t numzeros = rankbv_length(rbv) - numones;
    for (size_t i=1; i<=numones; i++) {
        CHECK(rankbv_select1(rbv,i)==rankbv_select1(rbvl,i));
    }
    for (size_t i=1; i<=numzeros; i++) {
        CHECK(rankbv_select0(rbv,i)==rankbv_select0(rbvl,i));
    }

    rankbv_free(rbv);
    rankbv_free(rbvl);
}

TEST(rankbv , mmap)
{
    uint32_t A[14] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,0};
    rankbv_t* rbv = rankbv_create(A,13*32,2);

    FILE* f = fopen("rankbv.test1","w");
    rankbv_save(rbv,f);
    fclose(f);

    int fd = open("rankbv.test1",O_RDONLY);
    struct stat sb;
    if (fstat(fd,&sb)==-1) {
        perror("fstat");
        return;
    }

    void* mem = mmap(0,sb.st_size,PROT_READ,MAP_PRIVATE,fd,0);

    if (mem != MAP_FAILED) {
        rankbv_t* rbvl = (rankbv_t*)((((char*)mem)+sizeof(size_t)));

        CHECK(rankbv_length(rbv)==rankbv_length(rbvl));
        CHECK(rankbv_ones(rbv)==rankbv_ones(rbvl));
        CHECK(rankbv_spaceusage(rbv)==rankbv_spaceusage(rbvl));

        CHECK(rbv->n == rbvl->n);
        CHECK(rbv->s == rbvl->s);
        CHECK(rbv->ones == rbvl->ones);
        CHECK(rbv->factor == rbvl->factor);

        for (size_t i=0; i<rankbv_length(rbv); i++) {
            CHECK(rankbv_access(rbv,i)==rankbv_access(rbvl,i));
            CHECK(rankbv_rank1(rbv,i)==rankbv_rank1(rbvl,i));
        }

        size_t numones = rankbv_ones(rbv);
        size_t numzeros = rankbv_length(rbv) - numones;
        for (size_t i=1; i<=numones; i++) {
            CHECK(rankbv_select1(rbv,i)==rankbv_select1(rbvl,i));
        }
        for (size_t i=1; i<=numzeros; i++) {
            CHECK(rankbv_select0(rbv,i)==rankbv_select0(rbvl,i));
        }

        munmap(mem,sb.st_size);
    }
    rankbv_free(rbv);
}

TEST(rankbv , select0)
{
    rankbv_t* rbv = rankbv_create(500,2);
    rankbv_setbit(rbv,1);
    rankbv_setbit(rbv,3);
    rankbv_setbit(rbv,32);
    rankbv_setbit(rbv,50);
    rankbv_setbit(rbv,63);
    rankbv_setbit(rbv,499);

    rankbv_build(rbv);

    CHECK(rankbv_select0(rbv,1)==0);
    CHECK(rankbv_select0(rbv,2)==2);
    CHECK(rankbv_select0(rbv,3)==4);
    CHECK(rankbv_select0(rbv,4)==5);
    CHECK(rankbv_select0(rbv,5)==6);
    CHECK(rankbv_select0(rbv,6)==7);


    rankbv_free(rbv);
}

TEST(rankbv , select1)
{
    rankbv_t* rbv = rankbv_create(500,2);
    rankbv_setbit(rbv,1);
    rankbv_setbit(rbv,3);
    rankbv_setbit(rbv,32);
    rankbv_setbit(rbv,50);
    rankbv_setbit(rbv,63);
    rankbv_setbit(rbv,499);

    rankbv_build(rbv);

    CHECK(rankbv_select1(rbv,1)==1);
    CHECK(rankbv_select1(rbv,2)==3);
    CHECK(rankbv_select1(rbv,3)==32);
    CHECK(rankbv_select1(rbv,4)==50);
    CHECK(rankbv_select1(rbv,5)==63);
    CHECK(rankbv_select1(rbv,6)==499);


    rankbv_free(rbv);
}

TEST(rankbv , access)
{
    rankbv_t* rbv = rankbv_create(500,2);
    rankbv_setbit(rbv,1);
    rankbv_setbit(rbv,3);
    rankbv_setbit(rbv,50);
    rankbv_setbit(rbv,32);
    rankbv_setbit(rbv,63);
    rankbv_setbit(rbv,499);

    CHECK(rankbv_access(rbv,0)==0);
    CHECK(rankbv_access(rbv,1)==1);
    CHECK(rankbv_access(rbv,2)==0);
    CHECK(rankbv_access(rbv,3)==1);
    CHECK(rankbv_access(rbv,4)==0);
    CHECK(rankbv_access(rbv,49)==0);
    CHECK(rankbv_access(rbv,50)==1);
    CHECK(rankbv_access(rbv,51)==0);
    CHECK(rankbv_access(rbv,31)==0);
    CHECK(rankbv_access(rbv,32)==1);
    CHECK(rankbv_access(rbv,33)==0);
    CHECK(rankbv_access(rbv,63)==1);
    CHECK(rankbv_access(rbv,499)==1);

    rankbv_free(rbv);

    uint32_t A[14] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,0};
    rbv = rankbv_create(A,13*32,2);

    CHECK(rankbv_access(rbv,0)==1);
    CHECK(rankbv_access(rbv,33)==1);
    CHECK(rankbv_access(rbv,32)==0);
    CHECK(rankbv_access(rbv,66)==1);
    CHECK(rankbv_access(rbv,67)==0);
    CHECK(rankbv_access(rbv,100)==0);

    rankbv_free(rbv);
}

TEST(rankbv , rank)
{
    uint32_t A[14] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,0};
    rankbv_t* rbv = rankbv_create(A,13*32,2);

    CHECK(rankbv_rank1(rbv,5)==1);
    CHECK(rankbv_rank1(rbv,40)==2);
    CHECK(rankbv_rank1(rbv,65)==2);
    CHECK(rankbv_rank1(rbv,66)==3);
    CHECK(rankbv_rank1(rbv,67)==3);
    CHECK(rankbv_rank1(rbv,100)==4);

    rankbv_free(rbv);
}




