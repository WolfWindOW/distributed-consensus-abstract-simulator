#include "PartitionPeer.hpp"
#include <iostream>

int PartitionPeer::doubleDelay = 0;
int PartitionPeer::NextblockIdNumber = 1;
bool PartitionPeer::PostSplit = false;
bool PartitionPeer::Lying = false;
int PartitionPeer::DropRate = 0;

PartitionPeer::~PartitionPeer() {

}

PartitionPeer::PartitionPeer(std::string id) : Peer(id) {
	counter = 0;
	PartitionBlock Genesis;
	Genesis.blockIdNumber = 0;
	Genesis.trans = 0;
	Genesis.length = 0;
	Genesis.postSplit = false;
	blockChain.push_back(Genesis);
}

void PartitionPeer::incrementMergeWaiting() {
	if (mergeWaiting > 0) {
		mergeWaiting--;
	}
}

void PartitionPeer::preformComputation() {
	//std::cout << "Peer:" << _id << " preforming computation" << std::endl;
	sortTransactions();
	incrementDroppedBlocks();
	linkUnlinkedBlocks(false);
	int foundNew = checkInStrm();
	findDroppedBlocks();
	if (mineBlock()) {
		
		std::set<int> transList = findVerifTrans();
		PartitionBlock newBlock;
		bool makeNew = false;
		for (int i = 0; i < transactions.size(); ++i) {
			if (transList.find(transactions[i].transBlock.trans) == transList.end()) {
				newBlock = transactions[i].transBlock;
				makeNew = true;
				break;
			}
		}
		if (makeNew) {
			newBlock.blockIdNumber = NextblockIdNumber++;
			if (!PostSplit) {
				newBlock.TipIndex = preSplitTip;
				newBlock.tipBlockIdNumbers = blockChain[preSplitTip].blockIdNumber;
				blockChain[preSplitTip].VerifIndex.push_back(blockChain.size());
				newBlock.length = blockChain[preSplitTip].length + 1;
				newBlock.postSplit = false;
				preSplitTip = blockChain.size();
				blockChain.push_back(newBlock);
			}
			else {
				newBlock.TipIndex = postSplitTip;
				newBlock.tipBlockIdNumbers = postSplitBlockChain[postSplitTip].blockIdNumber;
				postSplitBlockChain[postSplitTip].VerifIndex.push_back(postSplitBlockChain.size());
				newBlock.length = postSplitBlockChain[postSplitTip].length + 1;
				newBlock.postSplit = true;
				postSplitTip = postSplitBlockChain.size();
				postSplitBlockChain.push_back(newBlock);
			}
			sendBlock(newBlock);
		}
	}
	incrementMergeWaiting();
	//std::cout << std::endl;
	counter++;
}

void PartitionPeer::intialSplitSetup() {
	for (int j = 0; j < blockChain.size(); ++j) {
		if (blockChain[j].length == blockChain[preSplitTip].length && blockChain[j].blockIdNumber < blockChain[preSplitTip].blockIdNumber) {
			preSplitTip = j;
		}
	}
	postSplitBlockChain.push_back(blockChain[preSplitTip]);
	postSplitBlockChain[0].length = 0;
}

void PartitionPeer::findPostSplitNeighbors(vector<string> idList) {
	for (int i = 0; i < idList.size(); i++) {
		for (auto it = _neighbors.begin(); it != _neighbors.end(); it++) {
			if (idList[i] == it->first) {
				PostSplitNeighbors[idList[i]] = it->second;
			}
		}
	}
}

std::set<int> PartitionPeer::findVerifTrans() {
	std::set<int> transList;
	int Index = preSplitTip;
	while (Index != 0) {
		transList.insert(blockChain[Index].trans);
		Index = blockChain[Index].TipIndex;
	}
	Index = postSplitTip;
	while (Index != 0) {
		transList.insert(postSplitBlockChain[Index].trans);
		Index = postSplitBlockChain[Index].TipIndex;
	}
	
	return transList;
}

std::ostream& PartitionPeer::printTo(std::ostream &out)const {
	Peer<PartitionBlockMessage>::printTo(out);

	out << _id << std::endl;
	out << "counter:" << counter << std::endl;

	return out;
}

std::ostream& operator<< (std::ostream &out, const PartitionPeer &peer) {
	peer.printTo(out);
	return out;
}

