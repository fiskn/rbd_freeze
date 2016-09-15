#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <rados/librados.h>
#include <rados/rados_types.h>

uint64_t cookie;
rados_ioctx_t io;
rados_t cluster;
char cluster_name[] = "ceph";
char* object;
char* pool;

void watch_notify2_test_cb(void *arg,
				  uint64_t notify_id,
				  uint64_t cookie,
				  uint64_t notifier_gid,
				  void *data,
				  size_t data_len)
{
	const char *notify_oid = 0;
	char *temp = (char*)data+4;
	char *temp2;
	char action[10];
	char rbd[20];
	int index,ret;
	printf("%s\n",temp);
	if(strcmp(temp,"freeze")==0)
	{
		char execstring[100];
		sprintf(execstring,"/usr/local/bin/rbd_freeze.sh %s %s",pool,object);
		ret=system(execstring);
		ret=WEXITSTATUS(ret);
		if(ret==0)
		{
			printf("Ok");
			rados_notify_ack(io, object, notify_id, cookie, "Frozen", 6);
		}
		else if(ret==1)
		{
			printf("Fail - No RBD");
			rados_notify_ack(io, object, notify_id, cookie, "NoRBD", 5);
		}
                else if(ret==2)
                {
                        printf("Fail - No Mount");
                        rados_notify_ack(io, object, notify_id, cookie, "NoMount", 7);
                }
                else if(ret==3)
                {
                        printf("Fail - Busy");
                        rados_notify_ack(io, object, notify_id, cookie, "Busy", 4);
                }
		else
		{
                        printf("Unknown Fail");
                        rados_notify_ack(io, object, notify_id, cookie, "Error", 5);
                }

	}
	else if(strcmp(temp,"unfreeze")==0)
	{
                char execstring[100];
                sprintf(execstring,"/usr/local/bin/rbd_unfreeze.sh %s %s",pool,object);
                ret=system(execstring);
                if(ret==0)
                {
                        printf("Ok");
                        rados_notify_ack(io, object, notify_id, cookie, "UnFrozen", 8);
                }
                else
                {
                        printf("Fail");
                        rados_notify_ack(io, object, notify_id, cookie, "Error", 5);
                }
	}
}

void watch_notify2_test_errcb(void *arg, uint64_t cookie, int err)
{
        printf("Removing Watcher on object %s\n",object);
	err = rados_unwatch2(io,cookie);
        printf("Creating Watcher on object %s\n",object);
        err = rados_watch2(io,object,&cookie,watch_notify2_test_cb,watch_notify2_test_errcb,NULL);
        if (err < 0) {
                fprintf(stderr, "%s: cannot create watcher %s: %s\n", object, pool, strerror(-err));
                rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        }
}

int main (int argc, char **argv)
{
	char* user_name;
    	if (argc < 4) { 
        	printf("Usage is -p <pool> -o <object> -u <client.username>\n");
        	exit(0);
    	} else {
        	for (int i = 1; i < argc; i++) {
            		if (i + 1 != argc) {
                		if (strcmp(argv[i],"-p")==0) {
                    			pool = argv[i + 1];
                		} else if (strcmp(argv[i],"-o")==0) {
                    			object = argv[i + 1];
				} else if (strcmp(argv[i],"-u")==0) {
                                        user_name = argv[i + 1];
                                }

            		}
        	}
	}
        /* Declare the cluster handle and required arguments. */
        uint64_t flags;

        /* Initialize the cluster handle with the "ceph" cluster name and the "client.admin" user */
        int err;
        err = rados_create2(&cluster, cluster_name, user_name, flags);

        if (err < 0) {
                fprintf(stderr, "%s: Couldn't create the cluster handle! %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nCreated a cluster handle.\n");
        }


        /* Read a Ceph configuration file to configure the cluster handle. */
        err = rados_conf_read_file(cluster, "/etc/ceph/ceph.conf");
        if (err < 0) {
                fprintf(stderr, "%s: cannot read config file: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the config file.\n");
        }

        /* Read command line arguments */
        err = rados_conf_parse_argv(cluster, argc, argv);
        if (err < 0) {
                fprintf(stderr, "%s: cannot parse command line arguments: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nRead the command line arguments.\n");
        }

        /* Connect to the cluster */
        err = rados_connect(cluster);
        if (err < 0) {
                fprintf(stderr, "%s: cannot connect to cluster: %s\n", argv[0], strerror(-err));
                exit(EXIT_FAILURE);
        } else {
                printf("\nConnected to the cluster.\n");
        }

	err = rados_ioctx_create(cluster, pool, &io);
	if (err < 0) {
        	fprintf(stderr, "%s: cannot open rados pool %s: %s\n", argv[0], pool, strerror(-err));
        	rados_shutdown(cluster);
        	exit(1);
	}
	printf("Creating Watcher on object %s\n",object);
	err = rados_watch2(io,object,&cookie,watch_notify2_test_cb,watch_notify2_test_errcb,NULL);
        if (err < 0) {
                fprintf(stderr, "%s: cannot create watcher %s: %s\n", argv[0], pool, strerror(-err));
		rados_ioctx_destroy(io);
                rados_shutdown(cluster);
                exit(1);
        }
	while(1){
		sleep(1);
	}
	rados_ioctx_destroy(io);
	rados_shutdown(cluster);

}
