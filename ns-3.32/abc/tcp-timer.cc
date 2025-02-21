/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "tcp-timer.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpTimer");

TcpTimer::TcpTimer(Time retransmissionTimeout)
  : m_retransmissionTimeout(retransmissionTimeout) {
  m_timer.SetFunction(&TcpTimer::Retransmit, this);
}

void TcpTimer::Start() {
  m_timer.Schedule(m_retransmissionTimeout);
  NS_LOG_INFO("TCP timer started for " << m_retransmissionTimeout.GetSeconds() << " seconds");
}

void TcpTimer::Stop() {
  if (m_timer.IsRunning()) {
    m_timer.Cancel();
    NS_LOG_INFO("TCP timer stopped");
  }
}

bool TcpTimer::IsRunning() const {
  return m_timer.IsRunning();
}

void TcpTimer::Retransmit() {
  NS_LOG_INFO("TCP retransmission timeout occurred");
  // Implement retransmission logic here
}

} // namespace ns3

