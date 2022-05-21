/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Implementation Done By:
 * Adarsh Honawad <adarsh2397@gmail.com>
 * Sagar Bharadwaj <sagarbharadwaj50@gmail.com>
 * Samvid Dharanikota <samvid25@yahoo.com>
 */

/* 
 * The topology used to simulate this attack contains 5 nodes as follows:
 * n0 -> alice (sender)
 * n1 -> eve (attacker)
 * n2 -> router (common router between alice and eve)
 * n3 -> router (router conneced to bob)
 * n4 -> bob (receiver)

     n1
        \ pp1 (100 Mbps, 2ms RTT)
         \
          \             -> pp1 (100 Mbps, 2ms RTT)
           \            |
            n2 ---- n3 ---- n4
           /    |
          /     -> pp2 (1.5 Mbps, 40ms RTT)
         /
        / pp1 (100 Mbps, 2ms RTT)
     n0

*/

#include <fstream>
//#include "ns3/nstime.h" //d
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h" //d
#include "ns3/netanim-module.h"
#include "ns3/stats-module.h"
//#include <cstring> 
//#include <cstdlib>


#define TCP_SINK_PORT 9000
#define UDP_SINK_PORT 9001

// Experimentation parameters
#define BULK_SEND_MAX_BYTES 2097152
#define MAX_SIMULATION_TIME 100.0
#define ATTACKER_START 0.0
#define ATTACKER_RATE (std::string)"12000kb/s"
#define ON_TIME (std::string)"0.25"
#define BURST_PERIOD 1
#define OFF_TIME std::to_string(BURST_PERIOD - stof(ON_TIME))
#define SENDER_START 0.75 // Must be equal to OFF_TIME

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("FirstScriptExample");

int main(int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  Time::SetResolution(Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);


  NodeContainer nodes;
  nodes.Create(5);


  // Default TCP is TcpNewReno (no need to change, unless experimenting with other variants)
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue("ns3::TcpNewReno")); //d


  // Define the Point-to-Point links (helpers) and their paramters
  PointToPointHelper pp1, pp2;
  pp1.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  pp1.SetChannelAttribute("Delay", StringValue("1ms"));

  // Add a DropTailQueue to the bottleneck link 
  //pp2.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(1)); //d
  pp2.SetDeviceAttribute("DataRate", StringValue("1.5Mbps")); //d
  pp2.SetChannelAttribute("Delay", StringValue("20ms")); //d


  // Install the Point-to-Point links between nodes
  NetDeviceContainer d02, d12, d23, d34;
  d02 = pp1.Install(nodes.Get(0), nodes.Get(2));
  d12 = pp1.Install(nodes.Get(1), nodes.Get(2));
  d23 = pp2.Install(nodes.Get(2), nodes.Get(3));
  d34 = pp1.Install(nodes.Get(3), nodes.Get(4));


  InternetStackHelper stack;
  stack.Install(nodes);


  Ipv4AddressHelper a02, a12, a23, a34;
  a02.SetBase("10.1.1.0", "255.255.255.0");
  a12.SetBase("10.1.2.0", "255.255.255.0");
  a23.SetBase("10.1.3.0", "255.255.255.0");
  a34.SetBase("10.1.4.0", "255.255.255.0");


  Ipv4InterfaceContainer i02, i12, i23, i34;
  i02 = a02.Assign(d02);
  i12 = a12.Assign(d12);
  i23 = a23.Assign(d23);
  i34 = a34.Assign(d34);

 // totally different
  // UDP On-Off Application - Application used by attacker (eve) to create the low-rate bursts.
  OnOffHelper onoff("ns3::UdpSocketFactory",
                    Address(InetSocketAddress(i34.GetAddress(1), UDP_SINK_PORT)));
  onoff.SetConstantRate(DataRate(ATTACKER_RATE));
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=" + ON_TIME + "]"));
  onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=" + OFF_TIME + "]"));
  ApplicationContainer onOffApp = onoff.Install(nodes.Get(1));
  onOffApp.Start(Seconds(ATTACKER_START));
  onOffApp.Stop(Seconds(MAX_SIMULATION_TIME));


  // TCP Bulk Send Application - Application used by the legit node (alice) to send data to a receiver.
  BulkSendHelper bulkSend("ns3::TcpSocketFactory",
                          InetSocketAddress(i34.GetAddress(1), TCP_SINK_PORT));
  bulkSend.SetAttribute("MaxBytes", UintegerValue(BULK_SEND_MAX_BYTES));
  ApplicationContainer bulkSendApp = bulkSend.Install(nodes.Get(0));
  bulkSendApp.Start(Seconds(SENDER_START));
  bulkSendApp.Stop(Seconds(MAX_SIMULATION_TIME));


  // UDP sink on the receiver (bob).
  PacketSinkHelper UDPsink("ns3::UdpSocketFactory",
                           Address(InetSocketAddress(Ipv4Address::GetAny(), UDP_SINK_PORT)));
  ApplicationContainer UDPSinkApp = UDPsink.Install(nodes.Get(4));
  UDPSinkApp.Start(Seconds(0.0));
  UDPSinkApp.Stop(Seconds(MAX_SIMULATION_TIME));


  // TCP sink on the receiver (bob).
  PacketSinkHelper TCPsink("ns3::TcpSocketFactory",
                           InetSocketAddress(Ipv4Address::GetAny(), TCP_SINK_PORT));
  ApplicationContainer TCPSinkApp = TCPsink.Install(nodes.Get(4));
  TCPSinkApp.Start(Seconds(0.0));
  TCPSinkApp.Stop(Seconds(MAX_SIMULATION_TIME));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  //pp1.EnablePcapAll("PCAPs/tcplow");
  // tcp 
  AnimationInterface anim("tcp3.xml");
