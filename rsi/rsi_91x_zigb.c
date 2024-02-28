// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright 2020-2023 Silicon Labs, Inc.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include "rsi_main.h"
#include "rsi_zigb.h"

#define RSI_ZIGB_GENL_FAMILY "Obx-ZIGBgenl"

#define RSI_USER_A_MAX (__RSI_USER_A_MAX - 1)
#define RSI_VERSION_NR 1

static struct genl_cb *global_gcb;

struct rsi_proto_ops *g_proto_ops;

/*
 * attribute policy: defines which attribute has
 * which type (e.g int, char * etc)
 * possible values defined in net/netlink.h
 */
static struct nla_policy zigb_genl_policy[RSI_USER_A_MAX + 1] = {
  [RSI_USER_A_MSG] = { .type = NLA_NUL_STRING },
};

static struct genl_ops zigb_genl_ops = {
  .cmd   = RSI_USER_C_CMD,
  .flags = 0,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0))
  .policy = zigb_genl_policy,
#endif
  .doit   = zigb_genl_recv,
  .dumpit = NULL,
};

struct rsi_mod_ops rsi_zb_ops = {
  .attach   = rsi_zigb_attach,
  .detach   = rsi_zigb_detach,
  .recv_pkt = rsi_zigb_recv_pkt,
};

u16 rsi_zb_zone_enabled = (ERR_ZONE | 0);

void rsi_zb_dbg(u32 zone, const char *fmt, ...)
{
  struct va_format vaf;
  va_list args;

  va_start(args, fmt);

  vaf.fmt = fmt;
  vaf.va  = &args;

  if (zone & rsi_zb_zone_enabled)
    pr_info("%pV", &vaf);
  va_end(args);
}

/* This function gets command request from user space over netlink socket */
int zigb_genl_recv(struct sk_buff *skb, struct genl_info *info)
{
  struct rsi_zb_adapter *zb_adapter;
  u8 *data;
  u16 desc;
  int rc = -1, len, pkttype;
  struct genl_cb *gcb;
  struct nlattr *na;
  u32 dword_align_req_bytes = 0;

  gcb = global_gcb;
  if (!gcb) {
    rsi_zb_dbg(ERR_ZONE, "%s: NULL gcb\n", __func__);
    return -EINVAL;
  }
  zb_adapter = (struct rsi_zb_adapter *)global_gcb->gc_drvpriv;
  if (!zb_adapter) {
    rsi_zb_dbg(ERR_ZONE, "%s: NULL zb_adapter\n", __func__);
    return -EINVAL;
  }

  gcb->gc_info = info;
  gcb->gc_skb  = skb;

  if (!info) {
    rsi_zb_dbg(ERR_ZONE, "%s: NULL info\n", __func__);
    goto err;
  }

  if (!gcb->gc_done) {
    rsi_zb_dbg(ERR_ZONE, "%s: NULL gcb->gc_done\n", __func__);
    goto err;
  }

  gcb->gc_pid = get_portid(info);
  gcb->gc_seq = info->snd_seq;

  na = info->attrs[RSI_USER_A_MSG];
  if (na) {
    data = (u8 *)nla_data(na);
    if (!data) {
      rsi_zb_dbg(ERR_ZONE, "%s: no data recevied on family `%s'\n", __func__, gcb->gc_name);
      goto err;
    }
  } else {
    rsi_zb_dbg(ERR_ZONE, "%s: netlink attr is NULL on family `%s'\n", __func__, gcb->gc_name);
    goto err;
  }

  if (!data) {
    rsi_zb_dbg(ERR_ZONE, "%s: NULL data\n", __func__);
    goto err;
  }

  gcb->gc_info = NULL;
  gcb->gc_skb  = NULL;

  desc    = *(u16 *)&data[0];
  len     = desc & 0x0FFF;
  pkttype = ((desc & 0xF000) >> 12);

  rsi_zb_dbg(INFO_ZONE, "%s: rx data, desc %x len %x pkttype %x\n", __func__, desc, len, pkttype);

  skb = dev_alloc_skb(len + FRAME_DESC_SZ + DWORD_ALIGN_SZ);
  if (!skb) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to alloc skb\n", __func__);
    return -ENOMEM;
  }
  memset(skb->data, 0, FRAME_DESC_SZ);
  skb_reserve(skb, FRAME_DESC_SZ + DWORD_ALIGN_SZ);
  dword_align_req_bytes = ((unsigned long)skb->data) & 0x3f;
  if (dword_align_req_bytes)
    skb_push(skb, dword_align_req_bytes);
  skb_put(skb, len);
  memcpy(skb->data, data, skb->len);
  if (g_proto_ops->coex_send_pkt)
    g_proto_ops->coex_send_pkt(zb_adapter->priv, skb, RSI_ZIGB_Q);
  rsi_zb_dbg(INFO_ZONE, "%s: sent the coex_send_pkt\n", __func__, rc);

  return 0;

