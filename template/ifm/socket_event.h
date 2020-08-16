// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_IFM_SOCKET_EVENT_H_
#define TEMPLATE_IFM_SOCKET_EVENT_H_

#include "xi/socket_addr.hxx"
#include "rp/payload.hxx"
#include "template/ifm/define.h"

namespace ifm {

class SocketEvent : public xi::rp::Payload
{
   public :
      typedef enum {
         P_Unknown,
         P_Sip,
         P_Msrp,
         P_OutOfRange
      } Protocol_t;

      typedef enum {
         E_Unknown,
         E_Accepted,
         E_TryConnect,
         E_Connected,
         E_ConnectFail,
         E_Closed,
         E_OutOfRange
      } Event_t;

   public :
      SocketEvent();
      virtual ~SocketEvent();

      virtual xi::rp::Payload  *Clone();

      Protocol_t            GetProtocol();
      void                  SetProtocol(Protocol_t prot);

      Event_t               GetEvent();
      void                  SetEvent(Event_t event);

      void                  GetSocketAddr(xi::SocketAddr &rAddr);
      void                  SetSocketAddr(xi::SocketAddr &rAddr);

   private :
      Protocol_t            m_Protocol;
      Event_t               m_Event;
      xi::SocketAddr        m_Addr;
};

}

#endif
