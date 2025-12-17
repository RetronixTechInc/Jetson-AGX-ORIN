// SPDX-License-Identifier: GPL-2.0
/*
 * Marvell 88Q2XXX automotive 100BASE-T1/1000BASE-T1 PHY driver
 *
 * Derived from Marvell Q222x API
 *
 * Copyright (C) 2024 Liebherr-Electronics and Drives GmbH
 */
#include <linux/ethtool_netlink.h>
#include <linux/hwmon.h>
#include <linux/marvell_phy.h>
#include <linux/of.h>
#include <linux/phy.h>

#define MDIO_MMD_AN_MV_STAT				32769
#define MDIO_MMD_AN_MV_STAT_ANEG			0x0100
#define MDIO_MMD_AN_MV_STAT_LOCAL_RX			0x1000
#define MDIO_MMD_AN_MV_STAT_REMOTE_RX			0x2000
#define MDIO_MMD_AN_MV_STAT_LOCAL_MASTER		0x4000
#define MDIO_MMD_AN_MV_STAT_MS_CONF_FAULT		0x8000

#define MDIO_MMD_AN_MV_STAT2				32794
#define MDIO_MMD_AN_MV_STAT2_AN_RESOLVED		0x0800
#define MDIO_MMD_AN_MV_STAT2_100BT1			0x2000
#define MDIO_MMD_AN_MV_STAT2_1000BT1			0x4000

#define MDIO_MMD_PCS_MV_RESET_CTRL			32768
#define MDIO_MMD_PCS_MV_RESET_CTRL_TX_DISABLE		0x8

#define MDIO_MMD_PCS_MV_INT_EN				32784
#define MDIO_MMD_PCS_MV_INT_EN_LINK_UP			0x0040
#define MDIO_MMD_PCS_MV_INT_EN_LINK_DOWN		0x0080
#define MDIO_MMD_PCS_MV_INT_EN_100BT1			0x1000

#define MDIO_MMD_PCS_MV_GPIO_INT_STAT			32785
#define MDIO_MMD_PCS_MV_GPIO_INT_STAT_LINK_UP		0x0040
#define MDIO_MMD_PCS_MV_GPIO_INT_STAT_LINK_DOWN		0x0080
#define MDIO_MMD_PCS_MV_GPIO_INT_STAT_100BT1_GEN	0x1000

#define MDIO_MMD_PCS_MV_GPIO_INT_CTRL			32787
#define MDIO_MMD_PCS_MV_GPIO_INT_CTRL_TRI_DIS		0x0800

#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL			32790
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LED_1_MASK	GENMASK(7, 4)
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LED_0_MASK	GENMASK(3, 0)
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LINK		0x0 /* Link established */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LINK_RX_TX	0x1 /* Link established, blink for rx or tx activity */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LINK_1000BT1	0x2 /* Blink 3x for 1000BT1 link established */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_RX_TX_ON		0x3 /* Receive or transmit activity */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_RX_TX		0x4 /* Blink on receive or transmit activity */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_TX		0x5 /* Transmit activity */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LINK_COPPER	0x6 /* Copper Link established */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_LINK_1000BT1_ON	0x7 /* 1000BT1 link established */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_FORCE_OFF		0x8 /* Force off */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_FORCE_ON		0x9 /* Force on */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_FORCE_HIGHZ	0xa /* Force Hi-Z */
#define MDIO_MMD_PCS_MV_LED_FUNC_CTRL_FORCE_BLINK	0xb /* Force blink */

#define MDIO_MMD_PCS_MV_TEMP_SENSOR1			32833
#define MDIO_MMD_PCS_MV_TEMP_SENSOR1_RAW_INT		0x0001
#define MDIO_MMD_PCS_MV_TEMP_SENSOR1_INT		0x0040
#define MDIO_MMD_PCS_MV_TEMP_SENSOR1_INT_EN		0x0080

#define MDIO_MMD_PCS_MV_TEMP_SENSOR2			32834
#define MDIO_MMD_PCS_MV_TEMP_SENSOR2_DIS_MASK		0xc000

