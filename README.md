
protomonitor
============



Il programma permette di catturare la creazione e la distruzione di un processo
e tenere traccia delle suoi dati attraverso l'uso di due tabelle hash: una per 
il main process ed una per ogni thread. Inoltre periodicamente il programma 
esegue un calcolo dell'uso della cpu del main process e dei suoi page fault.

Maggiori dettagli a seguire:
. Nella tabella dei thread vengono memorizzate le sue informazioni generali come:
  Nome, tid, pid, ppid, uid, gid.
. Nella tabella dei Main process vengono memorizzate oltre al pid i dati per 
  eseguire le misurazioni della cpud, dei pagefault e della peak_memory e actual_memory.
. Periodicamente avviene la stampa delle informazioni a video.
. I processi morti vengono solo marcati come non validi.
. Le tabella hash vengono pulite dai processi non validi durante l'amministrazione 
  periodica della tabella.
. All'atto dell'inizalizzazione del programma se selezionata con l'argomento -a
  vengono portati in memoria i dati riguardanti i processi attivi sulla macchina.
. Trammite l'opzione -l si può ottenere un export su due file di log con le statistiche
  dei processi in attività e tener traccia della nascita e morte di un processo.


Elenco sintetico problemi aperti:
. Si è scelto di utilizzare due tabelle diverse per aver la possibilità di avere
  informazioni condivise tra tutti i thread di un processo e informazioni specifiche
  per ognuno di questi. Bisogna valutare se la scelta può comportare vantaggi o vale
  la pena provare altre strade più vantaggiose.
. Sembra che anche se gestiti casi di errore la e timeout libreria scap riproponga 
  eventi già valutati più di una volta.
. I commenti al momento in italiano devono essere tradotti.

