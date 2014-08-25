

#include "uthash.h"
#include "scap.h"
#include "proto.h"
/*
Si utilizzano due strutture globali organizzate in una tabella hash:
nella prima si memorizzano i dati relativi al consumo di memoria e cpu
dei processi, mentre nella seconda si memorizzano i dati relativi ad i
singoli thread nati.
*/
struct	proc_data* g_procs = NULL;
struct task_data* g_tasks = NULL;

int32_t uth_status;
