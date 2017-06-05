//generic cache simulator
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int L1read_hits=0,L2read_hits=0;
int L1reads=0,L2reads=0;
int L1writes=0,L2writes=0;
int L1write_hits=0,L2write_hits=0;
int L1read_misses=0,L2read_misses=0;
int L1write_misses=0,L2write_misses=0;
int L1writebacks=0,L2writebacks=0;
int L1policy_counter=0,L2policy_counter=0;
int L1index_width=0,L2index_width=0;
int L1offset_width=0,L2offset_width=0;
int L1tag_width=0,L2tag_width=0;
char L1policy[10],L2policy[10];
int L1cache_size,L2cache_size;
int L1assoc,L2assoc;
int L1block_size,L2block_size;
int l=1;
float L1missrate,L2missrate;
int memtraffic;
unsigned long long address,L1tag,L2tag;
unsigned long L1dec_index=0,L2dec_index=0;
unsigned long L1dec_offset=0,L2dec_offset=0;


typedef struct Cache
{
    int valid;
    unsigned long long tag;
    int dirty;
    int policycounter;
}Cache;

void L1readcache(Cache L1cache[][32],Cache L2cache[][32]);
void L1writecache(Cache L1cache[][32],Cache L2cache[][32]);
int L1cachepolicy(Cache L1cache[][32]);
int LRU(Cache L1cache[][32]);
void L2writecache(Cache L1cache[][32],Cache L2cache[][32],int);
void L2readcache(Cache L2cache[][32]);
int L2cachepolicy(Cache L2cache[][32]);
int L2LRU(Cache L2cache[][32]);
int L2writecachepolicy(Cache L2cache[][32],int);
int L2writeLRU(Cache L2cache[][32],int);


