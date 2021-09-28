#ifndef __I_PERIPHERAL_COMMUNICATION_H__
#define __I_PERIPHERAL_COMMUNICATION_H__


#include <functional>

class IPeripheralCommunication {

        public:

            virtual bool start() = 0;
            virtual void stop() = 0;
            virtual bool is_connected() const = 0;
            virtual bool is_running() const = 0;
            virtual bool send(const char * data, size_t size) = 0;

            virtual     boost::signals2::connection  connect_receive(std::function<void(char * data, size_t size)> func) = 0;
            virtual     boost::signals2::connection  connect_on_disconnection(std::function<void()> func) = 0;

            virtual ~IPeripheralCommunication() {}

        };

#endif // __I_PERIPHERAL_COMMUNICATION_H__
