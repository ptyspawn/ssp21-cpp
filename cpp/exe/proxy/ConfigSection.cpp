
#include "ConfigSection.h"

#include "ConfigProperties.h"

#include "ssp21/crypto/ICertificateHandler.h"

#include "ssp21/util/SecureFile.h"
#include "ssp21/util/Exception.h"
#include "ssp21/stack/LogLevels.h"
#include "ssp21/stack/Factory.h"
#include "ssp21/crypto/gen/ContainerFile.h"
#include "ssp21/link/Addresses.h"

#include "ssp21/util/Exception.h"
#include "ssp21/stack/LogLevels.h"
#include "ssp21/crypto/gen/ContainerFile.h"

#include "qix/QIXKeyCache.h"

#include <iostream>

using namespace ssp21;

void ConfigSection::add(const std::string& propertyId, const std::string& value)
{
    this->values[propertyId] = value;
}

std::unique_ptr<ProxyConfig> ConfigSection::get_config(const log4cpp::Logger& logger, const std::string& id)
{
    const auto proto_type = this->get_proto_type();

    std::unique_ptr<ProxyConfig> ret = nullptr;
    if(proto_type == ProxyConfig::ProtoType::tcp)
    {
        ret = get_tcp_config(logger, id);
    }
    else
    {
        ret = get_udp_config(logger, id);
    }

    for (auto& value : this->values)
    {
        std::cerr << "warning: " << "unused value: " << value.first << "=" << value.second << std::endl;
    }

    return std::move(ret);
}

std::unique_ptr<TcpProxyConfig> ConfigSection::get_tcp_config(const log4cpp::Logger& logger, const std::string& id)
{
    const auto endpoint_mode = this->get_mode();
    const log4cpp::LogLevels levels(this->get_levels());

    return std::make_unique<TcpProxyConfig>(
        this->get_stack_factory(logger, endpoint_mode),
        id,
        levels,
        endpoint_mode,
        this->get_integer_value<uint16_t>(props::max_sessions),
        this->get_integer_value<uint16_t>(props::listen_port),
        this->consume_value(props::listen_endpoint),
        this->get_integer_value<uint16_t>(props::connect_port),
        this->consume_value(props::connect_endpoint)
    );
}

std::unique_ptr<UdpProxyConfig> ConfigSection::get_udp_config(const log4cpp::Logger& logger, const std::string& id)
{
    const auto endpoint_mode = this->get_mode();
    const auto crypto_only = this->get_boolean_value(props::crypto_only);
    const log4cpp::LogLevels levels(this->get_levels());

    return std::make_unique<UdpProxyConfig>(
        this->get_stack_factory(logger, endpoint_mode, crypto_only),
        id,
        levels,
        endpoint_mode,
        Endpoint(
            this->consume_value(props::raw_tx_ip), 
            this->get_integer_value<uint16_t>(props::raw_tx_port)
        ),
        Endpoint(
            this->consume_value(props::raw_rx_ip),
            this->get_integer_value<uint16_t>(props::raw_rx_port)
        ),
        Endpoint(
            this->consume_value(props::secure_tx_ip),
            this->get_integer_value<uint16_t>(props::secure_tx_port)
        ),
        Endpoint(
            this->consume_value(props::secure_rx_ip),
            this->get_integer_value<uint16_t>(props::secure_rx_port)
        )
    );
}

stack_factory_t ConfigSection::get_stack_factory(const log4cpp::Logger& logger, ProxyConfig::EndpointMode ep_mode, bool crypto_only)
{
    if (!crypto_only)
    {
        const Addresses addresses(this->get_addresses());
        return (ep_mode == ProxyConfig::EndpointMode::initiator) ? this->get_initiator_factory(logger, &addresses) : this->get_responder_factory(logger, &addresses);
    }
    else
    {
        return (ep_mode == ProxyConfig::EndpointMode::initiator) ? this->get_initiator_factory(logger, nullptr) : this->get_responder_factory(logger, nullptr);
    }
}