int main(int argc , char *argv[])
{
    setbuf(stdout,NULL);


    //printf("Entered parameters are:\nBlocksize: %s L1_size: %s L1_assoc: %s L2_size: %s L2_assoc: %s Repl_policy: %s Inclusion: %s Trace_file: %s \n",argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8]);

    int i,j,x,y;
	x=atoi(argv[6]);
	y=atoi(argv[7]);
    /*Creating L1 cache*/
        int l1cachesize=atoi(argv[2]),l1assoc=atoi(argv[3]),l1blocksize=atoi(argv[1]);
        Cache L1cache[1000][32];
        L1cache_size=l1cachesize;
        L1assoc=l1assoc;
        L1block_size=l1blocksize;
        L1index_width=(int)(log(l1cachesize/(l1assoc*l1blocksize))/log(2));

        L1offset_width=(int)(log(l1blocksize)/log(2));

        strcpy(L1policy,argv[6]);
        L1tag_width=64-L1index_width-L1offset_width;
        for(i = 0;i<l1cachesize/(l1assoc*l1blocksize);i++)
        {
            for(j=0;j<l1assoc;j++)
            {
                L1cache[i][j].valid = 0;
                L1cache[i][j].dirty = 0;
                L1cache[i][j].tag=0;
                L1cache[i][j].policycounter=0;
            }
        }

    /*Creating L2 cache*/
    int l2cachesize=atoi(argv[4]),l2assoc=atoi(argv[5]),l2blocksize=atoi(argv[1]);
        Cache L2cache[2000][32];
    if(l2assoc==0 || l2blocksize==0)
    {
        L2cache_size=0;
    }

    else
    {

        L2cache_size=l2cachesize;
        L2assoc=l2assoc;
        L2block_size=l2blocksize;
        L2index_width=(int)(log(l2cachesize/(l2assoc*l2blocksize))/log(2));
        L2offset_width=(int)(log(l2blocksize)/log(2));
        strcpy(L2policy,argv[6]);

        for(i = 0;i<l2cachesize/(l2assoc*l2blocksize);i++)
        {
            for(j=0;j<l2assoc;j++)
            {
                L2cache[i][j].valid = 0;
                L2cache[i][j].dirty = 0;
                L2cache[i][j].tag=0;
                L2cache[i][j].policycounter=0;
            }
        }
    }

     char r_w='0';

    FILE *fp;
    fp=fopen(argv[8],"r");//reading trace file
    fseek(fp,0,SEEK_SET);
    if(fp==NULL)
        {
            printf("\nFile does not contain any data: Exiting\n");
            exit(1);
        }

    while(1)
    {
        if(feof(fp))
            {
                break;
            }
        fscanf(fp,"%c %llx\n",&r_w,&address);

        /*decoding trace for L1*/
        L1tag=address>>(L1index_width+L1offset_width);
        L1dec_index=(address<<L1tag_width)>>(L1tag_width+L1offset_width);
        L1dec_offset=(address<<(L1tag_width+L1index_width))>>(L1tag_width+L1index_width);


        /*decoding trace for L2*/
        if(L2cache_size!=0)
        {
            L2tag_width=64-L2index_width-L2offset_width;
            L2tag=address>>(L2index_width+L2offset_width);
            L2dec_index=(address<<L2tag_width)>>(L2tag_width+L2offset_width);
            L2dec_offset=(address<<(L2tag_width+L2index_width))>>(L2tag_width+L2index_width);
        }

        if(r_w=='r')
            L1readcache(L1cache,L2cache);
        else if(r_w=='w')
            L1writecache(L1cache,L2cache);
            else printf("There is some error\n");
    }

    L1missrate=(float)(((float)L1read_misses+(float)L1write_misses)/((float)L1reads+(float)L1writes));

    if(L2cache_size==0)
    L2missrate=0;
    else
        L2missrate=(float)(((float)L2read_misses+(float)L2write_misses)/((float)L2reads+(float)L2writes));

    if(L2cache_size==0)
    memtraffic=L1read_misses+L1write_misses+L1writebacks;
    else
        memtraffic=L2read_misses+L2write_misses+L2writebacks;

printf("===== Simulator configuration =====\n");
printf("BLOCKSIZE:             %s\n",argv[1]);
printf("L1_SIZE:               %s\n",argv[2]);
printf("L1_ASSOC:              %s\n",argv[3]);
printf("L2_SIZE:               %s\n",argv[4]);
printf("L2_ASSOC:              %s\n",argv[5]);
if(x==0)
	printf("REPLACEMENT POLICY:    LRU\n");
else if(x==1)
	printf("REPLACEMENT POLICY:    FIFO\n");
else if(x==2)
	printf("REPLACEMENT POLICY:    Pseudo\n");
else if(x==3)
	printf("REPLACEMENT POLICY:    Optimal\n");
	else printf("REPLACEMENT POLICY:    ---\n");

if(y==0)
	printf("INCLUSION PROPERTY:    non-inclusive\n");
else if(y==1)
	printf("INCLUSION PROPERTY:    inclusive\n");
else if(y==2)
	printf("INCLUSION PROPERTY:    exclusive\n");
else printf("INCLUSION PROPERTY:    ---\n");

printf("trace_file:            %s\n",argv[8]);
printf("===== Simulation results (raw) =====\n");
printf("a. number of L1 reads:        %d\n",L1reads);
printf("b. number of L1 read misses:  %d\n",L1read_misses);
printf("c. number of L1 writes:       %d\n",L1writes);
printf("d. number of L1 write misses: %d\n",L1write_misses);
printf("e. L1 miss rate:              %f\n",L1missrate);
printf("f. number of L1 writebacks:   %d\n",L1writebacks);
printf("g. number of L2 reads:        %d\n",L2reads);
printf("h. number of L2 read misses:  %d\n",L2read_misses);
printf("i. number of L2 writes:       %d\n",L2writes);
printf("j. number of L2 write misses: %d\n",L2write_misses);
printf("k. L2 miss rate:              %f\n",L2missrate);
printf("l. number of L2 writebacks:   %d\n",L2writebacks);
printf("m. total memory traffic:      %d",memtraffic);
    //printf("Number of L1 reads:%d\nNumber of L1 read misses:%d\nNumber of L1 writes:%d\nNumber of L1 write misses:%d\nL1 miss rate:%f\nNumber of L1 writebacks:%d\n",L1reads,L1read_misses,L1writes,L1write_misses,L1missrate,L1writebacks);
    //printf("Number of L2 reads:%d\nNumber of L2 read misses:%d\nNumber of L2 writes:%d\nNumber of L2 write misses:%d\nL2 miss rate:%f\nNumber of L2 writebacks:%d\nTotal memory traffic:%d\n",L2reads,L2read_misses,L2writes,L2write_misses,L2missrate,L2writebacks,memtraffic);

    fclose(fp);

    return 0;
}


