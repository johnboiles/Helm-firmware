#!/bin/sh

# Configure the following two for your system
HOST=root@boat-pi.local
LOADER=/root/repos/teensy_loader_cli/teensy_loader_cli

# Shouldn't need to change these
FIRMWARE_FILE=firmware.hex
TEMP_DIR=$(ssh $HOST 'mktemp -d')
MMCU=mk20dx256
scp $FIRMWARE_FILE $HOST:$TEMP_DIR
ssh $HOST "service nmea_proxy stop; $LOADER -s -mmcu=$MMCU -v -w $TEMP_DIR/$FIRMWARE_FILE; rm -rf $TEMP_DIR; service nmea_proxy start"
