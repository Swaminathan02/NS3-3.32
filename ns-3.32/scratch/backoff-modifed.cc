/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/core-module.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/mobility-module.h"
#include "backoff-modifed.h"
#include <cmath>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BackoffExample");

void Backoff::StartTcpTimer (Time timeout)
{
  NS_LOG_INFO ("Starting TCP Timer for " << timeout.GetSeconds () << " seconds");
  Simulator::Schedule (timeout, &Backoff::HandleTcpTimeout, this);
}

void Backoff::HandleTcpTimeout (void)
{
  NS_LOG_INFO ("TCP Timer expired at " << Simulator::Now ().GetSeconds () << " seconds");
}

void Backoff::SendHelloPacket (void)
{
  NS_LOG_INFO ("Sending Hello packet at " << Simulator::Now ().GetSeconds () << " seconds");
}

void CalculatePerformanceMetrics(const Backoff& backoff, uint32_t totalPacketsSent, uint32_t totalUAVs)
{
  // Assume packet loss increases with congestion due to more UAVs.
  uint32_t totalPacketsReceived = totalPacketsSent - (totalUAVs * std::min(50U, totalPacketsSent / totalUAVs));
  
  double simulationTime = Simulator::Now ().GetSeconds ();
  double totalDataSent = 1024 * totalPacketsSent; 
  double totalDataReceived = 1024 * totalPacketsReceived;
  
  // Simulate energy consumption scaling with the number of UAVs
  double energyConsumed = 0.1 * totalPacketsSent * std::log(totalUAVs + 1);

  uint32_t packetsLost = totalPacketsSent - totalPacketsReceived;
  double packetLossRatio = static_cast<double>(packetsLost) / totalPacketsSent;

  double throughput = 0.0;
  if (simulationTime > 0)
  {
    throughput = (totalDataReceived * 8) / (simulationTime * 1e6); 
  }

  double overheadEfficiency = (totalDataReceived / totalDataSent) * 100.0;

  std::cout << "\n--- Backoff Modified Performance Metrics for " << totalUAVs << " UAVs ---\n";
  std::cout << "Total Packets Sent: " << totalPacketsSent << "\n";
  std::cout << "Total Packets Received: " << totalPacketsReceived << "\n";
  std::cout << "Total Packets Lost: " << packetsLost << "\n";
  std::cout << "Packet Loss Ratio: " << packetLossRatio * 100 << " %\n";
  std::cout << "Throughput: " << throughput << " Mbps\n";
  std::cout << "Total Energy Consumed: " << energyConsumed << " J\n";
  std::cout << "Overhead Efficiency: " << overheadEfficiency << " %\n";
}

int main (int argc, char *argv[])
{
  LogComponentEnable ("BackoffExample", LOG_LEVEL_ALL);

  // Set default parameters for Backoff and UAVs
  Time slotTime = MicroSeconds (1);
  uint32_t minSlots = 1;
  uint32_t maxSlots = 16;
  uint32_t ceiling = 10;
  uint32_t maxRetries = 5;

  // Default UAV count
  uint32_t numberOfUAVs = 5; 

  // Parse command-line arguments to allow changing UAV count
  CommandLine cmd;
  cmd.AddValue ("nUAV", "Number of UAVs", numberOfUAVs);
  cmd.Parse (argc, argv);

  // Create UAV nodes
  NodeContainer uavNodes;
  uavNodes.Create(numberOfUAVs);

  // Set mobility for UAVs (optional, for simulation realism)
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(uavNodes);

  // Initialize Backoff and packets
  Backoff customBackoff (slotTime, minSlots, maxSlots, ceiling, maxRetries);
  uint32_t totalPacketsSent = 1000 * numberOfUAVs;  // More UAVs mean more packets

  for (uint32_t i = 0; i < totalPacketsSent; ++i)
  {
    if (customBackoff.MaxRetriesReached ())
    {
      NS_LOG_INFO ("Max retries reached");
      break;
    }

    Time backoffTime = customBackoff.GetBackoffTime ();
    NS_LOG_INFO ("Backoff time: " << backoffTime.GetMicroSeconds () << " microseconds");
    customBackoff.IncrNumRetries ();
  }

  // Schedule TCP Timer and Hello Packet events (once for all UAVs)
  Simulator::Schedule (Seconds (1.0), &Backoff::StartTcpTimer, &customBackoff, Seconds (2.0));
  Simulator::Schedule (Seconds (1.0), &Backoff::SendHelloPacket, &customBackoff);

  Simulator::Run ();
  Simulator::Destroy ();

  // Calculate and display performance metrics once
  CalculatePerformanceMetrics(customBackoff, totalPacketsSent, numberOfUAVs);

  return 0;
}