#define MDIO_MMD_PCS_MV_TEMP_SENSOR3			32835
#define MDIO_MMD_PCS_MV_TEMP_SENSOR3_INT_THRESH_MASK	0xff00
#define MDIO_MMD_PCS_MV_TEMP_SENSOR3_MASK		0x00ff

#define MDIO_MMD_PCS_MV_100BT1_STAT1			33032
#define MDIO_MMD_PCS_MV_100BT1_STAT1_IDLE_ERROR		0x00ff
#define MDIO_MMD_PCS_MV_100BT1_STAT1_JABBER		0x0100
#define MDIO_MMD_PCS_MV_100BT1_STAT1_LINK		0x0200
#define MDIO_MMD_PCS_MV_100BT1_STAT1_LOCAL_RX		0x1000
#define MDIO_MMD_PCS_MV_100BT1_STAT1_REMOTE_RX		0x2000
#define MDIO_MMD_PCS_MV_100BT1_STAT1_LOCAL_MASTER	0x4000

#define MDIO_MMD_PCS_MV_100BT1_STAT2			33033
#define MDIO_MMD_PCS_MV_100BT1_STAT2_JABBER		0x0001
#define MDIO_MMD_PCS_MV_100BT1_STAT2_POL		0x0002
#define MDIO_MMD_PCS_MV_100BT1_STAT2_LINK		0x0004
#define MDIO_MMD_PCS_MV_100BT1_STAT2_ANGE		0x0008

#define MDIO_MMD_PCS_MV_100BT1_INT_EN			33042
#define MDIO_MMD_PCS_MV_100BT1_INT_EN_LINKEVENT		0x0400

#define MDIO_MMD_PCS_MV_COPPER_INT_STAT			33043
#define MDIO_MMD_PCS_MV_COPPER_INT_STAT_LINKEVENT	0x0400

#define MDIO_MMD_PCS_MV_RX_STAT				33328

#define MDIO_MMD_PCS_MV_TDR_RESET			65226
#define MDIO_MMD_PCS_MV_TDR_RESET_TDR_RST		0x1000

#define MDIO_MMD_PCS_MV_TDR_OFF_SHORT_CABLE		65241

#define MDIO_MMD_PCS_MV_TDR_OFF_LONG_CABLE		65242

#define MDIO_MMD_PCS_MV_TDR_STATUS			65245
#define MDIO_MMD_PCS_MV_TDR_STATUS_MASK			0x0003
#define MDIO_MMD_PCS_MV_TDR_STATUS_OFF			0x0001
#define MDIO_MMD_PCS_MV_TDR_STATUS_ON			0x0002
#define MDIO_MMD_PCS_MV_TDR_STATUS_DIST_MASK		0xff00
#define MDIO_MMD_PCS_MV_TDR_STATUS_VCT_STAT_MASK	0x00f0
#define MDIO_MMD_PCS_MV_TDR_STATUS_VCT_STAT_SHORT	0x0030
#define MDIO_MMD_PCS_MV_TDR_STATUS_VCT_STAT_OPEN	0x00e0
#define MDIO_MMD_PCS_MV_TDR_STATUS_VCT_STAT_OK		0x0070
#define MDIO_MMD_PCS_MV_TDR_STATUS_VCT_STAT_IN_PROGR	0x0080
#define MDIO_MMD_PCS_MV_TDR_STATUS_VCT_STAT_NOISE	0x0050

#define MDIO_MMD_PCS_MV_TDR_OFF_CUTOFF			65246

#define MV88Q2XXX_LED_INDEX_TX_ENABLE			0
#define MV88Q2XXX_LED_INDEX_GPIO			1

struct mmd_val {
	int devad;
	u32 regnum;
	u16 val;
};

static const struct mmd_val mv88q2110_init_seq0[] = {
	{ MDIO_MMD_PCS, 0xffe4, 0x07b5 },
	{ MDIO_MMD_PCS, 0xffe4, 0x06b6 },
};

static const struct mmd_val mv88q2110_init_seq1[] = {
	{ MDIO_MMD_PCS, 0xffde, 0x402f },
	{ MDIO_MMD_PCS, 0xfe34, 0x4040 },
	{ MDIO_MMD_PCS, 0xfe2a, 0x3c1d },
	{ MDIO_MMD_PCS, 0xfe34, 0x0040 },
	{ MDIO_MMD_AN, 0x8032, 0x0064 },
	{ MDIO_MMD_AN, 0x8031, 0x0a01 },
	{ MDIO_MMD_AN, 0x8031, 0x0c01 },
	{ MDIO_MMD_PCS, 0xffdb, 0x0010 },
};

