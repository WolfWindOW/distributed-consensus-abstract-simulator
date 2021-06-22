//
//  main.cpp
//  BlockGuard
//
//  Created by Kendric Hood on 3/8/19.
//  Copyright © 2019 Kent State University. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <set>
#include <iostream>
#include <chrono>
#include <random>

#include "./Common/Network.hpp"
#include "./Common/Peer.hpp"
#include "ExamplePeer.hpp"
#include "./Common/ByzantineNetwork.hpp"
// UTIL
#include "./Common/Logger.hpp"

void Example(std::ofstream& logFile);

int main(int argc, const char* argv[]) {
	srand((float)time(NULL));
	if (argc < 3) {
		std::cerr << "Error: need algorithm and output path" << std::endl;
		return 0;
	}


	std::string algorithm = argv[1];
	std::string filePath = argv[2];

	if (algorithm == "example") {
		std::ofstream out;
		std::string file = filePath + "/example.log";
		out.open(file);
		if (out.fail()) {
			std::cerr << "Error: could not open file" << file << std::endl;
		}
		Example(out);
	}
	else {
		std::cout << algorithm << " not recognized" << std::endl;
	}

	return 0;
}

void Example(std::ofstream& logFile) {
	srand (time(NULL));
	int NUMBER_OF_PEERS = 5; 

	ByzantineNetwork<ExampleMessage, ExamplePeer> system;
	system.setLog(logFile); // set the system to write log to file logFile
	system.setToRandom(); // set system to use a uniform random distribution of weights on edges (channel delays)
	system.setMaxDelay(3); // set the max weight an edge can have to 3 (system will now pick randomly between [1, 3])
	system.initNetwork(NUMBER_OF_PEERS); // Initialize the system (create it) with 5 peers given the above settings

	for (int i = 0; i < 3; i++) {
		logFile << "-- STARTING ROUND " << i << " --" << std::endl; // write in the log when the round started

		// each round we select a peer at random to submit a new TX to. 
		// this resultes in a trasaction rate of 1TX per round
		system[rand() % NUMBER_OF_PEERS]->makeRequest();

		system.receive(); // do the receive phase of the round
		//system.log(); // log the system state
		system.preformComputation();  // do the preform computation phase of the round
		//system.log();
		system.transmit(); // do the transmit phase of the round
		//system.log();

		logFile << "-- ENDING ROUND " << i << " --" << std::endl; // log the end of a round
	}

	NUMBER_OF_PEERS = 3;
	system = ByzantineNetwork<ExampleMessage, ExamplePeer>(); // clear old setup by creating a fresh object
	system.setLog(std::cout); // set the system to write log to terminal
	system.setToRandom();
	system.setMaxDelay(10);
	system.initNetwork(NUMBER_OF_PEERS);

	for (int i = 0; i < 3; i++) {
		std::cout << "-- STARTING ROUND " << i << " --" << std::endl; // print outwhen the round started

		// this time we submit a new TX to each peer increasing 
		// the trasaction rate to 3TX per round (sense we have 3 peers)
		for(int i = 0; i < system.size(); i++){
			system[i]->makeRequest();
		}

		system.receive();
		system.log(); // log now goes to the terminal
		system.preformComputation();
		system.log();
		system.transmit();
		system.log();

		std::cout << "-- ENDING ROUND " << i << " --" << std::endl; // print out the end of a round
	}

	// The base peer class tracks number of messages sent by a(1) peer. To calculate the total number of messages that where in the system we need
	//      to add up each peers indivudal message count (example of looping thouth the Network class)

	int numberOfMessages = 0;
	for (int i = 0; i < system.size(); i++) {
		numberOfMessages += system[i]->getMessageCount(); // notice that the index operator ([]) return a pointer to a peer. NEVER DELETE THIS PEER INSTANCE.
														  //    The netwrok class deconstructor will get ride off ALL peer instances once it is deconstructed.
														  //    Use the -> to call method on the peer instance. The Network class will also cast the instance to
														  //    your derived class so all methods that you add will be avalable via the -> operator
	}
	std::cout << "Number of Messages: " << numberOfMessages << std::endl;
}