stack_factory_t ConfigSection::get_initiator_factory(const log4cpp::Logger& logger, const ssp21::Addresses* addresses)
{
    const auto mode = this->get_handshake_mode();
    switch (mode)
    {
    case(HandshakeMode::shared_secret):
        return this->get_initiator_shared_secret_factory(addresses);
    case(HandshakeMode::quantum_key_distribution):
        return this->get_initiator_qkd_factory(logger, addresses);
    case(HandshakeMode::preshared_public_keys):
        return this->get_initiator_preshared_public_key_factory(addresses);
    case(HandshakeMode::industrial_certificates):
        return this->get_initiator_certificate_mode_factory(addresses);
    default:
        throw Exception("undefined initiator handshake mode: ", HandshakeModeSpec::to_string(mode));
    }
}

stack_factory_t ConfigSection::get_initiator_shared_secret_factory(const ssp21::Addresses* addresses)
{
    const auto shared_secret = this->get_shared_secret();

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::shared_secret_mode(
                    addresses_copy,
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    suite,
                    shared_secret
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::shared_secret_mode(
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    suite,
                    shared_secret
                );
        };
    }
}

stack_factory_t ConfigSection::get_initiator_qkd_factory(const log4cpp::Logger& logger, const ssp21::Addresses* addresses)
{
    const auto key_cache = this->get_qix_key_cache(logger);

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::none;

            return initiator::factory::qkd_mode(
                    addresses_copy,
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    suite,
                    key_cache
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::qkd_mode(
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    suite,
                    key_cache
                );
        };
    }
}

stack_factory_t ConfigSection::get_initiator_preshared_public_key_factory(const ssp21::Addresses* addresses)
{
    const auto local_keys = this->get_local_static_keys();
    const auto remote_public_key = this->get_crypto_key<ssp21::PublicKey>(props::remote_public_key_path, ContainerEntryType::x25519_public_key);

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::preshared_public_key_mode(
                    addresses_copy,
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    CryptoSuite(),		// TODO: default
                    local_keys,
                    remote_public_key
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::preshared_public_key_mode(
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    CryptoSuite(),		// TODO: default
                    local_keys,
                    remote_public_key
                );
        };
    }
}

stack_factory_t ConfigSection::get_initiator_certificate_mode_factory(const ssp21::Addresses* addresses)
{
    const auto local_keys = this->get_local_static_keys();
    const auto anchor_cert_data = this->get_file_data(props::authority_cert_path);
    const auto local_cert_data = this->get_file_data(props::local_cert_path);

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::certificate_public_key_mode(
                    addresses_copy,
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    CryptoSuite(),		// TODO: default
                    local_keys,
                    anchor_cert_data,
                    local_cert_data
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return initiator::factory::certificate_public_key_mode(
                    InitiatorConfig(),	// TODO: default
                    logger,
                    executor,
                    CryptoSuite(),		// TODO: default
                    local_keys,
                    anchor_cert_data,
                    local_cert_data
                );
        };
    }
}

stack_factory_t ConfigSection::get_responder_factory(const log4cpp::Logger& logger, const ssp21::Addresses* addresses)
{
    const auto mode = this->get_handshake_mode();
    switch (mode)
    {
    case(HandshakeMode::shared_secret):
        return this->get_responder_shared_secret_factory(addresses);
    case(HandshakeMode::quantum_key_distribution):
        return this->get_responder_qkd_factory(logger, addresses);
    case(HandshakeMode::preshared_public_keys):
        return this->get_responder_preshared_public_key_factory(addresses);
    case(HandshakeMode::industrial_certificates):
        return this->get_responder_certificate_mode_factory(addresses);
    default:
        throw Exception("undefined responder handshake mode: ", HandshakeModeSpec::to_string(mode));
    }
}

stack_factory_t ConfigSection::get_responder_shared_secret_factory(const ssp21::Addresses* addresses)
{
    const auto shared_secret = this->get_shared_secret();

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::shared_secret_mode(
                    addresses_copy,
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    shared_secret
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::shared_secret_mode(
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    shared_secret
                );
        };
    }
}

