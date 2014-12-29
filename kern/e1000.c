#include <kern/e1000.h>
#include <inc/string.h>
#include <inc/error.h>

// LAB 6: Your driver code here
struct tx_desc tx_queue[E1000_TXDESC] __attribute__ ((aligned (16)));
struct packet pkt_bufs[E1000_TXDESC];

int pci_network_attach(struct pci_func *pcif) {

	//TODO
	pci_func_enable(pcif);
	physaddr_t e1000_phys = pcif->reg_base[0];
	e1000 = mmio_map_region(e1000_phys, pcif->reg_size[0]);
	//cprintf("%08x\n", e1000[E1000_STATUS]);

	//initialisation
	memset(tx_queue, 0, sizeof(struct tx_desc) * E1000_TXDESC);
	memset(pkt_bufs, 0, sizeof(struct packet) * E1000_TXDESC);
	int i;
	for(i = 0; i < E1000_TXDESC; i++ ) {
		tx_queue[i].addr = PADDR(pkt_bufs[i].pkt);
		tx_queue[i].status |= E1000_TXD_STAT_DD;
	}
	
	e1000[E1000_TDBAL] = PADDR(tx_queue);
	e1000[E1000_TDBAH] = 0;
	e1000[E1000_TDLEN] = sizeof(struct tx_desc) * E1000_TXDESC;
	e1000[E1000_TDH] = 0;
	e1000[E1000_TDT] = 0;

	// Program the Transmit control Register
	e1000[E1000_TCTL] |= E1000_TCTL_EN;
	e1000[E1000_TCTL] |= E1000_TCTL_PSP;
	e1000[E1000_TCTL] &= ~E1000_TCTL_CT;
	e1000[E1000_TCTL] |= (0x10) << 4;
	e1000[E1000_TCTL] &= ~E1000_TCTL_COLD;
	e1000[E1000_TCTL] |= (0x40) << 12;

	// Program the Transmit IPG Register
	e1000[E1000_TIPG] = 0x0;
	e1000[E1000_TIPG] |= (0x6) << 20; // IPGR2 
	e1000[E1000_TIPG] |= (0x4) << 10; // IPGR1
	e1000[E1000_TIPG] |= 0xA; // IPGR
	return 0;
}

int e1000_transmit(const char* data, int len) {

	if ( len > PKTSIZE ) return -E_PKT_LONG;
	uint32_t tdt = e1000[E1000_TDT];
	if ( !(tx_queue[tdt].status & E1000_TXD_STAT_DD) ) return -E_NO_FREE;

	memmove(pkt_bufs[tdt].pkt, data, len);
	tx_queue[tdt].length = len;

	//reset DD bit
	tx_queue[tdt].status &= ~E1000_TXD_STAT_DD;
	//set report status bit
	tx_queue[tdt].cmd |= E1000_TXD_CMD_RS | E1000_TXD_CMD_EOP;
	e1000[E1000_TDT] = (tdt + 1) % E1000_TXDESC;

	return 0;

}