void L1readcache(Cache L1cache[][32],Cache L2cache[][32])
{
    int i,counter=0,counter2=0,rep_candidate=0;

    L1reads+=1;
    L1policy_counter+=1;

    if(strcasecmp(L1policy,"0")==0 || strcasecmp(L1policy,"2")==0 || strcasecmp(L1policy,"3")==0)
    {
        for(i=0;i<L1assoc;i++)
    {
	if(L1cache[L1dec_index][i].valid==1 && (L1cache[L1dec_index][i].tag==L1tag))        //handling read hits
        {
            L1read_hits+=1;
            counter=1;
            L1cache[L1dec_index][i].policycounter=L1policy_counter;
            break;
        }
    }
    if(counter==0)          //handling read misses
    {
        L1read_misses+=1;
        for(i=0;i<L1assoc;i++)
        {

            if(L1cache[L1dec_index][i].valid==0)
            {
                rep_candidate=i;
                break;
            }
            if(L1cache[L1dec_index][i].valid==1)
            {
                counter2+=1;
            }
        }
        if(counter2==L1assoc)
        {
            rep_candidate=L1cachepolicy(L1cache);
        }

        if(L1cache[L1dec_index][rep_candidate].dirty==1 && L1cache[L1dec_index][rep_candidate].valid==1)
        {
            L1writebacks+=1;
            if(L2cache_size!=0)
            L2writecache(L1cache,L2cache,rep_candidate);
        }
        if(L2cache_size!=0)
        L2readcache(L2cache);

        L1cache[L1dec_index][rep_candidate].tag=L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter=L1policy_counter;
        L1cache[L1dec_index][rep_candidate].dirty=0;
        L1cache[L1dec_index][rep_candidate].valid=1;
    }
    }

    else if(strcasecmp(L1policy,"1")==0)
    {
        for(i=0;i<L1assoc;i++)
    {
	if(L1cache[L1dec_index][i].valid==1 && (L1cache[L1dec_index][i].tag==L1tag))        //handling read hits
        {
            L1read_hits+=1;
            counter=1;
            break;
        }
    }
    if(counter==0)          //handling read misses
    {
        L1read_misses+=1;
        for(i=0;i<L1assoc;i++)
        {

            if(L1cache[L1dec_index][i].valid==0)
            {
                rep_candidate=i;
                break;
            }
            if(L1cache[L1dec_index][i].valid==1)
            {
                counter2+=1;
            }
        }
        if(counter2==L1assoc)
        {
            rep_candidate=L1cachepolicy(L1cache);
        }

        if(L1cache[L1dec_index][rep_candidate].dirty==1 && L1cache[L1dec_index][rep_candidate].valid==1)
        {
            L1writebacks+=1;
            if(L2cache_size!=0)
            L2writecache(L1cache,L2cache,rep_candidate);
        }
        if(L2cache_size!=0)
        L2readcache(L2cache);

        L1cache[L1dec_index][rep_candidate].tag=L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter=L1policy_counter;
        L1cache[L1dec_index][rep_candidate].dirty=0;
        L1cache[L1dec_index][rep_candidate].valid=1;
    }
    }

}

