/*

  Protomonitor v.0.0

*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <string.h> /*strncpy(), strlen()*/
#include "uthash.h"
#include "scap.h"
#include "global.h"
#include "proc_manager.h"
#include "proto_io.h"

//#define DEBUG_MODE


int main(int argc, char *argv[]) {

  char error[256];
  int capture = 1;
  time_t last_refresh;
  scap_t *h ;
  //apro la cattura live degli eventi
  printf("\n\t\t INIZIO INIZIALIZZAZIONE \n");
  
  read_argv(argc, argv);
  if(global_data.show_help_enabled){
    print_help();
    return(1);
  }
  
  if( ( h = scap_open_live(error)) == NULL){
    printf("Unable to connect to open sysdig: %s\n", error);
    return(false);
    }


  //setto i filtri per gli eventi da catturare solo se la cattura è live
    scap_clear_eventmask(h);
    if(scap_set_eventmask(h, PPME_CLONE_16_X) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
    if(scap_set_eventmask(h, PPME_PROCEXIT_E) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
    if(scap_set_eventmask(h, PPM_SC_EXIT_GROUP) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
    if(scap_set_eventmask(h, PPM_SC_EXIT) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");

  if(global_data.get_all_proc)
    init_add_active_proc(h);

  printf("\n\t\t FINE INIZIALIZZAZIONE \n");
  //ciclo di cattura
  last_refresh = time(NULL);
  global_data.refresh_man = global_data.refresh_t; //temporaneo
  while(capture)
    {	
      struct ppm_evt_hdr* ev;
      u_int16_t cpuid;
      int32_t res = scap_next(h, &ev, &cpuid);

      if(res > 0 ) {
	printf("[ERROR] %s\n", scap_getlasterr(h));
	scap_close(h);
	break;
      } else if( res == SCAP_SUCCESS ) {
	handle_event(ev,cpuid,h);
      } else if( res != -1 ) 	//timeout
	fprintf(stderr, "scap_next() returned %d\n", res);
      /*si aggiornano i dati ogni refresh_t secondi (5 default)
	(XXX numero da regolare) */
      if( (time(NULL) - last_refresh) > global_data.refresh_t){
	print_tasks_procs();
	last_refresh = time(NULL);
      }
      if( (time(NULL) - last_refresh) > global_data.refresh_man){
        manage_data(h);
        last_refresh = time(NULL);
      }
    }
  //chiudo la cattura live degli eventi
  scap_close(h);
}


