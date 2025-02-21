/*
#include "ns3/core-module.h"
#include "backoff.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BackoffExample");

Backoff::Backoff ()
  : m_minSlots (1),
    m_maxSlots (16),
    m_ceiling (10),
    m_maxRetries (5),
    m_slotTime (MicroSeconds (1)),
    m_packetsSent (0),
    m_packetsReceived (0),
    m_packetsLost (0),
    m_throughput (0.0),
    m_energyConsumed (0.0),
    m_overheadEfficiency (0.0),
    m_collisionRatio (0.0),
    m_numBackoffRetries (0),
    m_rng (CreateObject<UniformRandomVariable> ())
{
}

Backoff::Backoff (Time slotTime, uint32_t minSlots, uint32_t maxSlots, uint32_t ceiling, uint32_t maxRetries)
  : m_minSlots (minSlots),
    m_maxSlots (maxSlots),
    m_ceiling (ceiling),
    m_maxRetries (maxRetries),
    m_slotTime (slotTime),
    m_packetsSent (0),
    m_packetsReceived (0),
    m_packetsLost (0),
    m_throughput (0.0),
    m_energyConsumed (0.0),
    m_overheadEfficiency (0.0),
    m_collisionRatio (0.0),
    m_numBackoffRetries (0),
    m_rng (CreateObject<UniformRandomVariable> ())
{
}

Time Backoff::GetBackoffTime ()
{
  uint32_t backoffSlots = m_rng->GetInteger (m_minSlots, m_maxSlots);
  return MicroSeconds (backoffSlots * m_slotTime.GetMicroSeconds ());
}

void Backoff::ResetBackoffTime ()
{
  m_numBackoffRetries = 0;
}

bool Backoff::MaxRetriesReached ()
{
  return m_numBackoffRetries >= m_maxRetries;
}

void Backoff::IncrNumRetries ()
{
  ++m_numBackoffRetries;
}

// New methods for updating metrics
void Backoff::IncrementPacketsSent ()
{
  ++m_packetsSent;
}

void Backoff::IncrementPacketsReceived ()
{
  ++m_packetsReceived;
}

void Backoff::IncrementPacketsLost ()
{
  ++m_packetsLost;
}

void Backoff::CalculateMetrics ()
{
  // Calculate throughput, assuming slotTime represents the packet transmission time
  m_throughput = (m_packetsReceived * 8.0) / (m_slotTime.GetSeconds () * m_packetsSent);
  
  // Placeholder for energy consumed calculation
  // Assuming a simple model where energy consumed is proportional to packets sent
  m_energyConsumed = m_packetsSent * 0.001;  // Example: 0.001 units of energy per packet

  // Calculate overhead efficiency
  m_overheadEfficiency = static_cast<double>(m_packetsReceived) / m_packetsSent;

  // Calculate collision ratio
  m_collisionRatio = static_cast<double>(m_packetsLost) / (m_packetsSent + m_packetsLost);
}

// New method to get a random value
double Backoff::GetRandomValue ()
{
  return m_rng->GetValue ();
}

int main (int argc, char *argv[])
{
  // Enable logging for the Backoff class
  LogComponentEnable ("Backoff", LOG_LEVEL_ALL);
  LogComponentEnable ("BackoffExample", LOG_LEVEL_ALL);

  // Create a Backoff object with default parameters
  Backoff backoff;

  // Example parameters
  Time slotTime = MicroSeconds (1);
  uint32_t minSlots = 1;
  uint32_t maxSlots = 16;
  uint32_t ceiling = 10;
  uint32_t maxRetries = 4;

  // Create a Backoff object with specific parameters
  Backoff customBackoff (slotTime, minSlots, maxSlots, ceiling, maxRetries);

  // Manually set the number of packets to send
  uint32_t packetsToSend = 100;  // Change this value as needed

  // Example usage of the Backoff object
  for (uint32_t i = 0; i < packetsToSend; ++i)
  {
    customBackoff.ResetBackoffTime();
    customBackoff.IncrementPacketsSent();

    while (true)
    {
      if (customBackoff.MaxRetriesReached())
      {
        customBackoff.IncrementPacketsLost();
        NS_LOG_INFO("Max retries reached for packet " << i);
        break;
      }

      Time backoffTime = customBackoff.GetBackoffTime();
      NS_LOG_INFO("Backoff time: " << backoffTime.GetMicroSeconds() << " microseconds");

      // Simulate successful packet reception with a 90% probability
      if (customBackoff.GetRandomValue() < 0.9)
      {
        customBackoff.IncrementPacketsReceived();
        break;
      }
      else
      {
        customBackoff.IncrNumRetries();
      }
    }
  }

  customBackoff.CalculateMetrics();
  double plr = static_cast<double>(customBackoff.m_packetsLost) / customBackoff.m_packetsSent;

  NS_LOG_INFO("Packets sent: " << customBackoff.m_packetsSent);
  NS_LOG_INFO("Packets received: " << customBackoff.m_packetsReceived);
  NS_LOG_INFO("Packets lost: " << customBackoff.m_packetsLost);
  NS_LOG_INFO("Packet Loss Ratio (PLR): " << plr * 100 << " %");
  NS_LOG_INFO("Throughput: " << (double)(customBackoff.m_throughput / 1000000) << " Mbps");
  NS_LOG_INFO("Overhead efficiency: " << customBackoff.m_overheadEfficiency);
  NS_LOG_INFO("Collision ratio: " << customBackoff.m_collisionRatio);
  NS_LOG_INFO("Energy consumed: " << customBackoff.m_energyConsumed << " units");

  return 0;
}
*/

