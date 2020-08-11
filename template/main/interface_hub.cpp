// vim:ts=3:sts=3:sw=3

#include "interface_hub.h"
#include <memory>
#include <typeinfo>
#include "xi/logger.hxx"
#include "xi/util.hxx"
#include "template/ifm/http1_protocol.h"
#include "template/ifm/timer_event.h"
#include "template/tum/manager.h"
#include "session.h"
#include "rp_pool.h"
#include "rp_timer.h"
#include "load_generator.h"
#include "http1_api.h"


namespace ih {


InterfaceHub::InterfaceHub()
{
   thread_pool_ = NULL;
   timer_ = NULL;
   http1api_ = NULL;
}

InterfaceHub::~InterfaceHub()
{
   if (thread_pool_) {
      delete thread_pool_;
      thread_pool_ = NULL;
   }

   if (http1api_) {
      delete http1api_;
      http1api_ = NULL;
   }
}

bool InterfaceHub::Initialize()
{
   static const char *FN = "[HUB::Initialize] ";

   if (thread_pool_) {
      WLOG(FN << "already up");
      return false;
   }

   // thread-pool
   thread_pool_ = new xi::ThreadPool;
   if (false == thread_pool_->Initialize(4, 512 * 1024)) {
      delete thread_pool_;
      thread_pool_ = NULL;
      WLOG(FN << "ThreadPool::Initialize fail");
      return false;
   }

   // timer
   timer_ = new ih::RpTimer(this);

   // load-generator
   if (false == LoadGenerator::Instance()->Initialize()) {
      WLOG(FN << "LoadGenerator::Initialize() fail");
      return false;
   }

   // http
   http1api_ = new ih::Http1Api(this, ih::InterfaceHub::Instance()->ThreadPool());
   http1api_->Start();

   return true;
}

xi::ThreadPool *InterfaceHub::ThreadPool()
{
   return thread_pool_;
}

bool InterfaceHub::ChangeResource(std::vector<std::string> param)
{
   static const char *FN = "[HUB::ChangeResource] ";

   std::string name = param[0];
   if (name == "http2-reload-server-certificate") {
      std::string cert = param[1];
      std::string key = param[2];
      if (cert.empty() || key.empty()) {
         WLOG(FN << "invalid parameter cert:" << cert << " key:" << key);
         return false;
      }

      //return http2api_->ReloadServerCertificate(cert.c_str(), key.c_str());
   }

   WLOG(FN << "unsupported param:" << name);

   return false;
}

std::string InterfaceHub::RetrieveDebug(const char *name)
{
   //if (xi::StrEqualNoCase(name, "HTTP2-STATUS"))
   //   return http2api_->Status();
   if (xi::StrEqualNoCase(name, "HTTP1-DEBUG-STATISTICS"))
      return http1api_->DebugStat();

   return "(unknown)";
}

void InterfaceHub::ExecThread(std::unique_ptr<xi::rp::Payload> primitive)
{
   xi::Task *pJob = new xi::TaskWithParam<InterfaceHub, xi::rp::Payload>(this, &InterfaceHub::HandleAndDestruct, primitive.release());
   thread_pool_->Enqueue(pJob);
}

void InterfaceHub::Handle(xi::rp::Payload &message)
{
   static const char *FN = "[HUB::Handle] ";
   static RpPool *rp_pool = RpPool::Instance();

   xi::rp::sessid_t sid = message.GetDstSessId();

   if (sid) {
      ih::Session *ihsess = dynamic_cast<ih::Session*>(rp_pool->Find(td::functype::IH_SESSION, sid));
      if (ihsess) {
         ihsess->OnReceive(message);
      } else {
         WLOG(FN << "not-found ih-session:" << sid << " " << typeid(message).name());
         ReplyForNotFound(message);
      }
      return;
   }


   switch (message.GetType()) {
      case ifm::msgtype::LOAD_COMMAND :
      case ifm::msgtype::HTTP1_PROTOCOL :
         {
            ih::Session *ihsess = dynamic_cast<ih::Session*>(rp_pool->Obtain(td::functype::IH_SESSION));
            if (ihsess) {
               ihsess->OnReceive(message);

            } else {
               WLOG(FN << "obtain-fail ih-session discard:" << typeid(message).name());
               ReplyForNotFound(message, 500);
            }

         } break;

      default :
         {
            DLOG(FN << RED("not-support!!"));
         } break;
   }
}

void InterfaceHub::HandleAndDestruct(xi::rp::Payload *message)
{
   std::unique_ptr<xi::rp::Payload> autoptr(message);
   Handle(*message);
}

xi::rp::result::e InterfaceHub::Send(xi::rp::Payload &message)
{
   static const char *FN = "[HUB::Send] ";

   xi::rp::result::e result = xi::rp::result::FAIL;

   switch (message.GetType()) {
      case ifm::msgtype::HTTP1_PROTOCOL :
         {
            if (NULL == http1api_)
               return xi::rp::result::INTERFACE_FAIL;

            result = (http1api_->Send(message) ? xi::rp::result::SUCCESS : xi::rp::result::FAIL);
         } break;

      default :
         {
            WLOG(FN << "unsupported msgtype:" << ifm::msgtype::name((ifm::msgtype::e)message.GetType()));
         } break;
   }

   return result;
}

xi::timerid_t InterfaceHub::StartTimer(xi::rp::Payload &primitive)
{
   return timer_->StartTimer(primitive);
}

bool InterfaceHub::StopTimer(xi::rp::Payload &primitive)
{
   return timer_->StopTimer(primitive);
}

bool InterfaceHub::CloseHttpSocket(xi::SocketAddr &addr)
{
   return (http1api_ ? http1api_->Close(addr) : false);
}

bool InterfaceHub::ExistHttpSocket(xi::SocketAddr &addr)
{
   return (http1api_ ? http1api_->Exist(addr) : false);
}

void InterfaceHub::ReplyForNotFound(xi::rp::Payload &message, uint16_t status_code)
{
   static const char *FN = "[HUB::ReplyForNotFound] ";

   switch (message.GetType()) {
      case ifm::msgtype::TIMER_EVENT :
         {
            // NONE
         } break;

      case ifm::msgtype::HTTP1_PROTOCOL :
         {
            SendHttpFailResponse(message, status_code);
         } break;

      default :
         {
            WLOG(FN << "unsupported msgtype:" << ifm::msgtype::name((ifm::msgtype::e)message.GetType()));
         } break;
   }
}

void InterfaceHub::SendHttpFailResponse(IN xi::rp::Payload &message, uint16_t status_code)
{
   ifm::Http1Protocol &proto = static_cast<ifm::Http1Protocol&>(message);
   if (NULL == proto.GetRequest())
      return;

   std::unique_ptr<xi::h1::Response> response(new xi::h1::Response(*proto.GetRequest(), status_code));
   response->AddHeader("Connection", "close");
   if (429 == status_code)
      response->AddHeader("Retry-After", "5");

   ifm::Http1Protocol reply;
   reply.PushResponse(std::move(response));

   http1api_->Send(reply);
}


}