err:
  rsi_zb_dbg(ERR_ZONE, "%s: error(%d) occured\n", __func__, rc);
  return rc;
}

/* This function register the network device */
static int register_dev(struct net_device *dev)
{
  if (rtnl_is_locked())
    return register_netdevice(dev);
  else
    return register_netdev(dev);
}

/*This function unregisters the network device &
  returns it back to the kernel */
void unregister_dev(struct net_device *dev)
{
  if (rtnl_is_locked()) {
    unregister_netdevice(dev);
  } else {
    unregister_netdev(dev);
    free_netdev(dev);
  }
}

static int zigb_deregister_fw(void *priv)
{
  struct rsi_zb_adapter *zb_adapter = (struct rsi_zb_adapter *)g_proto_ops->get_zb_context(priv);
  u8 *frame_desc;
  struct sk_buff *skb       = NULL;
  int status                = 0;
  u32 dword_align_req_bytes = 0;

  rsi_zb_dbg(INFO_ZONE, "===> Deregister ZIGB FW <=== \n");

  skb = dev_alloc_skb(FRAME_DESC_SZ + DWORD_ALIGN_SZ);
  if (!skb) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to alloc skb\n", __func__);
    return -ENOMEM;
  }
  skb_reserve(skb, DWORD_ALIGN_SZ);
  dword_align_req_bytes = ((unsigned long)skb->data & 0x3f);
  if (dword_align_req_bytes)
    skb_push(skb, dword_align_req_bytes);
  skb_put(skb, FRAME_DESC_SZ);
  memset(skb->data, 0, FRAME_DESC_SZ);

  frame_desc = skb->data;

  frame_desc[12] = 0x0;             /* Sequence Number */
  frame_desc[13] = 0x1;             /* Direction */
  frame_desc[14] = 0x7;             /* Interface */
  frame_desc[15] = ZIGB_DEREGISTER; /* Packet Type */

  if (g_proto_ops->coex_send_pkt)
    g_proto_ops->coex_send_pkt(zb_adapter->priv, skb, RSI_WLAN_Q);
  dev_kfree_skb(skb);

  return status;
}

/* This function is called by OS when we UP the interface */
int rsi_zigb_open(struct net_device *dev)
{
  rsi_zb_dbg(INFO_ZONE, "RSI ZiGB device open...\n");
  dev->flags |= IFF_RUNNING;

  return 0;
}

/* This function is called by OS when the interface status changed to DOWN */
int rsi_zigb_close(struct net_device *dev)
{
  rsi_zb_dbg(INFO_ZONE, "RSI ZigB device close\n", __func__);

  if (!netif_queue_stopped(dev))
    netif_stop_queue(dev);

  return 0;
}

/* This function is used by Os to send packets to driver */
int rsi_zigb_xmit(struct sk_buff *skb, struct net_device *dev)
{
  rsi_zb_dbg(INFO_ZONE, " %s\n", __func__);

  return 0;
}

int rsi_zigb_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
  return 0;
}

