/*

  Protomonitor v.0.0

*/
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h> /*strncpy(), strlen()*/
#include "uthash.h"
#include "scap.h"
#include "global.h"
#include "proc_manager.h"



//#define DEBUG_MODE

#ifdef DEBUG_MODE
/*
  MASK = 1 procs DB
  MASK = 2 threads DB
  MASK = 3 procs & threads DB
*/
unsigned int MASK = 2;

#define DBG(A,B) { if( (A) & MASK ) { B;}}

#else

#define DBG(A,B)

#endif

#define DB1(a) DBG(1,a)
#define DB2(a) DBG(2,a)


int main(int argc, char *argv[]) {

  char error[256];
  int capture = 1,live = true, time=0;
  scap_t *h ;
  //apro la cattura live degli eventi
  if(argc == 2){
    live = false;
    if( (h = scap_open_offline(argv[1],error)) == NULL){
      printf("%s :path non valido\n",argv[1]);
      return(-1);
    }
  }else{ if( ( h = scap_open_live(error)) == NULL){
      printf("Unable to connect to open sysdig: %s\n", error);
      return(false);
    }
  }

  //setto i filtri per gli eventi da catturare solo se la cattura è live
  if(live){
    scap_clear_eventmask(h);
    if(scap_set_eventmask(h, PPME_CLONE_16_X) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
    if(scap_set_eventmask(h, PPME_PROCEXIT_E) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
    if(scap_set_eventmask(h, PPM_SC_EXIT_GROUP) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
    if(scap_set_eventmask(h, PPM_SC_EXIT) != SCAP_SUCCESS)
      printf("[ERROR] scap call failed: old driver ?\n");
  }
  //ciclo di cattura
  while(capture)
    {	time+=1;
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
      /*si aggiornano i dati ogni 400 eventi catturati
	(XXX numero da regolare) */
      if( time == 400){
	manage_data(h);
	time = 0;
      }
    }
  //chiudo la cattura live degli eventi
  scap_close(h);
}


