// SPDX-License-Identifier: GPL-2.0
/*
 * PS4 Baikal DWMAC1000 Ethernet glue driver
 *
 * The Baikal southbridge exposes a Synopsys DWMAC1000 ethernet controller
 * whose registers live within the glue device's (func 4) BAR2 at offset
 * 0x10A000. The GBE PCI function's own BAR0 (4KB) is NOT used.
 *
 * Sony uses a custom MDIO controller at MAC base + 0x2880/0x2884 with a
 * non-standard 16-bit protocol (different from DWMAC1000's GMII at +0x10).
 *
 * RE from Orbis 12.50 kernel (if_mts.c / mtsc_pci / baikal_pcie.c)
 *
 * Copyright (c) 2026 Armandas Kvietkus <armandas.kvietkus@proton.me>
 */

#include <linux/pci.h>
#include <linux/pci_ids.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/phy.h>

#include <asm/ps4.h>

#include "stmmac.h"
#include "stmmac_libpci.h"

/* GBE DWMAC1000 registers within glue BAR2 */
#define BAIKAL_GBE_REGS_OFF	0x10A000
#define BAIKAL_GBE_REGS_SIZE	0x4000

/* Sony custom registers (offsets from GBE base) */
#define BAIKAL_GBE_RESET	0x0200

/* Sony custom MDIO controller (offsets from GBE base) */
#define BAIKAL_MDIO_CMD		0x2880
#define BAIKAL_MDIO_DATA	0x2884

/* MDIO command register bits */
#define BAIKAL_MDIO_CMD_PHY_SHIFT	11
#define BAIKAL_MDIO_CMD_REG_SHIFT	6
#define BAIKAL_MDIO_CMD_READ		BIT(5)
#define BAIKAL_MDIO_CMD_DONE		BIT(4)
#define BAIKAL_MDIO_CMD_BUSY		BIT(3)

/* DMA Bus Mode register */
#define BAIKAL_DMA_BUS_MODE	0x1000
#define BAIKAL_DMA_BUS_MODE_SWR	BIT(0)

/* MAC address: func 6 BAR5 + 0x2f000 */
#define BAIKAL_SPM_BP_OFF	0x2f000

#define BAIKAL_MDIO_TIMEOUT_US	100000

struct dwmac_ps4_priv {
	void __iomem *gbe_base;		/* BAR2 + 0x10A000 (MAC/DMA regs) */
	void __iomem *bar2_base;	/* BAR2 base (unused now) */
	void __iomem *mdio_base;	/* GBE BAR0 (Sony MDIO controller) */
	struct pci_dev *glue_dev;
	struct mii_bus *mii_bus;
};

/* --- Sony custom MDIO protocol ---
 *
 * There are two MDIO interfaces on Baikal:
 * 1. Sony custom MDIO at GBE_base + 0x00 (used by Orbis mts driver internally)
 * 2. Standard-like MDIO at BAR2 + 0x2880/0x2884 (used by Orbis miibus)
 *
 * Testing shows BAR2+0x2880 returns all zeros for PHY registers.
 * The Sony custom MDIO at GBE_base+0x00 is what actually talks to the PHY.
 *
 * Sony custom MDIO protocol (from Orbis RE at VA 0x977760):
 * - Wait: read 32-bit from GBE_base+0x00, check bit 3 (busy)
 * - Write cmd: (phy_addr << 11) | (reg << 6) | 0x20 (read) or 0x00 (write)
 *   Written as 16-bit to GBE_base+0x00 (via BAR2 vaddr + offset)
 * - Wait done: read 16-bit from same register, check bit 4 (done)
 * - Read data: read 16-bit from GBE_base+0x04 (data register)
 *
 * But the Orbis miibus at VA 0x977760 accesses BAR2+0x2880 directly
 * (not through the +0x10A000 helper). So BAR2+0x2880 IS the correct
 * miibus MDIO. The PHY just isn't responding because it needs power/init.
 *
 * For the Sony custom MDIO write function (VA 0x3d34d0 / 0xa524d0):
 * - Write 0x8000 to GBE_base+0x00 (start/busy)
 * - Write (data_hi16 | (reg & 0x1f) << 8 | 0x20) to GBE_base+0x00
 * - Poll GBE_base+0x00 for completion
 * - Read result from GBE_base+0x04
 *
 * Let's try using the Sony custom MDIO (GBE_base+0x00/0x04) as our bus.
 */

