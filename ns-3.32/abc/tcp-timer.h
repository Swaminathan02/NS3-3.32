/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef TCP_TIMER_H
#define TCP_TIMER_H

#include "ns3/nstime.h"
#include "ns3/timer.h"

namespace ns3 {

class TcpTimer {
public:
  TcpTimer(Time retransmissionTimeout);
  void Start();
  void Stop();
  bool IsRunning() const;
  void Retransmit();

private:
  Time m_retransmissionTimeout;
  Timer m_timer;
};

} // namespace ns3

#endif /* TCP_TIMER_H */

