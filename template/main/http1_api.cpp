// vim:ts=3:sts=3:sw=3

#include "http1_api.h"
#include "xi/logger.hxx"
#include "xi/util.hxx"
#include "template/ifm/http1_protocol.h"
#include "property.h"
#include "interface_hub.h"
#include "statistics.h"

namespace ih {


Http1Api::Http1Api(InterfaceHub *callback, xi::ThreadPool *threadpool)
{
   callback_ = callback;
   ih::Property *property = ih::Property::Instance();

   xi::h1::Parameters conf;
   conf.thread_pool_ = threadpool;

   if (property->GetHttpBindTlsPort()) { // TLS 
      xi::SocketAddr addr;
      addr.SetIpPort("0.0.0.0", property->GetHttpBindTlsPort());
      addr.SetIpType(xi::iptype::TLS);

      conf.bind_list_.push_back(addr);

      conf.server_cert_path_ = property->GetHttpServerCertificate();
      conf.server_key_path_  = property->GetHttpServerCertKey();
   }

   if (property->GetHttpBindTcpPort()) { // TCP 
      xi::SocketAddr addr;
      addr.SetIpPort("0.0.0.0", property->GetHttpBindTcpPort());
      addr.SetIpType(xi::iptype::TCP);

      conf.bind_list_.push_back(addr);
   }

   if (false == Initialize(conf))
   {
      ELOG("http initialize fail");
      _EXIT(1);
   }
}

Http1Api::~Http1Api()
{
}

void Http1Api::OnNotify(xi::ioevt::e status, xi::SocketAddr &addr)
{
   static const char *FN = "[Http1Api::OnSocketStatus] ";
   char IPSTR[8*sizeof("ffff")];

   switch (status) {
      case xi::ioevt::PEER_CLOSE :
         DLOG(FN << "CLOSE FD:" << addr.GetFD() << " SID:" << addr.GetSocketID()
               << " " << addr.GetIpStr(IPSTR, sizeof(IPSTR)) << ":" << addr.GetPort());
         break;
      case xi::ioevt::ACCEPT :
         DLOG(FN << "ACCEPT FD:" << addr.GetFD() << " SID:" << addr.GetSocketID()
               << " " << addr.GetIpStr(IPSTR, sizeof(IPSTR)) << ":" << addr.GetPort());
         break;
      case xi::ioevt::CONNECT :
         DLOG(FN << "CONNECTED FD:" << addr.GetFD() << " SID:" << addr.GetSocketID()
               << " " << addr.GetIpStr(IPSTR, sizeof(IPSTR)) << ":" << addr.GetPort());
         break;
      case xi::ioevt::CONNECT_FAIL :
         DLOG(FN << "CONNECT-FAIL FD:" << addr.GetFD() << " SID:" << addr.GetSocketID()
               << " " << addr.GetIpStr(IPSTR, sizeof(IPSTR)) << ":" << addr.GetPort());
         break;
      default :
         DLOG(FN << "UNKNOWN:" << status << ":" << xi::ioevt::name(status) << " FD:" << addr.GetFD() << " SID:" << addr.GetSocketID()
               << " " << addr.GetIpStr(IPSTR, sizeof(IPSTR)) << ":" << addr.GetPort());
         break;
   }
}

void Http1Api::OnReceive(std::unique_ptr<xi::h1::Request> request)
{
   ih::Statistics::Instance()->StatProtocol(true, stat::protocol::code(xi::h1::method::name(request->GetMethod())), 0);

   ifm::Http1Protocol proto;
   proto.PushRequest(std::move(request));
   callback_->Handle(proto);
}

void Http1Api::OnReceive(std::unique_ptr<xi::h1::Response> response)
{
   ih::Statistics::Instance()->StatProtocol(true,
         stat::protocol::code(xi::h1::method::name(response->GetRequestMethod())),
         (int16_t)response->GetStatusCode());

   xi::rp::sessid_t sess_id = response->GetUserParam() & 0xFFFFFFFF;
   xi::rp::membid_t memb_id = (response->GetUserParam() >> 32) & 0xFFFF;

   ifm::Http1Protocol proto;
   proto.SetDstSessId(sess_id);
   proto.SetDstMembId(memb_id);
   proto.PushResponse(std::move(response));
   callback_->Handle(proto);
}

void Http1Api::OnSendFail(std::unique_ptr<xi::h1::Request> request)
{
}

bool Http1Api::Send(xi::rp::Payload &message)
{
   static const char *FN = "[Http1Api::Send] ";

   ifm::Http1Protocol &proto = static_cast<ifm::Http1Protocol&>(message);

   if (proto.GetRequest())
   {
      std::unique_ptr<xi::h1::Request> request(proto.PopRequest());
      uint64_t param = proto.GetSrcMembId();
      param <<= 32;
      param |= proto.GetSrcSessId();
      request->SetUserParam(param);

      ih::Statistics::Instance()->StatProtocol(false, stat::protocol::code(xi::h1::method::name(request->GetMethod())), 0);

      if (request->GetSocketAddr().GetSocketID()) {
         if (false == Exist(request->GetSocketAddr())) {
            WLOG(FN << "not-exist connection fd:" << request->GetSocketAddr().GetFD() << " sid:" << request->GetSocketAddr().GetSocketID());
            return false;
         }

         return xi::h1::Api::Send(std::move(request));
      }

      return xi::h1::Api::Connect(std::move(request), proto.RefAssignAddr());
   }
   else if (proto.GetResponse())
   {
      std::unique_ptr<xi::h1::Response> response(proto.PopResponse());

      ih::Statistics::Instance()->StatProtocol(false,
            stat::protocol::code(xi::h1::method::name(response->GetRequestMethod())),
            (int16_t)response->GetStatusCode());

      return xi::h1::Api::Send(std::move(response));
   }
   else
   {
      WLOG(FN << "invalid message(null)");
   }

   return false;
}

//void Http1Api::OnReceive(xi::h1::HttpMessage &http)
//{
//   //LLT(false, http, http.GetRemoteAddr());
//   callback_->OnHttpReceive(http);
//}
//
//bool Http1Api::Send(xi::h1::HttpRequest &req, xi::SocketAddr &return_addr)
//{
//   bool result = xi::h1::Stack::Send(req, return_addr);
//   if (result)
//      //LLT(true, req, return_addr);
//   return result;
//}
//
//bool Http1Api::Send(xi::h1::HttpMessage &msg, xi::ioctrl::e sendopt)
//{
//   bool result = xi::h1::Stack::Send(msg, sendopt);
//   if (result)
//      //LLT(true, msg, msg.GetRemoteAddr());
//   return result;
//}
//
//bool Http1Api::Close(xi::SocketAddr &addr)
//{
//   return xi::h1::Stack::Close(addr);
//}

//void Http1Api::LLT(bool is_send, xi::h1::HttpMessage &httpmsg, xi::SocketAddr &remote_addr)
//{
//   // if (false == CANLLTLOGGING())
//   //    return;
//   
//   xi::String LLT(512);
//   ih::stat::protocol::e stat_protocol = ih::stat::protocol::NONE;
//   int status_code = 0;
//
//   if (httpmsg.IsRequest())
//   {
//      xi::h1::HttpRequest &request = static_cast<xi::h1::HttpRequest&>(httpmsg);
//      LLT.Csnprintf(256, "P:HTTP S:%d:%u D:%s M:REQ:%s",
//            remote_addr.GetFD(), remote_addr.GetSocketID(),
//            (is_send ? "SEND" : "RECV"),
//            xi::h1::methodtype::name(request.GetMethod()));
//
//      LLT += " H-RU:";
//      LLT += request.GetRequestUrl();
//   }
//   else
//   {
//      xi::h1::HttpResponse &response = static_cast<xi::h1::HttpResponse&>(httpmsg);
//      LLT.Csnprintf(256, "P:HTTP S:%d:%u D:%s M:RES:-:%d",
//            remote_addr.GetFD(), remote_addr.GetSocketID(),
//            (is_send ? "SEND" : "RECV"),
//            response.GetStatusCode());
//      status_code = response.GetStatusCode();
//   }
//
//   xi::h1::ContentTypeHeader *content_type = dynamic_cast<xi::h1::ContentTypeHeader*>(httpmsg.GetHeader(xi::h1::headertype::ContentType));
//   if (content_type) {
//      LLT += " H-C:";
//      LLT += content_type->GetType();
//      LLT += "/";
//      LLT += content_type->GetSubtype();
//   }
//
//   if (httpmsg.IsRequest())
//      status_code = 0;
//
//   //ih::Statistics::Instance()->StatProtocol(!is_send, stat_protocol, status_code);
//   LLTOUT(LLT);
//}


}
