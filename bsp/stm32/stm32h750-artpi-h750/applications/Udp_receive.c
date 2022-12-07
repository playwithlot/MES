/**
 * @file Udp_receive.c
 * @author King (lzj..@com)
 * @brief udp广播，单播，组播监听处理。主要用于设备发现,平时命令交互应该尽量用TCP。
 * @version 0.1
 * @date 2021-09-15
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include "iot_parameter.h"
#include "Udp_receive.h"

#define LOG_TAG "udp.server"
#define LOG_LVL LOG_LVL_DBG // LOG_LVL_INFO
#include <ulog.h>

//组播地址
#define IPADDR_MULTICAST_STR "224.4.5.6"

#define NET_RECV_BUF_SIZE (1500) //接收缓存大小

//全局缓冲 用于网络收包, stm32h750需要注意此缓存的地址
// uint8_t NetRecvBuf[NET_RECV_BUF_SIZE];
uint8_t *NetRecvBuf;
// UDP收包处理结构 网络收包完成后，复制数据过去
//Net_pack_deal_t udp_deal;

//共四个socket，分别为普通udp，广播，组播.
int gSocketNormal, gSocketBC, gSocketMC;

/////////////////////////////////////////////////////////////////////////////////////////
// UDP通信接口初始化
int UdpCommonInit(void)
{
  struct sockaddr_in local_addr = {0};
  int sock;
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == -1)
  {
    return -1;
  }

  // maxsock++;
  //设置本地UDP地址
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  local_addr.sin_port = htons(iot_para.udp_recv_port);

  //绑定UDP socket
  if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
  {
    closesocket(sock);
    return -1;
  }

  return sock;
}

//广播接口初始化
int UdpBroadCastInit(void)
{
  struct sockaddr_in broadcast_addr = {0};
  int sock;
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == -1)
  {
    return -1;
  }

  //设置本地广播地址
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  broadcast_addr.sin_port = htons(iot_para.udp_recv_bc_port);
  // rt_memset(&(broadcast_addr.sin_zero), 0, sizeof(broadcast_addr.sin_zero));

  //绑定Broadcast socket
  if (bind(sock, (struct sockaddr *)&broadcast_addr, sizeof(struct sockaddr)) == -1)
  {
    closesocket(sock);
    return -1;
  }

  return sock;
}

//组播接口初始化
int UdpMultiCastInit(void)
{
  struct ip_mreq multicast;                //组播地址定义 abi 110822
  struct sockaddr_in multicast_addr = {0}; //组播地址
  int sock;
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == -1)
  {
    return -1;
  }

  //设置组地址 并加入组 abi 110822
  multicast.imr_multiaddr.s_addr = inet_addr(IPADDR_MULTICAST_STR); //多播组地址 这个地址可以在inet.h等将去定义好
  multicast.imr_interface.s_addr = htonl(INADDR_ANY);               //设置发送组播消息的源主机的地址信息
  setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&multicast, sizeof(multicast));
  //设置本地组播地址
  //组播地址定义 abi 110822
  multicast_addr.sin_family = AF_INET;
  multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  multicast_addr.sin_port = htons(iot_para.udp_recv_mc_port);

  //绑定multicast socket
  //组播地址定义 abi 110822
  if (bind(sock, (struct sockaddr *)&multicast_addr, sizeof(struct sockaddr)) == -1)
  {
    closesocket(sock);
    return -1;
  }

  return sock;
}

/*
 *此函数是UDP的接收和处理执行函数
 *函数名：Udp_Receive_Handle
 *参数：paramemter
 *用法：用于UDP socket的创建，包括单播，广播和组播，协议的处理，
 *
 */
