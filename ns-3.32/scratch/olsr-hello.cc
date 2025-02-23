#include <iostream>
#include <cmath>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/energy-module.h"
#include "ns3/wifi-radio-energy-model-helper.h"

using namespace ns3;

class OlsrExample {
public:
    OlsrExample();

    bool Configure(int argc, char **argv);
    void Run();
    void Report(std::ostream &os);

private:
    uint32_t size;
    double step;
    double totalTime;
    bool pcap;
    bool printRoutes;
    double helloInterval; // Define helloInterval as a double

    NodeContainer nodes;
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowMonitorHelper;

    void CreateNodes();
    void CreateDevices();
    void InstallInternetStack();
    void InstallApplications();
    void MonitorThroughput();
    void InstallEnergyModel();
};

OlsrExample::OlsrExample() : size(10), step(30), totalTime(20), pcap(true), printRoutes(true), helloInterval(1.0) // Set hello interval to 1 second
{
}

bool OlsrExample::Configure(int argc, char **argv)
{
    SeedManager::SetSeed(12345);
    CommandLine cmd(__FILE__);

    cmd.AddValue("pcap", "Write PCAP traces.", pcap);
    cmd.AddValue("printRoutes", "Print routing table dumps.", printRoutes);
    cmd.AddValue("size", "Number of nodes.", size);
    cmd.AddValue("time", "Simulation time, s.", totalTime);
    cmd.AddValue("step", "Grid step, m", step);
    cmd.AddValue("helloInterval", "OLSR hello interval in seconds.", helloInterval); // Added hello interval parameter

    cmd.Parse(argc, argv);
    return true;
}

void OlsrExample::Run()
{
    CreateNodes();
    CreateDevices();
    InstallInternetStack();
    InstallEnergyModel();
    InstallApplications();

    std::cout << "Starting simulation for " << totalTime << " s ...\n";

    Simulator::Stop(Seconds(totalTime));
    Simulator::Run();
    Simulator::Destroy();

    MonitorThroughput();
}

void OlsrExample::Report(std::ostream &)
{
    // Report function placeholder; you can extend this to output more detailed results if needed.
}

void OlsrExample::CreateNodes()
{
    std::cout << "Creating " << size << " nodes " << step << " m apart.\n";
    nodes.Create(size);

    for (uint32_t i = 0; i < size; ++i)
    {
        std::ostringstream os;
        os << "node-" << i;
        Names::Add(os.str(), nodes.Get(i));
    }

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(step),
                                  "DeltaY", DoubleValue(0),
                                  "GridWidth", UintegerValue(size),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
}

void OlsrExample::CreateDevices()
{
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue(0));
    devices = wifi.Install(wifiPhy, wifiMac, nodes);

    if (pcap)
    {
        wifiPhy.EnablePcapAll(std::string("olsr"));
    }
}

void OlsrExample::InstallInternetStack()
{
    OlsrHelper olsr;

    // Set hello interval through attributes
    olsr.Set("HelloInterval", TimeValue(Seconds(helloInterval)));

    InternetStackHelper stack;
    stack.SetRoutingHelper(olsr);
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.0.0.0");
    interfaces = address.Assign(devices);

    if (printRoutes)
    {
        Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("olsr.routes", std::ios::out);
        olsr.PrintRoutingTableAllAt(Seconds(8), routingStream);
    }
}

void OlsrExample::InstallApplications()
{
    // Install a UdpEchoServer on the last node
    uint16_t port = 9;
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(size - 1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(totalTime));

    // Install a UdpEchoClient on the first node, targeting the server
    UdpEchoClientHelper echoClient(interfaces.GetAddress(size - 1), port);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.1))); // 100ms
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(totalTime));

    // Install FlowMonitor on all nodes
    flowMonitor = flowMonitorHelper.InstallAll();
}

void OlsrExample::InstallEnergyModel()
{
    BasicEnergySourceHelper basicSourceHelper;
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(100.0));
    EnergySourceContainer sources = basicSourceHelper.Install(nodes);

    WifiRadioEnergyModelHelper radioEnergyHelper;
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
    radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.0197));

    DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(devices, sources);
}

