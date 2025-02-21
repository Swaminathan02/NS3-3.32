/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef HELLO_PACKET_H
#define HELLO_PACKET_H

#include "ns3/nstime.h"
#include "ns3/timer.h"

namespace ns3 {

class HelloPacket {
public:
  HelloPacket(Time helloInterval);
  void Start();
  void Stop();
  bool IsRunning() const;

private:
  void SendHello();

  Time m_helloInterval;
  Timer m_timer;
};

} // namespace ns3

#endif /* HELLO_PACKET_H */