#define BAIKAL_SONY_MDIO_REG	0x00	/* Sony MDIO control (from GBE base) */
#define BAIKAL_SONY_MDIO_DATA	0x04	/* Sony MDIO data (from GBE base) */

static int dwmac_ps4_mdio_wait_idle(void __iomem *base)
{
	/* The BAR0 MDIO register doesn't clear bit 15 after completion.
	 * A fixed delay is sufficient — hardware completes in ~10us.
	 */
	udelay(50);
	return 0;
}

/* wait_done is the same as wait_idle */
#define dwmac_ps4_mdio_wait_done dwmac_ps4_mdio_wait_idle

static int dwmac_ps4_mdio_read(struct mii_bus *bus, int phyaddr, int phyreg)
{
	struct dwmac_ps4_priv *priv = bus->priv;
	void __iomem *base = priv->mdio_base;
	u32 cmd, val;
	int ret;

	/* Sony custom MDIO read via GBE BAR0:
	 * 1. Write 0x8000 to clear/reset the register
	 * 2. Write (reg << 8) | 0x4000 (read command)
	 * 3. Poll until bit 15 clears
	 * 4. Data is in upper 16 bits of the register
	 */
	writel(0x8000, base + BAIKAL_SONY_MDIO_REG);
	udelay(10);

	cmd = ((phyreg & 0x1f) << 8) | 0x4000;
	writel(cmd, base + BAIKAL_SONY_MDIO_REG);

	ret = dwmac_ps4_mdio_wait_idle(base);
	if (ret) {
		if (phyaddr == 0 && phyreg == 0)
			dev_err(bus->parent, "MDIO done timeout, reg=0x%08x\n",
				readl(base + BAIKAL_SONY_MDIO_REG));
		return ret;
	}

	val = readl(base + BAIKAL_SONY_MDIO_REG);
	return (val >> 16) & 0xffff;
}

static int dwmac_ps4_mdio_write(struct mii_bus *bus, int phyaddr, int phyreg,
				u16 phydata)
{
	struct dwmac_ps4_priv *priv = bus->priv;
	void __iomem *base = priv->mdio_base;
	u32 cmd;
	int ret;

	/* Sony custom MDIO write via GBE BAR0:
	 * 1. Write 0x8000 to clear/reset the register
	 * 2. Write (data << 16) | (reg << 8) | 0x20
	 * 3. Poll until bit 15 clears
	 */
	writel(0x8000, base + BAIKAL_SONY_MDIO_REG);
	udelay(10);

	cmd = ((u32)phydata << 16) | ((phyreg & 0x1f) << 8) | 0x20;
	writel(cmd, base + BAIKAL_SONY_MDIO_REG);

	ret = dwmac_ps4_mdio_wait_idle(base);
	if (ret)
		return ret;

	return 0;
}

/* --- stmmac platform callbacks --- */

static void dwmac_ps4_get_interfaces(struct stmmac_priv *priv, void *bsp_priv,
				     unsigned long *interfaces)
{
	__set_bit(PHY_INTERFACE_MODE_MII, interfaces);
	__set_bit(PHY_INTERFACE_MODE_GMII, interfaces);
	__set_bit(PHY_INTERFACE_MODE_RGMII, interfaces);
}

/* --- MAC address from SPM boot params --- */

static void dwmac_ps4_get_mac(struct pci_dev *pdev, unsigned char *addr)
{
	struct pci_dev *mem_dev;
	phys_addr_t bar5_start;
	void __iomem *bp;

	mem_dev = pci_get_slot(pdev->bus,
			       PCI_DEVFN(PCI_SLOT(pdev->devfn), 6));
	if (!mem_dev)
		goto random;

	bar5_start = pci_resource_start(mem_dev, 5);
	if (!bar5_start)
		goto put;

	bp = ioremap(bar5_start + BAIKAL_SPM_BP_OFF, 32);
	if (!bp)
		goto put;

	memcpy_fromio(addr, bp, ETH_ALEN);
	iounmap(bp);

	if (is_valid_ether_addr(addr)) {
		pci_dev_put(mem_dev);
		return;
	}
put:
	pci_dev_put(mem_dev);
random:
	eth_random_addr(addr);
}