int Udp_Receive_Handle(void *args)
{
  uint8_t ip[4];
  int recvlen, ret;
  uint16_t len;
  int sendlen;
  rt_uint32_t addr_len;           //系统类型，所以不用int
  fd_set rfd;                     //字符集
  struct sockaddr_in remote_addr; //本地地址
  int32_t maxfd;
  struct timeval timeout; //超时时间

  //创建一个udp socket，类型是SOCK DGRAM，UDP类型
  gSocketNormal = UdpCommonInit();
  if (gSocketNormal == -1)
  {
    return -1;
  }
  //创建一个broadcast socket，类型是SOCK DGRAM，UDP类型
  gSocketBC = UdpBroadCastInit();
  if (gSocketBC == -1)
  {
    closesocket(gSocketNormal);
    return -1;
  }
  //创建一个multicast socket，类型是SOCK DGRAM，UDP类型
  gSocketMC = UdpMultiCastInit();
  if (gSocketMC == -1)
  {
    closesocket(gSocketNormal);
    closesocket(gSocketBC);
    return -1;
  }
  //长度
  addr_len = sizeof(struct sockaddr);
  maxfd = (gSocketNormal > gSocketBC) ? gSocketNormal : gSocketBC;
  maxfd = (maxfd > gSocketMC) ? maxfd : gSocketMC;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  while (1)
  {
    recvlen = 0;
    //开始使用select//在使用之前总是要清空
    FD_ZERO(&rfd);
    FD_SET(gSocketNormal, &rfd);                     //把sock放入要测试的描述符集中
    FD_SET(gSocketBC, &rfd);                         //把sockbc放入要测试的描述符集中
    FD_SET(gSocketMC, &rfd);                         //把sockmc放入要测试的描述符集中 abi
    rt_memset(&NetRecvBuf[0], 0, NET_RECV_BUF_SIZE); //清空缓冲区
    //检测是否有套接口是否可读
    ret = select(maxfd + 1, &rfd, RT_NULL, RT_NULL, &timeout);
    if (ret == -1)
    {
      break;
    }
    else if (ret == 0)
    {
      rt_thread_delay(Msec(10)); //增加，给线程调度
      continue;
    }
	
	#if 1
    else //有套接字活跃
    {

      recvlen = 0;
      if (FD_ISSET(gSocketNormal, &rfd))
      { // sock 可读,UDP协议 	单播数据
        recvlen = recvfrom(gSocketNormal, &NetRecvBuf[0], NET_RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);
        rt_memcpy(&ip[0], &remote_addr.sin_addr.s_addr, 4);
        LOG_I("udp recv ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }
      if (FD_ISSET(gSocketBC, &rfd))
      { // sockbc可读，UDP Broadcast 广播数据
        recvlen = recvfrom(gSocketBC, &NetRecvBuf[0], NET_RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);
        rt_memcpy(&ip[0], &remote_addr.sin_addr.s_addr, 4);
        LOG_I("udp bc recv ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }
      if (FD_ISSET(gSocketMC, &rfd))
      { // sockmc 可读,UDP协议 组播数据
        recvlen = recvfrom(gSocketMC, &NetRecvBuf[0], NET_RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);
        rt_memcpy(&ip[0], &remote_addr.sin_addr.s_addr, 4);
        LOG_I("udp mc recv ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }
      //接收网络数据，发送消息队列
      if (recvlen <= 0)
      { //网络不通
        break;
      }
      else
      { //成功接收到数据
		  
		  
		  
		  
		  
		 
		  
		  
		  
//        LOG_HEX("recv", 30, NetRecvBuf, recvlen);
//        //进行协议处理
//        len = net_pro_recv(&udp_deal, &udp_deal.err);
//        if (len > 0)
//        {
//          /*
//          remote_addr.sin_family = AF_INET;
//          rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
//          remote_addr.sin_addr.s_addr = udp_deal.dest_ip;
//          remote_addr.sin_port = udp_deal.dest_port;
//          sock = udp_deal.sock_fd;
//          //发送数据
//          sendlen = sendto(sock, udp_deal.pack, udp_deal.data_len, 0,
//                           (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
//          */
//          sendlen = net_udp_send(&udp_deal);
//          if (sendlen <= 0)
//          {
//            break;
//          }
//        }
		
		
		
      }
    }
	
	#endif
	
  }
  closesocket(gSocketNormal);
  closesocket(gSocketBC);
  closesocket(gSocketMC);
  return -1;
}

//int net_udp_send(Net_pack_deal_t *deal)
//{
//  int sendlen = 0;
//  struct sockaddr_in remote_addr; //本地地址

//  if (deal->transfer_type == TRANSFER_TYPE_udp ||
//      deal->transfer_type == TRANSFER_TYPE_udp_broadcast ||
//      deal->transfer_type == TRANSFER_TYPE_udp_mutibroadcast)
//  {
//    remote_addr.sin_family = AF_INET;
//    rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
//    remote_addr.sin_addr.s_addr = deal->dest_ip;
//    remote_addr.sin_port = deal->dest_port;
//    sendlen = sendto(deal->sock_fd, deal->pack, deal->data_len, 0,
//                     (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
//    if (sendlen <= 0)
//    { //发送不成功
//      LOG_E("Send fail\r\n");
//    }
//  }
//  return sendlen;
//}

void udp_recv_thread(void *args)
{
  int val;
  rt_uint8_t retryCnt = 0;

  //stm32h750的网络较特殊，需要使用eth的DMA支持的地址作为缓存。
  NetRecvBuf = rt_malloc(NET_RECV_BUF_SIZE);
  if (!NetRecvBuf)
  {
    LOG_E("udp recv buf err");
    while (1)
    {
      rt_thread_delay(Sec(500));
    }
  }
  rt_thread_delay(Msec(500));
  LOG_I("udp recive thread running");
  while (1)
  {
    do
    {
      val = Udp_Receive_Handle(args);
    } while ((val == -1) && (retryCnt++ < 3));
    rt_thread_delay(Msec(100));
    LOG_E("get Udp_Receive port failed.");
  }
}


#if 0
uint16_t npl_get_iot_infor(Net_pack_deal_t *deal, uint32_t *err)
{
    uint16_t ret_len;
    uint8_t addr_temp;
    uint16_t para_len = (uint32_t)&iot_para.udp_recv_port - (uint32_t)&iot_para;

    LOG_I("get iot information");
    *err = NET_CMD_ERR_succeed;
    //应上位机要求，命令打破常规通信机制。不回应答，直接回结果，结果带数据
    addr_temp = deal->pack->dest_addr;
    if (deal->pack->src_addr == 0)
    {
        deal->pack->src_addr = 0;
    }
    else
    {
        deal->pack->src_addr = iot_para.net_addr;
    }
    deal->pack->dest_addr = addr_temp;
    rt_memcpy(&deal->pack->buf[0], &iot_para, para_len);
    deal->pack->type = NET_PACK_TYPE_result;
    ret_len = sizeof(Net_pack_t) + para_len;
    deal->data_len = ret_len;
    return ret_len;
}


static uint16_t npl_set_iot_infor(Net_pack_deal_t *deal, uint32_t *err)
{
    uint16_t ret_len;
    uint16_t para_len = (uint32_t)&iot_para.udp_recv_port - (uint32_t)&iot_para;
    Net_pack_ack_t *ack = (Net_pack_ack_t *)deal->pack;
    uint8_t i;
    Iot_parameter_t *p = (Iot_parameter_t *)&deal->pack->buf[0];

    ret_len = sizeof(Net_pack_ack_t);
    LOG_I("set iot information");
    for (i = 0; i < sizeof(iot_para.uid); i++)
    {
        if (iot_para.uid[i] != p->uid[i]) // uid与本机地址不符，不理会
        {
            LOG_I("uid err, ignore cmd");
            return 0;
        }
    }
    //对相关参数的范围等进行限制。
    if (deal->pack->len < para_len || iot_para_is_legal(p) == RT_FALSE)
    {
        LOG_I("some para err");
        *err = NET_CMD_ERR_para;
        ack->err = *err;
        return ret_len;
    }
    deal->pack->type = NET_PACK_TYPE_result;
    //只保存部分参数，uid、软硬件版本等不支持更改
    //也许先对比一下设置值与当前值会更好，如果没改变，就不需要重启。
    iot_para.net_addr = p->net_addr;
    rt_memcpy(&iot_para.name, &p->name, sizeof(iot_para.name));
    rt_memcpy(&iot_para.ip_addr, &p->ip_addr, sizeof(iot_para.ip_addr));
    rt_memcpy(&iot_para.gw_addr, &p->gw_addr, sizeof(iot_para.gw_addr));
    rt_memcpy(&iot_para.netmask, &p->netmask, sizeof(iot_para.netmask));
    rt_memcpy(&iot_para.mac, &p->mac, sizeof(iot_para.mac));
    iot_para.local_tcp_server_port = p->local_tcp_server_port;
    iot_para.tcp_log_port = p->tcp_log_port;
    rt_memcpy(&iot_para.tcp_log_server_addr, &p->tcp_log_server_addr, sizeof(iot_para.tcp_log_server_addr));
    iot_para_save(&iot_para);
    LOG_I("save iot para");
    // ip或端口改变时，则需要重启才能生效。由其它线程处理重启，此处只置位重启标志即可。
    iot_para.status.is_need_reboot = RT_TRUE;
    *err = NET_CMD_ERR_succeed;
    para_len = sizeof(iot_para.uid);
    //应上位机要求，此命令打破常规通信机制。不回应答，直接回结果。
    rt_memcpy(&ack->buf[0], &iot_para.uid[0], para_len);
    deal->pack->src_addr = 0;
    deal->pack->type = NET_PACK_TYPE_result;
    ack->err = *err;
    ret_len = sizeof(Net_pack_ack_t) + para_len;
    return ret_len;
}

//组播的处理函数
net_pro_local_recv_deal

#endif