bool PartitionPeer::mineBlock() {
	if (Lying || mergeWaiting > 0) {
		return false;
	} else if (rand() % 50 == 0) {
		//(doubleDelay * (_neighbors.size() + 1) / 2)
		return true;
	}
	else {
		return false;
	}

}

bool PartitionPeer::linkUnlinkedBlocks(bool foundLonger) {
	std::deque<PartitionBlock>::iterator it = unlinkedBlocks.begin();
	while (it != unlinkedBlocks.end()) {
		PartitionBlock newBlock = *it;
		// Check to see if the tip exists in your blockchain and where it is
		if (!newBlock.postSplit) {
			for (int j = 0; j < blockChain.size(); ++j) {
				if (newBlock.tipBlockIdNumbers == blockChain[j].blockIdNumber) {
					int TipIndex = j;
					newBlock.TipIndex = TipIndex;
					blockChain[TipIndex].VerifIndex.push_back(blockChain.size());
					blockChain.push_back(newBlock);
					unlinkedBlocks.erase(it);
					if (newBlock.length > blockChain[preSplitTip].length) {
						preSplitTip = blockChain.size() - 1;
						if (PostSplit) {
							postSplitBlockChain.clear();
							postSplitBlockChain.push_back(blockChain[preSplitTip]);
							postSplitBlockChain[0].length = 0;
							postSplitTip = 0;
						}
						foundLonger = true;
					}
					else if (newBlock.length == blockChain[preSplitTip].length && newBlock.blockIdNumber < blockChain[preSplitTip].blockIdNumber && PostSplit) {
						preSplitTip = blockChain.size() - 1;
						postSplitBlockChain.clear();
						postSplitBlockChain.push_back(blockChain[preSplitTip]);
						postSplitBlockChain[0].length = 0;
						postSplitTip = 0;
						foundLonger = true;
					}
					it = unlinkedBlocks.begin();
					break;
				}
			}
		}
		else {
			for (int j = 0; j < postSplitBlockChain.size(); ++j) {
				if (newBlock.tipBlockIdNumbers == postSplitBlockChain[j].blockIdNumber) {
					int TipIndex = j;
					newBlock.TipIndex = TipIndex;
					postSplitBlockChain[TipIndex].VerifIndex.push_back(postSplitBlockChain.size());
					postSplitBlockChain.push_back(newBlock);
					unlinkedBlocks.erase(it);
					if (newBlock.length > postSplitBlockChain[postSplitTip].length) {
						postSplitTip = postSplitBlockChain.size() - 1;
						foundLonger = true;
					}
					it = unlinkedBlocks.begin();
					break;
				}
			}
		}
		if (it != unlinkedBlocks.end()) {
			it++;
		}
	}
	return foundLonger;
}

void PartitionPeer::findDroppedBlocks() {
	for (auto it = unlinkedBlocks.begin(); it != unlinkedBlocks.end(); it++) {
		for (int j = 0; j < droppedBlocks.size(); ++j) {
			if (it->tipBlockIdNumbers == droppedBlocks[j].block.blockIdNumber) {
				if (droppedBlocks[j].delay == -1) {
					droppedBlocks[j].delay = doubleDelay * 3 / 2;
				}
			}
		}
	}
}

void PartitionPeer::incrementDroppedBlocks() {
	for (auto it = droppedBlocks.begin(); it != droppedBlocks.end(); it++) {
		if (it->delay != -1) {
			it->delay--;
		}
	}

	auto it = droppedBlocks.begin();
	while (it != droppedBlocks.end()) {
		if (it->delay == 0) {
			unlinkedBlocks.push_back(it->block);
			it = droppedBlocks.erase(it);
		}
		else {
			it++;
		}
	}
}

