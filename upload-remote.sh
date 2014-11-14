#!/bin/sh
HOST=root@boat-pi.local
LOADER=/root/repos/teensy_loader_cli/teensy_loader_cli
FIRMWARE_FILE=firmware.hex
TEMP_DIR=$(ssh root@boat-pi.local 'mktemp -d')
MMCU=mk20dx256
scp $FIRMWARE_FILE $HOST:$TEMP_DIR
ssh $HOST "service ser2net stop; $LOADER -mmcu=$MMCU -v -w $TEMP_DIR/$FIRMWARE_FILE; rm -rf $TEMP_DIR; service ser2net start"
