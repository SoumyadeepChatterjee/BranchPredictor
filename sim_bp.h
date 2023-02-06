#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params{
    unsigned long int K; //num of PC bits to index chooser table (hybrid)
    unsigned long int M1;//num of PC bits (gshare)
	unsigned long int M2;//num of PC bits (bimodal)
    unsigned long int N;//num of BHT reg bits (gshare)
	char* bp_name;//cline of bimodal, gshare or hybrid
}bp_params;

// Put additional data structures here as per your requirement




#endif
