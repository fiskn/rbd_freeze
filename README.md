# rbd_freeze
Small C utility and Bash scripts to remotely freeze a Ceph RBD before taking a Snapshot, so that the contents are consistent

rbd_freeze.c - is the utility that runs on the server with the mounted RBD's and data you wish to snapshot

The below two script are expected to found in /usr/local/bin:rbd_freeze.sh - is the example script the utility runs to carry out checks and freeze the filesystem
rbd_unfreeze.sh - is the example script that the utility runs to unfreeze the filesystem once the snapshot has been taken

rbd_take_snap.sh - is an example script that is run on something like a backup server and is responsible for sending the RADOS notify, checking the return status, taking the RBD snapshot and mounting it
rbd_remove_snap.sh - is and example script to unmount and remove the RBD snapshot

rbd_freeze parameters
Usage is -p <pool> -o <object> -u <client.username>

Where the: 
pool is the RADOS pool
object is a RADOS object. I use rbd_id.<Object_Name>
client.username is a valid Ceph keyring user

take/remove snap parameters
Usage is <RADOS client name> Pool RBD
