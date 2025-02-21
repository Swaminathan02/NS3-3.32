#include "ns3/core-module.h"
#include "backoff.h"
#include "tcp-timer.h"
#include "hello-packet.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BackoffExample");

int main (int argc, char *argv[]) {
  // Enable logging for the Backoff, TcpTimer, and HelloPacket classes
  LogComponentEnable ("Backoff", LOG_LEVEL_ALL);
  LogComponentEnable ("BackoffExample", LOG_LEVEL_ALL);
  LogComponentEnable ("TcpTimer", LOG_LEVEL_ALL);
  LogComponentEnable ("HelloPacket", LOG_LEVEL_ALL);

  // Create a Backoff object with default parameters
  Backoff backoff;

  // Example parameters for Backoff
  Time slotTime = MicroSeconds (1);
  uint32_t minSlots = 1;
  uint32_t maxSlots = 16;
  uint32_t ceiling = 10;
  uint32_t maxRetries = 5;

  // Create a Backoff object with specific parameters
  Backoff customBackoff (slotTime, minSlots, maxSlots, ceiling, maxRetries);

  // Example usage of the Backoff object
  for (uint32_t i = 0; i < 10; ++i) {
    if (customBackoff.MaxRetriesReached ()) {
      NS_LOG_INFO ("Max retries reached");
      break;
    }

    Time backoffTime = customBackoff.GetBackoffTime ();
    NS_LOG_INFO ("Backoff time: " << backoffTime.GetMicroSeconds () << " microseconds");

    customBackoff.IncrNumRetries ();
  }

  // TCP Timer
  Time retransmissionTimeout = Seconds(1);
  TcpTimer tcpTimer(retransmissionTimeout);
  tcpTimer.Start();

  // Hello Packet
  Time helloInterval = Seconds(2);
  HelloPacket helloPacket(helloInterval);
  helloPacket.Start();

  // Run the simulation for a specific time to observe timers
  Simulator::Stop(Seconds(10));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

