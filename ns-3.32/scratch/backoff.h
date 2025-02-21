#ifndef BACKOFF_H
#define BACKOFF_H

#include <stdint.h>
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

/**
 * \ingroup csma
 * \brief The backoff class is used for calculating backoff times
 * when many net devices can write to the same channel
 */

class Backoff {
public:
  // Existing variables
  uint32_t m_minSlots; 
  uint32_t m_maxSlots; 
  uint32_t m_ceiling;
  uint32_t m_maxRetries;
  Time m_slotTime;

  // New variables for performance metrics
  uint32_t m_packetsSent;
  uint32_t m_packetsReceived;
  uint32_t m_packetsLost;
  double m_throughput;
  double m_energyConsumed;
  double m_overheadEfficiency;
  double m_collisionRatio;

  Backoff (void);
  Backoff (Time slotTime, uint32_t minSlots, uint32_t maxSlots, uint32_t ceiling, uint32_t maxRetries);

  Time GetBackoffTime ();
  void ResetBackoffTime (void);
  bool MaxRetriesReached (void);
  void IncrNumRetries (void);

  int64_t AssignStreams (int64_t stream);

  // New methods for updating metrics
  void IncrementPacketsSent ();
  void IncrementPacketsReceived ();
  void IncrementPacketsLost ();
  void CalculateMetrics ();

  // New method to get a random value
  double GetRandomValue ();

private:
  uint32_t m_numBackoffRetries;
  Ptr<UniformRandomVariable> m_rng;
};

} // namespace ns3

#endif /* BACKOFF_H */

