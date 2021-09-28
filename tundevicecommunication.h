#ifndef TUNDEVICECOMMUNICATION_H
#define TUNDEVICECOMMUNICATION_H

#include <fcntl.h>  /* O_RDWR */
#include <string.h> /* memset(), memcpy() */
#include <stdio.h> /* perror(), printf(), fprintf() */
#include <stdlib.h> /* exit(), malloc(), free() */
#include <sys/ioctl.h> /* ioctl() */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include "IPeripheralCommunication.h"

class TunDeviceCommunication : public IPeripheralCommunication
{


public:
    TunDeviceCommunication(std::string dev_name);

    // IPeripheralCommunication interface
public:
    bool start();
    void stop();
    bool is_connected() const;
    bool is_running() const;
    bool send(const char *data, size_t size);
    boost::signals2::connection  connect_receive(std::function<void (char *, size_t)> func);
    boost::signals2::connection  connect_on_disconnection(std::function<void ()> func);

private:
    void read_thread();
    std::string dev_name_;
    int tun_open(char *devname);
    bool is_running_;
    int fd_;
    boost::signals2::signal<void(char * data, size_t size)> data_received_connections_;
    boost::signals2::signal<void()> disconetion_connections_;
    boost::thread_group threadGroup_;
};

#endif // TUNDEVICECOMMUNICATION_H
