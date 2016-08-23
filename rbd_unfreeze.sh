#!/bin/bash
FORCE=${3:-0}
Pool=$1
Object=$2
RBD=${2#*\.}
echo Watched Object is $Object
echo RBD is $RBD
RBDPath=`ls -l /dev/rbd/$Pool/$RBD`
RBDDev=${RBDPath##*../}
echo $RBDDev
if [ ! $RBDDev ]; then
        echo "ERROR - RBD is not mapped"
        exit 1
fi

MountPath=`mount | grep $RBDDev | cut -d ' ' -f3`
if [ ! $MountPath ]; then
        echo "ERROR - Mount for RBD doesn't exist"
        exit 2
fi

echo $MountPath
logger Un-Freezing $MountPath on $RBDDev
fsfreeze --unfreeze $MountPath
