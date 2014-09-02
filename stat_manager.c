/*		Modulo gestione statistiche processi	*/
/*
Funzioni per la racconta di informazioni e le misurazioni delle 
statistiche dei processi.
*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>	
#include <string.h> /*strncpy(), strlen()*/
#include "uthash.h"
#include "scap.h"
#include "global.h"

void readProcessTicks(u_int16_t pid, u_int64_t *ticks, u_int64_t *iowait_ticks, u_int32_t *page_faults) {

	char path[256], buf[512], *line, *what;
	FILE *f;

	snprintf(path, sizeof(path), "/proc/%u/stat", pid);

	if((f = fopen(path, "r")) != NULL) {
		if((line = fgets(buf, sizeof(buf), f)) != NULL) {
			char *what, *el = strtok_r(line, " ", &what);
			int idx = 0;

			while(el != NULL) {
				el = strtok_r(NULL, " ", &what);
				if(el == NULL) break;

				if(idx == 11)      *page_faults = atoll(el);
				else if(idx == 12) *ticks = atoll(el);  // utime %lu   (14) Amount of time that this process has been scheduled in user mode
				else if(idx == 13) *ticks += atoll(el); // stime %lu   (15) Amount of time that this process has been scheduled in kernel mode
				else if(idx == 41) {
					*iowait_ticks = atoll(el); // delayacct_blkio_ticks %llu (since Linux 2.6.18) (42) Aggregated block I/O delays, measured in clock ticks (centiseconds).
					break;
				}

			idx++;
			}
		}

	fclose(f);
	} else {
      DB1(printf("%s(pid=%u) failed", __FUNCTION__, pid);)
	}
}

/* *********************************************** */

u_int64_t read_system_ticks() {

	u_int64_t ticks = 0;

  //ticks = clock();

	FILE *f;
	char *line, buf[512], *w;

	if((f = fopen("/proc/stat", "r")) != NULL) {
		if((line = fgets(buf, sizeof(buf), f)) != NULL) {
			char *w, *el = strtok_r(line, " ", &w);

			while(el) {
				el = strtok_r(NULL, " ", &w);
				if(el == NULL) break;
				ticks += atoll(el);
			}
		}

    fclose(f);
	}

	DB1(printf("%s() = ticks=%lu / clock=%lu", __FUNCTION__, ticks,  clock());)

	return(ticks);
}

/* *********************************************** */

void updateProcessStats(proc_data *stats) {

	char path[256];
	FILE *f;

	snprintf(path, sizeof(path), "/proc/%u/status", stats->pid);

	if((f = fopen(path, "r")) != NULL) {
		char *line, buf[128], *w, *l;

		while((line = fgets(buf, sizeof(buf), f)) != NULL) {
			if(strncmp(line, "VmPeak:", 7) == 0) {
				l = strtok_r(line, " ", &w);
				if(l) {
					l = strtok_r(NULL, " ", &w);
					if(l)
						stats->peak_memory = atol(l);
				}
			} else if(strncmp(line, "VmSize:", 7) == 0) {
				l = strtok_r(line, " ", &w);
				if(l) {
					l = strtok_r(NULL, " ", &w);
					if(l)
						stats->actual_memory = atol(l);
				}
				break;
			}
		}

		fclose(f);
	}

	readProcessTicks(stats->pid, &stats->start_process_ticks,
		&stats->start_process_iowait_ticks,
		&stats->start_num_page_faults);
	stats->start_system_ticks = read_system_ticks();
}


void finalizeProcess(proc_data *pp) {


	readProcessTicks(pp->pid, &pp->end_process_ticks,
		&pp->end_process_iowait_ticks,
		&pp->end_num_page_faults);
	pp->end_system_ticks = read_system_ticks();
	pp->tot_system_ticks = pp->end_system_ticks - pp->start_system_ticks;

	if(pp->tot_system_ticks >= 3) {
		pp->avg_load = ((pp->start_process_ticks == 0) || (pp->tot_system_ticks == 0)
			|| (pp->end_process_ticks == 0)) ? 0 :
			(float)((pp->end_process_ticks - pp->start_process_ticks)*100)/(float)pp->tot_system_ticks;
		if(pp->avg_load > 100 || pp->avg_load < 0) pp->avg_load = 0; /* Something went wrong */
     
		pp->iowait_time_pctg = ((pp->start_process_iowait_ticks == 0) || (pp->tot_system_ticks == 0)
			|| (pp->end_process_iowait_ticks == 0)) ? 0 :
			(float)((pp->end_process_iowait_ticks - pp->start_process_iowait_ticks)*100)/(float)pp->tot_system_ticks;
	
	if(pp->iowait_time_pctg > 100 || pp->iowait_time_pctg < 0) 
		pp->iowait_time_pctg = 0; /* Something went wrong */
	}/* else {
		pp->avg_load = 0, pp->iowait_time_pctg = 0; // Too little data 
	}*/
	pp->process_page_faults = pp->end_num_page_faults - pp->start_num_page_faults;
 	DB1(printf("###### [pid: %u][actual_memory: %lu][peak_memory: %lu][avg_load: %.2f %%][iowait: %.2f]"
		"[process_page_faults: %u][proc_ticks %u / sys_ticks %u / io ticks: %u]",
		pp->pid, pp->actual_memory, pp->peak_memory,
		pp->avg_load, pp->iowait_time_pctg, pp->process_page_faults,
		(pp->end_process_ticks - pp->start_process_ticks), pp->tot_system_ticks,
		pp->end_process_iowait_ticks-pp->start_process_iowait_ticks);)
}

/*		Fine modulo gestione statistiche	*/