struct mv88q2xxx_priv {
    u32 trylink;     /* 1 : enable link try; other : disable link try */
    u32 baset1_mode; /* 0 is master/100; 1 is master/1000; 2 is slave/100; 3 is slave/1000; other : HW define */
    u32 tx_delay;    /* 0 : disalbe delay; 1 : enable dealy; other : HW define */
    u32 rx_delay;    /* 0 : disalbe delay; 1 : enable dealy; other : HW define */
    u32 trycount;
};


/**
 * genphy_c45_pma_baset1_read_abilities - read supported baset1 link modes from PMA
 * @phydev: target phy_device struct
 *
 * Read the supported link modes from the extended BASE-T1 ability register
 */
int genphy_c45_pma_baset1_read_abilities(struct phy_device *phydev)
{
	int val;

	val = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_PMD_BT1);
	if (val < 0)
		return val;

	linkmode_mod_bit(ETHTOOL_LINK_MODE_100baseT1_Full_BIT,
			 phydev->supported,
			 val & MDIO_PMA_PMD_BT1_B100_ABLE);

	linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT1_Full_BIT,
			 phydev->supported,
			 val & MDIO_PMA_PMD_BT1_B1000_ABLE);

	val = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_T1_STAT);
	if (val < 0)
		return val;

	linkmode_mod_bit(ETHTOOL_LINK_MODE_Autoneg_BIT,
			 phydev->supported,
			 val & MDIO_AN_STAT1_ABLE);

	return 0;
}

/* Read master/slave preference from registers.
 * The preference is read from the BIT(4) of BASE-T1 AN
 * advertisement register 7.515 and whether the preference
 * is forced or not, it is read from BASE-T1 AN advertisement
 * register 7.514.
 */
int genphy_c45_baset1_read_status(struct phy_device *phydev)
{
	int ret;
	int cfg;

	phydev->master_slave_get = MASTER_SLAVE_CFG_UNKNOWN;
	phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;

	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_T1_ADV_L);
	if (ret < 0)
		return ret;

	cfg = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_AN_T1_ADV_M);
	if (cfg < 0)
		return cfg;

	if (ret & MDIO_AN_T1_ADV_L_FORCE_MS) {
		if (cfg & MDIO_AN_T1_ADV_M_MST)
			phydev->master_slave_get = MASTER_SLAVE_CFG_MASTER_FORCE;
		else
			phydev->master_slave_get = MASTER_SLAVE_CFG_SLAVE_FORCE;
	} else {
		if (cfg & MDIO_AN_T1_ADV_M_MST)
			phydev->master_slave_get = MASTER_SLAVE_CFG_MASTER_PREFERRED;
		else
			phydev->master_slave_get = MASTER_SLAVE_CFG_SLAVE_PREFERRED;
	}

	return 0;
}

