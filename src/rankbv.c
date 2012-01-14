
#include "rankbv.h"

rankbv_t*
rankbv_create(size_t n,uint32_t f)
{
    if (!f) f = rankbv_bits(n); /* lg(n) */
    size_t s = RBVW*f;
    size_t num_sblocks = n/s+1;
    size_t ints = n/RBVW+1;

    rankbv_t* rbv = (rankbv_t*) rankbv_safecalloc(sizeof(rankbv_t)
                    + (num_sblocks*sizeof(uint64_t))
                    + (ints*sizeof(uint32_t)));

    rbv->n = n;
    rbv->factor = f;
    rbv->s = s;

    return rbv;
}

rankbv_t*
rankbv_create(uint32_t* A,size_t n,uint32_t f)
{
    size_t i;
    rankbv_t* rbv = rankbv_create(n,f);
    size_t ints = n/RBVW+1;
    uint32_t* B = rankbv_getdata(rbv);
    for (i=0; i<ints; i++) B[i] = A[i];
    rankbv_build(rbv);
    return rbv;
}

void
rankbv_free(rankbv_t* rbv)
{
    if (rbv) {
        free(rbv);
    }
}

void
rankbv_build(rankbv_t* rbv)
{
    size_t i,j,start,stop;
    uint64_t tmp;
    size_t num_sblocks = rankbv_numsblocks(rbv);
    uint32_t* B = rankbv_getdata(rbv);
    for (i=1; i<num_sblocks; i++) {
        rbv->S[i] = rbv->S[i-1];
        tmp = 0;
        start = (i-1)*rbv->factor; stop = start+rbv->factor;
        for (j=start; j<stop; j++) tmp += rankbv_popcount(B[i]);
        rbv->S[i] += tmp;
    }
    rbv->ones = rankbv_rank1(rbv,rbv->n-1);
}

int
rankbv_access(rankbv_t* rbv,size_t i)
{
    uint32_t* B = rankbv_getdata(rbv);
    return rankbv_bitget(B,i);
}

size_t
rankbv_rank1(rankbv_t* rbv,size_t i)
{
    size_t j;
    i++;
    uint64_t resp = rbv->S[i/rbv->s];
    size_t start = (i/rbv->s)*rbv->factor;
    size_t stop = i/RBVW;
    uint32_t* B = rankbv_getdata(rbv);
    for (j=start; j<stop; j++) resp+=rankbv_popcount(B[j]);
    resp += rankbv_popcount(B[i/RBVW]&((1<<(i & rankbv_mask31))-1));
    return resp;
}


size_t
rankbv_ones(rankbv_t* rbv)
{
    return rbv->ones;
}

void
rankbv_print(rankbv_t* rbv)
{
    size_t i;
    uint32_t* B = rankbv_getdata(rbv);
    for (i = 0; i < rbv->n; ++i) {
        if (rankbv_bitget(B,i)) printf("1");
        else printf("0");
    }
    printf("\n");
}

size_t
rankbv_select0(rankbv_t* rbv,size_t x)
{
    if (x>rbv->n-rbv->ones) return (size_t)(-1);

    /* binary search over first level rank structure */
    if (x==0) return 0;
    size_t l=0, r=rankbv_numsblocks(rbv)-1;
    size_t mid=(l+r)/2;
    size_t rankmid = (mid * rbv->factor * RBVW) - rbv->S[mid];
    while (l<=r) {
        if (rankmid<x)
            l = mid+1;
        else
            r = mid-1;
        mid = (l+r)/2;
        rankmid = (mid * rbv->factor * RBVW) - rbv->S[mid];
    }
    /* sequential search using popcount over a int */
    size_t left;
    left= mid * rbv->factor;
    x-=rankmid;
    uint32_t* B = rankbv_getdata(rbv);
    size_t j= B[left];
    size_t zeros = RBVW - rankbv_popcount(j);
    size_t ints = rbv->n/RBVW+1;
    while (zeros < x) {
        x-=zeros; left++;
        if (left > ints) return rbv->n;
        j = B[left];
        zeros = RBVW-rankbv_popcount(j);
    }
    //sequential search using popcount over a char
    left=left*RBVW;
    rankmid = 8-rankbv_popcount8(j);
    if (rankmid < x) {
        j=j>>8;
        x-=rankmid;
        left+=8;
        rankmid = 8-rankbv_popcount8(j);
        if (rankmid < x) {
            j=j>>8;
            x-=rankmid;
            left+=8;
            rankmid = 8-rankbv_popcount8(j);
            if (rankmid < x) {
                j=j>>8;
                x-=rankmid;
                left+=8;
            }
        }
    }

    // then sequential search bit a bit
    while (x>0) {
        if (j%2 == 0) x--;
        j=j>>1;
        left++;
    }
    left--;
    if (left > rbv->n)  return rbv->n;
    else return left;
}


