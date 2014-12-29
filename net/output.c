#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";
	int ret;

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
	while(1) {
		ret = sys_ipc_recv(&nsipcbuf);
		if ((thisenv->env_ipc_from != ns_envid) || (thisenv->env_ipc_value != NSREQ_OUTPUT)) continue;\
		while((ret = sys_net_output(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len)) < 0) ;
	}
}
