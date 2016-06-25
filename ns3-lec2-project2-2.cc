

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ns3-lec2-project2-2");		//定义记录组件

int
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 2;			//csma节点数量
  uint32_t nWifi = 3;
  uint32_t nCsmaadd = 3;			//wifi节点数量
   bool tracing = true;


  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nCsmaadd", "Number of \"extra\" CSMA nodes/devices add", nCsmaadd);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses
  // soon become an issue		//判断是否超过了250个，超过报错 , 原因？
  if (nWifi > 250 || nCsma+nCsmaadd > 250)
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

  //再次创建2个节点，p2p链路两端
  NodeContainer p2pNodesadd;
  p2pNodesadd.Create (2);

  //再次创建信道，设置信道参数，在设备安装到节点上
  PointToPointHelper pointToPointadd;
  pointToPointadd.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPointadd.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer p2pDevicesadd;
  p2pDevicesadd = pointToPointadd.Install (p2pNodesadd);

  //再一次创建2个节点，p2p链路两端
  NodeContainer p2pNodesadd2;
  p2pNodesadd2.Create (2);

  //再一次创建信道，设置信道参数，在设备安装到节点上
  PointToPointHelper pointToPointadd2;
  pointToPointadd2.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPointadd2.SetChannelAttribute ("Delay", StringValue ("1ms"));

  NetDeviceContainer p2pDevicesadd2;
  p2pDevicesadd2 = pointToPointadd2.Install (p2pNodesadd2);

  //创建csma节点，包含两个p2p节点

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Add (p2pNodesadd.Get (0));
  csmaNodes.Create (nCsma);

  //创建信道，设置信道参数，在设备安装到节点上
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  //再次创建csma节点，包含两个p2p节点

  NodeContainer csmaNodesadd;
  csmaNodesadd.Add (p2pNodesadd.Get (1));
  csmaNodesadd.Add (p2pNodesadd2.Get (1));
  csmaNodesadd.Create (nCsmaadd);

  //再次创建信道，设置信道参数，在设备安装到节点上
  CsmaHelper csmaadd;
  csmaadd.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csmaadd.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevicesadd;
  csmaDevicesadd = csmaadd.Install (csmaNodesadd);

//创建wifista无线终端，AP接入点
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  wifiStaNodes.Add(p2pNodesadd2.Get(0));
  NodeContainer wifiApNode = p2pNodes.Get (0);

  //创建无线设备于无线节点之间的互联通道，并将通道对象与物理层对象关联
  //确保所有物理层对象使用相同的底层信道，即无线信道
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  //配置速率控制算法，AARF算法
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  //配置mac类型为sta模式，不发送探测请求
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  //创建无线设备，将mac层和phy层安装到设备上
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  //配置AP节点的mac层为AP模式，创建AP设备
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  //配置移动模型，起始位置
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

//配置STA移动方式，ConstantVelocityMobilityModel,加速移动模型



  mobility.SetMobilityModel ("ns3::ConstantAccelerationMobilityModel");
  mobility.Install (wifiStaNodes);
  for (uint n=0 ; n < wifiStaNodes.GetN() ; n++){
    Ptr<ConstantAccelerationMobilityModel> mob = wifiStaNodes.Get(n)->GetObject<ConstantAccelerationMobilityModel>();
    mob->SetVelocityAndAcceleration(Vector(10, 0, 0),Vector(0, 10, 0));
  }


//配置AP移动方式，ConstantPositionMobilityModel，固定位置模型
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  //已经创建了节点，设备，信道和移动模型，接下来配置协议栈
  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (csmaNodesadd);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  //分配IP地址
  Ipv4AddressHelper address;
 //P2P信道
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
 //csma信道
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);
 //wifi信道
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);
  //增加的P2P信道
   address.SetBase ("10.1.4.0", "255.255.255.0");
   Ipv4InterfaceContainer p2pInterfacesadd;
   p2pInterfacesadd = address.Assign (p2pDevicesadd);
   //再次增加的P2P信道
    address.SetBase ("10.1.6.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfacesadd2;
    p2pInterfacesadd2 = address.Assign (p2pDevicesadd2);
  //增加的csma信道
   address.SetBase ("10.1.5.0", "255.255.255.0");
   Ipv4InterfaceContainer csmaInterfacesadd;
   csmaInterfacesadd = address.Assign (csmaDevicesadd);

  //放置echo服务端程序在最右边的csma节点,端口为9
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodesadd.Get (nCsmaadd));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  //回显客户端放在最后的STA节点，指向CSMA网络的服务器，上面的节点地址，端口为9
  UdpEchoClientHelper echoClient (csmaInterfacesadd.GetAddress (nCsmaadd), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  //安装其他节点应用程序
  ApplicationContainer clientApps1 = echoClient.Install (wifiStaNodes.Get (0));
  clientApps1.Start (Seconds (2.0));
  clientApps1.Stop (Seconds (10.0));
  ApplicationContainer clientApps2 = echoClient.Install (wifiStaNodes.Get (1));
  clientApps2.Start (Seconds (3.0));
  clientApps2.Stop (Seconds (10.0));
  ApplicationContainer clientApps3 = echoClient.Install (wifiStaNodes.Get (2));
  clientApps3.Start (Seconds (4.0));
  clientApps3.Stop (Seconds (10.0));
  ApplicationContainer clientApps4 = echoClient.Install (csmaNodesadd.Get(1));
  clientApps4.Start (Seconds (5.0));
  clientApps4.Stop (Seconds (10.0));

  //启动互联网络路由
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("ns3-lec2-project2-2");
      phy.EnablePcap ("ns3-lec2-project2-2", apDevices.Get (0));
      csma.EnablePcap ("ns3-lec2-project2-2", csmaDevices.Get (0), true);
    }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
