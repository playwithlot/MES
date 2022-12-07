#include "multicast_cmd_handle.h"


#include <rtthread.h>
#include <sys/socket.h> /* ??BSD socket,????socket.h??? */
#include "iot_parameter.h"
#include "Udp_receive.h"

#define LOG_TAG "udp.server"
#define LOG_LVL LOG_LVL_DBG // LOG_LVL_INFO
#include <ulog.h>



static int udp_sock_fd = -1;
static uint32_t udp_dest_ip = 0;      
static uint16_t udp_dest_port = 0;
static uint8_t udp_transfer_type = 0;

//????
#define IPADDR_MULTICAST_STR "224.4.5.6"
#define NET_RECV_BUF_SIZE (1500) //??????
uint8_t *NetRecvBuf;

static uint8_t *udp_send_buf = RT_NULL;


rt_mutex_t multicast_rsp_mux = RT_NULL; 

//rt_mutex_take(multicast_rsp_mux, RT_WAITING_FOREVER);
//rt_mutex_release(multicast_rsp_mux);

//
extern void network_recv_pack_handle(uint8_t *buf);
extern void network_recv_pack_handle2(uint8_t *buf, uint16_t len);
extern uint16_t cal_network_pack_crc(network_pack_t *pack);




int gSocketNormal, gSocketBC, gSocketMC;

/////////////////////////////////////////////////////////////////////////////////////////
// UDP???????
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
  //????UDP??
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  local_addr.sin_port = htons(iot_para.udp_recv_port);

  //??UDP socket
  if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
  {
    closesocket(sock);
    return -1;
  }

  return sock;
}

//???????
int UdpBroadCastInit(void)
{
  struct sockaddr_in broadcast_addr = {0};
  int sock;
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == -1)
  {
    return -1;
  }

  //????????
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  broadcast_addr.sin_port = htons(iot_para.udp_recv_bc_port);
  // rt_memset(&(broadcast_addr.sin_zero), 0, sizeof(broadcast_addr.sin_zero));

  //??Broadcast socket
  if (bind(sock, (struct sockaddr *)&broadcast_addr, sizeof(struct sockaddr)) == -1)
  {
    closesocket(sock);
    return -1;
  }

  return sock;
}

//???????
int UdpMultiCastInit(void)
{
  struct ip_mreq multicast;                //?????? abi 110822
  struct sockaddr_in multicast_addr = {0}; //????
  int sock;
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (sock == -1)
  {
    return -1;
  }

  //????? ???? abi 110822
  multicast.imr_multiaddr.s_addr = inet_addr(IPADDR_MULTICAST_STR); //????? ???????inet.h??????
  multicast.imr_interface.s_addr = htonl(INADDR_ANY);               //?????????????????
  setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&multicast, sizeof(multicast));
  //????????
  //?????? abi 110822
  multicast_addr.sin_family = AF_INET;
  multicast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  multicast_addr.sin_port = htons(iot_para.udp_recv_mc_port);

  //??multicast socket
  //?????? abi 110822
  if (bind(sock, (struct sockaddr *)&multicast_addr, sizeof(struct sockaddr)) == -1)
  {
    closesocket(sock);
    return -1;
  }

  return sock;
}


