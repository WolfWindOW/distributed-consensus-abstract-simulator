//
// Created by srai on 3/31/19.
//

#ifndef DS_bitcoinPeer_hpp
#define DS_bitcoinPeer_hpp
#include <utility>
#include <random>
#include <deque>
#include "Peer.hpp"
#include "DAG.hpp"

struct DS_bCoinMessage {
	std::string 							peerId ;
	std::vector<std::string> 				message;
	int                                     length = 0;
	DAGBlock                                dagBlock;
	bool                                    txFlag = false;
	bool                                    dagBlockFlag =false;
	bool									blockByzantineFlag = false;

	DS_bCoinMessage(const DS_bCoinMessage& rhs){
		peerId = rhs.peerId;
		message = rhs.message;
		length = rhs.length;
		txFlag = rhs.txFlag;
		dagBlock = rhs.dagBlock;
		dagBlockFlag = rhs.dagBlockFlag;
		blockByzantineFlag = rhs.blockByzantineFlag;

	}
	DS_bCoinMessage() = default;

	DS_bCoinMessage& operator=(const DS_bCoinMessage& rhs){
		if(this == &rhs)
			return *this;
		peerId = rhs.peerId;
		message = rhs.message;
		length = rhs.length;
		txFlag = rhs.txFlag;
		dagBlock = rhs.dagBlock;
		dagBlockFlag = rhs.dagBlockFlag;
		blockByzantineFlag = rhs.blockByzantineFlag;
		return *this;
	}

	~DS_bCoinMessage() = default;

};


class DS_bCoin_Peer : public Peer<DS_bCoinMessage> {

	int 											counter;
	int                                             mineNextAt;
	std::deque<string>                              consensusQueue = {};
	std::string			                            consensusTx="";
	DS_bCoinMessage                                    messageToSend = {};
	std::map<std::string, Peer<DS_bCoinMessage>* >		committeeNeighbours;
	bool terminated = false;

public:
	static std::default_random_engine               generator;
	static std::binomial_distribution<int>          distribution;
	DAG												dag;
	std::vector<DAGBlock>							dagBlocks={};
	DAGBlock										*minedBlock;
	void											updateDAG();

	explicit DS_bCoin_Peer																	(std::string);
	DS_bCoin_Peer                                                                      		(const DS_bCoin_Peer &rhs);
	void 											setCommitteeNeighbours					(std::map<std::string, Peer<DS_bCoinMessage>* > n) { committeeNeighbours = std::move(n); }
	std::map<std::string, Peer<DS_bCoinMessage>* > 	getCommitteeNeighbours					() { return committeeNeighbours ; }
	void                                    		setMineNextAt                           (int iter) { mineNextAt = iter; }
	int                                     		getMineNextAt                           () { return mineNextAt; }
	void                                     		resetMineNextAt                         () { mineNextAt = DS_bCoin_Peer::distribution(DS_bCoin_Peer::generator); }
	void 											setDAG									(const DAG &dagChain) { this->dag = dagChain; }
	DAG                             				getDAG                           		() { return this->dag; }
	void 											preformComputation						() override;

	bool 											mineBlock                               ();
	void                                    		receiveMsg                              ();
	void                                    		sendBlock								();
	~DS_bCoin_Peer                                                                     		() override { delete minedBlock; }
	void                                    		makeRequest                             () override;
	void                                    		makeRequest                             (const std::vector<DS_bCoin_Peer*>&, const std::string&);
	void 											setTerminated							(bool flag){ this->terminated = flag;}
	void 											clearConsensusTx						();
	std::string 									getConsensusTx							(){ return consensusTx; }
	bool 											isTerminated							(){ return terminated; }
	void 											resetMiningClock						();
	void											deleteMinedBlock						(){ delete minedBlock; minedBlock= nullptr; }
	void											addToBlocks								();

};


#endif
