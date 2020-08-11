#include "http1_protocol.h"

namespace ifm {


Http1Protocol::Http1Protocol() : xi::rp::Payload(ifm::msgtype::HTTP1_PROTOCOL)
{
}

Http1Protocol::~Http1Protocol()
{
}

xi::rp::Payload *Http1Protocol::Clone()
{
   Http1Protocol *clone = new Http1Protocol();

   if (request_.get()) {
      std::unique_ptr<xi::h1::Request> req(request_->Clone());
      clone->PushRequest(std::move(req));
   }

   if (response_.get()) {
      std::unique_ptr<xi::h1::Response> res(response_->Clone());
      clone->PushResponse(std::move(res));
   }

   return clone;
}


xi::h1::Request *Http1Protocol::GetRequest()
{
   return request_.get();
}

xi::h1::Response *Http1Protocol::GetResponse()
{
   return response_.get();
}

void Http1Protocol::PushRequest(std::unique_ptr<xi::h1::Request> request)
{
   if (request_.get()) {
      delete request_.release();
   }

   request_ = std::move(request);
}

void Http1Protocol::PushResponse(std::unique_ptr<xi::h1::Response> response)
{
   if (response_.get()) {
      delete response_.release();
   }

   response_ = std::move(response);
}

std::unique_ptr<xi::h1::Request> Http1Protocol::PopRequest()
{
   return std::move(request_);
}

std::unique_ptr<xi::h1::Response> Http1Protocol::PopResponse()
{
   return std::move(response_);
}

xi::SocketAddr &Http1Protocol::RefAssignAddr()
{
   return assign_addr_;
}


}
