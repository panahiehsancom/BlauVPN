#include "tundevicecommunication.h"

TunDeviceCommunication::TunDeviceCommunication(std::string dev_name) : dev_name_(dev_name)
{

}

bool TunDeviceCommunication::start()
{
    int fd, nbytes;
    fd_ = tun_open(const_cast<char *>(dev_name_.c_str())); /* devname = ifr.if_name = "tun0" */
    is_running_ =true;
    threadGroup_.create_thread(std::bind(&TunDeviceCommunication::read_thread, this));
    return true;
}

void TunDeviceCommunication::stop()
{
    is_running_ =false;
}

bool TunDeviceCommunication::is_connected() const
{
    return true;
}

bool TunDeviceCommunication::is_running() const
{
    return true;
}

bool TunDeviceCommunication::send(const char *data, size_t size)
{
    if(is_running_)
    {
        size_t result = write(fd_, data, size);
        return true;
    }
    else
        return false;
}

boost::signals2::connection TunDeviceCommunication::connect_receive(std::function<void (char *, size_t)> func)
{
    return data_received_connections_.connect(func);
}

boost::signals2::connection TunDeviceCommunication::connect_on_disconnection(std::function<void ()> func)
{
    return disconetion_connections_.connect(func);
}

void TunDeviceCommunication::read_thread()
{
    char buf[1600];
    while(is_running_) {
        int   nbytes = read(fd_, buf, sizeof(buf));
        printf("Read %d bytes from tun \n", nbytes);
    }
}

int TunDeviceCommunication::tun_open(char *devname)
{
    struct ifreq ifr;
    int fd, err;

    if ( (fd = open("/dev/net/tun", O_RDWR)) == -1 ) {
        perror("open /dev/net/tun");exit(1);
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN;
    strncpy(ifr.ifr_name, devname, IFNAMSIZ); // devname = "tun0" or "tun1", etc

    /* ioctl will use ifr.if_name as the name of TUN
     * interface to open: "tun0", etc. */
    if ( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) == -1 ) {
        perror("ioctl TUNSETIFF");close(fd);exit(1);
    }

    /* After the ioctl call the fd is "connected" to tun device specified
     * by devname ("tun0", "tun1", etc)*/

    return fd;
}
