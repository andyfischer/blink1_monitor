
#include <sys/types.h>

#define IOKIT 1 /* For io_name_t in device/device_types.h. */

#include <device/device_types.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/storage/IOBlockStorageDriver.h>

#include <fcntl.h>
#include <nlist.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <pwd.h>


#include <mach/mach_host.h>
#include <mach/mach_error.h>

#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <ifaddrs.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blink1-lib.h"

// Switch to enable/disable logging
#define printf_log printf
// #define printf_log

/* Mach port, used for various Mach calls. */
static mach_port_t libtop_port;
static mach_port_t libtop_master_port;


const int interval_millis = 500;

const double network_max_mb_sec = 1;
const double disk_max_mb_sec = 20;

double
get_cpu_usage()
{
    host_cpu_load_info_data_t load_info;
	kern_return_t		error;
	mach_msg_type_number_t	count;

    static int64_t prev_system_ticks;
    static int64_t prev_user_ticks;
    static int64_t prev_idle_ticks;

	count = HOST_CPU_LOAD_INFO_COUNT;

	error = host_statistics(libtop_port,
        HOST_CPU_LOAD_INFO,
	    (host_info_t) &load_info, &count);
	if (error != KERN_SUCCESS) {
		printf("Error in host_statistics(): %s", mach_error_string(error));
		return 0.0;
	}

    int64_t system_ticks = load_info.cpu_ticks[CPU_STATE_SYSTEM];
    int64_t user_ticks = load_info.cpu_ticks[CPU_STATE_USER];
    int64_t idle_ticks = load_info.cpu_ticks[CPU_STATE_IDLE];

    int64_t total_ticks = system_ticks + user_ticks + idle_ticks;

    printf_log("current: %lld, %lld, %lld\n",
        system_ticks,
        user_ticks,
        idle_ticks);

    printf_log("prev: %lld, %lld, %lld\n",
        prev_system_ticks,
        prev_user_ticks,
        prev_idle_ticks);
        

    int64_t delta_system = system_ticks - prev_system_ticks;
    int64_t delta_user = user_ticks - prev_user_ticks;
    int64_t delta_idle = idle_ticks - prev_idle_ticks;
    int64_t delta_total = delta_system + delta_user + delta_idle;

    printf_log("delta: %lld, %lld, %lld\n", delta_system, delta_user, delta_idle);

    if (delta_total <= 0)
        delta_total = 1;

    double active_ratio = ((double) delta_system + delta_user) / delta_total;
    printf_log("cpu in use: %f\n", active_ratio);

    if (active_ratio > 1.0)
        active_ratio = 1.0;

    prev_system_ticks = system_ticks;
    prev_user_ticks = user_ticks;
    prev_idle_ticks = idle_ticks;

    return active_ratio;
}

double get_disk_usage()
{
	io_registry_entry_t	drive;
	io_iterator_t		drive_list;
	CFNumberRef		number;
	CFDictionaryRef		properties, statistics;
	UInt64			value;

    static int64_t prev_rbytes;
    static int64_t prev_wbytes;

    int64_t rbytes = 0;
    int64_t wbytes = 0;

	/* Get the list of all drive objects. */
	if (IOServiceGetMatchingServices(libtop_master_port,
	    IOServiceMatching("IOBlockStorageDriver"), &drive_list)) {
		printf("Error in IOServiceGetMatchingServices()\n");
		goto ERROR_NOLIST;
	}

#if 0
	tsamp.p_disk_rops = tsamp.disk_rops;
	tsamp.p_disk_wops = tsamp.disk_wops;
	tsamp.p_disk_rbytes = tsamp.disk_rbytes;
	tsamp.p_disk_wbytes = tsamp.disk_wbytes;

	tsamp.disk_rops = 0;
	tsamp.disk_wops = 0;
	tsamp.disk_rbytes = 0;
	tsamp.disk_wbytes = 0;
#endif

	while ((drive = IOIteratorNext(drive_list)) != 0) {
		number = 0;
		properties = 0;
		statistics = 0;
		value = 0;

		/* Obtain the properties for this drive object. */
		if (IORegistryEntryCreateCFProperties(drive,
		    (CFMutableDictionaryRef *)&properties, kCFAllocatorDefault,
		    kNilOptions)) {
			printf("Error in IORegistryEntryCreateCFProperties()\n");
			goto RETURN;
		}

		if (properties != 0) {
			/* Obtain the statistics from the drive properties. */
			statistics
			    = (CFDictionaryRef)CFDictionaryGetValue(properties,
			    CFSTR(kIOBlockStorageDriverStatisticsKey));

			if (statistics != 0) {
#if 0
				/* Get number of reads. */
				number =
				    (CFNumberRef)CFDictionaryGetValue(statistics,
				    CFSTR(kIOBlockStorageDriverStatisticsReadsKey));
				if (number != 0) {
					CFNumberGetValue(number,
					    kCFNumberSInt64Type, &value);
					tsamp.disk_rops += value;
				}
#endif

				/* Get bytes read. */
				number =
				    (CFNumberRef)CFDictionaryGetValue(statistics,
				    CFSTR(kIOBlockStorageDriverStatisticsBytesReadKey));
				if (number != 0) {
					CFNumberGetValue(number,
					    kCFNumberSInt64Type, &value);
					rbytes += value;
				}

#if 0
				/* Get number of writes. */
				number =
				    (CFNumberRef)CFDictionaryGetValue(statistics,
				    CFSTR(kIOBlockStorageDriverStatisticsWritesKey));
				if (number != 0) {
					CFNumberGetValue(number,
					    kCFNumberSInt64Type, &value);
					tsamp.disk_wops += value;
				}
#endif

				/* Get bytes written. */
				number =
				    (CFNumberRef)CFDictionaryGetValue(statistics,
				    CFSTR(kIOBlockStorageDriverStatisticsBytesWrittenKey));
				if (number != 0) {
					CFNumberGetValue(number,
					    kCFNumberSInt64Type, &value);
					wbytes += value;
				}
			}

			/* Release. */
			CFRelease(properties);
		}

		/* Release. */
		IOObjectRelease(drive);
	}
	IOIteratorReset(drive_list);
#if 0
	if (tsamp.seq == 1) {
		tsamp.b_disk_rops = tsamp.disk_rops;
		tsamp.p_disk_rops = tsamp.disk_rops;

		tsamp.b_disk_wops = tsamp.disk_wops;
		tsamp.p_disk_wops = tsamp.disk_wops;

		tsamp.b_disk_rbytes = tsamp.disk_rbytes;
		tsamp.p_disk_rbytes = tsamp.disk_rbytes;

		tsamp.b_disk_wbytes = tsamp.disk_wbytes;
		tsamp.p_disk_wbytes = tsamp.disk_wbytes;
	}
#endif

	RETURN:
	/* Release. */
	IOObjectRelease(drive_list);
	ERROR_NOLIST:

    int64_t total_bytes_delta = (rbytes + wbytes) - (prev_rbytes + prev_wbytes);

    printf_log("disk i/o in sample; %lld\n", total_bytes_delta);

    prev_rbytes = rbytes;
    prev_wbytes = wbytes;

	double result = (double) total_bytes_delta / (disk_max_mb_sec * 1024 * 1024 * (interval_millis / 1000.0)); 

    if (result > 1.0)
        result = 1.0;

    return result;
}

