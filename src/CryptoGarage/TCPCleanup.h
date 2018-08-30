//dirty fix to prevent running out of heap when an attacker sends
//sends many TCP packets very quickly, thus creating multiple TCP connections.

#ifndef TCPCLEANUP_H
#define TCPCLEANUP_H

struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);

void tcpCleanup () {
  while (tcp_tw_pcbs != NULL){
    tcp_abort(tcp_tw_pcbs);
  }
}

#endif
