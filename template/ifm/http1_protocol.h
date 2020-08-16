// vim:ts=3:sts=3:sw=3

#ifndef TEMPLATE_IFM_HTTP1_PROTOCOL_H_
#define TEMPLATE_IFM_HTTP1_PROTOCOL_H_

#include "rp/payload.hxx"
#include "stack/http1/request.hxx"
#include "stack/http1/response.hxx"
#include "template/ifm/define.h"

namespace ifm {

class Http1Protocol : public xi::rp::Payload
{
   public :
      Http1Protocol();
      virtual ~Http1Protocol();

      virtual xi::rp::Payload *Clone();

      xi::h1::Request        *GetRequest();
      xi::h1::Response       *GetResponse();
      void                    PushRequest(std::unique_ptr<xi::h1::Request> request);
      void                    PushResponse(std::unique_ptr<xi::h1::Response> response);
      std::unique_ptr<xi::h1::Request> PopRequest();
      std::unique_ptr<xi::h1::Response> PopResponse();
      xi::SocketAddr         &RefAssignAddr();

   private :
      std::unique_ptr<xi::h1::Request> request_;
      std::unique_ptr<xi::h1::Response> response_;
      xi::SocketAddr          assign_addr_;
};

}

#endif
