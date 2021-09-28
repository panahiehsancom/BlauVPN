# BlauVPN

Arguments:
				-t Tun Device name
				-b binding port number
## Introduction

## Features

## Usage

[create an virtual ethernet(tun tap device)]
sudo ip tuntap add mode tun dev tun0 
[specific range of tun tap device]
sudo ip addr add 192.168.120.0/24 dev tun0
[make tun tap device up]
sudo ip link set dev tun0 up
[check the route packet]
sudo ip route get 192.168.120.3 
[create ping process to check application]
ping 192.168.120.3 