static int mv88q2xxx_write_mmd_vals(struct phy_device *phydev,
				    const struct mmd_val *vals, size_t len)
{
	int ret;

	for (; len; vals++, len--) {
		ret = phy_write_mmd(phydev, vals->devad, vals->regnum,
				    vals->val);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int mv88q2xxx_read_link_gbit(struct phy_device *phydev)
{
	int ret;
	bool link = false;

	/* Read vendor specific Auto-Negotiation status register to get local
	 * and remote receiver status according to software initialization
	 * guide. However, when not in polling mode the local and remote
	 * receiver status are not evaluated due to the Marvell 88Q2xxx APIs.
	 */
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_MMD_AN_MV_STAT);
	if (ret < 0) {
		return ret;
	} else if (((ret & MDIO_MMD_AN_MV_STAT_LOCAL_RX) &&
		   (ret & MDIO_MMD_AN_MV_STAT_REMOTE_RX)) ||
		   !phy_polling_mode(phydev)) {
		/* The link state is latched low so that momentary link
		 * drops can be detected. Do not double-read the status
		 * in polling mode to detect such short link drops except
		 * the link was already down.
		 */
		if (!phy_polling_mode(phydev) || !phydev->link) {
			ret = phy_read_mmd(phydev, MDIO_MMD_PCS,
					   MDIO_PCS_1000BT1_STAT);
			if (ret < 0)
				return ret;
			else if (ret & MDIO_PCS_1000BT1_STAT_LINK)
				link = true;
		}

		if (!link) {
			ret = phy_read_mmd(phydev, MDIO_MMD_PCS,
					   MDIO_PCS_1000BT1_STAT);
			if (ret < 0)
				return ret;
			else if (ret & MDIO_PCS_1000BT1_STAT_LINK)
				link = true;
		}
	}

	phydev->link = link;

	return 0;
}

static int mv88q2xxx_read_link_100m(struct phy_device *phydev)
{
	int ret;

	/* The link state is latched low so that momentary link
	 * drops can be detected. Do not double-read the status
	 * in polling mode to detect such short link drops except
	 * the link was already down. In case we are not polling,
	 * we always read the realtime status.
	 */
	if (!phy_polling_mode(phydev)) {
		phydev->link = false;
		ret = phy_read_mmd(phydev, MDIO_MMD_PCS,
				   MDIO_MMD_PCS_MV_100BT1_STAT2);
		if (ret < 0)
			return ret;

		if (ret & MDIO_MMD_PCS_MV_100BT1_STAT2_LINK)
			phydev->link = true;

		return 0;
	} else if (!phydev->link) {
		ret = phy_read_mmd(phydev, MDIO_MMD_PCS,
				   MDIO_MMD_PCS_MV_100BT1_STAT1);
		if (ret < 0)
			return ret;
		else if (ret & MDIO_MMD_PCS_MV_100BT1_STAT1_LINK)
			goto out;
	}

	ret = phy_read_mmd(phydev, MDIO_MMD_PCS, MDIO_MMD_PCS_MV_100BT1_STAT1);
	if (ret < 0)
		return ret;

out:
	/* Check if we have link and if the remote and local receiver are ok */
	if ((ret & MDIO_MMD_PCS_MV_100BT1_STAT1_LINK) &&
	    (ret & MDIO_MMD_PCS_MV_100BT1_STAT1_LOCAL_RX) &&
	    (ret & MDIO_MMD_PCS_MV_100BT1_STAT1_REMOTE_RX))
		phydev->link = true;
	else
		phydev->link = false;

	return 0;
}

static int mv88q2xxx_read_link(struct phy_device *phydev)
{
	/* The 88Q2XXX PHYs do not have the PMA/PMD status register available,
	 * therefore we need to read the link status from the vendor specific
	 * registers depending on the speed.
	 */

	if (phydev->speed == SPEED_1000)
		return mv88q2xxx_read_link_gbit(phydev);
	else if (phydev->speed == SPEED_100)
		return mv88q2xxx_read_link_100m(phydev);

	phydev->link = false;
	return 0;
}

static int mv88q2xxx_read_master_slave_state(struct phy_device *phydev)
{
	int ret;

	phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_MMD_AN_MV_STAT);
	if (ret < 0)
		return ret;

	if (ret & MDIO_MMD_AN_MV_STAT_LOCAL_MASTER)
		phydev->master_slave_state = MASTER_SLAVE_STATE_MASTER;
	else
		phydev->master_slave_state = MASTER_SLAVE_STATE_SLAVE;

	return 0;
}

static int mv88q2xxx_read_aneg_speed(struct phy_device *phydev)
{
	int ret;

	phydev->speed = SPEED_UNKNOWN;
	ret = phy_read_mmd(phydev, MDIO_MMD_AN, MDIO_MMD_AN_MV_STAT2);
	if (ret < 0)
		return ret;

	if (!(ret & MDIO_MMD_AN_MV_STAT2_AN_RESOLVED))
		return 0;

	if (ret & MDIO_MMD_AN_MV_STAT2_100BT1)
		phydev->speed = SPEED_100;
	else if (ret & MDIO_MMD_AN_MV_STAT2_1000BT1)
		phydev->speed = SPEED_1000;

	return 0;
}

