#!/bin/bash

ip link set can0 down
ip link set can1 down
ip link set can0 type can bitrate 50000 dbitrate 1000000 fd on
ip link set can1 type can bitrate 50000 dbitrate 1000000 fd on
ip link set can0 up type can
ip link set can1 up type can
candump can1 &
cansend can0 300##1AC.AB.AD.AE.75.49.AD.D1.12.34.56.78.90.ab.cd.ef
killall candump

candump can0 &
cansend can1 300##112.34.56.78.90.ab.cd.ef.AC.AB.AD.AE.75.49.AD.D1
killall candump