bool PartitionPeer::checkInStrm() {
	bool foundNew = false;
	bool foundLonger = false;
	// Go through instream
	for (int i = 0; i < _inStream.size(); i++) {
		// Check if it is a mined block or a transaction
		if (_inStream[i].getMessage().mined) {
			PartitionBlock newBlock = _inStream[i].getMessage().block;
			if (DropRate != 0 && rand() % DropRate == 0) { // check to see if we are losing the block
				DroppedBlock dropped;
				dropped.block = _inStream[i].getMessage().block;
				droppedBlocks.push_back(dropped);
			}
			else {
				// Go through all the tips
				// Check to see if the tip exists in your blockchain and where it is
				bool found = false;
				if (!newBlock.postSplit) {
					for (int j = 0; j < blockChain.size(); ++j) {
						if (newBlock.tipBlockIdNumbers == blockChain[j].blockIdNumber) {
							int TipIndex = j;
							newBlock.TipIndex = TipIndex;
							blockChain[TipIndex].VerifIndex.push_back(blockChain.size());
							blockChain.push_back(newBlock);
							if (newBlock.length > blockChain[preSplitTip].length) {
								preSplitTip = blockChain.size() - 1;
								if (PostSplit) {
									postSplitBlockChain.clear();
									postSplitBlockChain.push_back(blockChain[preSplitTip]);
									postSplitBlockChain[0].length = 0;
									postSplitTip = 0;
								}
								foundLonger = true;
							}
							else if (newBlock.length == blockChain[preSplitTip].length && newBlock.blockIdNumber < blockChain[preSplitTip].blockIdNumber && PostSplit) {
								preSplitTip = blockChain.size() - 1;
								postSplitBlockChain.clear();
								postSplitBlockChain.push_back(blockChain[preSplitTip]);
								postSplitBlockChain[0].length = 0;
								postSplitTip = 0;
								foundLonger = true;
							}
							found = true;
							foundNew = true;
							break;
						}
					}
				}
				else {
					for (int j = 0; j < postSplitBlockChain.size(); ++j) {
						if (newBlock.tipBlockIdNumbers == postSplitBlockChain[j].blockIdNumber) {
							int TipIndex = j;
							newBlock.TipIndex = TipIndex;
							postSplitBlockChain[TipIndex].VerifIndex.push_back(postSplitBlockChain.size());
							postSplitBlockChain.push_back(newBlock);
							if (newBlock.length > postSplitBlockChain[postSplitTip].length) {
								postSplitTip = postSplitBlockChain.size() - 1;
								foundLonger = true;
							}
							found = true;
							foundNew = true;
							break;
						}
					}
				}
				// if not add the block to the unlinked blocks list to be checked when a new block is added to the chain
				if (!found) {
					unlinkedBlocks.push_back(newBlock);
				}
			}
		}
		else {
			Partitiontransaction newTransaction;
			newTransaction.transBlock = _inStream[i].getMessage().block;
			newTransaction.priority = (rand() % 5) + 1;
			transactions.push_back(newTransaction);
		}
	}
	if (foundNew) {
		foundLonger = linkUnlinkedBlocks(foundLonger);
	}
	
	_inStream.clear();
	return foundLonger;
}

void PartitionPeer::sortTransactions() {
	for (int i = 0; i < transactions.size(); ++i) {
		if (transactions[i].priority > 0) {
			--transactions[i].priority;
		}
	}
	std::stable_sort(transactions.begin(), transactions.end(), PartitionTransactionSortFunc);
}

void PartitionPeer::sendBlock(PartitionBlock minedBlock) {
	PartitionBlockMessage message;
	message.block = minedBlock;
	message.mined = true;
	if (!PostSplit) {
		for (auto it = _neighbors.begin(); it != _neighbors.end(); it++)
		{
			Packet<PartitionBlockMessage> newMessage(std::to_string(counter), it->first, _id);
			newMessage.setBody(message);
			_outStream.push_back(newMessage);
		}
	}
	else {
		for (auto it = PostSplitNeighbors.begin(); it != PostSplitNeighbors.end(); it++)
		{
			Packet<PartitionBlockMessage> newMessage(std::to_string(counter), it->first, _id);
			newMessage.setBody(message);
			_outStream.push_back(newMessage);
		}
	}
}

void PartitionPeer::sendTransaction(int tranID) {
	PartitionBlockMessage message;
	message.mined = false;
	message.block.trans = tranID;
	Partitiontransaction newTransaction;
	newTransaction.transBlock = message.block;
	newTransaction.priority = (rand() % 5) + 1;
	transactions.push_back(newTransaction);
	if (!PostSplit) {
		for (auto it = _neighbors.begin(); it != _neighbors.end(); it++)
		{
			Packet<PartitionBlockMessage> newMessage(std::to_string(counter), it->first, _id);
			newMessage.setBody(message);
			_outStream.push_back(newMessage);
		}
	}
	else {
		for (auto it = PostSplitNeighbors.begin(); it != PostSplitNeighbors.end(); it++)
		{
			Packet<PartitionBlockMessage> newMessage(std::to_string(counter), it->first, _id);
			newMessage.setBody(message);
			_outStream.push_back(newMessage);
		}
	}
}