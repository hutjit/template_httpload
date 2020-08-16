// vim:ts=3:sts=3:sw=3

#include "session.h"
#include <memory>
#include "xi/input_string_cursor.hxx"
#include "xi/logger.hxx"
#include "xi/util.hxx"

#include "template/main/session.h"
#include "template/main/interface_hub.h"

#include "manager.h"
#include "fsm_idle.h"
#include "fsm_api_1.h"

namespace tu {


#define FN fn << Tag() << " "

Session::Session() : xi::rp::Member(td::functype::TU_SESSION)
{
   fsm_object_ = NULL;

   for (int i = td::timer::NONE; i < td::timer::END_OF_ENUM; ++i) {
      timer_table_[i] = 0;
   }

   Clear();
}

Session::~Session()
{
}

void Session::Clear()
{
   fsm_type_ = fsmtype::IDLE;
   fsm_object_ = FsmIdle::Instance();

   seqno_ = 0;
   trace_tag_.Clear();
   test_scenario_ = test::scenario::NONE;

   TimerAllStop();


   start_time_ = 0;
   tu_data_id_ = 0;
   scenario_step_ = 0;


   // Do not change
   xi::rp::Member::Clear();
}

void Session::Release()
{
   static const char *fn = "[tu::Release] ";

   xi::rp::Session *ihsess = parent_;
   xi::rp::membid_t myid = GetMemberId();

   if (0 == myid) {
      DLOG(FN << "already released");
      return;
   }

   DLOG(FN << "mid:" << myid);

   Clear();

   ihsess->Detach(myid);
}

xi::rp::result::e Session::OnReceive(xi::rp::Payload &message)
{
   static const char *fn = "[tu::OnReceive] ";

   xi::rp::result::e result = fsm_object_->OnReceive(*this, message);
   DLOG(FN << " result:" << xi::rp::result::name(result));

   if (xi::rp::result::DO_RELEASE == result)
      Release();

   return result;
}

xi::rp::result::e Session::Send(xi::rp::Payload &message)
{
   static xi::rp::pbgid_t topology_id = ih::Property::Instance()->GetTopologyId();

   message.SetSrcPbgId(topology_id);
   message.SetSrcSessId(GetSessionId());
   message.SetSrcMembId(GetMemberId());

   return parent_->Send(message);
}

const char *Session::Tag()
{
   return (trace_tag_.IsEmpty() ? "(none)" : trace_tag_.c_log());
}

uint32_t Session::NextSeq()
{
   return ++seqno_;
}

void Session::ChangeFsm(const char *ltag, tu::fsmtype::e type)
{
   static const char *fn = "[ChangeFsm] ";

   switch (type) {
      case tu::fsmtype::IDLE :
         fsm_object_ = tu::FsmIdle::Instance();
         break;

      case tu::fsmtype::API_1:
         fsm_object_ = tu::FsmApi1::Instance();
         break;

      default :
         WLOG(ltag << FN << " unsupported fsmtype:" << tu::fsmtype::name(type));
         return;
   }

   DLOG(ltag << FN << CYAN(" " << tu::fsmtype::name(fsm_type_) << " --> " << tu::fsmtype::name(type)));
   fsm_type_ = type;
}

const char *Session::GetFsmName()
{
   return tu::fsmtype::name(fsm_type_);
}

bool Session::SendHttpRequest(IN std::unique_ptr<xi::h1::Request> request, OUT xi::SocketAddr &assign_addr)
{
   ifm::Http1Protocol proto;
   proto.SetSrcSessId(GetSessionId());
   proto.SetSrcMembId(GetMemberId());

   proto.PushRequest(std::move(request));
   if (xi::rp::result::SUCCESS == parent_->Send(proto)) {
      assign_addr = proto.RefAssignAddr();
      return true;
   }

   return false;
}

bool Session::SendHttpResponse(IN std::unique_ptr<xi::h1::Response> response)
{
   ifm::Http1Protocol proto;
   proto.SetSrcSessId(GetSessionId());
   proto.SetSrcMembId(GetMemberId());

   proto.PushResponse(std::move(response));
   if (xi::rp::result::SUCCESS == parent_->Send(proto)) {
      return true;
   }

   return false;
}

bool Session::CheckTimerEvent(ifm::TimerEvent &timeout)
{
   static const char *fn = "[tu::CheckTimerEvent] ";

   td::timer::e timer_type = (td::timer::e) timeout.GetParam1();

   if (false == td::timer::scope(timer_type)) {
      WLOG(FN << " unknown timer-type:" << timer_type << " FSM:" << tu::fsmtype::name(fsm_type_));
      return false;
   }

   if (timeout.GetTimerId() != timer_table_[timer_type]) {
      WLOG(FN << RED(" sid:" << GetSessionId() << " mid:" << GetMemberId()
               << " FSM:" << tu::fsmtype::name(fsm_type_)
               << " conflict timeout[" << td::timer::name(timer_type) << "] tid:" << timeout.GetTimerId()
               << " mismatch stored-tid:" << timer_table_[timer_type]));
      return false;
   }

   timer_table_[timer_type] = 0;

   return true;
}

bool Session::TimerStart(td::timer::e timer_type, uint32_t msec)
{
   static const char *fn = "[tu::TimerStart] ";
   static ih::Property *property = ih::Property::Instance();

   if (false == td::timer::valid(timer_type)) {
      WLOG(FN << RED(" unknown timertype:" << timer_type << " " << msec << " ms"));
      return false;         
   }

   if (0 != timer_table_[timer_type]) {
      WLOG(FN << RED(" ih:" << GetSessionId() << " mid:" << GetMemberId() 
               << " Exception StopTimer " << td::timer::name(timer_type) << ":" << timer_table_[timer_type]));
      ((ih::Session*)parent_)->StopTimer(timer_table_[timer_type]);
   } 

   if (0 == msec) {
      msec = property->GetTimerValue(timer_type);
   }

   timer_table_[timer_type] = ((ih::Session*)parent_)->StartTimer(msec, timer_type, GetMemberId());

   if (0 == timer_table_[timer_type]) {
      WLOG(FN << RED(" ih:" << GetSessionId() << " mid:" << GetMemberId() 
               << " " << td::timer::name(timer_type) << " Start FAIL"));
      return false;         
   }         

   DLOG(FN << PURPLE(" ih:" << GetSessionId() << " mid:" << GetMemberId() 
            << " " << td::timer::name(timer_type) << ":" << timer_table_[timer_type] 
            << " " << msec << " msec Start"));

   return true;  
}

bool Session::TimerStarted(td::timer::e timer_type)
{
   static const char *fn = "[tu::TimerStarted] ";

   if (false == td::timer::valid(timer_type)) {
      WLOG(FN << RED(" unknown timertype:" << timer_type));
      return false;
   }

   if (0 != timer_table_[timer_type]) {
      return true;
   }

   return false;
}

bool Session::TimerStop(td::timer::e timer_type)
{
   static const char *fn = "[tu::TimerStop] ";

   if (false == td::timer::valid(timer_type)) {
      WLOG(FN << RED(" unknown timertype:" << timer_type));
      return false;         
   } 

   if (0 != timer_table_[timer_type]) {
      DLOG(FN << CYAN(" ih:" << GetSessionId() << " mid:" << GetMemberId() 
               << " " << td::timer::name(timer_type) << ":" << timer_table_[timer_type]));
      ((ih::Session*)parent_)->StopTimer(timer_table_[timer_type]);
      timer_table_[timer_type] = 0;

      return true;
   } 

   WLOG(FN << RED(" ih:" << GetSessionId() << " mid:" << GetMemberId()
            << " already stopped !! " << td::timer::name(timer_type) << ":" << timer_table_[timer_type]));

   return false;
}

bool Session::TimerClear(td::timer::e timer_type)
{
   static const char *fn = "[tu::TimerClear] ";

   if (false == td::timer::valid(timer_type)) {
      WLOG(FN << RED(" unknown timertype:" << timer_type));
      return false;         
   } 

   if (0 != timer_table_[timer_type]) {
      timer_table_[timer_type] = 0;
      return true;
   }

   DLOG(FN << " already stopped timer:" << td::timer::name(timer_type));

   return false;
}


void Session::TimerAllStop()
{
   static const char *fn = "[tu::TimerAllStop] ";

   for (int i = td::timer::NONE+1; i < td::timer::END_OF_ENUM; ++i) {
      if (0 != timer_table_[i]) {
         DLOG(FN << CYAN(" ih:" << GetSessionId() << " mid:" << GetMemberId()
                  << " " << td::timer::name((td::timer::e)i) << ":" << timer_table_[i]));

         ((ih::Session*)parent_)->StopTimer(timer_table_[i]);
         timer_table_[i] = 0;
      }
   }
}


}
