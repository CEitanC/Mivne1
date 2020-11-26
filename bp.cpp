/* 046267 Computer Architecture - Winter 20/21 - HW #1                  */
/* This file should hold your implementation of the predictor simulator */

#include "bp_api.h"
#include <stdio.h>
#include <vector>
#include <cmath>
using namespace std;
using std::vector;

unsigned SNT=0, WNT=1 , WT=2, ST=3;
int NOT_USING_SHARED = 0, USING_SHARED_LSB = 1,USING_SHARED_MID = 2;


// FSM-finite state machine class
class FSM {
	unsigned current;
	 unsigned SNT = 0, WNT = 1, WT = 2, ST = 3;
public:
	FSM() = default; //default constructor
	// initiate FSM state
	FSM(unsigned S_init) { this->current = S_init; }
	// return predicted state according to machine state
	bool predict() {
		bool outcome;
		if (current == WNT || current == SNT) { outcome = false; }
		else { outcome = true; }
		return outcome;
	}
	// update FSM based on current state and prediction
	void update(bool outcome) {
		unsigned next;
		if ((current == SNT) && (outcome == true)) { next = WNT; }
		else if ((current == WNT) && (outcome == true)) { next = WT; }
		else if ((current == WNT) && (outcome == false)) { next = SNT; }
		else if ((current == WT) && (outcome == true)) { next = ST; }
		else if ((current == WT) && (outcome == false)) { next = WNT; }
		else if ((current == ST) && (outcome == false)) { next = WT; }
		else { next = current; }
		this->current = next;
	}
};

// Entry class		
class Entry {
	vector<FSM> FSM_V;
	uint32_t tg_;
	uint32_t target_;
	unsigned historySize_;
	unsigned fsmState_;
	bool isGlobalHist_;
	bool isGlobalTable_;
	unsigned bhr_;
	bool BitValid;
	
	public:
		// constructor
		Entry() = default; 
		
		Entry(unsigned historySize, unsigned fsmState, bool isGlobalHist, bool isGlobalTable) {
			historySize_ = historySize;
			fsmState_ = fsmState;
			isGlobalHist_ = isGlobalHist;
			isGlobalTable_ = isGlobalTable;
			BitValid = false;
			if (isGlobalHist_ == false) {
				this->bhr_ = 0;
				
			}

			if (isGlobalTable_ == false) {
				int vecSize = pow(2, historySize_);
				vector<FSM> fsm1(vecSize, FSM(this->fsmState_));
				this->FSM_V = fsm1;

			}
			

		}

		bool GetValid() { return this->BitValid; }
	
	// check if this is the same instruct like last time
	bool IsSame(uint32_t tg1, uint32_t target1) {
		if ((this->tg_ == tg1) && (this->target_ == target1))
		{
			
			return true;
			
		}
		return false;
	}
	void SetDst(uint32_t dst) { this->target_ = dst; }
	//initial the entry
	void UpdateEntry(uint32_t tg1, uint32_t target1) {
		this->tg_ = tg1;
		this->target_ = target1;
		this->BitValid = true;
		if (isGlobalHist_ == false) {
			this->bhr_ = 0;
		}
		if (isGlobalTable_ == false) {
			int vecSize = pow(2, this->historySize_);
			vector<FSM> fsm1(vecSize, FSM(this->fsmState_));
			this->FSM_V.clear(); 
			this->FSM_V = fsm1;
				
			return;
		}
	}

	//decide to take or not by the fsm vec
	bool predict(unsigned bhr) {

		return this->FSM_V[bhr].predict();

	}


	// get history
	unsigned get_bhr() {
		return this->bhr_;

	}

	//correct the fsm by the last outcome
	void Update_Fsm(unsigned bhr, bool res) {
		return this->FSM_V[bhr].update(res);
	}


	// get the target
	uint32_t Get_Dest() {
		return this->target_;
	}


	// update local bhr
		void Update_Bhr(bool take1) {
		unsigned tmp = this->bhr_;
		unsigned tmp2 = (tmp << 1) & ((1 << (this->historySize_)) - 1);
		if (take1 == false) {
			this->bhr_ = tmp2;

		}
		else
		{
			unsigned bit = 1;
			unsigned tmp3 = tmp2 | bit;
			this->bhr_ = tmp3;
		
			
		}
		//return true;

		
		}