void L1writecache(Cache L1cache[][32],Cache L2cache[][32])
{
    int i,counter=0,counter2=0,rep_candidate=0;
    L1writes+=1;
    L1policy_counter+=1;

    if(strcasecmp(L1policy,"0")==0 || strcasecmp(L1policy,"2")==0 || strcasecmp(L1policy,"3")==0)
    {
        for(i=0;i<L1assoc;i++)
    {

        if(L1cache[L1dec_index][i].valid==1 && (L1tag==L1cache[L1dec_index][i].tag))        //handling write hits
        {
            L1write_hits+=1;
            counter=1;
            L1cache[L1dec_index][i].policycounter=L1policy_counter;
            L1cache[L1dec_index][i].dirty=1;
            break;
        }
    }
    if(counter==0)          //handling write misses
    {
        L1write_misses+=1;
        for(i=0;i<L1assoc;i++)
        {
            if(L1cache[L1dec_index][i].valid==0)
            {
                rep_candidate=i;
                break;
            }
            else
                counter2+=1;
        }
        if(counter2==L1assoc)
        {
            rep_candidate=L1cachepolicy(L1cache);
        }

        if(L1cache[L1dec_index][rep_candidate].dirty==1 && L1cache[L1dec_index][rep_candidate].valid==1)
        {
            L1writebacks+=1;
            if(L2cache_size!=0)
            L2writecache(L1cache,L2cache,rep_candidate);
        }
        if(L2cache_size!=0)
        L2readcache(L2cache);

        L1cache[L1dec_index][rep_candidate].tag=L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter=L1policy_counter;
        L1cache[L1dec_index][rep_candidate].dirty=1;
        L1cache[L1dec_index][rep_candidate].valid=1;
    }
    }

    else if(strcasecmp(L1policy,"1")==0)
    {
        for(i=0;i<L1assoc;i++)
    {

        if(L1cache[L1dec_index][i].valid==1 && (L1tag==L1cache[L1dec_index][i].tag))        //handling write hits
        {
            L1write_hits+=1;
            counter=1;
            L1cache[L1dec_index][i].dirty=1;
            break;
        }
    }
    if(counter==0)          //handling write misses
    {
        L1write_misses+=1;
        for(i=0;i<L1assoc;i++)
        {
            if(L1cache[L1dec_index][i].valid==0)
            {
                rep_candidate=i;
                break;
            }
            else
                counter2+=1;
        }
        if(counter2==L1assoc)
        {
            rep_candidate=L1cachepolicy(L1cache);
        }

        if(L1cache[L1dec_index][rep_candidate].dirty==1 && L1cache[L1dec_index][rep_candidate].valid==1)
        {
            L1writebacks+=1;
            if(L2cache_size!=0)
            L2writecache(L1cache,L2cache,rep_candidate);
        }
        if(L2cache_size!=0)
        L2readcache(L2cache);

        L1cache[L1dec_index][rep_candidate].tag=L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter=L1policy_counter;
        L1cache[L1dec_index][rep_candidate].dirty=1;
        L1cache[L1dec_index][rep_candidate].valid=1;
    }
    }
}

int L1cachepolicy(Cache L1cache[][32])
{
    int rep_candidate;
    if(strcasecmp(L1policy,"0")==0 || strcasecmp(L1policy,"2")==0 || strcasecmp(L1policy,"3")==0)
        rep_candidate=LRU(L1cache);
    else if(strcasecmp(L1policy,"1")==0)
        rep_candidate=LRU(L1cache);
        /*else if(strcasecmp(L1cache.policy,"pseudoLRU")==0)
        rep_candidate=pseudoLRU(Cache L1cache,L1dec_index);*/

    return rep_candidate;
}

int LRU(Cache L1cache[][32])
{
    int i,lru=0,lruway=0;

    lru=L1cache[L1dec_index][0].policycounter;
    for(i=1;i<L1assoc;i++)
    {

        if(L1cache[L1dec_index][i].policycounter<lru)
        {
            lru=L1cache[L1dec_index][i].policycounter;
            lruway=i;
        }

    }
    return lruway;
}