/* --- Probe --- */

static int dwmac_ps4_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct plat_stmmacenet_data *plat;
	struct stmmac_resources res = {};
	struct dwmac_ps4_priv *priv;
	struct pci_dev *glue_dev;
	struct net_device *ndev;
	struct stmmac_priv *spriv;
	void __iomem *gbe_base;
	void __iomem *bar2_base;
	phys_addr_t bar2_start;
	u8 mac_addr[ETH_ALEN];
	u32 ver;
	int ret;

	dev_info(&pdev->dev, "dwmac-ps4: probing Baikal GBE\n");

	/* Find the glue device (func 4) which owns BAR2 */
	glue_dev = pci_get_slot(pdev->bus,
				PCI_DEVFN(PCI_SLOT(pdev->devfn), 4));
	if (!glue_dev) {
		dev_err(&pdev->dev, "dwmac-ps4: glue device (func 4) not found\n");
		return -ENODEV;
	}

	bar2_start = pci_resource_start(glue_dev, 2);
	if (!bar2_start) {
		dev_err(&pdev->dev, "dwmac-ps4: glue BAR2 not available\n");
		ret = -ENODEV;
		goto out_put_glue;
	}

	ret = pcim_enable_device(pdev);
	if (ret)
		goto out_put_glue;

	pci_set_master(pdev);

	/* Map BAR2 base (kept for reference) */
	bar2_base = ioremap(bar2_start, 0x4000);
	if (!bar2_base) {
		ret = -ENOMEM;
		goto out_put_glue;
	}

	/* Map GBE MAC/DMA registers from glue BAR2 + 0x10A000 */
	gbe_base = ioremap(bar2_start + BAIKAL_GBE_REGS_OFF, BAIKAL_GBE_REGS_SIZE);
	if (!gbe_base) {
		ret = -ENOMEM;
		goto out_unmap_bar2;
	}

	/* Sanity check */
	ver = readl(gbe_base + 0x20);
	dev_info(&pdev->dev, "dwmac-ps4: regs[0x00]=0x%08x [0x20]=0x%08x [0x1000]=0x%08x\n",
		 readl(gbe_base), ver, readl(gbe_base + BAIKAL_DMA_BUS_MODE));

	if (ver == 0xffffffff) {
		dev_err(&pdev->dev, "dwmac-ps4: hardware not responding\n");
		ret = -ENODEV;
		goto out_unmap;
	}

	/* Sony reset: write 0 to custom reset register */
	writel(0, gbe_base + BAIKAL_GBE_RESET);
	usleep_range(2000, 3000);

	/*
	 * Sony custom MDIO PHY init sequence (from Orbis RE):
	 * The PHY won't respond on the standard MDIO bus (BAR2+0x2880)
	 * until it's been initialized through the Sony custom MDIO
	 * controller at GBE_base+0x00.
	 *
	 * The Orbis kernel writes 0x9 to GBE offset 0xAC (link config),
	 * then uses the custom MDIO to write calibration data.
	 * For now, just do the link config write and a basic PHY reset
	 * through the custom MDIO to wake the PHY up.
	 */

	/* Note: ICC GBE power command (5,0x08) was tested but returns error
	 * (reply 01 05) and kills WiFi. PHY responds through BAR0 without it.
	 */

	writel(0x9, gbe_base + 0xAC);
	usleep_range(1000, 2000);

	/* Sony custom MDIO: write PHY reset via GBE_base+0x00
	 * Protocol: write 0x8000 (start), then write command word:
	 *   bits[31:16] = data, bits[12:8] = reg, bit[5] = execute
	 * To write BMCR reset (reg 0, data 0x8000):
	 *   cmd = (0x8000 << 16) | (0 << 8) | 0x20 = 0x80000020
	 */
	writel(0x8000, gbe_base + 0x00);
	usleep_range(100, 200);
	writel(0x80000020, gbe_base + 0x00);  /* BMCR reset via Sony MDIO */
	msleep(50);

	/* Write auto-negotiation enable + restart (reg 0, data 0x1200):
	 *   cmd = (0x1200 << 16) | (0 << 8) | 0x20 = 0x12000020
	 */
	writel(0x8000, gbe_base + 0x00);
	usleep_range(100, 200);
	writel(0x12000020, gbe_base + 0x00);
	msleep(50);

	/* Debug: dump MDIO controller state (at BAR2 + 0x2880) */
	dev_info(&pdev->dev, "dwmac-ps4: MDIO @ BAR2+0x2880: cmd=0x%04x data=0x%04x\n",
		 readw(bar2_base + BAIKAL_MDIO_CMD),
		 readw(bar2_base + BAIKAL_MDIO_DATA));

	/* Dump GBE base registers 0x00-0x10 to find the real MDIO */
	dev_info(&pdev->dev, "dwmac-ps4: GBE[0x00]=0x%08x [0x04]=0x%08x [0x08]=0x%08x [0x0c]=0x%08x\n",
		 readl(gbe_base + 0x00), readl(gbe_base + 0x04),
		 readl(gbe_base + 0x08), readl(gbe_base + 0x0c));
	dev_info(&pdev->dev, "dwmac-ps4: GBE[0x10]=0x%08x [0x14]=0x%08x [0x50]=0x%08x [0xAC]=0x%08x\n",
		 readl(gbe_base + 0x10), readl(gbe_base + 0x14),
		 readl(gbe_base + 0x50), readl(gbe_base + 0xAC));

	/* Try a Sony MDIO read and show what happens */
	{
		u32 before, after_start, after_cmd;
		void __iomem *gbe_bar0;
		phys_addr_t bar0_start;

		before = readl(gbe_base + 0x00);
		writel(0x8000, gbe_base + 0x00);
		udelay(10);
		after_start = readl(gbe_base + 0x00);
		writel(0x4000 | (2 << 8), gbe_base + 0x00); /* read PHY ID reg 2 */
		udelay(100);
		after_cmd = readl(gbe_base + 0x00);
		dev_info(&pdev->dev, "dwmac-ps4: MDIO test GBE_base+0: before=0x%08x after_start=0x%08x after_cmd=0x%08x\n",
			 before, after_start, after_cmd);

		/* Also try the GBE device's own BAR0 - MDIO might be there */
		bar0_start = pci_resource_start(pdev, 0);
		if (bar0_start) {
			gbe_bar0 = ioremap(bar0_start, 0x1000);
			if (gbe_bar0) {
				dev_info(&pdev->dev, "dwmac-ps4: GBE BAR0 @ 0x%pa: [0x00]=0x%08x [0x04]=0x%08x [0x08]=0x%08x [0x0c]=0x%08x\n",
					 &bar0_start,
					 readl(gbe_bar0 + 0x00), readl(gbe_bar0 + 0x04),
					 readl(gbe_bar0 + 0x08), readl(gbe_bar0 + 0x0c));
				/* Try MDIO read from BAR0 */
				writel(0x8000, gbe_bar0 + 0x00);
				udelay(10);
				writel(0x4000 | (2 << 8), gbe_bar0 + 0x00);
				udelay(100);
				dev_info(&pdev->dev, "dwmac-ps4: GBE BAR0 MDIO test: result=0x%08x\n",
					 readl(gbe_bar0 + 0x00));
				iounmap(gbe_bar0);
			}
		}
	}

	/* Allocate IRQ via bpcie domain */
	ret = bpcie_assign_irqs(pdev, 1);
	if (ret < 0) {
		dev_err(&pdev->dev, "dwmac-ps4: bpcie_assign_irqs failed: %d\n", ret);
		goto out_unmap;
	}

	/* Allocate private data */
	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto out_unmap;
	}
	priv->gbe_base = gbe_base;
	priv->bar2_base = bar2_base;
	priv->glue_dev = glue_dev;

	/* Map GBE device's own BAR0 for Sony MDIO controller */
	{
		phys_addr_t bar0_start = pci_resource_start(pdev, 0);
		if (!bar0_start) {
			dev_err(&pdev->dev, "dwmac-ps4: GBE BAR0 not available\n");
			ret = -ENODEV;
			goto out_unmap;
		}
		priv->mdio_base = ioremap(bar0_start, 0x1000);
		if (!priv->mdio_base) {
			ret = -ENOMEM;
			goto out_unmap;
		}
	}

	/* Register custom MDIO bus */
	priv->mii_bus = mdiobus_alloc();
	if (!priv->mii_bus) {
		ret = -ENOMEM;
		goto out_unmap;
	}

	priv->mii_bus->name = "dwmac-ps4-mdio";
	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "dwmac-ps4-%x",
		 pci_dev_id(pdev));
	priv->mii_bus->priv = priv;
	priv->mii_bus->parent = &pdev->dev;
	priv->mii_bus->read = dwmac_ps4_mdio_read;
	priv->mii_bus->write = dwmac_ps4_mdio_write;

	ret = mdiobus_register(priv->mii_bus);
	if (ret) {
		dev_err(&pdev->dev, "dwmac-ps4: MDIO bus registration failed: %d\n", ret);
		goto out_mdio_free;
	}

	/* Dump PHY state for debugging */
	{
		int r0, r1, r2, r3, r4, r5, r15, r16, r17;
		r0 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 0);  /* BMCR */
		r1 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 1);  /* BMSR */
		r2 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 2);  /* PHY ID1 */
		r3 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 3);  /* PHY ID2 */
		r4 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 4);  /* ANAR */
		r5 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 5);  /* ANLPAR */
		r15 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 15); /* Extended status */
		r16 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 16); /* Vendor specific */
		r17 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 17); /* Vendor specific */
		dev_info(&pdev->dev, "dwmac-ps4: PHY regs: BMCR=0x%04x BMSR=0x%04x ID=0x%04x:%04x\n",
			 r0 < 0 ? 0xffff : r0, r1 < 0 ? 0xffff : r1,
			 r2 < 0 ? 0xffff : r2, r3 < 0 ? 0xffff : r3);
		dev_info(&pdev->dev, "dwmac-ps4: PHY regs: ANAR=0x%04x ANLPAR=0x%04x R15=0x%04x R16=0x%04x R17=0x%04x\n",
			 r4 < 0 ? 0xffff : r4, r5 < 0 ? 0xffff : r5,
			 r15 < 0 ? 0xffff : r15, r16 < 0 ? 0xffff : r16,
			 r17 < 0 ? 0xffff : r17);

		/* Also scan other PHY addresses in case it's not at 0 */
		{
			int addr, id1, id2;
			for (addr = 1; addr < 4; addr++) {
				id1 = dwmac_ps4_mdio_read(priv->mii_bus, addr, 2);
				id2 = dwmac_ps4_mdio_read(priv->mii_bus, addr, 3);
				if (id1 > 0 || id2 > 0)
					dev_info(&pdev->dev, "dwmac-ps4: PHY at addr %d: ID=0x%04x:%04x\n",
						 addr, id1, id2);
			}
		}

		/* If PHY ID is zero, try soft reset */
		if (r2 <= 0 && r3 <= 0) {
			dev_info(&pdev->dev, "dwmac-ps4: PHY ID=0, attempting BMCR reset\n");
			dwmac_ps4_mdio_write(priv->mii_bus, 0, 0, 0x8000);
			msleep(100);
			r0 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 0);
			r1 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 1);
			r2 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 2);
			r3 = dwmac_ps4_mdio_read(priv->mii_bus, 0, 3);
			dev_info(&pdev->dev, "dwmac-ps4: After reset: BMCR=0x%04x BMSR=0x%04x ID=0x%04x:%04x\n",
				 r0 < 0 ? 0xffff : r0, r1 < 0 ? 0xffff : r1,
				 r2 < 0 ? 0xffff : r2, r3 < 0 ? 0xffff : r3);
		}
	}

	/* Allocate platform data */
	plat = stmmac_plat_dat_alloc(&pdev->dev);
	if (!plat) {
		ret = -ENOMEM;
		goto out_mdio_unreg;
	}

	plat->dma_cfg = devm_kzalloc(&pdev->dev, sizeof(*plat->dma_cfg),
				     GFP_KERNEL);
	if (!plat->dma_cfg) {
		ret = -ENOMEM;
		goto out_mdio_unreg;
	}

	/* DWMAC1000 configuration — skip stmmac's internal MDIO */
	plat->mdio_bus_data = NULL;
	plat->clk_csr = 2;
	plat->core_type = DWMAC_CORE_GMAC;
	plat->force_sf_dma_mode = 1;
	plat->bus_id = pci_dev_id(pdev);
	plat->phy_addr = 0;
	plat->phy_interface = PHY_INTERFACE_MODE_GMII;
	plat->get_interfaces = dwmac_ps4_get_interfaces;
	plat->dma_cfg->pbl = 8;
	plat->dma_cfg->fixed_burst = 1;
	plat->bsp_priv = priv;

	/* Get MAC address from SPM */
	dwmac_ps4_get_mac(pdev, mac_addr);

	/* Set up resources */
	res.addr = gbe_base;
	res.irq = pdev->irq;
	res.wol_irq = pdev->irq;
	memcpy(res.mac, mac_addr, ETH_ALEN);

	dev_info(&pdev->dev, "dwmac-ps4: MAC %pM, IRQ %d, regs at %pa+0x%x\n",
		 mac_addr, pdev->irq, &bar2_start, BAIKAL_GBE_REGS_OFF);

	ret = stmmac_dvr_probe(&pdev->dev, plat, &res);
	if (ret) {
		dev_err(&pdev->dev, "dwmac-ps4: stmmac_dvr_probe failed: %d\n", ret);
		goto out_mdio_unreg;
	}

	/* Connect our custom MDIO bus to stmmac so PHY scan works on open */
	ndev = dev_get_drvdata(&pdev->dev);
	spriv = netdev_priv(ndev);
	spriv->mii = priv->mii_bus;

	return 0;

