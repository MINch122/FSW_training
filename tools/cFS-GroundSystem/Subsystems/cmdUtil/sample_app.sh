#!/bin/sh

# ./cmdUtil -ELE --host=192.168.16.220 --port=1236 --pktid=0x1850 --pktfc=20


# Sample app native can
./cmdUtil -ELE --host=192.168.16.220 --port=1236 --pktid=0x1882 --pktfc=4


# SRL clear cnt
# ./cmdUtil -ELE --host=192.168.16.220 --port=1236 --pktid=0x180D --pktfc=2 --int32=3


# SRL close handle
# ./cmdUtil -ELE --host=192.168.16.220 --port=1236 --pktid=0x180D --pktfc=5 --int32=3


# SRL init handle - CAN
# indexer: 3, Name: CAN0, DevName: can0
# Devtype: 3
# ./cmdUtil -ELE --host=192.168.16.220 --port=1236 --pktid=0x180D --pktfc=4\
#             --int32=3 --string="16:CAN0" --string="16:can0"\
#             --uint8=3  --string="3:"