size_t
rankbv_select1(rankbv_t* rbv,size_t x)
{
    if (x == 0) return (size_t)(-1);
    if (x > rbv->ones)  return (size_t)(-1);
    size_t l=0, r=rankbv_numsblocks(rbv)-1;
    size_t mid=(l+r)/2;
    size_t rankmid = rbv->S[mid];
    /* binary search over first level rank structure */
    while (l<=r) {
        if (rankmid<x)
            l = mid+1;
        else
            r = mid-1;
        mid = (l+r)/2;
        rankmid = rbv->S[mid];
    }
    /* binary search over blocks */
    uint32_t* B = rankbv_getdata(rbv);
    size_t left;
    left=mid*rbv->factor;
    x-=rankmid;
    size_t ones = rankbv_popcount(B[left]);
    size_t ints = rbv->n/RBVW+1;
    while (ones < x) {
        x-=ones; left++;
        if (left > ints) return rbv->n;
        ones = rankbv_popcount(B[left]);
    }
    /* binsearch over integer */
    size_t j = B[left];
    left=left*RBVW;
    rankmid = rankbv_popcount8(j);
    if (rankmid < x) {
        j=j>>8;
        x-=rankmid;
        left+=8;
        rankmid = rankbv_popcount8(j);
        if (rankmid < x) {
            j=j>>8;
            x-=rankmid;
            left+=8;
            rankmid = rankbv_popcount8(j);
            if (rankmid < x) {
                j=j>>8;
                x-=rankmid;
                left+=8;
            }
        }
    }
    /* sequential search on bits */
    while (x>0) {
        if (j&1) x--;
        j=j>>1;
        left++;
    }
    return left-1;
}

size_t
rankbv_spaceusage(rankbv_t* rbv)
{
    size_t bytes;
    size_t num_sblocks = rankbv_numsblocks(rbv);
    bytes = sizeof(rankbv_t);
    bytes += sizeof(uint64_t)*(num_sblocks); /* S[] */
    bytes += sizeof(uint32_t)*(rbv->n/RBVW+1); /* A[] */
    return bytes;
}

rankbv_t*
rankbv_load(FILE* f)
{
    size_t bytes;
    /* read space */
    fread(&bytes,sizeof(size_t),1,f);

    /* read the bv */
    void* mem = rankbv_safecalloc(bytes);
    fread(mem,bytes,1,f);

    return (rankbv_t*)mem;
}

size_t
rankbv_save(rankbv_t* rbv,FILE* f)
{
    size_t ints = rbv->n/RBVW+1;
    size_t num_sblocks = rankbv_numsblocks(rbv);

    size_t bytes = sizeof(rankbv_t) + (sizeof(uint64_t)*num_sblocks) + (sizeof(uint32_t)*ints);

    fwrite(&bytes,sizeof(uint64_t),1,f);
    fwrite(rbv,sizeof(rankbv_t),1,f);
    fwrite(rbv->S,sizeof(uint64_t),num_sblocks,f);
    uint32_t* B = rankbv_getdata(rbv);
    fwrite(B,sizeof(uint32_t),ints,f);

    return bytes+sizeof(size_t);
}

void*
rankbv_safecalloc(size_t n)
{
    void* mem = calloc(n,1);
    if (!mem) {
        fprintf(stderr,"ERROR: rankbv_safecalloc()");
        exit(EXIT_FAILURE);
    }
    return mem;
}