#include "ns3/core-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BackoffExample");

class Backoff {
public:
  Backoff()
      : m_minSlots(1), m_maxSlots(16), m_ceiling(10), m_maxRetries(5),
        m_slotTime(MicroSeconds(1)), m_packetsSent(0), m_packetsReceived(0),
        m_packetsLost(0), m_throughput(0.0), m_energyConsumed(0.0),
        m_overheadEfficiency(0.0), m_collisionRatio(0.0),
        m_numBackoffRetries(0), m_rng(CreateObject<UniformRandomVariable>()) {}

  Backoff(Time slotTime, uint32_t minSlots, uint32_t maxSlots,
          uint32_t ceiling, uint32_t maxRetries)
      : m_minSlots(minSlots), m_maxSlots(maxSlots), m_ceiling(ceiling),
        m_maxRetries(maxRetries), m_slotTime(slotTime), m_packetsSent(0),
        m_packetsReceived(0), m_packetsLost(0), m_throughput(0.0),
        m_energyConsumed(0.0), m_overheadEfficiency(0.0),
        m_collisionRatio(0.0), m_numBackoffRetries(0),
        m_rng(CreateObject<UniformRandomVariable>()) {}

  Time GetBackoffTime() {
    uint32_t backoffSlots = m_rng->GetInteger(m_minSlots, m_maxSlots);
    return MicroSeconds(backoffSlots * m_slotTime.GetMicroSeconds());
  }

  void ResetBackoffTime() { m_numBackoffRetries = 0; }

  bool MaxRetriesReached() { return m_numBackoffRetries >= m_maxRetries; }

  void IncrNumRetries() { ++m_numBackoffRetries; }

  void IncrementPacketsSent() { ++m_packetsSent; }

  void IncrementPacketsReceived() { ++m_packetsReceived; }

  void IncrementPacketsLost() { ++m_packetsLost; }

  void CalculateMetrics() {
    m_throughput = (m_packetsReceived * 8.0) /
                   (m_slotTime.GetSeconds() * m_packetsSent);
    m_energyConsumed = m_packetsSent * 0.001;
    m_overheadEfficiency =
        static_cast<double>(m_packetsReceived) / m_packetsSent;
    m_collisionRatio = static_cast<double>(m_packetsLost) /
                       (m_packetsSent + m_packetsLost);
  }

  double GetRandomValue() { return m_rng->GetValue(); }