void OlsrExample::MonitorThroughput()
{
    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitorHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
    double totalTxPackets = 0;
    double totalRxPackets = 0;
    double totalTxBytes = 0;
    double totalRxBytes = 0;
    double totalDelay = 0;
    double throughput = 0.0;
    double packetLossRatio = 0.0;
    double averageDelay = 0.0;
    double totalEnergyConsumed = 0.0;
    double totalControlPackets = 0.0; // Add variable for control packets
    double totalCollisions = 0.0;     // Add variable for collisions

    for (auto &flow : stats)
    {
        totalTxPackets += flow.second.txPackets;
        totalRxPackets += flow.second.rxPackets;
        totalTxBytes += flow.second.txBytes;
        totalRxBytes += flow.second.rxBytes;
        totalDelay += flow.second.delaySum.GetSeconds();
        // Calculate control packets and collisions
        if (flow.second.txPackets > flow.second.rxPackets)
        {
            totalCollisions += (flow.second.txPackets - flow.second.rxPackets);
        }
    }

    if (totalTime > 0)
    {
        throughput = totalRxBytes * 8.0 / totalTime / 1000 / 1000; // Mbps
    }

    if (totalTxPackets > 0)
    {
        packetLossRatio = (totalTxPackets - totalRxPackets) / totalTxPackets;
    }

    if (totalRxPackets > 0)
    {
        averageDelay = totalDelay / totalRxPackets;
    }
    totalControlPackets = 10 * size;
    // Calculate total energy consumed
    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<EnergySourceContainer> energySources = node->GetObject<EnergySourceContainer>();
        if (energySources)
        {
            for (EnergySourceContainer::Iterator j = energySources->Begin(); j != energySources->End(); ++j)
            {
                Ptr<EnergySource> source = *j;
                totalEnergyConsumed += (source->GetInitialEnergy() - source->GetRemainingEnergy());
            }
        }
    }

    // Calculate overhead efficiency
    double overheadEfficiency = (totalRxPackets / (totalControlPackets + totalTxPackets)) * 100;

    std::cout << "\n--- OLSR Performance Metrics ---\n";
    std::cout << "Total Packets Sent: " << totalTxPackets << "\n";
    std::cout << "Total Packets Received: " << totalRxPackets << "\n";
    std::cout << "Total Packets Lost: " << totalTxPackets - totalRxPackets << "\n";
    std::cout << "Packet Loss Ratio: " << packetLossRatio * 100 << " %\n";
    std::cout << "Throughput: " << throughput << " Mbps\n";
    std::cout << "Total Energy Consumed: " << totalEnergyConsumed << " J\n";
    std::cout << "Overhead Efficiency: " << overheadEfficiency << " %\n";
    std::cout << "Average Delay: " << averageDelay * 1000 << " ms\n";
    std::cout << "Collision Rate: " << (totalCollisions / totalTxPackets) * 100 << " %\n";
}

int main(int argc, char **argv)
{
    OlsrExample test;
    if (!test.Configure(argc, argv))
    {
        NS_FATAL_ERROR("Configuration failed. Aborted.");
    }

    test.Run();
    test.Report(std::cout);

    return 0;
}