		uint32_t GetTag() { return this->tg_; }

	//given new entry, check if needed to replace current entry or if they are the same
	void checkEntry(uint32_t tg1, uint32_t target1){
		if (this->BitValid == false) {
			this->UpdateEntry(tg1, target1);
			return;
		}
		if (IsSame( tg1, target1)==false){
			UpdateEntry( tg1, target1);
		}
	}
};



			
		
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
	const int NOT_USING_SHARED = 0, USING_SHARED_LSB = 1, USING_SHARED_MID = 2;
	


public:
	//default constructor 
	BP() = default;
 

	//initial the BTB
	void InitBTB(unsigned btbSize, unsigned historySize, unsigned tagSize,
		unsigned fsmState,bool isGlobalHist, bool isGlobalTable, int Shared){
	
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

		//make a Global history if neccery
		if (this->isGlobalHist == true)
		{
			this->GlobHist = 0;
		}
		
		vector<Entry> EntryTable(this->btbSize, Entry( this->historySize,
			this->fsmState, this->isGlobalHist, this->isGlobalTable));
		this->Entries = EntryTable;
		//initial the stas
		this->stats.br_num = 0;
		this->stats.flush_num = 0;
		unsigned tarSize = 30;
		unsigned ValidBit = 1; 
		//size of one ;one in the BTB
		unsigned TmpSize = (this->btbSize) * ((this->tagSize) + tarSize+ValidBit);
		//add the size of the history
		if (this->isGlobalHist == true) {
			TmpSize += this->historySize;
		}
		else
		{
			TmpSize += (this->historySize)*(this->btbSize);
			 
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




	// predict if there's need to take or not using the history and fsm

	bool predict(uint32_t pc, uint32_t* dst) {
		uint32_t tg = this->Create_Tg(pc);
		int index = this->getIndex(pc);
		if ((this->Entries[index].GetValid()==false))													
		{
			*dst = (pc + 4);
			return false;
		}
		if((this->Entries[index].GetTag()!=tg))									
		{
			*dst = (pc + 4);
			return false;
		}
		
		bool taken;
		unsigned Lbhr = this->Entries[index].get_bhr();
		unsigned Gbhr = this->GlobHist;
		if ((this->Shared != this->NOT_USING_SHARED) && (this->isGlobalTable == true))
		{
			Lbhr = Gbhr = this->createShare(pc);
		}
		//1) hist l fsm l -> Entry.predict Enetries[tg].getpredict()

		if ((this->isGlobalHist == false) && (this->isGlobalTable == false))
			{
			taken = (this->Entries[index].predict(Lbhr));							
			}
			
				//2)his G fsm G-> check all in Class BP (share.mid....)
		if ((this->isGlobalHist == true) && (this->isGlobalTable == true)) {

			taken = (this->GFsm[Gbhr].predict());
			}
			
				//3)hist l fsm G-> Entry[].getbhr and then assign in the FSMG
		if ((this->isGlobalHist == false) && (this->isGlobalTable == true)) {
			taken = (this->GFsm[Lbhr].predict());					
		}													
				//4)hist G fsm l->Entry.getFSM() and assign there the Gbhr
		if((this->isGlobalHist==true)&&(this->isGlobalTable==false))
			{
			taken =  (this->Entries[index].predict(Gbhr));
			}

		if (taken == true)
		{
			*dst = this->Entries[index].Get_Dest();
		}
		else
		{
			*dst = (pc + 4);
		}

		return taken;
	} 

		

		

		//update the fsm and history ater the program decided whether to take or not
		void Update_After(uint32_t tg, bool outcome, uint32_t pc) {
			

			int index = this->getIndex(pc);
			
			unsigned Lbhr = this->Entries[index].get_bhr();
			unsigned Gbhr = this->GlobHist;
			if ((this->Shared != this->NOT_USING_SHARED)&&(this->isGlobalTable==true))
				{
				Lbhr = Gbhr = this->createShare(pc);
				}


			//1) hist l fsm l
			if ((this->isGlobalHist == false) && (this->isGlobalTable == false)) {
				this->Entries[index].Update_Fsm(Lbhr, outcome);
				this->Entries[index].Update_Bhr(outcome);
			}
			//2)his G fsm G
			if ((this->isGlobalHist == true) && (this->isGlobalTable == true)){
				this->GFsm[Gbhr].update(outcome);
				this->Update_Bhr(outcome);
			}
			//3)hist l fsm G
			if ((this->isGlobalHist == false) && (this->isGlobalTable == true)) {
				this->GFsm[Lbhr].update(outcome);
				this->Entries[index].Update_Bhr(outcome);
			}
			//4)hist G fsm l
			if ((this->isGlobalHist == true) && (this->isGlobalTable == false)){
				this->Entries[index].Update_Fsm(Gbhr, outcome);
				this->Update_Bhr(outcome);
			}
		}
		//update the Global history of the BTB
		void Update_Bhr(bool take1) {
			unsigned tmp = this->GlobHist;
			unsigned tmp2 = (tmp << 1) & ((1 << (this->historySize)) - 1);
			if (take1 == false) {
				this->GlobHist = tmp2;

			}
			else
			{

				unsigned tmp3 = tmp2 | 1;
				this->GlobHist = tmp3;


			}
			return ;
		}
		//return the stas
		void BP_Get_Stats(SIM_stats *cur) {
			cur->br_num = this->stats.br_num;
			cur->flush_num = this->stats.flush_num;
			cur->size = this->stats.size;
		}

		//create tag by using the pc                                                          
		uint32_t Create_Tg(uint32_t pc){//remove tagsize btbsize
			int btbNumOfEntries = log2(this->btbSize);
			//get desierd n bits
			return (pc >>(2+btbNumOfEntries))&((1 << this->tagSize) - 1);
		}

		//return entry index for given pc
		int getIndex(uint32_t pc){
		int btbNumOfEntries = log2(this->btbSize);// sizeBTB=4 -> NUM=2
		
		return( (pc >> 2) & ((1 << (btbNumOfEntries))-1)); 	
		}

		// update the stat and the entry after the program decided to take or not
		void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
			this->stats.br_num++;
			//chek if there was a mistke
			if (((pred_dst != targetPc) && (taken == true)) || (((pc + 4) != pred_dst) && (taken == false)))
			{
				this->stats.flush_num++;
			}
			uint32_t tag = this->Create_Tg(pc);
			int index = this->getIndex(pc);
			if (tag == this->Entries[index].GetTag()) { this->Entries[index].SetDst(targetPc);}
			this->Entries[index].checkEntry(tag, targetPc); 
			this->Update_After(tag, taken,pc); 
		}
		
		//create an index to the FSM the is based on the pc and on the history in the situation of "share"
		uint32_t createShare(uint32_t pc){
			unsigned pcBits;  
			if (this->isGlobalTable){
				uint32_t pc1 = pc;
				if (this->Shared == USING_SHARED_LSB){
					 pcBits = (pc1 >> 2) & ((1 << (this->historySize))-1);  ///omit +2
				}
				else if (this->Shared == USING_SHARED_MID){
					 pcBits = (pc1 >> 16) & ((1 << (this->historySize))-1); //omit +16
				}
				int indx = getIndex(pc);
				unsigned HistId;
				if(isGlobalHist==true)
				{
					HistId = this->GlobHist;
				}
				else
				{
					HistId = this->Entries[indx].get_bhr();
				}
				
				return (pcBits^(HistId));//xor
			}
			return 0;//if something went wrong
		}
	};



	// 'global' BP
	BP bp; 

	int BP_init(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,
		bool isGlobalHist, bool isGlobalTable, int Shared) {
		if((Shared>2)||(Shared<0))
		return -1;
		else
		{
			bp.InitBTB(btbSize, historySize, tagSize, fsmState, isGlobalHist, isGlobalTable, Shared);
			return 0; 
		}
	}

	bool BP_predict(uint32_t pc, uint32_t* dst) {

		return bp.predict(pc,dst);
	}

	void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
		bp.BP_update(pc, targetPc, taken, pred_dst);
		return;
	}
	
	void BP_GetStats(SIM_stats* curStats) {
		bp.BP_Get_Stats(curStats);
		return;
	}

			
				
	
	
	
	
	
	
	
	
	
	
	
	
	
