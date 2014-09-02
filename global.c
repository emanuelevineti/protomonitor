

#include "uthash.h"
#include "scap.h"
#include "global.h"
/*
Si utilizzano due strutture globali organizzate in una tabella hash:
nella prima si memorizzano i dati relativi al consumo di memoria e cpu
dei processi, mentre nella seconda si memorizzano i dati relativi ad i
singoli thread nati.
*/
proc_data* g_procs = NULL;
task_data* g_tasks = NULL;
global global_data;

int32_t uth_status;