anim.SetConstantPosition(nodes.Get(0),10.0,10.0);
anim.SetConstantPosition(nodes.Get(1),10.0,30.0);
anim.SetConstantPosition(nodes.Get(2),20.0,20.0);
anim.SetConstantPosition(nodes.Get(3),30.0,20.0);
anim.SetConstantPosition(nodes.Get(4),40.0,20.0);

// ascii files
//AsciiTraceHelper ascii;
//pointToPoint.EnableAsciiAll(ascii.CreateFileStream("tcp3.tr"));

/*AsciiTraceHelper ascii;
  pp1.EnableAsciiAll (ascii.CreateFileStream ("tcp_tr.tr"));
  pp1.EnablePcapAll ("low-rate-tcp");
 //
 
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", anyAddress);
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (1));
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (20.));

  Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());

 AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream ("tcp_low_rate.cwnd");
  ns3TcpSocket->TraceConnectWithoutContext ("CongestionWindow", MakeBoundCallback (&CwndChange, stream));
*/
 //--------------------------GNU-----------------------------------------------------
 // Use GnuplotHelper to plot the packet byte count over time
  GnuplotHelper plotHelper;

  // Configure the plot.  The first argument is the file name prefix
  // for the output files generated.  The second, third, and fourth
  // arguments are, respectively, the plot title, x-axis, and y-axis labels
  plotHelper.ConfigurePlot ("LOW-RATE-DOS",
                            "Packet Byte Count vs. Time",
                            "Time (Seconds)",
                            "Packet Byte Count");

  // Specify the probe type, trace source path (in configuration namespace), and
  // probe output trace source ("OutputBytes") to plot.  The fourth argument
  // specifies the name of the data series label on the plot.  The last
  // argument formats the plot by specifying where the key should be placed.
  //plotHelper.PlotProbe (probeType,
    //                    tracePath,
      //                  "OutputBytes",
        //                "Packet Byte Count",
          //              GnuplotAggregator::KEY_BELOW);

  // Use FileHelper to write out the packet byte count over time
  FileHelper fileHelper;

  // Configure the file to be written, and the formatting of output data.
  fileHelper.ConfigureFile ("LOW-RATE-ATTACK",
                            FileAggregator::FORMATTED);

  // Set the labels for this formatted output file.
  fileHelper.Set2dFormat ("Time (Seconds) = %.3e\tPacket Byte Count = %.0f");

  // Specify the probe type, trace source path (in configuration namespace), and
  // probe output trace source ("OutputBytes") to write.
  //fileHelper.WriteProbe (probeType,
    //                     tracePath,
      //                   "OutputBytes");

 //----------------------------------GNU-----------------------------------------------------
 
  
  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
