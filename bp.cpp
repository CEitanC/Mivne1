/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#inclue <stdio.h>
#include <vector>
#include <cmath>
using namespace std;
using std::vector;

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
	uint32_t tg_;
	uint32_t target_;
	unsigned historySize_;
	unsigned fsmState_;
	bool isGlobalHist_;
	bool isGlobalTable_;
	unsigned bhr_;
	public:
			// constructor
		Entry(uint32_t tg, uint32_t target, unsigned historySize, unsigned fsmState,
			bool isGlobalHist, bool isGlobalTable) {
			tg_ = tg;
			target_ = target;
			historySize_ = historySize;
			fsmState_ = fsmState;
			isGlobalHist_ = isGlobalHist;
			isGlobalTable_=isGlobalTable;
			if (isGlobalHist_ == 0) {
				this->bhr_ = 0;
			}

			if (isGlobalTable_ == 0) {
				unsigned vecSize = pow(2, historySize_);
				vector<FSM> fsm1(vecSize, FSM(this->fsmState_));
				this->FSM_V = fsm1;
				}
				

		};
			
			// check if this is the same instruct like last time
		bool IsSame(uint32_t tg1, uint32_t target1) {
			if ((this->tg_ == tg1) && (this->target_ == target1))
			{
				return true;
			}
			return false;
		}
			
			//initial the entry in case of collition
		bool UpdateEntry(uint32_t tg1, uint32_t target1) {
			this->tg_ = tg1;
			this->target_ = target1;
			if (isGlobalHist_ == 0) {
				this->bhr_ = 0;
			}
			if (isGlobalTable_ == 0) {
				for (int i = 0; i < this->FSM_V.size(); i++) {
					this->FSM_V[i] = this->fsmState_;
				}
				return true;
			}

			
			//decide to take or not by the fsm vec
		Take predict(unsigned bhr) {
			return this->FSM_V[bhr].predict();
			}

			
			// get history
		unsigned get_bhr() {
			return this->bhr_;
		}
			
			//correct the fsm by the last outcome
		bool Update_Fsm(unsigned bhr, take res) {
			return this->FSM_V[bhr].update(res);
		}

			
			// get the target
		uint32_t Get_Dest() {
			return this->target_;
		}

			
			// 
			bool Update_Bhr(bool take1){
				unsigned tmp = this->bhr_;
				unsigned tmp2 = (tmp << 1) & (1 << ((this->historySize_) - 1)-1);
				if (take1 == false) {
					this->bhr_ = tmp2;
				}
				else
				{
					unsigned tmp3 = tmp2 | 1;
					this->bhr_ = tmp3;
				}
				return true;
			
		}


			
		
	class BP{
		
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
		
			InitBTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
				bool isGlobalHist, bool isGlobalTable, int Shared) {
				this->

			}
			



				bool Insert_En(uint32_t tg, uint32_t target, unsigned historySize, unsigned fsmState,
					bool isGlobalHist, bool isGlobalTable);

				//clac the tag
				uint32_t Calc_Tg(unsigned tagSize, uint32_t pc);

				// chek in the fsm what should we do
				Take IsTake(uint32_t tg);
				//update the history and FSM
				bool Update_After(uint32_t tg, Take outcome);

				void BP_Get_Stats();


			}
		
			
		
			
			
	
			
			
				
	
	
	
	
	
	
	
	
	
	
	
	
	