stack_factory_t ConfigSection::get_responder_qkd_factory(const log4cpp::Logger& logger, const ssp21::Addresses* addresses)
{
    const auto key_cache = this->get_qix_key_cache(logger);

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::qkd_mode(
                    addresses_copy,
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    key_cache
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::qkd_mode(
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    key_cache
                );
        };
    }
}

stack_factory_t ConfigSection::get_responder_preshared_public_key_factory(const ssp21::Addresses* addresses)
{
    const auto local_keys = this->get_local_static_keys();
    const auto remote_public_key = this->get_crypto_key<ssp21::PublicKey>(props::remote_public_key_path, ContainerEntryType::x25519_public_key);

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::preshared_public_key_mode(
                    addresses_copy,
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    local_keys,
                    remote_public_key
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::preshared_public_key_mode(
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    local_keys,
                    remote_public_key
                );
        };
    }
}

stack_factory_t ConfigSection::get_responder_certificate_mode_factory(const ssp21::Addresses* addresses)
{
    const auto local_keys = this->get_local_static_keys();
    const auto anchor_cert_data = this->get_file_data(props::authority_cert_path);
    const auto local_cert_data = this->get_file_data(props::local_cert_path);

    if(addresses)
    {
        const auto addresses_copy = *addresses;
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::certificate_public_key_mode(
                    addresses_copy,
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    local_keys,
                    anchor_cert_data,
                    local_cert_data
            );
        };
    }
    else
    {
        return [ = ](const log4cpp::Logger & logger, const std::shared_ptr<exe4cpp::IExecutor>& executor) {
            CryptoSuite suite;
            suite.handshake_ephemeral = HandshakeEphemeral::nonce;

            return responder::factory::certificate_public_key_mode(
                    ResponderConfig(),	// TODO: default
                    logger,
                    executor,
                    local_keys,
                    anchor_cert_data,
                    local_cert_data
                );
        };
    }
}

Addresses ConfigSection::get_addresses()
{
    return Addresses(
               this->get_integer_value<uint16_t>(props::remote_address),
               this->get_integer_value<uint16_t>(props::local_address)
           );
}

ssp21::StaticKeys ConfigSection::get_local_static_keys()
{
    return ssp21::StaticKeys(
               this->get_crypto_key<PublicKey>(props::local_public_key_path, ContainerEntryType::x25519_public_key),
               this->get_crypto_key<PrivateKey>(props::local_private_key_path, ContainerEntryType::x25519_private_key)
           );
}

std::shared_ptr<const SymmetricKey> ConfigSection::get_shared_secret()
{
    return this->get_crypto_key<SymmetricKey>(props::shared_secret_key_path, ContainerEntryType::shared_secret);
}

std::shared_ptr<QIXKeyCache> ConfigSection::get_qix_key_cache(const log4cpp::Logger& logger)
{
    const auto port = this->consume_value(props::serial_port);

    return std::make_shared<QIXKeyCache>(
               port,
               logger.detach_and_append("-qix"),
               100 // TODO - make this configurable?
           );
}

log4cpp::LogLevels ConfigSection::get_levels()
{
    log4cpp::LogLevels levels;
    for (auto flag : this->consume_value(props::log_levels))
    {
        levels |= this->get_levels_for_char(flag);
    }
    return levels;
}

std::string ConfigSection::consume_value(const std::string& propertyId)
{
    const auto iter = this->values.find(propertyId);
    if (iter == values.end())
    {
        throw Exception("Required property not found: ", propertyId);
    }
    const auto ret = iter->second;
    this->values.erase(iter);
    return ret;
}

template <typename T>
T ConfigSection::get_integer_value(const std::string& propertyId)
{
    const auto value = this->consume_value(propertyId);
    std::istringstream reader(value);
    T val = 0;
    if (!(reader >> val))
    {
        throw Exception("bad integer value: ", value);
    }
    return val;
}