/*
 * autioneg default is enable. 
 * speed default is unknow then will be 1000 after init.
 * 
 */
static int mv88q2xxx_probe(struct phy_device *phydev)
{   
	struct mv88q2xxx_priv *priv;
    struct device_node *node = phydev->mdio.dev.of_node;
    u32 index;
    int ret;

    //~ phydev->autoneg = AUTONEG_DISABLE;  //預設為enable, 在 init 做：Fail, 原因不明
    printk(KERN_NOTICE "***** [%d]===%s *****\n", __LINE__, __func__);
    
	priv = devm_kzalloc(&phydev->mdio.dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;
    
    if (!of_property_read_u32(node, "trylink", &index))
    {
        priv->trylink = index;
    }else{
        priv->trylink = 0xFF;
    }
    
    if (!of_property_read_u32(node, "baset1_mode", &index))
    {
        priv->baset1_mode = index;
    }else{
        priv->baset1_mode = 0xFF;
    }
    
    if (!of_property_read_u32(node, "tx-delay", &index)) {
		priv->tx_delay = index;
    }else{
        priv->tx_delay = 0xFF;
	}

    if (!of_property_read_u32(node, "rx-delay", &index)) {
		priv->rx_delay = index;
    }else{
        priv->rx_delay = 0xFF;
	}

//~ printk(KERN_INFO "***** [%d]===%s *****%d, %d, %d, %d\n", __LINE__, __func__, priv->trylink, priv->baset1_mode, priv->tx_delay, priv->rx_delay);
  
	return 0;
}

static int mv88q2xxx_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_c45_pma_read_abilities(phydev);
	if (ret)
		return ret;

	/* We need to read the baset1 extended abilities manually because the
	 * PHY does not signalize it has the extended abilities register
	 * available.
	 */
	ret = genphy_c45_pma_baset1_read_abilities(phydev);
	if (ret)
		return ret;

	return 0;
}

static int mv88q2110_config_init(struct phy_device *phydev)
{
    struct mv88q2xxx_priv *priv = phydev->priv;
    u16 ms_sel, speed_sel, tdelay;
    bool DT_set = true;
	int ret;

    switch(priv->baset1_mode)
    {
        case 0:
            ms_sel = 0x4000;    /* Set as Master */
            speed_sel = 0x0;    /* Set type selection 100 */
            //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0200);
            break;
        case 1:
            ms_sel = 0x4000;    /* Set as Master */
            speed_sel = 0x1;    /* Set type selection 1000 */
            //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0000);
            break;
        case 2:
            ms_sel = 0x0000;    /* Set as Slave */
            speed_sel = 0x0;    /* Set type selection 100 */
            //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0200);
            break;
        case 3:
            ms_sel = 0x0000;    /* Set as Slave */
            speed_sel = 0x1;    /* Set type selection 1000 */
            //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0000);
            break;
        default:
            DT_set = false;
            break;
    }
    
    if( DT_set )
    {
        ret = phy_modify_mmd(phydev, 1, 0x834, 0x400F, ms_sel|speed_sel);
        printk(KERN_INFO "mv88q2xxx %s mode,default is %s %d Mbps\n"
        ,priv->trylink == 1?"trylink":"fixed", ms_sel==0x4000?"Master":"Slave", speed_sel==0x0?100:1000);
    }
    
    //~ TX[15] and RX[14] delay.
    DT_set = false;
    if (priv->tx_delay == 0) {
        tdelay = 0x0000;
        DT_set = true;
    }else if (priv->tx_delay == 1) {
        tdelay = 0x8000;
        DT_set = true;
	}
    if( DT_set )
    {
        ret = phy_modify_mmd(phydev, 31, 0x8001, 0x8000, tdelay);
        printk(KERN_INFO "mv88q2xxx set tx clk delay : %s \n",tdelay==0x0?"false":"true");
    }

    DT_set = false;
    if (priv->rx_delay == 0) {
        tdelay = 0x0000;
        DT_set = true;
    }else if (priv->rx_delay == 1) {
        tdelay = 0x4000;
        DT_set = true;
	}
    if( DT_set )
    {
        ret = phy_modify_mmd(phydev, 31, 0x8001, 0x4000, tdelay);
        printk(KERN_INFO "mv88q2xxx set rx clk delay : %s \n",tdelay==0x0?"false":"true");
    }
    
	ret = mv88q2xxx_write_mmd_vals(phydev, mv88q2110_init_seq0,
				       ARRAY_SIZE(mv88q2110_init_seq0));
	if (ret < 0)
		return ret;

	usleep_range(5000, 10000);

	ret = mv88q2xxx_write_mmd_vals(phydev, mv88q2110_init_seq1,
				       ARRAY_SIZE(mv88q2110_init_seq1));
	if (ret < 0)
		return ret;

    //~ Disable TX Disable feature. TX_ENABLE pin for LED[0]
    ret = phy_clear_bits_mmd(phydev, MDIO_MMD_PCS,
                 MDIO_MMD_PCS_MV_RESET_CTRL,
                 MDIO_MMD_PCS_MV_RESET_CTRL_TX_DISABLE);
    if (ret < 0)
        return ret;

printk(KERN_INFO "=== 31.8001=%x\n",phy_read_mmd(phydev, 31, 0x8001));

	return 0;
}

