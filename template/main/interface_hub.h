// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_INTERFACE_HUB_H_
#define TEMPLATE_MAIN_INTERFACE_HUB_H_

#include <queue>
#include "xi/thread.hxx"
#include "xi/thread_pool.hxx"
#include "rp/payload.hxx"
#include "template/main/property.h"

namespace ih {


class RpTimer;
class Http1Api;
class KafkaProducer;

class InterfaceHub : public xi::Singleton<InterfaceHub>
{
   public :
      InterfaceHub();
      ~InterfaceHub();

      bool                    Initialize();
      xi::ThreadPool         *ThreadPool();
      bool                    ChangeResource(std::vector<std::string> param);
      std::string             RetrieveDebug(const char *name);

      void                    ExecThread(std::unique_ptr<xi::rp::Payload> primitive);
      void                    Handle(xi::rp::Payload &message);
      void                    HandleAndDestruct(xi::rp::Payload *message);
      xi::rp::result::e       Send(xi::rp::Payload &message);

      // timer
      xi::timerid_t           StartTimer(xi::rp::Payload &primitive);
      bool                    StopTimer(xi::rp::Payload &primitive);

      // HTTP
      bool                    CloseHttpSocket(xi::SocketAddr &addr);
      bool                    ExistHttpSocket(xi::SocketAddr &addr);

   private :                
      void                    ReplyForNotFound(xi::rp::Payload &message, uint16_t status_code = 404);
      void                    SendHttpFailResponse(IN xi::rp::Payload &message, uint16_t status_code);

   private :                
      xi::ThreadPool         *thread_pool_;
      ih::RpTimer            *timer_;
      ih::Http1Api           *http1api_;
};


}
#endif