out_mdio_unreg:
	mdiobus_unregister(priv->mii_bus);
out_mdio_free:
	mdiobus_free(priv->mii_bus);
out_unmap:
	iounmap(gbe_base);
out_unmap_bar2:
	iounmap(bar2_base);
out_put_glue:
	pci_dev_put(glue_dev);
	return ret;
}

static void dwmac_ps4_remove(struct pci_dev *pdev)
{
	struct net_device *ndev = dev_get_drvdata(&pdev->dev);
	struct stmmac_priv *spriv;
	struct dwmac_ps4_priv *priv;

	if (!ndev)
		return;

	spriv = netdev_priv(ndev);
	priv = spriv->plat->bsp_priv;

	stmmac_dvr_remove(&pdev->dev);

	if (priv) {
		if (priv->mii_bus) {
			mdiobus_unregister(priv->mii_bus);
			mdiobus_free(priv->mii_bus);
		}
		if (priv->gbe_base)
			iounmap(priv->gbe_base);
		if (priv->mdio_base)
			iounmap(priv->mdio_base);
		if (priv->bar2_base)
			iounmap(priv->bar2_base);
		if (priv->glue_dev)
			pci_dev_put(priv->glue_dev);
	}
}

static const struct pci_device_id dwmac_ps4_id_table[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_SONY, PCI_DEVICE_ID_SONY_BAIKAL_GBE) },
	{}
};
MODULE_DEVICE_TABLE(pci, dwmac_ps4_id_table);

static struct pci_driver dwmac_ps4_driver = {
	.name		= "dwmac-ps4",
	.id_table	= dwmac_ps4_id_table,
	.probe		= dwmac_ps4_probe,
	.remove		= dwmac_ps4_remove,
	.driver		= {
		.pm	= &stmmac_simple_pm_ops,
	},
};
module_pci_driver(dwmac_ps4_driver);

MODULE_DESCRIPTION("PS4 Baikal DWMAC1000 Ethernet glue driver");
MODULE_LICENSE("GPL");