bool ConfigSection::get_boolean_value(const std::string& propertyId)
{
    const auto value = this->consume_value(propertyId);

    if (value == "true")
    {
        return true;
    }
    else if (value == "false")
    {
        return false;
    }
    else
    {
        throw Exception("bad boolean value: ", value);
    }
}


ProxyConfig::ProtoType ConfigSection::get_proto_type()
{
    const auto value = this->consume_value(props::proto_type);

    if (value == "tcp")
    {
        return ProxyConfig::ProtoType::tcp;
    }
    else if (value == "udp")
    {
        return ProxyConfig::ProtoType::udp;
    }
    else
    {
        throw Exception("Unknown protocol type: ", value);
    }
}

ProxyConfig::EndpointMode ConfigSection::get_mode()
{
    const auto value = this->consume_value(props::mode);

    if (value == "initiator")
    {
        return ProxyConfig::EndpointMode::initiator;
    }
    else if (value == "responder")
    {
        return ProxyConfig::EndpointMode::responder;
    }
    else
    {
        throw Exception("Unknown mode: ", value);
    }
}

ssp21::HandshakeMode ConfigSection::get_handshake_mode()
{
    const auto value = this->consume_value(props::handshake_mode);

    if (value == "shared_secret")
    {
        return HandshakeMode::shared_secret;
    }
    else if (value == "qkd")
    {
        return HandshakeMode::quantum_key_distribution;
    }
    else if (value == "preshared")
    {
        return HandshakeMode::preshared_public_keys;
    }
    else if (value == "certificate")
    {
        return HandshakeMode::industrial_certificates;
    }
    else
    {
        throw Exception("Unknown certificate mode: ", value);
    }
}

std::shared_ptr<ssp21::SecureDynamicBuffer> ConfigSection::get_file_data(const std::string& key)
{
    return ssp21::SecureFile::read(this->consume_value(key));
}

template <class T>
std::shared_ptr<const T> ConfigSection::get_crypto_key(const std::string& key, ssp21::ContainerEntryType expectedType)
{
    const auto path = this->consume_value(key);
    const auto file_data = SecureFile::read(path);

    ContainerFile file;
    const auto err = file.read_all(file_data->as_rslice());
    if (any(err))
    {
        throw Exception("Error (", ParseErrorSpec::to_string(err), ")  reading container file: ", path);
    }

    if (file.container_entry_type != expectedType)
    {
        throw Exception("Unexpected file type (", ContainerEntryTypeSpec::to_string(file.container_entry_type), ") in file: ", path);
    }

    const auto expected_length = ssp21::BufferBase::get_buffer_length(ssp21::BufferType::x25519_key);
    if (file.payload.length() != expected_length)
    {
        throw Exception("Unexpected key length: ", file.payload.length());
    }

    const auto ret = std::make_shared<T>();
    ret->as_wseq().copy_from(file.payload);
    ret->set_type(BufferType::x25519_key);

    return ret;
}

log4cpp::LogLevels ConfigSection::get_levels_for_char(char value)
{
    switch (value)
    {
    case('v'):
        return log4cpp::LogLevels(ssp21::levels::event.value);
    case('e'):
        return log4cpp::LogLevels(ssp21::levels::error.value);
    case('w'):
        return log4cpp::LogLevels(ssp21::levels::warn.value);
    case('i'):
        return log4cpp::LogLevels(ssp21::levels::info.value);
    case('d'):
        return log4cpp::LogLevels(ssp21::levels::debug.value);
    case('m'):
        return log4cpp::LogLevels(ssp21::levels::rx_crypto_msg.value | ssp21::levels::tx_crypto_msg.value);
    case('f'):
        return log4cpp::LogLevels(ssp21::levels::rx_crypto_msg_fields.value | ssp21::levels::tx_crypto_msg_fields.value);
    default:
        throw ssp21::Exception("unknown log level: ", value);
    }
}