double get_network_usage()
{
    short network_layer;
	short link_layer;
 	int mib[6];
     	char *buf = NULL, *lim, *next;
	size_t len;
	struct if_msghdr *ifm;


    static int64_t prev_ipackets;
    static int64_t prev_opackets;
    static int64_t prev_ibytes;
    static int64_t prev_obytes;

    int64_t ipackets = 0;
    int64_t opackets = 0;
    int64_t ibytes = 0;
    int64_t obytes = 0;

	mib[0]	= CTL_NET;			// networking subsystem
	mib[1]	= PF_ROUTE;			// type of information
	mib[2]	= 0;				// protocol (IPPROTO_xxx)
	mib[3]	= 0;				// address family
	mib[4]	= NET_RT_IFLIST2;	// operation
	mib[5]	= 0;
	if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
        return 0.0;

	if ((buf = (char*) malloc(len)) == NULL) {
		printf("malloc failed\n");
		exit(1);
	}
	if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
		if (buf) free(buf);
		return 0.0;
	}

	lim = buf + len;
	for (next = buf; next < lim; ) {
		network_layer = link_layer = 0;
	        ifm = (struct if_msghdr *)next;
		next += ifm->ifm_msglen;

	        if (ifm->ifm_type == RTM_IFINFO2) {
			struct if_msghdr2 	*if2m = (struct if_msghdr2 *)ifm;

			if(if2m->ifm_data.ifi_type!=18) { /* do not count loopback traffic */
				opackets += if2m->ifm_data.ifi_opackets;
				ipackets += if2m->ifm_data.ifi_ipackets;
				obytes   += if2m->ifm_data.ifi_obytes;
				ibytes   += if2m->ifm_data.ifi_ibytes;
			}
		} 
	}

    int64_t delta_obytes = obytes - prev_obytes;
    int64_t delta_ibytes = ibytes - prev_ibytes;

	free(buf);

    prev_ipackets = ipackets;
    prev_opackets = opackets;
    prev_ibytes = ibytes;
    prev_obytes = obytes;

    printf_log("total network bytes in sample: %f\n", (double) delta_obytes + delta_ibytes);

    double result = ((double) delta_obytes + delta_ibytes) / ((double) network_max_mb_sec * 1024 * 1024 * (interval_millis / 1000.0));

    if (result > 1.0)
        result = 1.0;

    // todo: apply a logarithmic scale?

    return result;
}

int main(int argc, char** argv)
{
	libtop_port = mach_host_self();

	if (IOMasterPort(bootstrap_port, &libtop_master_port)) {
		printf_log("Error in IOMasterPort()\n");
	}

    hid_device* device = blink1_open();

    if (device == NULL)
        return -1;

    int iteration_count = 0;


    while (1) {

        double cpu_usage = get_cpu_usage();
        double network_usage = get_network_usage();
        double disk_usage = get_disk_usage();

        printf_log("cpu usage (red) %i\n", int(255 * cpu_usage));
        printf_log("network usage (green) %i\n", int(255 * network_usage));
        printf_log("disk usage (blue) %i\n", int(255 * disk_usage));

        // Only start talking to USB after a few iterations, otherwise we get
        // misleading deltas.
        if (iteration_count >= 2) {

            // Fade time is the same as sleep interval, for nice smoothness.
            blink1_fadeToRGB(device, interval_millis,
                255 * cpu_usage, 
                255 * network_usage,
                255 * disk_usage);
        }

        blink1_sleep(interval_millis);

        iteration_count++;
    }


    return 0;
}
