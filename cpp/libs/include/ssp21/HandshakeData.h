
#ifndef SSP21_HANDSHAKE_DATA_H
#define SSP21_HANDSHAKE_DATA_H

#include "ssp21/HandshakeAlgorithms.h"

namespace ssp21
{		
	
	/**
	* Class that holds all of the data during the handshake process
	*/
	class HandshakeData : private openpal::Uncopyable
	{
		public:	

		HandshakeData(
			const KeyPair& local_static_keys, 
			const HandshakeAlgorithms& algorithms
		);

		// generates new ephemeral keys and resets all state
		void initialize();

		// wipes ephemeral private key data
		void clear();

		// calculate a new handshake hash: h = hash(input)
		void set_hash(const openpal::RSlice& input);

		// mix the data into the handshake_hash: h = hash(h || input)
		void mix_hash(const openpal::RSlice& input);
	
		private:

		// running hash value
		HashOutput handshake_hash_;

		// ephemeral keys
		KeyPair local_ephemeral_keys_;
		PublicKey remote_ephemeral_key_;		

		// this only gets cleared on destruction
		KeyPair local_static_keys_;
		HandshakeAlgorithms algorithms_;
	};


	
}

#endif