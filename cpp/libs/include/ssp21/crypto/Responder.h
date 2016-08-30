
#ifndef SSP21_RESPONDER_H
#define SSP21_RESPONDER_H

#include "ssp21/msg/RequestHandshakeBegin.h"
#include "ssp21/msg/RequestHandshakeAuth.h"
#include "ssp21/msg/UnconfirmedSessionData.h"

#include "ssp21/gen/Function.h"
#include "ssp21/gen/HandshakeError.h"

#include "ssp21/link/LinkConstants.h"
#include "ssp21/LayerInterfaces.h"
#include "ssp21/crypto/Handshake.h"

#include "openpal/logging/Logger.h"
#include "openpal/container/Buffer.h"
#include "openpal/executor/IExecutor.h"

#include <memory>


namespace ssp21
{
    /**
    	WIP - this class will implement the stateful part of the responder.
    */
    class Responder final : public IUpperLayer, private IMessageProcessor
    {
		enum HandshakeState 
		{
			wait_for_handshake_begin,
			wait_for_request_auth
		};


    public:

        struct Config
        {
            /// The maximum message size that this layer should transmit to the link layer
            /// This constant determines the size of a buffer allocated when the responder
            /// is constructed
            uint16_t max_tx_message_size = consts::max_config_link_payload_size;
        };

        Responder(	const Config& config, 
					std::unique_ptr<KeyPair> local_static_key_pair,
					std::unique_ptr<PublicKey> remote_static_public_key,
					openpal::IExecutor& executor,
					openpal::Logger logger, 
					ILowerLayer& lower
		);

    private:

        // ---- implement IUpperLayer -----

        virtual void on_open_impl() override;
        virtual void on_close_impl() override;
        virtual void on_tx_ready_impl() override;
        virtual void on_rx_ready_impl() override;

        // ---- implement IMessageProcessor -----

        virtual void process(const Message& message) override;

        template <class MsgType>
        inline void read_any(const openpal::RSlice& data);

        void on_message(const openpal::RSlice& data, const RequestHandshakeBegin& msg);
        void on_message(const openpal::RSlice& data, const UnconfirmedSessionData& msg);
        void on_message(const openpal::RSlice& data, const RequestHandshakeAuth& msg);

        template <class MsgType>
        void handle_parse_error(ParseError err);

        void reply_with_handshake_error(HandshakeError err);

		HandshakeError validate_handshake_begin(const RequestHandshakeBegin& msg);		

        Config config_;
		
		std::unique_ptr<KeyPair> local_static_key_pair_;
		std::unique_ptr<PublicKey> remote_static_public_key_;

        openpal::IExecutor* const executor_;
        openpal::Logger logger_;
        ILowerLayer* const lower_;
        openpal::Buffer tx_buffer_;
		
		Handshake handshake_;
		HandshakeState handshake_state_;

    };

    template <>
    inline void Responder::handle_parse_error<RequestHandshakeBegin>(ParseError err)
    {
        this->reply_with_handshake_error(HandshakeError::bad_message_format);
    }

    template <>
    inline void Responder::handle_parse_error<RequestHandshakeAuth>(ParseError err)
    {
        this->reply_with_handshake_error(HandshakeError::bad_message_format);
    }

    template <>
    inline void Responder::handle_parse_error<UnconfirmedSessionData>(ParseError err) {}
}

#endif
