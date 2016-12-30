
#ifndef SSP21_LAYER_INTERFACES_H
#define SSP21_LAYER_INTERFACES_H

#include "openpal/util/Uncopyable.h"

#include "openpal/container/SequenceTypes.h"

namespace ssp21
{

    class ILowerLayer
    {

    public:

        virtual bool transmit(const seq32_t& data) = 0;

        // tell this layer to push any data it might have
        virtual void receive() = 0;

        inline bool get_is_tx_ready() const
        {
            return this->is_tx_ready;
        }

    protected:

        void reset_lower_layer()
        {
            this->is_tx_ready = true;
            this->is_rx_ready = false;
        }

        bool is_tx_ready = true;
        bool is_rx_ready = false;

    };

    class IUpperLayer
    {

    public:

        inline void on_open()
        {
            if (!this->is_open)
            {
                this->is_open = true;
                this->on_open_impl();
            }
        }

        inline void on_close()
        {
            if (this->is_open)
            {
                this->is_open = false;
                this->on_close_impl();
            }
        }

        inline void on_tx_ready()
        {
            if (this->is_open)
            {
                this->on_tx_ready_impl();
            }
        }

        inline bool on_rx_ready(const seq32_t& data)
        {
            return (this->is_open) ? this->on_rx_ready_impl(data) : false;
        }

        inline bool get_is_open() const
        {
            return is_open;
        }

    protected:

        virtual void on_open_impl() = 0;

        virtual void on_close_impl() = 0;

        virtual void on_tx_ready_impl() = 0;

        virtual bool on_rx_ready_impl(const seq32_t& data) = 0;

    private:

        bool is_open = false;

    };

}

#endif