

#include "ssp21/crypto/IndustrialCertificateHandler.h"

#include "ssp21/crypto/gen/CertificateChain.h"
#include "ssp21/crypto/gen/ContainerFile.h"

#include "ssp21/crypto/Chain.h"

#include "ssp21/util/Exception.h"
#include "ssp21/util/SecureFile.h"

namespace ssp21
{

    IndustrialCertificateHandler::IndustrialCertificateHandler(const std::string& anchor_certificate_path, const std::string& presented_chain_path) :
        anchor_certificate_file_data(SecureFile::read(anchor_certificate_path)),
        presented_chain_file_data(SecureFile::read(presented_chain_path)),
        anchor_certificate_body(read_anchor_cert(anchor_certificate_file_data->as_rslice())),
        presented_certificate_data(verify_presented_chain(presented_chain_file_data->as_rslice()))
    {

    }

    CertificateBody IndustrialCertificateHandler::read_anchor_cert(const seq32_t& file_data)
    {
        ContainerFile file;
        {
            const auto err = file.read_all(file_data);
            if (any(err)) throw Exception("Unable to read container file: ", ParseErrorSpec::to_string(err));
        }

        if (file.container_entry_type != ContainerEntryType::certificate_chain)
        {
            throw Exception("Unexpected file type: ", ContainerEntryTypeSpec::to_string(file.container_entry_type));
        }

        CertificateChain chain;
        {
            const auto err = file.read_all(file_data);
            if (any(err)) throw Exception("Unable to read certificate chain: ", ParseErrorSpec::to_string(err));
        }

        if (chain.certificates.count() != 1) throw Exception("Unexpected number of certificates: ", chain.certificates.count());

        CertificateBody body;
        {
            const auto err = body.read_all(chain.certificates.get(0)->certificate_body);
            if (any(err)) throw Exception("Unable to read certificate body: ", ParseErrorSpec::to_string(err));
        }

        return body;
    }

    seq32_t IndustrialCertificateHandler::verify_presented_chain(const seq32_t& file_data)
    {
        ContainerFile file;
        {
            const auto err = file.read_all(file_data);
            if (any(err)) throw Exception("Unable to read container file: ", ParseErrorSpec::to_string(err));
        }

        if (file.container_entry_type != ContainerEntryType::certificate_chain)
        {
            throw Exception("Unexpected file type: ", ContainerEntryTypeSpec::to_string(file.container_entry_type));
        }

        CertificateChain chain;
        {
            const auto err = file.read_all(file_data);
            if (any(err)) throw Exception("Unable to read certificate chain: ", ParseErrorSpec::to_string(err));
        }

        // TODO - go deeper?
        return file.payload;
    }

    seq32_t IndustrialCertificateHandler::certificate_data() const
    {
        return this->presented_chain_file_data->as_rslice();
    }

    HandshakeError IndustrialCertificateHandler::validate(CertificateMode mode, const seq32_t& certificate_data, seq32_t& public_key_output)
    {
        if (mode != CertificateMode::icf) return HandshakeError::unsupported_certificate_mode;

        // first parse the data as a certificate chain
        CertificateChain chain;
        {
            const auto err = chain.read_all(certificate_data);
            if (any(err)) return HandshakeError::bad_certificate_format;
        }


        CertificateBody endpoint_cert;
        const auto err = Chain::verify(this->anchor_certificate_body, chain.certificates, endpoint_cert);
        if (any(err)) return err;

        public_key_output = endpoint_cert.public_key;

        return HandshakeError::none;
    }

}


