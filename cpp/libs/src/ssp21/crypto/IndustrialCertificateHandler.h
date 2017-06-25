
#ifndef SSP21_INDUSTRIALCERTIFICATEHANDLER_H
#define SSP21_INDUSTRIALCERTIFICATEHANDLER_H

#include "ssp21/crypto/ICertificateHandler.h"

#include "ssp21/crypto/BufferTypes.h"

#include "ssp21/util/SecureDynamicBuffer.h"

#include "ssp21/crypto/gen/CertificateEnvelope.h"
#include "ssp21/crypto/gen/CertificateBody.h"

#include <memory>
#include <string>

namespace ssp21
{

    /**
    * A certificate validator based on the ICF format
    */
    class IndustrialCertificateHandler final : public ICertificateHandler
    {
    public:

        IndustrialCertificateHandler(const std::string& anchor_certificate_path, const std::string& presented_chain_path);

        virtual seq32_t certificate_data() const override;

        virtual CertificateMode mode() const override
        {
            return CertificateMode::icf;
        }

        virtual HandshakeError validate(CertificateMode mode, const seq32_t& certificate_data, seq32_t& public_key_output) override;

    private:

        const std::shared_ptr<ssp21::SecureDynamicBuffer> anchor_certificate_file_data;
        const std::shared_ptr<ssp21::SecureDynamicBuffer> presented_chain_file_data;

        const CertificateBody anchor_certificate_body;
        const seq32_t presented_certificate_data;


        static CertificateBody read_anchor_cert(const seq32_t& envelope_data);
        static seq32_t verify_presented_chain(const seq32_t& chain_data);
    };



}

#endif