/*
#include <iostream>
#include <cmath>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/energy-module.h"
#include "ns3/wifi-radio-energy-model-helper.h"

using namespace ns3;

class OlsrExample {
public:
    OlsrExample();

    bool Configure(int argc, char **argv);
    void Run();
    void Report(std::ostream &os);

private:
    uint32_t size;
    double step;
    double totalTime;
    bool pcap;
    bool printRoutes;
    double helloInterval; // Define helloInterval as a double

    NodeContainer nodes;
    NetDeviceContainer devices;
    Ipv4InterfaceContainer interfaces;
    Ptr<FlowMonitor> flowMonitor;
    FlowMonitorHelper flowMonitorHelper;

    void CreateNodes();
    void CreateDevices();
    void InstallInternetStack();
    void InstallApplications();
    void MonitorThroughput();
    void InstallEnergyModel();

    void SendPacket(uint32_t sender, uint32_t receiver);
};

OlsrExample::OlsrExample() : size(20), step(30), totalTime(30), pcap(true), printRoutes(true), helloInterval(1.0) // Set hello interval to 1 second
{
}

bool OlsrExample::Configure(int argc, char **argv)
{
    SeedManager::SetSeed(12345);
    CommandLine cmd(__FILE__);

    cmd.AddValue("pcap", "Write PCAP traces.", pcap);
    cmd.AddValue("printRoutes", "Print routing table dumps.", printRoutes);
    cmd.AddValue("size", "Number of nodes.", size);
    cmd.AddValue("time", "Simulation time, s.", totalTime);
    cmd.AddValue("step", "Grid step, m", step);
    cmd.AddValue("helloInterval", "OLSR hello interval in seconds.", helloInterval); // Added hello interval parameter

    cmd.Parse(argc, argv);
    return true;
}

void OlsrExample::Run()
{
    CreateNodes();
    CreateDevices();
    InstallInternetStack();
    InstallEnergyModel();
    InstallApplications();

    std::cout << "Starting simulation for " << totalTime << " s ...\n";

    Simulator::Stop(Seconds(totalTime));
    Simulator::Run();
    Simulator::Destroy();

    MonitorThroughput();
}

void OlsrExample::Report(std::ostream &)
{
    // Report function placeholder; you can extend this to output more detailed results if needed.
}

void OlsrExample::CreateNodes()
{
    std::cout << "Creating " << size << " nodes " << step << " m apart.\n";
    nodes.Create(size);

    for (uint32_t i = 0; i < size; ++i)
    {
        std::ostringstream os;
        os << "node-" << i;
        Names::Add(os.str(), nodes.Get(i));
    }

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(step),
                                  "DeltaY", DoubleValue(0),
                                  "GridWidth", UintegerValue(size),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);
}

void OlsrExample::CreateDevices()
{
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default();
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue(0));
    devices = wifi.Install(wifiPhy, wifiMac, nodes);

    if (pcap)
    {
        wifiPhy.EnablePcapAll(std::string("olsr"));
    }
}

void OlsrExample::InstallInternetStack()
{
    OlsrHelper olsr;

    // Set hello interval through attributes
    olsr.Set("HelloInterval", TimeValue(Seconds(helloInterval)));

    InternetStackHelper stack;
    stack.SetRoutingHelper(olsr);
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.0.0.0");
    interfaces = address.Assign(devices);

    if (printRoutes)
    {
        Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("olsr.routes", std::ios::out);
        olsr.PrintRoutingTableAllAt(Seconds(8), routingStream);
    }
}

void OlsrExample::InstallApplications()
{
    // Install a UdpEchoServer on the last node
    uint16_t port = 9;
    UdpEchoServerHelper echoServer(port);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(size - 1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(totalTime));

    // Install a UdpEchoClient on the first node, targeting the server
    UdpEchoClientHelper echoClient(interfaces.GetAddress(size - 1), port);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1000));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(0.1))); // 100ms
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(totalTime));

    // Install FlowMonitor on all nodes
    flowMonitor = flowMonitorHelper.InstallAll();
}

void OlsrExample::InstallEnergyModel()
{
    BasicEnergySourceHelper basicSourceHelper;
    basicSourceHelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(100.0));
    EnergySourceContainer sources = basicSourceHelper.Install(nodes);

    WifiRadioEnergyModelHelper radioEnergyHelper;
    radioEnergyHelper.Set("TxCurrentA", DoubleValue(0.0174));
    radioEnergyHelper.Set("RxCurrentA", DoubleValue(0.0197));

    DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install(devices, sources);
}

void OlsrExample::MonitorThroughput()
{
    flowMonitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonitorHelper.GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();
    double totalTxPackets = 0;
    double totalRxPackets = 0;
    double totalTxBytes = 0;
    double totalRxBytes = 0;
    double totalDelay = 0;
    double throughput = 0.0;
    double packetLossRatio = 0.0;
    double averageDelay = 0.0;
    double totalEnergyConsumed = 0.0;
    double totalControlPackets = 0.0; // Add variable for control packets
    double totalCollisions = 0.0;     // Add variable for collisions

    for (auto &flow : stats)
    {
        totalTxPackets += flow.second.txPackets;
        totalRxPackets += flow.second.rxPackets;
        totalTxBytes += flow.second.txBytes;
        totalRxBytes += flow.second.rxBytes;
        totalDelay += flow.second.delaySum.GetSeconds();
        // Calculate control packets and collisions
        if (flow.second.txPackets > flow.second.rxPackets)
        {
            totalCollisions += (flow.second.txPackets - flow.second.rxPackets);
        }
    }

    if (totalTime > 0)
    {
        throughput = totalRxBytes * 8.0 / totalTime / 1000 / 1000; // Mbps
    }

    if (totalTxPackets > 0)
    {
        packetLossRatio = (totalTxPackets - totalRxPackets) / totalTxPackets;
    }

    if (totalRxPackets > 0)
    {
        averageDelay = totalDelay / totalRxPackets;
    }
    totalControlPackets = 10 * size;
    // Calculate total energy consumed
    for (NodeContainer::Iterator i = nodes.Begin(); i != nodes.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<EnergySourceContainer> energySources = node->GetObject<EnergySourceContainer>();
        if (energySources)
        {
            Ptr<BasicEnergySource> basicSource = DynamicCast<BasicEnergySource>(energySources->Get(0));
            if (basicSource)
            {
                totalEnergyConsumed += (100.0 - basicSource->GetRemainingEnergy());
            }
        }
    }

    std::cout << "Total Packets Sent: " << totalTxPackets << "\n";
    std::cout << "Total Packets Received: " << totalRxPackets << "\n";
    std::cout << "Total Packets Lost: " << (totalTxPackets - totalRxPackets) << "\n";
    std::cout << "Throughput: " << throughput << " Mbps\n";
    std::cout << "Packet Loss Ratio: " << packetLossRatio * 100 << " %\n";
    std::cout << "Average Delay: " << averageDelay << " s\n";
    std::cout << "Total Energy Consumed: " << totalEnergyConsumed << " J\n";
    std::cout << "Overhead Efficiency: " << (totalRxPackets / totalControlPackets) * 100 << " %\n";
    std::cout << "Collision Rate: " << (totalCollisions / totalTxPackets) * 100 << " %\n";
}

int main(int argc, char *argv[])
{
    OlsrExample test;
    if (!test.Configure(argc, argv))
    {
        NS_FATAL_ERROR("Configuration failed. Aborted.");
    }
    test.Run();
    test.Report(std::cout);
    return 0;
}
*/
