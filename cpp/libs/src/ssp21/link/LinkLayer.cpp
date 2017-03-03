
#include "ssp21/link/LinkLayer.h"

namespace ssp21
{

    LinkLayer::LinkLayer(uint16_t local_addr, uint16_t remote_addr) :
        local_addr(local_addr),
        remote_addr(remote_addr),
        parser(ssp21::consts::link::max_config_payload_size, *this)
    {}

    void LinkLayer::on_open_impl()
    {
        upper->on_open();
    }

    void LinkLayer::on_close_impl()
    {
        upper->on_close();
    }

    void LinkLayer::on_tx_ready_impl()
    {
        upper->on_tx_ready();
    }

    void LinkLayer::start_rx_impl(const seq32_t& data)
    {
        /*

        TODO

        if (!result.payload.is_empty()) // still have data waiting to be read
        {
            return false;
        }

        if (parser.parse(data) && parser.read(this->result))
        {
            if (this->upper->on_rx_ready(result.payload))
            {
                this->result.payload.make_empty();
            }
        }

        return false;
        */
    }

    bool LinkLayer::is_rx_ready_impl()
    {
        return this->result.payload.is_empty() && this->remainder.is_empty();
    }

    bool LinkLayer::is_tx_ready() const
    {
        return this->lower->is_tx_ready();
    }

    void LinkLayer::discard_rx_data()
    {
        this->result.payload.make_empty();
    }

    bool LinkLayer::start_tx(const seq32_t& data)
    {
        // The upper layer formats the frame for efficiency purposes
        return this->lower->start_tx(data);
    }

    void LinkLayer::on_rx_ready_impl()
    {
        /*

        TODO

        if (result.payload.is_not_empty() && upper->on_rx_ready(result.payload))
        {
            result.payload.make_empty();
            lower->receive();
        }
        */
    }

}