static int mv88q2xxx_config_aneg(struct phy_device *phydev)
{
    struct mv88q2xxx_priv *priv = phydev->priv;
	int ret;

    if (priv->trylink == 1) {
        /* Link try process enable. */
        phydev->autoneg = AUTONEG_ENABLE;
        priv->trycount = 0;
    }

	ret = genphy_c45_config_aneg(phydev);
	if (ret)
		return ret;

	return phydev->drv->soft_reset(phydev);
}

static int mv88q2xxx_read_status(struct phy_device *phydev)
{
    struct mv88q2xxx_priv *priv = phydev->priv;
    u16 ms_sel, speed_sel;
	int ret, loop;

	if (phydev->autoneg == AUTONEG_ENABLE) {
        switch(priv->baset1_mode)
        {
            case 0:
                ms_sel = 0x4000;    /* Set as Master */
                speed_sel = 0x0;    /* Set type selection 100 */
                //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0200);
                break;
            case 1:
                ms_sel = 0x4000;    /* Set as Master */
                speed_sel = 0x1;    /* Set type selection 1000 */
                //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0000);
                break;
            case 2: //***** 88Q211x Not support this mode. *****
                ms_sel = 0x0000;    /* Set as Slave */
                speed_sel = 0x0;    /* Set type selection 100 */
                //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0200);
                break;
            case 3:
                ms_sel = 0x0000;    /* Set as Slave */
                speed_sel = 0x1;    /* Set type selection 1000 */
                //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0000);
                break;
            default:
                ms_sel = 0x4000;    /* Set as Master */
                speed_sel = 0x0;    /* Set type selection 100 */
                //~ phy_modify_mmd(phydev, 3, 0x8100, 0x0200, 0x0200);
                break;
        }

        ret = phy_modify_mmd(phydev, 1, 0x834, 0x400F, ms_sel|speed_sel);
        msleep(10);
        
		/* We have to get the negotiated speed first, otherwise we are
		 * not able to read the link.
		 */
		ret = mv88q2xxx_read_aneg_speed(phydev);
		if (ret < 0)
			return ret;

		ret = mv88q2xxx_read_link(phydev);
		if (ret < 0)
			return ret;

		ret = genphy_c45_read_lpa(phydev);
		if (ret < 0)
			return ret;

		ret = genphy_c45_baset1_read_status(phydev);
		if (ret < 0)
			return ret;

		ret = mv88q2xxx_read_master_slave_state(phydev);
		if (ret < 0)
			return ret;

		phy_resolve_aneg_linkmode(phydev);

        for(loop = 0; loop < 10; loop++)
        {
            ret = phy_read_mmd(phydev, 7, 0x8016);
            if( (ret & 0x4000) == 0x4000 )
            {
                phydev->autoneg = AUTONEG_DISABLE;
                printk(KERN_INFO "mv88q2xxx set %s %d Mbps\n", ms_sel==0x4000?"Master":"Slave", speed_sel==0x0?100:1000);
                break;
            }
            msleep(1);
        }
        
        if (phydev->autoneg == AUTONEG_ENABLE)
        {
            priv->baset1_mode += 1;
            if(priv->baset1_mode > 3)
            {
                priv->baset1_mode = 0;
            }
        }

        //~ printk(KERN_INFO "=== 1.901=%x, 3.8100=%x, 3.8109=%x \n",phy_read_mmd(phydev, 1, 0x901),phy_read_mmd(phydev, 3, 0x8100),phy_read_mmd(phydev, 3, 0x8109));
        //~ printk(KERN_INFO "***** [%d]===%s *****link = %d, %d, %d\n", __LINE__, __func__, phydev->link, phydev->speed, phydev->master_slave_state);

		return 0;
	}

	ret = mv88q2xxx_read_link(phydev);
    ret = mv88q2xxx_read_master_slave_state(phydev);
    //~ printk(KERN_INFO "=== 1.901=%x, 3.8100=%x, 3.8109=%x \n"
    //~ ,phy_read_mmd(phydev, 1, 0x901),phy_read_mmd(phydev, 3, 0x8100),phy_read_mmd(phydev, 3, 0x8109));
    if(!phydev->link)
    {
        priv->trycount += 1;
        //~ printk(KERN_INFO "***** [%d]===%s *****trycount = %d\n", __LINE__, __func__, priv->trycount);
        if(priv->trycount > 3)
        {
            mv88q2xxx_config_aneg(phydev);
        }
    }
    
    //~ printk(KERN_INFO "***** [%d]===%s *****link = %d, %d, %d\n", __LINE__, __func__, phydev->link, phydev->speed, phydev->master_slave_state);

	return genphy_c45_read_pma(phydev);
}