int zigb_genl_send(struct genl_cb *gcb, struct sk_buff *skb)
{
  u8 *data;
  u32 len;
  int rc = -1;
  void *hdr;
  struct sk_buff *gskb;
  struct net *net = &init_net;

  if (!gcb || !skb)
    return -EFAULT;

  if (!gcb->gc_done)
    return -ENODEV;

  data = skb->data;
  len  = skb->len;

  if (!data || !len)
    return -ENODATA;

  rsi_zb_dbg(INFO_ZONE,
             "%s: sending data-%p len-%d pid-%d family-`%s'\n",
             __func__,
             data,
             len,
             gcb->gc_pid,
             gcb->gc_name);

  gskb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
  if (!gskb) {
    rsi_zb_dbg(ERR_ZONE, "%s: 'genlmsg_new' failed\n", __func__);
    rc = -ENOMEM;
    goto err;
  }

  hdr = genlmsg_put(gskb, 0, gcb->gc_seq + 1, gcb->gc_family, 0, RSI_USER_C_CMD);
  if (!hdr) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to set msg\n", __func__);
    rc = -EINVAL;
    goto err_fill;
  }

  rc = nla_put(gskb, RSI_USER_A_MAX, len, data);
  if (rc) {
    rsi_zb_dbg(ERR_ZONE, "%s: 'nla_put' fail(%d) family '%s'\n", __func__, rc, gcb->gc_name);
    goto err_fill;
  }
  genlmsg_end(gskb, hdr);
  rc = genlmsg_unicast(net, gskb, gcb->gc_pid);
  if (rc) {
    rsi_zb_dbg(ERR_ZONE, "%s: 'genlmsg_unicast' fail(%d) family '%s'\n", __func__, rc, gcb->gc_name);
    goto err;
  }
  return rc;

err_fill:
  nlmsg_free(gskb);
err:
  return rc;
}

int rsi_zigb_recv_pkt(void *priv, u8 *pkt)
{
  struct rsi_zb_adapter *zb_adapter = g_proto_ops->get_zb_context(priv);
  struct net_device *dev            = NULL;
  struct sk_buff *skb               = NULL;
  struct genl_cb *gcb;
  int status;
  int pkt_len = (le16_to_cpu(*(__le16 *)pkt)) & 0x0fff;

  pkt_len += FRAME_DESC_SZ;
  gcb = global_gcb;
  if (!gcb) {
    rsi_zb_dbg(ERR_ZONE, "NULL GCB\n");
    return -EINVAL;
  }

  if (zb_adapter->fsm_state != RSI_ZB_FSM_DEVICE_READY) {
    rsi_zb_dbg(INFO_ZONE, "ZIGB Device not ready\n");
    return 0;
  }
  skb = dev_alloc_skb(pkt_len);
  if (!skb) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to alloc skb\n", __func__);
    return -ENOMEM;
  }
  dev = zb_adapter->dev;
  memcpy(skb->data, pkt, pkt_len);
  skb_put(skb, pkt_len);

  skb->dev = (void *)dev;
  status   = zigb_genl_send(gcb, skb);
  if (status != 0)
    rsi_zb_dbg(ERR_ZONE, "%s: Failed return form zigb_genl_send \n", __func__);
  dev_kfree_skb(skb);

  return status;
}

#define ASCII_NUMERIC_OFFSET 48
int rsi_zigb_attach(void *priv, struct rsi_proto_ops *ops)
{
  struct rsi_zb_adapter *zb_adapter;
#ifdef CONFIG_RSI_MULTI_MODE
  struct rsi_common *common = (struct rsi_common *)priv;
#endif
  struct net_device *dev;
  struct genl_cb *gcb                  = NULL;
  struct genl_family *zigb_genl_family = NULL;

  static const struct net_device_ops zigb_netdev_ops = {
    .ndo_open       = rsi_zigb_open,
    .ndo_stop       = rsi_zigb_close,
    .ndo_start_xmit = rsi_zigb_xmit,
    .ndo_do_ioctl   = rsi_zigb_ioctl,
  };
  dev = alloc_netdev(sizeof(struct rsi_zb_adapter), "zigb%d", NET_NAME_UNKNOWN, ether_setup);
  if (!dev) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to alloc netdev\n", __func__);
    return -EINVAL;
  }
  dev->hard_header_len = FRAME_DESC_SZ + DWORD_ALIGN_SZ;
  dev->netdev_ops      = &zigb_netdev_ops;

  zb_adapter       = (struct rsi_zb_adapter *)netdev_priv(dev);
  zb_adapter->priv = priv;
  ops->set_zb_context(priv, zb_adapter);
  g_proto_ops           = ops;
  ops->zb_ops           = &rsi_zb_ops;
  zb_adapter->fsm_state = RSI_ZB_FSM_DEVICE_READY;

  if (register_dev(dev) != 0) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to register zigb device\n", __func__);
    goto err;
  }
  zb_adapter->dev = dev;
  memcpy(zb_adapter->mac_addr, device_mac_addr, 6);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 0, 0)
  memcpy(zb_adapter->dev->dev_addr, zb_adapter->mac_addr, 6);
