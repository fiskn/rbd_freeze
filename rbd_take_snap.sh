#!/bin/bash
Client=$1
Pool=$2
RBD=$3
SnapName=VeeamBackup
logger Sending fsfreeze command via RADOS notify
Output=`rados -n client.$Client -p $Pool notify rbd_id.$RBD freeze | grep '|' | cut -d '|' -f2`
logger "Output from RADOS notify: $Output"
if [ "$Output" = "Error" ]; then
        logger "RBD_Snap: Unknown Error for $Pool - $RBD - Freeze"
        exit 99
elif [ "$Output" = "NoRBD" ]; then
       	logger RBD_Snap: RBD is not mapped on Watcher for $Pool - $RBD - Freeze
       	exit 1
elif [ "$Output" = "NoMount" ]; then
        logger RBD_Snap: RBD is not mounted on Watcher for $Pool - $RBD - Freeze
        exit 2
elif [ "$Output" = "Busy" ]; then
        logger RBD_Snap: RBD is busy on Watcher for $Pool - $RBD - Freeze
        exit 3
elif [ "$Output" = "Frozen" ]; then
        logger RBD_Snap: RBD FS has been Frozen for $Pool - $RBD - Freeze
        :
else
        logger Other Unknown Error - Is watcher Running?
        exit 98
fi

logger Taking RBD Snapshot for $Pool - $RBD
rbd -n client.$Client snap create $Pool/$RBD@$SnapName
if [ $? -ne 0 ]; then
        logger Failed to take RBD Snapshot $Pool/$RBD$SnapName
        logger Sending a unfreeze via RADOS
        rados -n client.$Client -p $Pool notify rbd_id.$RBD unfreeze
        exit 4;
fi
logger Snapshot succesfully taken $Pool/$RBD@$SnapName
logger Unfreezing FS
Output=`rados -n client.$Client -p $Pool notify rbd_id.$RBD unfreeze | grep '|' | cut -d '|' -f2`
if [ "$Output" = "Error" ]; then
        logger unable to unfreeze FS!!! for $Pool - $RBD - Freeze
        exit 5
fi
logger Unfroze FS
logger Mapping RBD Snapshot
Output=`rbd -n client.$Client map $Pool/$RBD@$SnapName`
logger Mounting RBD FS
if [ ! -d /mnt/backup/$RBD ]; then
        logger "Mount Folder Missing - Creating it"
        mkdir /mnt/backup/$RBD
fi

mount $Output /mnt/backup/$RBD -o ro,norecovery
logger Done
exit 0
