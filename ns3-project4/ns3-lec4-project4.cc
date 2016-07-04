#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

#include <iostream>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("project2");		//定义记录组件

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi1 = 6;
  uint32_t nWifi2 = 6;				//wifi节点数量
   bool tracing = true;


  CommandLine cmd;
  cmd.AddValue ("nWifi2", "Number of wifi2 STA devices", nWifi2);
  cmd.AddValue ("nWifi1", "Number of wifi1 STA devices", nWifi1);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses
  // soon become an issue		//判断是否超过了250个，超过报错 , 原因？
  if (nWifi1 > 250 || nWifi2 > 250)
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);	//启动记录组件
    }


  //创建2个节点，p2p链路两端
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  //创建信道，设置信道参数，在设备安装到节点上
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

                /*搭建Wifi1拓扑*/
  //创建wifi1sta无线终端，AP接入点
  NodeContainer wifi1StaNodes;
  wifi1StaNodes.Create (nWifi1);
  NodeContainer wifi1ApNode = p2pNodes.Get (0);

  //创建无线设备于无线节点之间的互联通道，并将通道对象与物理层对象关联
  //确保所有物理层对象使用相同的底层信道，即无线信道
  YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default ();
  channel1.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  YansWifiPhyHelper phy1 = YansWifiPhyHelper::Default ();
  phy1.SetChannel (channel1.Create ());

  //配置速率控制算法，AARF算法
  WifiHelper wifi1 = WifiHelper::Default ();
  wifi1.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac1 = NqosWifiMacHelper::Default ();

  //配置mac类型为sta模式，不发送探测请求
  Ssid ssid1 = Ssid ("ns-3-ssid1");
  mac1.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid1),
               "ActiveProbing", BooleanValue (false));

  //创建无线设备，将mac层和phy层安装到设备上
  NetDeviceContainer staDevices1;
  staDevices1 = wifi1.Install (phy1, mac1, wifi1StaNodes);

  //配置AP节点的mac层为AP模式，创建AP设备
  mac1.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid1));

  NetDeviceContainer apDevices1;
  apDevices1 = wifi1.Install (phy1, mac1, wifi1ApNode);

                /*搭建Wifi2拓扑*/
  //创建wifi2sta无线终端，AP接入点
  NodeContainer wifi2StaNodes;
  wifi2StaNodes.Create (nWifi2);
  NodeContainer wifi2ApNode = p2pNodes.Get (1);

  //创建无线设备于无线节点之间的互联通道，并将通道对象与物理层对象关联
  //确保所有物理层对象使用相同的底层信道，即无线信道
  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
  channel2.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default ();
  phy2.SetChannel (channel2.Create ());

  //配置速率控制算法，AARF算法
  WifiHelper wifi2 = WifiHelper::Default ();
  wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac2 = NqosWifiMacHelper::Default ();

  //配置mac类型为sta模式，不发送探测请求
  Ssid ssid2 = Ssid ("ns-3-ssid2");
  mac2.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid2),
               "ActiveProbing", BooleanValue (false));

  //创建无线设备，将mac层和phy层安装到设备上
  NetDeviceContainer staDevices2;
  staDevices2 = wifi2.Install (phy2, mac2, wifi2StaNodes);

  //配置AP节点的mac层为AP模式，创建AP设备
  mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid2));

  NetDeviceContainer apDevices2;
  apDevices2 = wifi2.Install (phy2, mac2, wifi2ApNode);


  //配置移动模型，起始位置
  MobilityHelper mobility1;
  //wifi1配置STA移动方式，RandomWalk2dMobilityModel，随机游走模型
  mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-205, 295, -230, 270)));
  mobility1.Install (wifi1StaNodes);
  Ptr<RandomWalk2dMobilityModel> mob1 = wifi1StaNodes.Get(0)->GetObject<RandomWalk2dMobilityModel>();
    mob1->SetPosition(Vector(15.0, 28.66, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob2 = wifi1StaNodes.Get(1)->GetObject<RandomWalk2dMobilityModel>();
    mob2->SetPosition(Vector(25.0, 28.66, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob3 = wifi1StaNodes.Get(2)->GetObject<RandomWalk2dMobilityModel>();
    mob3->SetPosition(Vector(30.0, 20.0, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob4 = wifi1StaNodes.Get(3)->GetObject<RandomWalk2dMobilityModel>();
    mob4->SetPosition(Vector(25.0, 11.34, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob5 = wifi1StaNodes.Get(4)->GetObject<RandomWalk2dMobilityModel>();
    mob5->SetPosition(Vector(15.0, 11.34, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob6 = wifi1StaNodes.Get(5)->GetObject<RandomWalk2dMobilityModel>();
    mob6->SetPosition(Vector(10.0, 20.0, 0.0));
//配置AP移动方式，ConstantPositionMobilityModel，固定位置模型
  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility1.Install (wifi1ApNode);
  Ptr<ConstantPositionMobilityModel> mob00 = wifi1ApNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
  mob00->SetPosition(Vector(20.0, 20.0, 0.0));


  //配置移动模型，起始位置
  MobilityHelper mobility2;
  //wifi1配置STA移动方式，RandomWalk2dMobilityModel，随机游走模型
  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-205, 295, -230, 270)));
  mobility2.Install (wifi2StaNodes);
  Ptr<RandomWalk2dMobilityModel> mob11 = wifi2StaNodes.Get(0)->GetObject<RandomWalk2dMobilityModel>();
    mob11->SetPosition(Vector(65.0, 28.66, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob22 = wifi2StaNodes.Get(1)->GetObject<RandomWalk2dMobilityModel>();
    mob22->SetPosition(Vector(75.0, 28.66, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob33 = wifi2StaNodes.Get(2)->GetObject<RandomWalk2dMobilityModel>();
    mob33->SetPosition(Vector(80.0, 20.0, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob44 = wifi2StaNodes.Get(3)->GetObject<RandomWalk2dMobilityModel>();
    mob44->SetPosition(Vector(75.0, 11.34, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob55 = wifi2StaNodes.Get(4)->GetObject<RandomWalk2dMobilityModel>();
    mob55->SetPosition(Vector(65.0, 11.34, 0.0));
  Ptr<RandomWalk2dMobilityModel> mob66 = wifi2StaNodes.Get(5)->GetObject<RandomWalk2dMobilityModel>();
    mob66->SetPosition(Vector(60.0, 20.0, 0.0));
//配置AP移动方式，ConstantPositionMobilityModel，固定位置模型
  mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility2.Install (wifi2ApNode);
  Ptr<ConstantPositionMobilityModel> mob01 = wifi2ApNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
  mob01->SetPosition(Vector(70.0, 20.0, 0.0));


  //已经创建了节点，设备，信道和移动模型，接下来配置协议栈
  InternetStackHelper stack;
  stack.Install (wifi2ApNode);
  stack.Install (wifi2StaNodes);
  stack.Install (wifi1ApNode);
  stack.Install (wifi1StaNodes);

  //分配IP地址
  Ipv4AddressHelper address;
 //P2P信道
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
 //wifi1信道
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer wifi1staipadd;
  wifi1staipadd = address.Assign (staDevices1);
  address.Assign (apDevices1);
 //wifi2信道
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices2);
  address.Assign (apDevices2);

  //放置echo服务端程序在最右边的wifi1节点,端口为9
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (wifi1StaNodes.Get (nWifi1-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  //回显客户端放在wifi2最后的STA节点，指向CSMA网络的服务器，上面的节点地址，端口为9
  UdpEchoClientHelper echoClient (wifi1staipadd.GetAddress (nWifi1-1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));



  ApplicationContainer clientApps =
  echoClient.Install (wifi2StaNodes.Get(2));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //启动互联网络路由
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      phy1.EnablePcap ("project4", apDevices1.Get (0));
      phy2.EnablePcap ("project4", apDevices2.Get (0));
    }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