#else
  memcpy((char *)zb_adapter->dev->dev_addr, zb_adapter->mac_addr, 6);
#endif
  zb_adapter->fsm_state = RSI_ZB_FSM_DEVICE_READY;

  gcb = kzalloc(sizeof(*gcb), GFP_KERNEL);
  if (!gcb) {
    rsi_zb_dbg(ERR_ZONE, "%s: Failed to alloc genl control block\n", __func__);
    goto err;
  }
  zb_adapter->gcb = gcb;
  global_gcb      = gcb;

  gcb->gc_drvpriv  = zb_adapter;
  zigb_genl_family = kzalloc(sizeof(*zigb_genl_family), GFP_KERNEL);
  if (!zigb_genl_family)
    goto err;
  zigb_genl_family->hdrsize = 0;
  strncpy(zigb_genl_family->name, RSI_ZIGB_GENL_FAMILY, GENL_NAMSIZ);
  /*
	 * for multiple modules support differentiate the devices with
	 * unique id, here we have obtained that from the common instance
	 * id.
	 */
#ifdef CONFIG_RSI_MULTI_MODE
  /*FIXME*/
  zigb_genl_family->id = (int)(common->priv->drv_instance_index) - 1;
  /* Below 8th byte differentiates family names. */
  zigb_genl_family->name[8] = (char)(zigb_genl_family->id + ASCII_NUMERIC_OFFSET);
#else
  zigb_genl_family->id = 0;
#endif
  zigb_genl_family->version = RSI_VERSION_NR;
  zigb_genl_family->maxattr = RSI_USER_A_MAX;
  gcb->gc_family            = zigb_genl_family;
  gcb->gc_policy            = &zigb_genl_policy[0];
  gcb->gc_ops               = &zigb_genl_ops;
  gcb->gc_n_ops             = 1;
  gcb->gc_name              = RSI_ZIGB_GENL_FAMILY;
  gcb->gc_done              = 0;
  gcb->gc_pid               = gcb->gc_done;

  rsi_zb_dbg(INIT_ZONE, "genl-register: nl_family `%s'\n", gcb->gc_name);

  gcb->gc_family->ops   = gcb->gc_ops;
  gcb->gc_family->n_ops = gcb->gc_n_ops;

  if (genl_register_family(gcb->gc_family)) {
    rsi_zb_dbg(ERR_ZONE, "%s: genl_register_family failed\n", __func__);
    goto err;
  }
  gcb->gc_done = 1;
  rsi_zb_dbg(ERR_ZONE, "RSI ZIGBEE module init done\n");

  return 0;

err:
  if (dev) {
    unregister_dev(dev);
    free_netdev(dev);
    zb_adapter->dev = NULL;
  }

  kfree(zigb_genl_family);

  if (gcb) {
    genl_unregister_family(gcb->gc_family);
    kfree(gcb);
  }
  zb_adapter->gcb = NULL;
  return -EINVAL;
}

/*This function de-initializes zigbee interface*/
void rsi_zigb_detach(void *priv)
{
  struct rsi_zb_adapter *zb_adapter = g_proto_ops->get_zb_context(priv);
  struct net_device *dev            = zb_adapter->dev;
  struct genl_cb *gcb               = NULL;
  int status;
  gcb = zb_adapter->gcb;

  status = zigb_deregister_fw(priv);
  if (!status)
    rsi_zb_dbg(ERR_ZONE, "%s: Failed sending deregister zigb\n", __func__);
  if (dev) {
    unregister_dev(dev);
    zb_adapter->dev = NULL;
  }
  if (gcb) {
    genl_unregister_family(gcb->gc_family);
    kfree(gcb->gc_family);
    kfree(gcb);
  }
  rsi_zb_dbg(ERR_ZONE, "%s: ZiGB detach done\n", __func__);
}

struct rsi_mod_ops *rsi_get_zb_ops(void)
{
  return &rsi_zb_ops;
};
