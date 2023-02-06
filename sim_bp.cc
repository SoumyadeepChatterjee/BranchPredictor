#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include "sim_bp.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/

//Bimodal Helper Functions
unsigned long int calculate_bimodal_index(unsigned long int input_addr_b, unsigned long int num_of_sets_b);
unsigned char get_bimodal_prediction(unsigned long int* bimodal_table, unsigned long int bimodal_index_b);
void update_bimodal_table(unsigned long int* bimodal_table, unsigned long int bimodal_index_bb, char outcome);
//Gshare Helper Functions
unsigned long int calculate_gshare_index(unsigned long int input_addr_b, unsigned long int num_of_sets_g, unsigned long int gBHR, unsigned long int M1_ip, unsigned long int N_ip);
unsigned char get_gshare_prediction(unsigned long int* gshare_table, unsigned long int gshare_index_b);
void update_gshare_table(unsigned long int* gshare_table, unsigned long int gshare_index_bb, char outcome);
//Hybrid Helper Functions
unsigned long int calculate_hybrid_index(unsigned long int input_addr, unsigned long int num_of_index_bits);
//Print
void print_bimodal_contents(unsigned long int num_predictions, unsigned long int num_mispredictions, unsigned long int num_of_sets, unsigned long int* bimodal_table);
void print_gshare_contents(unsigned long int num_predictions, unsigned long int num_mispredictions, unsigned long int num_of_sets, unsigned long int* gshare_table);
void print_hybrid_contents(unsigned long int num_predictions, unsigned long int num_mispredictions, unsigned long int num_of_hybrid_sets, unsigned long int* hybrid_table);