void L2writecache(Cache L1cache[][32],Cache L2cache[][32],int rep_candidate)
{
    unsigned long long decode_add=0,tag=0,l2tag=0;
    unsigned long l2dec_index=0;

    tag=L1cache[L1dec_index][rep_candidate].tag;
    decode_add=( (((tag<<L1index_width) | (L1dec_index)) << (L1offset_width)) | (L1dec_offset) );//decoding address from tag of L1 eviction block


    l2tag=decode_add>>(L2index_width+L2offset_width);
    l2dec_index=(decode_add<<L2tag_width)>>(L2tag_width+L2offset_width);

    int i,counter=0,counter2=0,rep2_candidate=0;
    L2writes+=1;
    L2policy_counter+=1;

    if(strcasecmp(L2policy,"0")==0 || strcasecmp(L2policy,"2")==0 || strcasecmp(L2policy,"3")==0)
    {
        for(i=0;i<L2assoc;i++)
    {

        if(L2cache[l2dec_index][i].valid==1 && (l2tag==L2cache[l2dec_index][i].tag))        //handling write hits
        {
            L2write_hits+=1;
            counter=1;
            L2cache[l2dec_index][i].policycounter=L2policy_counter;
            L2cache[l2dec_index][i].dirty=1;
            break;
        }
    }
    if(counter==0)          //handling read misses
    {
        L2write_misses+=1;
        for(i=0;i<L2assoc;i++)
        {
            if(L2cache[l2dec_index][i].valid==0)
            {
                rep2_candidate=i;
                break;
            }
            else
                counter2+=1;
        }
        if(counter2==L2assoc)
        {
            rep2_candidate=L2writecachepolicy(L2cache,l2dec_index);
        }

        if(L2cache[l2dec_index][rep2_candidate].dirty==1 && L2cache[l2dec_index][rep2_candidate].valid==1)
        {
            L2writebacks+=1;
        }

        L2cache[l2dec_index][rep2_candidate].tag=l2tag;
        L2cache[l2dec_index][rep2_candidate].policycounter=L2policy_counter;
        L2cache[l2dec_index][rep2_candidate].dirty=1;
        L2cache[l2dec_index][rep2_candidate].valid=1;
    }
    }

    else if(strcasecmp(L2policy,"1")==0)
    {
        for(i=0;i<L2assoc;i++)
    {

        if(L2cache[l2dec_index][i].valid==1 && (l2tag==L2cache[l2dec_index][i].tag))        //handling write hits
        {
            L2write_hits+=1;
            counter=1;
            L2cache[l2dec_index][i].dirty=1;
            break;
        }
    }
    if(counter==0)          //handling read misses
    {
        L2write_misses+=1;
        for(i=0;i<L2assoc;i++)
        {
            if(L2cache[l2dec_index][i].valid==0)
            {
                rep2_candidate=i;
                break;
            }
            else
                counter2+=1;
        }
        if(counter2==L2assoc)
        {
            rep2_candidate=L2writecachepolicy(L2cache,l2dec_index);
        }

        if(L2cache[l2dec_index][rep2_candidate].dirty==1 && L2cache[l2dec_index][rep2_candidate].valid==1)
        {
            L2writebacks+=1;
        }

        L2cache[l2dec_index][rep2_candidate].tag=l2tag;
        L2cache[l2dec_index][rep2_candidate].policycounter=L2policy_counter;
        L2cache[l2dec_index][rep2_candidate].dirty=1;
        L2cache[l2dec_index][rep2_candidate].valid=1;
    }
    }
}

