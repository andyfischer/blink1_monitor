
#include <cstdio>
#include <cstdlib>

#include <mach/mach_host.h>
#include <mach/mach_error.h>


#include "blink1-lib.h"

/* Mach port, used for various Mach calls. */
static mach_port_t libtop_port;


double
libtop_p_load_get()
{
    host_cpu_load_info_data_t load_info;
	boolean_t		retval;
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

    printf("current: %lld, %lld, %lld\n",
        system_ticks,
        user_ticks,
        idle_ticks);

    printf("prev: %lld, %lld, %lld\n",
        prev_system_ticks,
        prev_user_ticks,
        prev_idle_ticks);
        

    int64_t delta_system = system_ticks - prev_system_ticks;
    int64_t delta_user = user_ticks - prev_user_ticks;
    int64_t delta_idle = idle_ticks - prev_idle_ticks;
    int64_t delta_total = delta_system + delta_user + delta_idle;

    printf("delta: %lld, %lld, %lld\n", delta_system, delta_user, delta_idle);

    if (delta_total <= 0)
        delta_total = 1;

    double active_percent = ((double) delta_system + delta_user) / delta_total;
    printf("cpu in use: %f\n", active_percent);

    prev_system_ticks = system_ticks;
    prev_user_ticks = user_ticks;
    prev_idle_ticks = idle_ticks;

    return active_percent;
}


int main(int argc, char** argv)
{
	libtop_port = mach_host_self();

    // call this initially to set prev_load_info.
    libtop_p_load_get();
    libtop_p_load_get();

    hid_device* device = blink1_open();

    if (device == NULL)
        return -1;

    while (1) {

        double cpu_usage = libtop_p_load_get();

        printf("cpu usage (red) %i\n", int(255 * cpu_usage));

        // Fade time should be the same as sleep interval, for nice smoothness.
        blink1_fadeToRGB(device, 500, 255 * cpu_usage, 0, 0);

        blink1_sleep(500);
    }


    return 0;
}
