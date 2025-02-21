/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef BACKOFF_H
#define BACKOFF_H

#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class Backoff {
public:
  Backoff (void);
  Backoff (Time slotTime, uint32_t minSlots, uint32_t maxSlots, uint32_t ceiling, uint32_t maxRetries);

  Time GetBackoffTime (void);
  void ResetBackoffTime (void);
  bool MaxRetriesReached (void);
  void IncrNumRetries (void);
  void StartTcpTimer (Time timeout);
  void HandleTcpTimeout (void);
  void SendHelloPacket (void);

private:
  uint32_t m_minSlots;
  uint32_t m_maxSlots;
  uint32_t m_ceiling;
  uint32_t m_maxRetries;
  Time m_slotTime;
  uint32_t m_numBackoffRetries;
  Ptr<UniformRandomVariable> m_rng;
};

} // namespace ns3

#endif /* BACKOFF_H */

