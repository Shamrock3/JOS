#include <kern/e1000.h>
#include <inc/string.h>
#include <inc/error.h>

// LAB 6: Your driver code here
struct tx_desc tx_queue[E1000_TXDESC] __attribute__ ((aligned (16)));
struct packet pkt_bufs[E1000_TXDESC];

struct rx_desc rx_queue[E1000_RXDESC] __attribute__ ((aligned (16)));
struct rx_packet rx_pkt_bufs[E1000_RXDESC];

int pci_network_attach(struct pci_func *pcif) {

	//TODO
	pci_func_enable(pcif);
	physaddr_t e1000_phys = pcif->reg_base[0];
	e1000 = mmio_map_region(e1000_phys, pcif->reg_size[0]);

	//initialisation Transmission
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

	//Initialise Reception
	memset(rx_queue, 0, sizeof(struct rx_desc) * E1000_RXDESC);
	memset(rx_pkt_bufs, 0, sizeof(struct rx_packet) * E1000_RXDESC);
	for(i = 0; i < E1000_RXDESC; i++ ) {
		rx_queue[i].addr = PADDR(rx_pkt_bufs[i].pkt);
		rx_queue[i].status &= ~E1000_RXD_STAT_DD;
	}

	//Program the Receive addresses
	volatile uint32_t* ral = &e1000[E1000_RA];
	volatile uint32_t* rah = &e1000[E1000_RA + 1];
	*rah = 0;
	*rah |= E1000_RAH_AV;
	*rah |= 0x34|(0x56 << 8);
	*ral = 0;
	*ral |= 0x52|(0x54 << 8)|(0x00 << 16)|(0x12 << 24);

	//Program the controls
	e1000[E1000_RDBAL] = PADDR(rx_queue);
	e1000[E1000_RDBAH] = 0;
	e1000[E1000_RDLEN] = sizeof(struct rx_desc) * E1000_RXDESC;
	e1000[E1000_RDH] = 0x0;
	e1000[E1000_RDT] = E1000_RXDESC - 1; 
	e1000[E1000_RCTL] |= E1000_RCTL_EN | E1000_RCTL_BAM | E1000_RCTL_SECRC | E1000_RCTL_SZ_2048;
	e1000[E1000_RCTL] &= ~E1000_RCTL_LPE;
	e1000[E1000_RCTL] |= E1000_RCTL_LBM_NO;
	e1000[E1000_RCTL] &= ~E1000_RCTL_RDMTS;
	e1000[E1000_RCTL] &= ~E1000_RCTL_MO;

	return 0;
}

int e1000_transmit(const char* data, int len) {

	if ( len > TX_PKTSIZE ) return -E_PKT_LONG;
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

int e1000_receive(char* data, int* len) {

	uint32_t rdt = (e1000[E1000_RDT] + 1) % E1000_RXDESC;		
	if (!(rx_queue[rdt].status & E1000_RXD_STAT_DD)) return -E_NO_FREE;

	*len = rx_queue[rdt].length;
	memmove(data, rx_pkt_bufs[rdt].pkt, *len);

	//reset DD bit
	rx_queue[rdt].status &= ~E1000_RXD_STAT_DD;
	rx_queue[rdt].status &= ~E1000_RXD_STAT_EOP;
	e1000[E1000_RDT] = rdt;
	
	return 0;
}
