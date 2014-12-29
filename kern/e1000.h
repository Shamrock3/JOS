#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#endif	// JOS_KERN_E1000_H
#include <kern/pci.h>
#include <inc/stdio.h>
#include <kern/pmap.h>

#define DEV_ID_E1000    0x100E
#define VEN_ID_E1000	0x8086

// Amount of memory
#define E1000_TXDESC 	64
#define PKTSIZE 	1518

// MMIO E1000 registers, divided by 4 for use as uint32_t[] indices.
#define E1000_STATUS   (0x00008/4)  /* Device Status - RO */
#define E1000_TCTL     (0x00400/4)  /* TX Control - RW */
#define E1000_TCTL_EXT (0x00404/4)  /* Extended TX Control - RW */
#define E1000_TIPG     (0x00410/4)  /* TX Inter-packet gap -RW */
#define E1000_TDBAL    (0x03800/4)  /* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH    (0x03804/4)  /* TX Descriptor Base Address High - RW */
#define E1000_TDLEN    (0x03808/4)  /* TX Descriptor Length - RW */
#define E1000_TDH      (0x03810/4)  /* TX Descriptor Head - RW */
#define E1000_TDT      (0x03818/4)  /* TX Descripotr Tail - RW */

//Transmit control bits
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */

//Transmit Descriptor bits
#define E1000_TXD_CMD_RS     0x00000008 /* Report Status */
#define E1000_TXD_CMD_EOP    0x00000001 /* End of Packet */
#define E1000_TXD_STAT_DD    0x00000001 /* Descriptor Done */

//Transmission Descriptor
struct tx_desc
{
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

//wrapper for packet
struct packet {
	uint8_t pkt[PKTSIZE];
};

volatile uint32_t* e1000;
int pci_network_attach(struct pci_func *pcif);
int e1000_transmit(const char* msg, int len);
