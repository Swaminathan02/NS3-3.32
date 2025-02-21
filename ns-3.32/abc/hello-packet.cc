/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "hello-packet.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HelloPacket");

HelloPacket::HelloPacket(Time helloInterval)
  : m_helloInterval(helloInterval) {
  m_timer.SetFunction(&HelloPacket::SendHello, this);
}

void HelloPacket::Start() {
  m_timer.Schedule(m_helloInterval);
  NS_LOG_INFO("Hello packet timer started with interval " << m_helloInterval.GetSeconds() << " seconds");
}

void HelloPacket::Stop() {
  if (m_timer.IsRunning()) {
    m_timer.Cancel();
    NS_LOG_INFO("Hello packet timer stopped");
  }
}

bool HelloPacket::IsRunning() const {
  return m_timer.IsRunning();
}

void HelloPacket::SendHello() {
  NS_LOG_INFO("Sending hello packet");
  // Implement hello packet sending logic here
  m_timer.Schedule(m_helloInterval);
}

} // namespace ns3

