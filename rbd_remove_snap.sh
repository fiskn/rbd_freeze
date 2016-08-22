#!/bin/bash
Client=$1
Pool=$2
RBD=$3
SnapName=VeeamBackup
RBDDev=`rbd showmapped | grep $RBD | grep $SnapName | cut -d ' ' -f9`
logger unmounting FS
umount $RBDDev
if [ $? -ne 0 ]; then
        logger Failed to unmount FS on $Pool/$RBD@$SnapName
        exit 1
fi

logger unmapping RBD
rbd unmap $RBDDev
if [ $? -ne 0 ]; then
        logger Failed to unmap $Pool/$RBD@$SnapName
        exit 1
fi

logger Removing Snapshot $Pool/$RBD@$SnapName
rbd -n client.$Client snap remove $Pool/$RBD@$SnapName
if [ $? -ne 0 ]; then
        logger Failed to remove snapshot $Pool/$RBD@$SnapName
        exit 1
fi
logger Snapshot $Pool/$RBD@$SnapName removed succesfully