int main(int argc, char* argv[])
{
    FILE* FP;               // File handler
    char* trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc - 1);
        exit(EXIT_FAILURE);
    }

    params.bp_name = argv[1];

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
	//strcmp is comparing 2 strings; 0 means equal, > or < 0 means not equal, non-matching char position is output
	
    if (strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if (argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.M2 = strtoul(argv[2], NULL, 10);
        trace_file = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if (strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if (argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.M1 = strtoul(argv[2], NULL, 10);
        params.N = strtoul(argv[3], NULL, 10);
        trace_file = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if (strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if (argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
            exit(EXIT_FAILURE);
        }
        params.K = strtoul(argv[2], NULL, 10);
        params.M1 = strtoul(argv[3], NULL, 10);
        params.N = strtoul(argv[4], NULL, 10);
        params.M2 = strtoul(argv[5], NULL, 10);
        trace_file = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if (FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    //Need to build prediction table vector
	//Indexing - need to get indexes for each type of table, predictor

    char str[2];
	
	//BIMODAL, M2 only index (3.1.1)
    if (strcmp(params.bp_name, "bimodal") == 0) {
        std::string predictor = "bimodal";
        //M2 is number of index bits = log2(num_of_sets)
		//We need num_of_sets, so 2^M2 gives us that value
        unsigned long int num_of_index_bits = params.M2;
        unsigned long int num_of_sets = pow(2, num_of_index_bits);
        //std::cout << num_of_sets;
		
        //We now build the bimodal table, initialize to weakly taken
        unsigned long int* bimodal_table = NULL;
		bimodal_table = new unsigned long int[num_of_sets];
        for (unsigned long int i = 0; i < num_of_sets; i++) {
            bimodal_table[i] = 2;
        }
		//Prediction result
		char pred_result;
        //Final stats
        //Stats
        unsigned long int num_predictions = 0;
        unsigned long int num_mispredictions = 0;
        
		
        while (fscanf(FP, "%lx %s", &addr, str) != EOF)
        {
            num_predictions++;
            outcome = str[0];
            //STEP 1: Determine branch's index into prediction table
            unsigned long int index = calculate_bimodal_index(addr, num_of_sets);
			//STEP 2: Make a prediction (use index to look at prediction table)
            pred_result = get_bimodal_prediction(bimodal_table, index);
			//STEP 3: Update the branch predictor table
            update_bimodal_table(bimodal_table, index, outcome);
			//STEP 4: Calculate stats
			//  If prediction is wrong, increment misprediction counter
			if (pred_result != outcome) {
				num_mispredictions++;
			}
        }
        //End of simulation, Outputs from Simulator
        print_bimodal_contents(num_predictions,num_mispredictions, num_of_sets, bimodal_table);
    }

    //GSHARE, M1(numPCbits) and N(BHT) used to index gshare table
    if (strcmp(params.bp_name, "gshare") == 0)  {
        //M1 is number of index bits = log2(num_of_sets)
        //We need num_of_sets, so 2^M1 gives us that value
        unsigned long int num_of_sets = pow(2, params.M1);
        //std::cout << num_of_sets;
        unsigned long int * gshare_table = NULL;
        unsigned long int gshare_history_register = 0;
        gshare_table = new unsigned long int[num_of_sets];
        for (unsigned long int i = 0; i < num_of_sets; i++) {
            gshare_table[i] = 2;
        }
        //Final stats
        //Stats
        unsigned long int num_predictions = 0;
        unsigned long int num_mispredictions = 0;
        //Prediction result
        char pred_result;
        while (fscanf(FP, "%lx %s", &addr, str) != EOF) {
            num_predictions++;
            outcome = str[0];
            //STEP 1: Determine a branch's index into the prediction table
            unsigned long int index = calculate_gshare_index(addr, num_of_sets, gshare_history_register, params.M1, params.N);
            //STEP 2: Make a prediction (use index to look at prediction table)
            pred_result = get_gshare_prediction(gshare_table, index);
            //STEP 3: Update the branch predictor table
            update_gshare_table(gshare_table, index, outcome);
            //STEP 4: Update BHR; >>1 and outcome in MSB
            if (outcome == 't') {
                // Set bit N to 1 of BHR register
                gshare_history_register = (gshare_history_register >> 1) | (1 << (params.N - 1));
            }
            else {
                gshare_history_register = (gshare_history_register >> 1);
            }
            //STEP 5: Calculate stats
            //  If prediction is wrong, increment misprediction counter
            if (pred_result != outcome) {
                num_mispredictions++;
            }
        }
        //End of simulation, Outputs from Simulator
        print_gshare_contents(num_predictions, num_mispredictions, num_of_sets, gshare_table);
    }
    
    //HYBRID
    if (strcmp(params.bp_name, "hybrid") == 0) {
        //Final stats
        //Stats
        unsigned long int num_predictions = 0;
        unsigned long int num_mispredictions = 0;
       
        //Build bimodal table, initialize to weakly taken
        unsigned long int* bimodal_table = NULL;
        const unsigned long int num_of_bimodal_sets = pow(2, params.M2);
        bimodal_table = new unsigned long int[num_of_bimodal_sets];
        for (unsigned long int i = 0; i < num_of_bimodal_sets; i++)
            bimodal_table[i] = 2;
     
        //Build gshare table, initialize to weakly taken
        unsigned long int* gshare_table = NULL;
        unsigned int gshare_history_register = 0;
        const unsigned long int num_of_gshare_sets = pow(2, params.M1);
        gshare_table = new unsigned long int[num_of_gshare_sets];
        for (unsigned long int i = 0; i < num_of_gshare_sets; i++)
            gshare_table[i] = 2;
        
        //Build hybrid predictor table, initialize to 1 at the start
        unsigned long int* hybrid_table = NULL;
        const unsigned long int num_of_hybrid_sets = pow(2, params.K);
        hybrid_table = new unsigned long int[num_of_hybrid_sets];
        for (unsigned long int i = 0; i < num_of_hybrid_sets; i++)
            hybrid_table[i] = 1;

        while (fscanf(FP, "%lx %s", &addr, str) != EOF) {
            num_predictions++;
            outcome = str[0];

            //STEP 1: Obtain 2 predictions
            //STEP 1.1: Get branch indices for bimodal and gshare
            unsigned long int gshare_index = calculate_gshare_index(addr, num_of_gshare_sets, gshare_history_register, params.M1, params.N);
            unsigned long int bimodal_index = calculate_bimodal_index(addr, num_of_bimodal_sets);
            //STEP 1.2: Get prediction from each
            unsigned char gshare_pred_result = get_gshare_prediction(gshare_table, gshare_index);
            unsigned char bimodal_pred_result = get_bimodal_prediction(bimodal_table, bimodal_index);

            //STEP 2: Determine current branch's index into hybrid chooser table
            unsigned long int curr_index = calculate_hybrid_index(addr, params.K);

            //STEP 3: Make overall prediction, if hybrid[index].value >=2, use gshare, else bimodal
            unsigned char prediction = ' ';
            bool bimodal_used = false;
            if (hybrid_table[curr_index] >= 2) {
                prediction = 'g';
                bimodal_used = false;
            }
            else if (hybrid_table[curr_index] < 2) {
                prediction = 'b';
                bimodal_used = true;
            }

            //STEP 4: Update used predictor
            if (!(bimodal_used)) {
                update_gshare_table(gshare_table, gshare_index, outcome);
            }
            else if (bimodal_used) {
                update_bimodal_table(bimodal_table, bimodal_index, outcome);
            }
            
            //STEP 5: Update BHR always
            if (outcome == 't') {
                // Set bit N to 1 of BHR register
                gshare_history_register = (gshare_history_register >> 1) | (1 << (params.N - 1));
            }
            else {
                gshare_history_register = (gshare_history_register >> 1);
            }
            
            //STEP 6: Update hybrid chooser counter as per matrix in project spec
            if (outcome == gshare_pred_result) {
                if (outcome != bimodal_pred_result) {
                    hybrid_table[curr_index] = hybrid_table[curr_index] == 3 ? 3 : hybrid_table[curr_index] + 1;
                }
            }
            else if (outcome != gshare_pred_result) {
                if (outcome == bimodal_pred_result) {
                    hybrid_table[curr_index] = hybrid_table[curr_index] == 0 ? 0 : hybrid_table[curr_index] - 1;
                }
            }

            //Calculate stats
            //If prediction is wrong, increment misprediction counter
            if (prediction == 'g') {
                if (outcome != gshare_pred_result) {
                    num_mispredictions++;
                }
            }
            if (prediction == 'b') {
                if (outcome != bimodal_pred_result) {
                    num_mispredictions++;
                }
            }
        }
       
        //End of simulation, Outputs from Simulator
        print_hybrid_contents(num_predictions, num_mispredictions, num_of_hybrid_sets, hybrid_table);
        
        printf("FINAL GSHARE CONTENTS\n");
        for (unsigned int i = 0; i < num_of_gshare_sets; i++){
            std::cout << i << "	" << gshare_table[i] << "\n";
        }
        printf("FINAL BIMODAL CONTENTS\n");
        for (unsigned int i = 0; i < num_of_bimodal_sets; i++){
            std::cout << i << "	" << bimodal_table[i] << "\n";
        }
    }
    return 0;
}

//Function Definitions

unsigned long int calculate_bimodal_index(unsigned long int input_addr_b, unsigned long int num_of_sets_b) {
    //discard lowest 2 bits of addr
    unsigned long int temp_addr = input_addr_b >> 2;
    //get index
    unsigned long int calculated_set = temp_addr & (num_of_sets_b - 1);
    return calculated_set;
}

unsigned char get_bimodal_prediction(unsigned long int* bimodal_table, unsigned long int bimodal_index_b) {
    //Make a prediction(use index to look at prediction table)
    //If value is greater than OR requal to 2 -> branch is predicted taken
    //Else, not taken
    unsigned char bimodal_pred_output;
    if (bimodal_table[bimodal_index_b] > 2 || bimodal_table[bimodal_index_b] == 2) {
        bimodal_pred_output = 't';
    }
    else if (bimodal_table[bimodal_index_b] < 2) {
        bimodal_pred_output = 'n';
    }
    return bimodal_pred_output;
}

void update_bimodal_table(unsigned long int* bimodal_table, unsigned long int bimodal_index_bb, char outcome) {
    //Increment if taken, decrement if not taken; saturates at 0 and 3.
    if (outcome == 't') {
        bimodal_table[bimodal_index_bb] = bimodal_table[bimodal_index_bb] == 3 ? 3 : bimodal_table[bimodal_index_bb] + 1;
    }
    if (outcome == 'n') {
        bimodal_table[bimodal_index_bb] = bimodal_table[bimodal_index_bb] == 0 ? 0 : bimodal_table[bimodal_index_bb] - 1;
    }
}

unsigned long int calculate_gshare_index(unsigned long int input_addr_g, unsigned long int num_of_sets_g, unsigned long int gBHR, unsigned long int M1_ip, unsigned long int N_ip) {
    //discard lowest 2 bits of addr
    unsigned long int temp_addr = input_addr_g >> 2;
    unsigned long int temp_index = temp_addr & (num_of_sets_g - 1);
    unsigned long int calculated_set = temp_index ^ (gBHR << (M1_ip - N_ip));
    return calculated_set;
}

unsigned char get_gshare_prediction(unsigned long int* gshare_table, unsigned long int gshare_index_g) {
    //If value is greater than OR requal to 2 -> branch is predicted taken
    //Else, not taken
    unsigned char gshare_pred_output;
    if (gshare_table[gshare_index_g] > 2 || gshare_table[gshare_index_g] == 2) {
        gshare_pred_output = 't';
    }
    else if (gshare_table[gshare_index_g] < 2) {
        gshare_pred_output = 'n';
    }
    return gshare_pred_output;
}

void update_gshare_table(unsigned long int* gshare_table, unsigned long int gshare_index_bb, char outcome) {
    //Increment if taken, decrement if not taken; saturates at 0 and 3.
    if (outcome == 't') {
        gshare_table[gshare_index_bb] = gshare_table[gshare_index_bb] == 3 ? 3 : gshare_table[gshare_index_bb] + 1;
    }
    if (outcome == 'n') {
        gshare_table[gshare_index_bb] = gshare_table[gshare_index_bb] == 0 ? 0 : gshare_table[gshare_index_bb] - 1;
    }
}

unsigned long int calculate_hybrid_index(unsigned long int input_addr, unsigned long int num_of_index_bits) {
    unsigned long int calculated_set = (input_addr >> 2) & ((1 << num_of_index_bits) - 1);
    return calculated_set;
}

void print_bimodal_contents(unsigned long int num_predictions, unsigned long int num_mispredictions, unsigned long int num_of_sets, unsigned long int* bimodal_table) {
    std::cout << "OUTPUT\n";
    std::cout << " number of predictions:    " << num_predictions << "\n";
    std::cout << " number of mispredictions: " << num_mispredictions << "\n";
    //std::cout << std::setprecision(4) << " misprediction rate:       " << num_mispredictions / num_predictions * 100 << "%" << "\n";
    printf("misprediction rate: %.2f%%\n",(float)num_mispredictions / num_predictions * 100);
    std::cout << "FINAL BIMODAL CONTENTS\n";
    for (unsigned long int i = 0; i < num_of_sets; i++) {
        std::cout << i << "	" << bimodal_table[i] << "\n";
    }
}

void print_gshare_contents(unsigned long int num_predictions, unsigned long int num_mispredictions, unsigned long int num_of_sets, unsigned long int* gshare_table) {
    std::cout << "OUTPUT\n";
    std::cout << " number of predictions:    " << num_predictions << "\n";
    std::cout << " number of mispredictions: " << num_mispredictions << "\n";
    //std::cout << std::setprecision(4) << " misprediction rate:       " << num_mispredictions / num_predictions * 100 << "%" << "\n";
    printf("misprediction rate: %.2f%%\n", (float)num_mispredictions / num_predictions * 100);
    std::cout << "FINAL GSHARE CONTENTS\n";
    for (unsigned long int i = 0; i < num_of_sets; i++) {
        std::cout << i << "	" << gshare_table[i] << "\n";
    }
}

void print_hybrid_contents(unsigned long int num_predictions, unsigned long int num_mispredictions, unsigned long int num_of_hybrid_sets, unsigned long int* hybrid_table) {
    std::cout << "OUTPUT\n";
    std::cout << " number of predictions:    " << num_predictions << "\n";
    std::cout << " number of mispredictions: " << num_mispredictions << "\n";
    //std::cout << std::setprecision(4) << " misprediction rate:       " << (num_mispredictions / num_predictions) * 100 << "%" << "\n";
    printf("misprediction rate: %.2f%%\n", (float)num_mispredictions / num_predictions * 100);
    std::cout << "FINAL CHOOSER CONTENTS\n";
    for (unsigned int i = 0; i < num_of_hybrid_sets; i++) {
        std::cout << i << "	" << hybrid_table[i] << "\n";
    }
}