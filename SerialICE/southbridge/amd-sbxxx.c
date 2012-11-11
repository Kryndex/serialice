/*
 * SerialICE
 *
 * Copyright (C) 2012 Rudolf Marek <r.marek@assembler.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* This initializes AMD SBxx that
 * o Ports 0x2e/0x2f 0x4e/0x4f and 0x3f8 are routed to superio
 * o the MMIO regs of SB are accesible
 * o enables POST card (see #if 0)
 */

#define MMIO_NON_POSTED_START 0xfed00000
#define MMIO_NON_POSTED_END   0xfedfffff
#define SB_MMIO 0xFED80000
#define SB_MMIO_MISC32(x) *(volatile u32 *)(SB_MMIO + 0xE00 + (x))

static void sbxxx_enable_48mhzout(void)
{
	/* most likely programming to 48MHz out signal */
	u32 reg32;
	reg32 = SB_MMIO_MISC32(0x28);
	reg32 &= 0xffc7ffff;
	reg32 |= 0x00100000;
	SB_MMIO_MISC32(0x28) = reg32;

	reg32 = SB_MMIO_MISC32(0x40);
	reg32 &= ~0x80u;
	SB_MMIO_MISC32(0x40) = reg32;
}

static void southbridge_init(void)
{
	u16 reg16;
	u32 reg32;

	/* route FED00000 - FEDFFFFF as non-posted to SB */
	pci_write_config32(PCI_ADDR(0, 0x18, 1, 0x84),
			   (((MMIO_NON_POSTED_END & ~0xffffu)  >> 8) | (1 << 7)));
	/* lowest NP address is HPET at FED00000 */
	pci_write_config32(PCI_ADDR(0, 0x18, 1, 0x80),
			    (MMIO_NON_POSTED_START >> 8) | 3);

	/* Send all IO (0000-FFFF) to southbridge. */
	pci_write_config32(PCI_ADDR(0, 0x18, 1, 0xc4),  0x0000f000);
	pci_write_config32(PCI_ADDR(0, 0x18, 1, 0xc0),  0x00000003);

	/* SB  MMIO range decode enable */
	outb(0x24, 0xcd6);
	outb(0x1, 0xcd7);

	/* Enable LPC decoding of 0x2e/0x2f, 0x4e/0x4f 0x3f8  */
	pci_write_config8(PCI_ADDR(0, 0x14, 3, 0x44), (1<<6));
	pci_write_config8(PCI_ADDR(0, 0x14, 3, 0x48), (1 << 1) | (1 << 0));
}
