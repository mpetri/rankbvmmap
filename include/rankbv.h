
#ifndef RANKBV_H
#define RANKBV_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct rankbv {
    uint64_t n;
    uint32_t s;
    uint64_t ones;
    uint8_t factor;
    uint64_t S[0];
} rankbv_t;

const uint8_t rankbv_popcount_tab[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

/** bit operations */
#define rankbv_mask31 		0x0000001F
#define RBVW				32
#define rankbv_bitget(e,p) ((((e)[(p)/RBVW] >> ((p)%RBVW))) & 1)
inline void rankbv_bitset(uint32_t* e, size_t p)
{
    e[p/RBVW] |= (1<<(p%RBVW));
}
inline uint32_t rankbv_bits(size_t n)
{
    uint32_t b = 0;
    while (n) {
        b++;
        n >>= 1;
    }
    return b;
}
inline uint32_t
rankbv_popcount(const uint32_t x)
{
    return rankbv_popcount_tab[(x >>  0) & 0xff]  + rankbv_popcount_tab[(x >>  8) & 0xff]
           + rankbv_popcount_tab[(x >> 16) & 0xff] + rankbv_popcount_tab[(x >> 24) & 0xff];
}
inline uint32_t
rankbv_popcount8(const uint32_t x)
{
    return rankbv_popcount_tab[x & 0xff];
}

inline size_t
rankbv_numsblocks(rankbv_t* rbv)
{
    return rbv->n/rbv->s+1;
}

inline uint32_t*
rankbv_getdata(rankbv_t* rbv)
{
    size_t num_sblocks = rankbv_numsblocks(rbv);
    return (uint32_t*)(((char*)rbv) + sizeof(rankbv_t) +
                       (sizeof(uint64_t)*num_sblocks));
}

inline void
rankbv_setbit(rankbv_t* rbv,size_t i)
{
    uint32_t* B = rankbv_getdata(rbv);
    rankbv_bitset(B,i);
}

inline int
rankbv_getbit(rankbv_t* rbv,size_t i)
{
    uint32_t* B = rankbv_getdata(rbv);
    return rankbv_bitget(B,i);
}

/* rankbv functions */
rankbv_t* rankbv_create(size_t n,uint32_t f);
rankbv_t* rankbv_create(uint32_t* A,size_t n,uint32_t f);
void      rankbv_free(rankbv_t* rbv);
void      rankbv_build(rankbv_t* rbv);
int       rankbv_access(rankbv_t* rbv,size_t i);
size_t    rankbv_rank1(rankbv_t* rbv,size_t i);
size_t    rankbv_select0(rankbv_t* rbv,size_t x);
size_t    rankbv_select1(rankbv_t* rbv,size_t x);
size_t    rankbv_ones(rankbv_t* rbv);
void      rankbv_print(rankbv_t* rbv);

inline size_t
rankbv_length(rankbv_t* rbv)
{
    return rbv->n;
}

/* save/load */
size_t    rankbv_spaceusage(rankbv_t* rbv);
rankbv_t* rankbv_load(FILE* f);
size_t    rankbv_save(rankbv_t* rbv,FILE* f);

/* misc */
void*     rankbv_safecalloc(size_t n);



#endif