extern uint8_t network_get_frame(uint8_t *rbuf, uint16_t rbuf_len, uint8_t *frame, uint16_t *frame_len);
int Udp_Receive_Handle(void *args)
{
  uint8_t ip[4];
  int recvlen, ret;
  uint16_t len;
  int sendlen;
  rt_uint32_t addr_len;           //????,????int
  fd_set rfd;                     //???
  struct sockaddr_in remote_addr; //????
  int32_t maxfd;
  struct timeval timeout; //????
  uint16_t frame_len = 0;

  //????udp socket,???SOCK DGRAM,UDP??
  gSocketNormal = UdpCommonInit();
  if (gSocketNormal == -1)
  {
    return -1;
  }
  //????broadcast socket,???SOCK DGRAM,UDP??
  gSocketBC = UdpBroadCastInit();
  if (gSocketBC == -1)
  {
    closesocket(gSocketNormal);
    return -1;
  }
  //????multicast socket,???SOCK DGRAM,UDP??
  gSocketMC = UdpMultiCastInit();
  if (gSocketMC == -1)
  {
    closesocket(gSocketNormal);
    closesocket(gSocketBC);
    return -1;
  }
  //??
  addr_len = sizeof(struct sockaddr);
  maxfd = (gSocketNormal > gSocketBC) ? gSocketNormal : gSocketBC;
  maxfd = (maxfd > gSocketMC) ? maxfd : gSocketMC;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  while (1)
  {
    recvlen = 0;
    //????select//??????????
    FD_ZERO(&rfd);
    FD_SET(gSocketNormal, &rfd);                     //?sock???????????
    FD_SET(gSocketBC, &rfd);                         //?sockbc???????????
    FD_SET(gSocketMC, &rfd);                         //?sockmc??????????? abi
    rt_memset(&NetRecvBuf[0], 0, NET_RECV_BUF_SIZE); //?????
    //????????????
    ret = select(maxfd + 1, &rfd, RT_NULL, RT_NULL, &timeout);
    if (ret == -1)
    {
      break;
    }
    else if (ret == 0)
    {
      rt_thread_delay(Msec(10)); //??,?????
      continue;
    }
    else //??????
    {

      recvlen = 0;
      if (FD_ISSET(gSocketNormal, &rfd))
      { // sock ??,UDP?? 	????
		 
		rt_mutex_take(multicast_rsp_mux, RT_WAITING_FOREVER); 
        recvlen = recvfrom(gSocketNormal, &NetRecvBuf[0], NET_RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);
		rt_mutex_release(multicast_rsp_mux);
		  
		  rt_memcpy(&ip[0], &remote_addr.sin_addr.s_addr, 4);
		udp_sock_fd = gSocketNormal;
		udp_transfer_type = TRANSFER_TYPE_udp;
        LOG_I("udp recv ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }
	  
      if (FD_ISSET(gSocketBC, &rfd))
      { // sockbc??,UDP Broadcast ????
		rt_mutex_take(multicast_rsp_mux, RT_WAITING_FOREVER); 
        recvlen = recvfrom(gSocketBC, &NetRecvBuf[0], NET_RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);
		rt_mutex_release(multicast_rsp_mux);
		  
		rt_memcpy(&ip[0], &remote_addr.sin_addr.s_addr, 4);
		udp_sock_fd = gSocketBC;   
		udp_transfer_type = TRANSFER_TYPE_udp_broadcast;
        LOG_I("udp bc recv ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }
	  
      if (FD_ISSET(gSocketMC, &rfd))
      { // sockmc ??,UDP?? ????
		rt_mutex_take(multicast_rsp_mux, RT_WAITING_FOREVER); 
        recvlen = recvfrom(gSocketMC, &NetRecvBuf[0], NET_RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t *)&addr_len);
		rt_mutex_release(multicast_rsp_mux);
		  
		rt_memcpy(&ip[0], &remote_addr.sin_addr.s_addr, 4);
		udp_sock_fd = gSocketMC;
		udp_transfer_type = TRANSFER_TYPE_udp_mutibroadcast;
        LOG_I("udp mc recv ip:%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
      }

      if (recvlen <= 0)
      { 
        break;
      }
      else  //recv udp data
      { 
		  
		//udp_deal.pack_len = NET_RECV_BUF_SIZE;
		//udp_deal.data_len = recvlen;
		//udp_deal.dest_ip = remote_addr.sin_addr.s_addr;
		//udp_deal.dest_port = remote_addr.sin_port;
		//udp_deal.pack = (Net_pack_t *)&NetRecvBuf[0];
		  
		udp_dest_ip = remote_addr.sin_addr.s_addr;
		udp_dest_port = remote_addr.sin_port;
		//network_recv_pack_handle(NetRecvBuf);
		  
//		 rt_memcpy(network_recv_buf, NetRecvBuf, recvlen);
//		  
//		  
//		while(1)
//		{
//			if(network_get_frame(network_recv_buf, recvlen, udp_send_buf, &frame_len))
//			{
//				LOG_I("recv frame size %d\n", frame_len);
//			}
//			else
//			{
//				break;
//			}
//			
//			
//		}
					
		 network_recv_pack_handle2(NetRecvBuf, recvlen);
		

		//net_udp_send(&NetRecvBuf)


      }
	  
    }
	
	rt_thread_delay(1); 
	
  }
  closesocket(gSocketNormal);
  closesocket(gSocketBC);
  closesocket(gSocketMC);
  return -1;
  
}




int net_udp_send(local_pack_t *pack)
{
	network_pack_t network_pack = {0};
	uint16_t pack_size = 0;
	int sendlen = 0;
	struct sockaddr_in remote_addr; 

	rt_mutex_take(multicast_rsp_mux, RT_WAITING_FOREVER);

	//set the network pack
	network_pack.len = pack->len;
	network_pack.serial = pack->serial;
	network_pack.type = pack->type;
	network_pack.src_addr = pack->src_addr;
	network_pack.dest_addr = pack->dest_addr;
	network_pack.head = NET_PACK_HEAD;
	network_pack.tail = NET_PACK_END;
	network_pack.cmd = pack->cmd;
	network_pack.buf = pack->buf;
	network_pack.crc = cal_network_pack_crc(&network_pack);
	
	pack_size = pack->len + NET_PACK_HEAD_LEN + 6;
	rt_memcpy(udp_send_buf, &network_pack, NET_PACK_HEAD_LEN);
	rt_memcpy(&udp_send_buf[NET_PACK_HEAD_LEN], network_pack.buf, network_pack.len);
	rt_memcpy(&udp_send_buf[NET_PACK_HEAD_LEN + pack->len], &network_pack.crc, 6);
	
	
	if(udp_transfer_type == TRANSFER_TYPE_udp ||
       udp_transfer_type == TRANSFER_TYPE_udp_broadcast ||
       udp_transfer_type == TRANSFER_TYPE_udp_mutibroadcast)
  {
		remote_addr.sin_family = AF_INET;
		rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
		remote_addr.sin_addr.s_addr = udp_dest_ip;
		remote_addr.sin_port = udp_dest_port;
		sendlen = sendto(udp_sock_fd, udp_send_buf, pack_size, 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));

		if (sendlen <= 0)
		{ 
			LOG_E("Send fail\r\n");
		}
  }
	
	rt_mutex_release(multicast_rsp_mux);

	return sendlen;

}


void multicast_handle_thread(void *args)
{
  int val;
  rt_uint8_t retryCnt = 0;

  //stm32h750??????,????eth?DMA??????????
  NetRecvBuf = rt_malloc(NET_RECV_BUF_SIZE);
  udp_send_buf = rt_malloc(NET_RECV_BUF_SIZE); //NetRecvBuf;

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

static struct rt_thread multicast_handle;

void create_multicast_handle_thread(void)
{
	rt_err_t err;
	
    err = rt_thread_init(&multicast_handle, "multicast_handle", 
                         multicast_handle_thread, 
                         RT_NULL,
                         (void *)MULTICAST_CMM_THREAD_STACK_BASE,
                         MULTICAST_CMM_THREAD_STACK_SIZE,
                         MULTICAST_CMM_THREAD_PRIORITY, 
						 MULTICAST_CMM_THREAD_TIME_SLINCE);
						 
    if (err == RT_EOK)
    {
		LOG_D("multicast_handle_thread done!\n");
        rt_thread_startup(&multicast_handle);
    }

}

uint16_t cmd_get_iot_info_handle(void *args)
{
	//uint16_t para_len = (uint32_t)&iot_para.udp_recv_port - (uint32_t)&iot_para;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = (local_pack_t*)args;
    //uint16_t len = (uint32_t)&iot_para.tcp_log_server_addr - (uint32_t)&iot_para.net_addr;
	uint16_t len = (uint32_t)&iot_para.udp_recv_port - (uint32_t)&iot_para.net_addr;

	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(len);
 
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->dest_addr = cmd_pack->src_addr; 
	rsp_pack->src_addr = cmd_pack->dest_addr;
	rsp_pack->type = PACK_TYPE_RESULT;
	rsp_pack->serial = cmd_pack->serial;
	rsp_pack->len = len;

    rt_memcpy(&rsp_pack->buf[0], &iot_para, rsp_pack->len);
	len = net_udp_send(rsp_pack); //local_send_response(rsp_pack);  //PRJ_NMAE
	
	if(rsp_pack->buf)
	{
		rt_free(rsp_pack->buf);
	}
	
	if(rsp_pack)
	{
		rt_free(rsp_pack);
	}
	
    return len;
}

uint16_t cmd_set_iot_info_handle(void *args)
{
	
	//uint16_t para_len = (uint32_t)&iot_para.udp_recv_port - (uint32_t)&iot_para;
	local_pack_t *rsp_pack = RT_NULL;
	local_pack_t *cmd_pack = (local_pack_t*)args;
	Iot_parameter_t *m_param = (Iot_parameter_t*)&cmd_pack->buf[0];
	uint16_t len = (uint32_t)&iot_para.udp_recv_port - (uint32_t)&iot_para.net_addr;
	
	
	
	if(rt_memcmp(&iot_para.uid[0], &m_param->uid[0], sizeof(iot_para.uid)) != 0)
	{
		if(rsp_pack->buf)
		{
			rt_free(rsp_pack->buf);
		}
		
		if(rsp_pack)
		{
			rt_free(rsp_pack);
		}
		LOG_W("dest dev err!\n");
		
		
		
		return 0;
	}

	iot_para.net_addr = m_param->net_addr;
    rt_memcpy(&iot_para.name, &m_param->name, sizeof(iot_para.name));
    rt_memcpy(&iot_para.ip_addr, &m_param->ip_addr, sizeof(iot_para.ip_addr));
    rt_memcpy(&iot_para.gw_addr, &m_param->gw_addr, sizeof(iot_para.gw_addr));
    rt_memcpy(&iot_para.netmask, &m_param->netmask, sizeof(iot_para.netmask));
    rt_memcpy(&iot_para.mac, &m_param->mac, sizeof(iot_para.mac));
    iot_para.local_tcp_server_port = m_param->local_tcp_server_port;
    iot_para.tcp_log_port = m_param->tcp_log_port;
    rt_memcpy(&iot_para.tcp_log_server_addr, &m_param->tcp_log_server_addr, sizeof(iot_para.tcp_log_server_addr));
    iot_para_save(&iot_para);
    LOG_I("save iot para");
	
	rsp_pack = rt_malloc(sizeof(local_pack_t));
	rsp_pack->buf = rt_malloc(len);
	
	rsp_pack->cmd = cmd_pack->cmd;
	rsp_pack->dest_addr = cmd_pack->src_addr; 
	rsp_pack->src_addr = cmd_pack->dest_addr;
	rsp_pack->type = PACK_TYPE_RESULT;
	rsp_pack->serial = cmd_pack->serial;
	rsp_pack->len = sizeof(iot_para.uid);
	rt_memcpy(&rsp_pack->buf[0], &iot_para.uid[0], rsp_pack->len);  
	len = net_udp_send(rsp_pack); //local_send_response(rsp_pack);
	
	if(rsp_pack->buf)
	{
		rt_free(rsp_pack->buf);
	}
	
	if(rsp_pack)
	{
		rt_free(rsp_pack);
	}
	
	iot_para.status.is_need_reboot = 1;
	
	
	
	
    return len;
	
}


