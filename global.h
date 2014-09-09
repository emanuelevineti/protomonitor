/*
  Protomonitor.h
*/


/*	Definizioni Debug Mode	*/
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


/*	Dichiarazione strutture		*/
typedef struct  task_data{
  char exe[128];
  u_int32_t tid;	//hash key
  u_int16_t pid;
  u_int16_t father_pid;
  u_int16_t uid;
  u_int16_t gid;
  u_int16_t valid;
  time_t birth_ts; /* birth timestamp */
  time_t death_ts; /* death timestamp */
  UT_hash_handle hh;
}task_data;

typedef struct	proc_data{
  u_int32_t pid;	//hash key
  u_int16_t valid;

  u_int32_t peak_memory;
  u_int32_t actual_memory;

  /* CPU usage */
  u_int64_t start_system_ticks;
  u_int64_t start_process_ticks;
  u_int64_t start_process_iowait_ticks;
  u_int64_t end_system_ticks;
  u_int64_t end_process_ticks;
  u_int64_t end_process_iowait_ticks;
  u_int64_t tot_system_ticks;
  float avg_load;
  float iowait_time_pctg;

  /* Memory usage */
  u_int32_t start_num_page_faults;
  u_int32_t end_num_page_faults;
  u_int32_t process_page_faults;
  UT_hash_handle hh;
}proc_data;

typedef struct global{

  bool log_onfile_enabled;
  bool show_help_enabled;
  bool show_only_procs;
  bool get_all_proc;
  bool log_json_format;
  char stat_path[128];
  char proclife_path[128];
  float min_cpu_val;
  float min_iowait;
  u_int32_t min_page_faults;
  int refresh_t;
  int refresh_man;
}global;

/*	Definizione variabili globali	*/
/*
  Si utilizzano due strutture globali organizzate in una tabella hash:
  nella prima si memorizzano i dati relativi al consumo di memoria e cpu
  dei processi, mentre nella seconda si memorizzano i dati relativi ad i
  singoli thread nati.
*/
extern proc_data* g_procs ;
extern task_data* g_tasks ;
extern global global_data ;
extern int32_t uth_status;

/*	Fine Header file		*/
