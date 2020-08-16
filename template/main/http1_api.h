// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_MAIN_HTTP1API_H_
#define TEMPLATE_MAIN_HTTP1API_H_ 

#include "rp/payload.hxx"
#include "stack/http1/api.hxx"

namespace ih {


class InterfaceHub;

class Http1Api : public xi::h1::Api
{
   public :
      Http1Api(InterfaceHub *callback, xi::ThreadPool *threadpool);
      virtual ~Http1Api();

      virtual void      OnNotify(xi::ioevt::e status, xi::SocketAddr &addr);
      virtual void      OnReceive(std::unique_ptr<xi::h1::Request> request);
      virtual void      OnReceive(std::unique_ptr<xi::h1::Response> response);
      virtual void      OnSendFail(std::unique_ptr<xi::h1::Request> request);
      bool              Send(xi::rp::Payload &message);

   private :
      //void              LLT(bool is_send, xi::h1::HttpMessage &httpmsg, xi::SocketAddr &remote_addr);

   private :
      ih::InterfaceHub *callback_;
};


}
#endif
