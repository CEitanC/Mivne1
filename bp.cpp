/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#inclue <stdio.h>
unsigned SNT=0, WNT=1 , WT=2, ST=3;
enum Take{NT=0,T=1};

int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared){
	return -1;
}

bool BP_predict(uint32_t pc, uint32_t *dst){
	return false;
}

void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst){
	return;
}

void BP_GetStats(SIM_stats *curStats){
	return;
}

class FSM{
	unsigned current;
	public:
			FSM(unsigned S_init);
			Take predict();
			bool update(Take outcome);
			
		}
		
class Entry{
	vector<FSm> FSM_V;
	uint32_t tg;
	uint32_t target;
	unsigned historySize;
	unsigned fsmState;
	bool isGlobalHist;
	bool isGlobalTable;
	public:
			// constructor
			Entry(uint32_t tg, uint32_t target,unsigned historySize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable);
			
			// check if this is the same instruct like last time
			bool IsSame(uint32_t tg1, uint32_t target1);
			
			//initial the entry in case of collition
			bool UpdateEntry(uint32_t tg, uint32_t target);
			
			//decide to take or not by the fsm vec
			Take predict(unsigned bhr);
			
			// get history
			unsigned get_bhr();
			
			//correct the fsm by the last outcome
			bool Update_Fsm(unsigned bhr, take res);
			
			// decide the target
			uint32_t Get_Dest();
			
			// 
			bool Update_Bhr(bool take);
			
		}	
		
	Class BP{
		
		unsigned btbSize;
		unsigned historySize;
		unsigned tagSize;
		unsigned fsmState;
		bool isGlobalHist;
		bool isGlobalTable;
		int Shared;
		vector<Entry> Entries;
		unsigned GlobHist;
		vector<FSM> GFsm;
		SIM_stats stats;

		
		
		
		public:
		
		BP(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable, int Shared);
			
		bool Insert_En(uint32_t tg, uint32_t target,unsigned historySize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable);
			
			//clac the tag
		uint32_t Calc_Tg(unsigned tagSize, uint32_t pc);
		
		// chek in the fsm what should we do
		Take IsTake(uint32_t tg);
		//update the history and FSM
		bool Update_After(uint32_t tg, Take outcome);
		
		void BP_Get_Stats();
		
		
		
		
			
		
			
			
	
			
			
				
	
	
	
	
	
	
	
	
	
	
	
	
	
