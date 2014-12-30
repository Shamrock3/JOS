#include "ns.h"

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.
	char buf[2048];
	int perm = PTE_U | PTE_P | PTE_W;
	int len = 2047;

	while(1) {

		int ret;
		while((ret = sys_net_input(buf, &len)) < 0) sys_yield();

		//previous page is automatically "page remove"ed
		while ((ret = sys_page_alloc(0, &nsipcbuf, perm)) < 0);

		nsipcbuf.pkt.jp_len = len;
		memmove(nsipcbuf.pkt.jp_data, buf, len);

		while ((ret = sys_ipc_try_send(ns_envid, NSREQ_INPUT, &nsipcbuf, perm)) < 0);
	} 
		
}