static int mv88q2xxx_soft_reset(struct phy_device *phydev)
{
	int ret;
	int val;

//~ printk(KERN_INFO "***** [%d]===%s *****auto = %d, speed = %d\n", __LINE__, __func__, phydev->autoneg, phydev->speed);
	/* Enable RESET of DCL */
	if (phydev->autoneg == AUTONEG_ENABLE || phydev->speed == SPEED_1000) {
		ret = phy_write_mmd(phydev, MDIO_MMD_PCS, 0xfe1b, 0x48);
		if (ret < 0)
			return ret;
	}

	ret = phy_write_mmd(phydev, 3, 0x900, 0x8000);
	if (ret < 0)
		return ret;

	ret = phy_read_mmd_poll_timeout(phydev, MDIO_MMD_PCS,
					MDIO_PCS_1000BT1_CTRL, val,
					!(val & MDIO_PCS_1000BT1_CTRL_RESET),
					50000, 600000, true);
	if (ret < 0)
		return ret;

	ret = phy_write_mmd(phydev, MDIO_MMD_PCS, 0xffe4, 0xc);
	if (ret < 0)
		return ret;

	/* Disable RESET of DCL */
	if (phydev->autoneg == AUTONEG_ENABLE || phydev->speed == SPEED_1000)
		return phy_write_mmd(phydev, MDIO_MMD_PCS, 0xfe1b, 0x58);

	return 0;
}

static struct phy_driver mv88q2xxx_driver[] = {
	{
		.phy_id			= MARVELL_PHY_ID_88Q2110,
		.phy_id_mask	= MARVELL_PHY_ID_MASK,
		.name			= "mv88q2110",
        .probe          = mv88q2xxx_probe,
		.get_features	= mv88q2xxx_get_features,
		.config_init	= mv88q2110_config_init,
		.config_aneg	= mv88q2xxx_config_aneg,
		.read_status	= mv88q2xxx_read_status,
		.soft_reset		= mv88q2xxx_soft_reset,
	},
};

module_phy_driver(mv88q2xxx_driver);

static const struct mdio_device_id __maybe_unused mv88q2xxx_tbl[] = {
	{ MARVELL_PHY_ID_88Q2110, MARVELL_PHY_ID_MASK },
	{ /*sentinel*/ }
};
MODULE_DEVICE_TABLE(mdio, mv88q2xxx_tbl);

MODULE_DESCRIPTION("Marvell 88Q2XXX 100/1000BASE-T1 Automotive Ethernet PHY driver");
MODULE_LICENSE("GPL");