  void SetCWRangeBasedOnRegion(double speed, double density) {
    if (density <= 50 && speed <= 10) {
      m_minSlots = 32;
      m_maxSlots = 64;
    } else if (density <= 50 && speed > 10 && speed <= 15) {
      m_minSlots = 64;
      m_maxSlots = 128;
    } else if (density <= 50 && speed > 15) {
      m_minSlots = 128;
      m_maxSlots = 256;
    } else if (density > 50 && density <= 75 && speed <= 10) {
      m_minSlots = 64;
      m_maxSlots = 128;
    } else if (density > 50 && density <= 75 && speed > 10 && speed <= 15) {
      m_minSlots = 128;
      m_maxSlots = 256;
    } else if (density > 50 && density <= 75 && speed > 15) {
      m_minSlots = 256;
      m_maxSlots = 384;
    } else if (density > 75 && speed <= 10) {
      m_minSlots = 256;
      m_maxSlots = 384;
    } else if (density > 75 && speed > 10 && speed <= 15) {
      m_minSlots = 384;
      m_maxSlots = 512;
    } else if (density > 75 && speed > 15) {
      m_minSlots = 512;
      m_maxSlots = 1024;
    }
  }

  // Public getter methods
  uint32_t GetPacketsSent() const { return m_packetsSent; }
  uint32_t GetPacketsReceived() const { return m_packetsReceived; }
  uint32_t GetPacketsLost() const { return m_packetsLost; }
  double GetThroughput() const { return m_throughput; }
  double GetEnergyConsumed() const { return m_energyConsumed; }
  double GetOverheadEfficiency() const { return m_overheadEfficiency; }
  double GetCollisionRatio() const { return m_collisionRatio; }

private:
  uint32_t m_minSlots;
  uint32_t m_maxSlots;
  uint32_t m_ceiling;
  uint32_t m_maxRetries;
  Time m_slotTime;
  uint32_t m_packetsSent;
  uint32_t m_packetsReceived;
  uint32_t m_packetsLost;
  double m_throughput;
  double m_energyConsumed;
  double m_overheadEfficiency;
  double m_collisionRatio;
  uint32_t m_numBackoffRetries;
  Ptr<UniformRandomVariable> m_rng;
};

int main(int argc, char *argv[]) {
  LogComponentEnable("Backoff", LOG_LEVEL_ALL);
  LogComponentEnable("BackoffExample", LOG_LEVEL_ALL);

  Backoff backoff;

  Time slotTime = MicroSeconds(1);
  uint32_t minSlots = 1;
  uint32_t maxSlots = 16;
  uint32_t ceiling = 10;
  uint32_t maxRetries = 4;

  Backoff customBackoff(slotTime, minSlots, maxSlots, ceiling, maxRetries);

  double speed = 12.0;
  double density = 65.0;

  customBackoff.SetCWRangeBasedOnRegion(speed, density);

  uint32_t packetsToSend = 100;

  for (uint32_t i = 0; i < packetsToSend; ++i) {
    customBackoff.ResetBackoffTime();
    customBackoff.IncrementPacketsSent();

    while (true) {
      if (customBackoff.MaxRetriesReached()) {
        customBackoff.IncrementPacketsLost();
        NS_LOG_INFO("Max retries reached for packet " << i);
        break;
      }

      Time backoffTime = customBackoff.GetBackoffTime();
      NS_LOG_INFO("Backoff time: " << backoffTime.GetMicroSeconds()
                                   << " microseconds");

      if (customBackoff.GetRandomValue() < 0.9) {
        customBackoff.IncrementPacketsReceived();
        break;
      } else {
        customBackoff.IncrNumRetries();
      }
    }
  }

  customBackoff.CalculateMetrics();
  double plr = static_cast<double>(customBackoff.GetPacketsLost()) /
               customBackoff.GetPacketsSent();

  NS_LOG_INFO("Packets sent: " << customBackoff.GetPacketsSent());
  NS_LOG_INFO("Packets received: " << customBackoff.GetPacketsReceived());
  NS_LOG_INFO("Packets lost: " << customBackoff.GetPacketsLost());
  NS_LOG_INFO("Packet Loss Ratio (PLR): " << plr * 100 << " %");
  NS_LOG_INFO("Throughput: " << (double)(customBackoff.GetThroughput() / 1000000)
                             << " Mbps");
  NS_LOG_INFO("Overhead efficiency: " << customBackoff.GetOverheadEfficiency());
  NS_LOG_INFO("Collision ratio: " << customBackoff.GetCollisionRatio());
  NS_LOG_INFO("Energy consumed: " << customBackoff.GetEnergyConsumed()
                                  << " units");

  return 0;
}