void L2readcache(Cache L2cache[][32])
{
    int i,counter=0,counter2=0,rep_candidate=0;

    L2reads+=1;
    L2policy_counter+=1;

    if(strcasecmp(L2policy,"0")==0 || strcasecmp(L2policy,"2")==0 || strcasecmp(L2policy,"3")==0)
    {
        for(i=0;i<L2assoc;i++)
    {
	if(L2cache[L2dec_index][i].valid==1 && (L2cache[L2dec_index][i].tag==L2tag))        //handling read hits
        {
            L2read_hits+=1;
            counter=1;
            L2cache[L2dec_index][i].policycounter=L2policy_counter;
            break;
        }
    }
    if(counter==0)          //handling read misses
    {
        L2read_misses+=1;
        for(i=0;i<L2assoc;i++)
        {

            if(L2cache[L2dec_index][i].valid==0)
            {
                rep_candidate=i;
                break;
            }
            if(L2cache[L2dec_index][i].valid==1)
            {
                counter2+=1;
            }
        }
        if(counter2==L2assoc)
        {
            rep_candidate=L2cachepolicy(L2cache);
        }

        if(L2cache[L2dec_index][rep_candidate].dirty==1 && L2cache[L2dec_index][rep_candidate].valid==1)
        {
            L2writebacks+=1;
        }

        L2cache[L2dec_index][rep_candidate].tag=L2tag;
        L2cache[L2dec_index][rep_candidate].policycounter=L2policy_counter;
        L2cache[L2dec_index][rep_candidate].dirty=0;
        L2cache[L2dec_index][rep_candidate].valid=1;
    }
    }

    else if(strcasecmp(L2policy,"1")==0)
    {
        for(i=0;i<L2assoc;i++)
    {
	if(L2cache[L2dec_index][i].valid==1 && (L2cache[L2dec_index][i].tag==L2tag))        //handling read hits
        {
            L2read_hits+=1;
            counter=1;
            break;
        }
    }
    if(counter==0)          //handling read misses
    {
        L2read_misses+=1;
        for(i=0;i<L2assoc;i++)
        {

            if(L2cache[L2dec_index][i].valid==0)
            {
                rep_candidate=i;
                break;
            }
            if(L2cache[L2dec_index][i].valid==1)
            {
                counter2+=1;
            }
        }
        if(counter2==L2assoc)
        {
            rep_candidate=L2cachepolicy(L2cache);
        }

        if(L2cache[L2dec_index][rep_candidate].dirty==1 && L2cache[L2dec_index][rep_candidate].valid==1)
        {
            L2writebacks+=1;
        }

        L2cache[L2dec_index][rep_candidate].tag=L2tag;
        L2cache[L2dec_index][rep_candidate].policycounter=L2policy_counter;
        L2cache[L2dec_index][rep_candidate].dirty=0;
        L2cache[L2dec_index][rep_candidate].valid=1;
    }
    }

}


int L2cachepolicy(Cache L2cache[][32])
{
    int rep_candidate;
    if(strcasecmp(L2policy,"0")==0 || strcasecmp(L2policy,"2")==0 || strcasecmp(L2policy,"3")==0)
        rep_candidate=L2LRU(L2cache);
    else if(strcasecmp(L2policy,"1")==0)
        rep_candidate=L2LRU(L2cache);
        /*else if(strcasecmp(L2cache.policy,"pseudoLRU")==0)
        rep_candidate=pseudoLRU(Cache L2cache,L2dec_index);*/

    return rep_candidate;
}

int L2LRU(Cache L2cache[][32])
{
    int i,lru=0,lruway=0;

    lru=L2cache[L2dec_index][0].policycounter;
    for(i=1;i<L2assoc;i++)
    {

        if(L2cache[L2dec_index][i].policycounter<lru)
        {
            lru=L2cache[L2dec_index][i].policycounter;
            lruway=i;
        }

    }
    return lruway;
}

int L2writecachepolicy(Cache L2cache[][32],int l2dec_index)
{
    int rep_candidate;
    if(strcasecmp(L2policy,"0")==0 || strcasecmp(L2policy,"2")==0 || strcasecmp(L2policy,"3")==0)
        rep_candidate=L2writeLRU(L2cache,l2dec_index);
    else if(strcasecmp(L2policy,"1")==0)
        rep_candidate=L2writeLRU(L2cache,l2dec_index);
        /*else if(strcasecmp(L2cache.policy,"pseudoLRU")==0)
        rep_candidate=pseudoLRU(Cache L2cache,L2dec_index);*/

    return rep_candidate;
}

int L2writeLRU(Cache L2cache[][32],int l2dec_index)
{
    int i,lru=0,lruway=0;

    lru=L2cache[l2dec_index][0].policycounter;
    for(i=1;i<L2assoc;i++)
    {

        if(L2cache[l2dec_index][i].policycounter<lru)
        {
            lru=L2cache[l2dec_index][i].policycounter;
            lruway=i;
        }

    }
    return lruway;
}

