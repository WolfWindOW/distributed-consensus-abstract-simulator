#ifndef PartitionPeer_hpp
#define PartitionPeer_hpp

#include "Common/Peer.hpp"
#include <string>
#include <vector>
#include <set>
#include <deque>
#include <random>
#include <algorithm>
using std::vector;
using std::deque;
using std::string;

struct PartitionBlock {
	int					blockIdNumber;
	int					trans;
	int					tipBlockIdNumbers;
	int					TipIndex;
	vector<int>			VerifIndex;
	int					length;
	bool				postSplit;
};

struct PartitionBlockMessage {
	PartitionBlock		block;
	bool				mined;
};

struct Partitiontransaction {
	PartitionBlock		transBlock;
	int					priority;
};

class PartitionPeer : public Peer<PartitionBlockMessage> {
protected:
	int counter;

public:
	PartitionPeer(std::string);
	PartitionPeer(const PartitionPeer &rhs);
	~PartitionPeer();
	vector<PartitionBlock> blockChain;
	vector<PartitionBlock> postSplitBlockChain;
	deque<PartitionBlock>  unlinkedBlocks;
	deque<Partitiontransaction>	  transactions;
	int					  preSplitTip = 0;
	int					  postSplitTip = 0;
	static int  		  doubleDelay;
	static int  		  NextblockIdNumber;
	static bool			  PostSplit;
	static bool PartitionTransactionSortFunc(Partitiontransaction i, Partitiontransaction j) {
		return (i.priority < j.priority);
	}
	void                  preformComputation();
	void				  intialSplitSetup();
	std::set<int>		  findVerifTrans();
	void                  linkUnlinkedBlocks();
	bool 				  mineBlock();
	bool                  checkInStrm();
	void				  sortTransactions();
	void                  sendBlock(PartitionBlock minedBlock);
	void                  sendTransaction(int tranID);
	void                  makeRequest() {};
	void                  log()const { printTo(*_log); };
	std::ostream&         printTo(std::ostream&)const;
	friend std::ostream& operator<<         (std::ostream&, const PartitionPeer&);
};
#endif PartitionPeer_hpp
#pragma once