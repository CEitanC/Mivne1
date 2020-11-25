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



class FSM {
	unsigned current;
	 unsigned SNT = 0, WNT = 1, WT = 2, ST = 3;
public:
	FSM() = default; 
	//~FSM() = default;
	//FSM(FSM&) = default;
	//FSM& operator=(const FSM&) = default;

	FSM(unsigned S_init) { this->current = S_init; }

	bool predict() {
		bool outcome;
		if (current == WNT || current == SNT) { outcome = false; }
		else { outcome = true; }
		return outcome;
	}

	void update(bool outcome) {
		unsigned next;
		if (current == SNT && outcome == true) { next = WNT; }
		//else if (current == SNT && outcome == NT){next = SNT;}
		else if (current == WNT && outcome == true) { next = WT; }
		else if (current == WNT && outcome == false) { next = SNT; }
		else if (current == WT && outcome == true) { next = ST; }
		else if (current == WT && outcome == false) { next = WNT; }
		//else if (current == ST && outcome == T){next = ST;}
		else if (current == ST && outcome == false) { next = WT; }
		else { next = current; }
		current = next;

	}
};
		
class Entry {
	vector<FSM> FSM_V;
	uint32_t tg_;
	uint32_t target_;
	unsigned historySize_;
	unsigned fsmState_;
	bool isGlobalHist_;
	bool isGlobalTable_;
	unsigned bhr_;
	
	public:
		// constructor
		Entry() = default; 
		//Entry(Entry&) = default; 
		//Entry& operator=(Entry&) = default;
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
				int vecSize = pow(2, historySize_);
				vector<FSM> fsm1(vecSize, FSM(this->fsmState_));
				this->FSM_V = fsm1;

			}
			

		}
	
	// check if this is the same instruct like last time
	bool IsSame(uint32_t tg1, uint32_t target1) {
		if ((this->tg_ == tg1) && (this->target_ == target1))
		{
			
			return true;
			
		}
		return false;
	}

	//initial the entry in case of collision
	void UpdateEntry(uint32_t tg1, uint32_t target1) {
		this->tg_ = tg1;
		this->target_ = target1;
		if (isGlobalHist_ == false) {
			this->bhr_ = 0;
		}
		if (isGlobalTable_ == false) {
			int vecSize = pow(2, this->historySize_);
			vector<FSM> fsm1(vecSize, FSM(this->fsmState_));
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


	// update bhr
		bool Update_Bhr(bool take1) {
		unsigned tmp = this->bhr_;
		unsigned tmp2 = (tmp << 1) & ((1 << ((this->historySize_) - 1)) - 1);
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

		uint32_t GetTag() { return this->tg_; }

	//given new entry, check if needed to replace current entry or if they are the same
	void checkEntry(uint32_t tg1, uint32_t target1){
		if (IsSame( tg1, target1)){
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
	BP() = default;
//	~BP() = default; 

	//initial the BTB
	void InitBTB(unsigned btbSize, unsigned historySize, unsigned tagSize, unsigned fsmState,bool isGlobalHist, bool isGlobalTable, int Shared){
	
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
		uint32_t DeafultTg = 1;
		uint32_t DeafultTarget = 1;
		vector<Entry> EntryTable(this->historySize, Entry(DeafultTg, DeafultTarget, this->historySize, this->fsmState, this->isGlobalHist, this->isGlobalTable));
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






	bool predict(uint32_t pc, uint32_t *dst) {
		uint32_t tg = this->Create_Tg(pc);
		int index = this->getIndex(tg);
		if((this->Entries[index].GetTag()!=tg)||(this->Entries[index].Get_Dest()==1))
		{
			*dst = (pc + 4);
			return false;
		}
																			//use func that get index to the Ghist by the pc too
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
			*dst = pc + 4;
		}

		return taken;
	} 

		

		

		//update the fsm and history 
		void Update_After(uint32_t tg, bool outcome, uint32_t pc) {
			

			int index = this->getIndex(tg);
			
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

		void Update_Bhr(bool take1) {
			unsigned tmp = this->GlobHist;
			unsigned tmp2 = (tmp << 1) & ((1 << ((this->historySize) - 1)) - 1);
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

		void BP_Get_Stats(SIM_stats *cur) {
			cur->br_num = this->stats.br_num;
			cur->flush_num = this->stats.flush_num;
			cur->size = this->stats.size;
		}

		//create tag
		uint32_t Create_Tg(uint32_t pc){//remove tagsize btbsize
			int btbNumOfEntries = log2(this->btbSize);
			//get desierd n bits
			int lsbBit = 2 + btbNumOfEntries - 1;
			int msbBit = lsbBit + this->tagSize;
			if (lsbBit > msbBit ) { return 1; } //check lsb is smaller then msb
			return ((pc >> lsbBit) & ((1 << msbBit)-1));
		}

		//return entry index for given tag
		int getIndex(uint32_t tag){
		int btbNumOfEntries = log2(this->btbSize);
			int entryIndex = (tag >> 0) & ((1 << btbNumOfEntries)-1);
			return entryIndex;
		}


		void BP_update(uint32_t pc, uint32_t targetPc, bool taken, uint32_t pred_dst) {
			this->stats.br_num++;
			//chek if there was a mistke
			if (((pred_dst != targetPc) && (taken == true)) || (((pc + 4) != pred_dst) && (taken == false)))
			{
				this->stats.flush_num++;
			}
			uint32_t tag = this->Create_Tg(pc);
			int index = this->getIndex(tag);
			this->Entries[index].checkEntry(tag, pred_dst); 
			this->Update_After(tag, taken,pc); 
		}
		
		//create l/g share
		uint32_t createShare(uint32_t pc){
			uint32_t pcBits;
			if (this->isGlobalTable){
				if (this->Shared == USING_SHARED_LSB){
					 pcBits = (pc >> 2) & ((1 << (2 + this->historySize))-1);
				}
				else if (this->Shared == USING_SHARED_MID){
					 pcBits = (pc >> 16) & ((1 << (16 + this->historySize))-1);
				}
				int indx = getIndex(Create_Tg(pc));
				return (pcBits^(this->Entries[indx].get_bhr()));//xor
			}
			return 0;//if somthing went wrong
		}
	};




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

			
				
	
	
	
	
	
	
	
	
	
	
	
	
	
