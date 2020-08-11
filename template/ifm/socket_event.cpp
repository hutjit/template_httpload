#include "socket_event.h"

namespace ifm {


SocketEvent::SocketEvent() : xi::rp::Payload(ifm::msgtype::SOCKET_EVENT)
{
   m_Protocol  = P_Unknown;
   m_Event     = E_Unknown;
}

SocketEvent::~SocketEvent()
{
}

xi::rp::Payload *SocketEvent::Clone()
{
   SocketEvent *pClone = new SocketEvent();
   *pClone = *this;

   return pClone;
}

SocketEvent::Protocol_t SocketEvent::GetProtocol()
{
   return m_Protocol;
}

void SocketEvent::SetProtocol(Protocol_t prot)
{
   m_Protocol = prot;
}

SocketEvent::Event_t SocketEvent::GetEvent()
{
   return m_Event;
}

void SocketEvent::SetEvent(Event_t event)
{
   m_Event = event;
}

void SocketEvent::GetSocketAddr(xi::SocketAddr &rAddr)
{
   rAddr = m_Addr;
}

void SocketEvent::SetSocketAddr(xi::SocketAddr &rAddr)
{
   m_Addr = rAddr;
}


}
