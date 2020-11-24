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
		FSM(unsigned S_init) { this->current = S_init; }

			Take predict(){
			Take outcome;
			if (current==WNT || current==NT){outcome=NT;}
			else {outcome=T;}
			return outcome;
			}

			bool update(Take outcome){
			unsigned next;
			if (current == SNT && outcome == T){next = WNT;}
			//else if (current == SNT && outcome == NT){next = SNT;}
			else if (current == WNT && outcome == T){next = WT;}
			else if (current == WNT && outcome == NT){next = SNT;}
			else if (current == WT && outcome == T){next = ST;}
			else if (current == WT && outcome == NT){next = WNT;}
			//else if (current == ST && outcome == T){next = ST;}
			else if (current == ST && outcome == NT){next = WT;}
			else {next=current;}
			current=next;
			}	
		}
		
class Entry {
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
		Entry(uint32_t tg, uint32_t target, unsigned historySize, unsigned fsmState, bool isGlobalHist, bool isGlobalTable) {
			tg_ = tg;
			target_ = target;
			historySize_ = historySize;
			fsmState_ = fsmState;
			isGlobalHist_ = isGlobalHist;
			isGlobalTable_ = isGlobalTable;
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
		bool Update_Bhr(bool take1) {
			unsigned tmp = this->bhr_;
			unsigned tmp2 = (tmp << 1) & (1 << ((this->historySize_) - 1) - 1);
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
	}
}


			
		
class BP {

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
	BP();

	//initial the BTB
	void InitBTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
		bool isGlobalHist, bool isGlobalTable, int Shared){
	
		this->btbSize = btbSize;
		this->historySize = historySize;
		this->tagSize = tagSize;
		this->fsmState = fsmState;
		this->isGlobalHist = isGlobalHist;
		this->isGlobalTable = isGlobalTable;
		this->Shared = Shared;
		
		
		//make a Global fsm if neccery
		if (this->isGlobalTable == true)
		{
			vector<FSM> GlobFsm(pow(2, this->historySize), FSM(this->fsmState));
			this->GFsm = GlobFsm;
		}

		//make a Global history if necceryu
		if (this->isGlobalHist == true)
		{
			this->GlobHist = 0;
		}
		uint32_t DeafultTg = 1;
		uint32_t DeafultTarget = 1;
		vector<Entry> EntryTable(this->historySize, Entry(DeafultTg, DeafultTarget, this->historySize,
			this->fsmState, this->isGlobalHist, this->isGlobalTable);
		this->Entries = EntryTable;
		//initial the stas
		this->stats.br_num = 0;
		this->stats.br_num = 0;
		unsigned tarSize = 30;
		//size of one ;one in the BTB
		unsigned TmpSize = (this->btbSize) * ((this->tagSize) + tarSize);
		//add the size of the history
		if (this->isGlobalHist == true) {
			TmpSize += this->historySize;
		}
		else
		{
			TmpSize += (this->GlobHist)*(this->btbSize);

		}
		//add the size of the fsm
		if (this->isGlobalTable == true) {
			TmpSize += 2 * (pow(2, this->historySize));
		}
		else {
			TmpSize += (this->btbSize) * (2 * (pow(2, this->historySize)));
		}
		this->stats.size = TmpSize;
		
		
	}






	bool predict(uint32_t tg, uint32_t target, unsigned historySize, unsigned fsmState,	bool isGlobalHist, bool isGlobalTable) {
		//Insert Entry
			//1) hist l fsm l -> Entry.predict Enetries[tg].getpredict()
		if ((this->isGlobalHist == false) && (this->isGlobalTable == false))
		{
			
			this->
				//2)his G fsm G-> check all in Class BP (share.mid....)
				//3)hist l fsm G-> Entry[].getbhr and then assign in the FSMG
				//4)hist G fsm l->Entry.getFSM() and assign there the Gbhr


		}
	} 

		//clac the tag
		uint32_t Calc_Tg(unsigned tagSize, uint32_t pc);

		// chek in the fsm what should we do
		Take IsTake(uint32_t tg);
		//update the history and FSM
		bool Update_After(uint32_t tg, Take outcome);

		void BP_Get_Stats();


	}







			
				
	
	
	
	
	
	
	
	
	
	
	
	
	
