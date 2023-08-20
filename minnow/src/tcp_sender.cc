#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include <iostream>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_bytes_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmission_times_;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if(queued_messages_.empty()){
    return {};
  }
  if(!timer_.is_running()){
    timer_.start();
  }
  TCPSenderMessage message=queued_messages_.front();
  queued_messages_.pop();
  return message;
}

void TCPSender::push( Reader& outbound_stream )
{
  size_t current_window_size=(window_size!=0)?window_size:1;
  while(outstanding_bytes_<current_window_size){
    TCPSenderMessage message;
    if(!syn_){
      syn_=message.SYN=true;
      outstanding_bytes_++;
    }
    message.seqno=Wrap32::wrap(next_seqno,isn_);
    read(outbound_stream,min(TCPConfig::MAX_PAYLOAD_SIZE,current_window_size-outstanding_bytes_),message.payload);
    outstanding_bytes_+=message.payload.size();
    //process fin
    if(outbound_stream.is_finished()&&!fin_&&outstanding_bytes_<current_window_size&&!fin_){
      fin_=message.FIN=true;
      outstanding_bytes_++;
    }
    if(message.sequence_length()>0){
      outstanding_messages_.push(message);
      queued_messages_.push(message);
      next_seqno+=message.sequence_length();
    }
    else{
      break;
    }
    if(fin_||!outbound_stream.bytes_buffered()){
      break;
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  // Your code here.
  TCPSenderMessage message;
  message.seqno=Wrap32::wrap(next_seqno,isn_);
  return message;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // Your code here.
  window_size=msg.window_size;
  if(!msg.ackno.has_value()){
    return;
  }
  if(msg.ackno.value().unwrap(isn_,next_seqno)>next_seqno){
    return;
  }
  acknowleged_seqno=msg.ackno.value().unwrap(isn_,next_seqno);
  while(!outstanding_messages_.empty()){
    auto front_message_=outstanding_messages_.front();
    if(front_message_.seqno.unwrap(isn_,next_seqno)+front_message_.sequence_length()<=acknowleged_seqno){
      outstanding_bytes_-=front_message_.sequence_length();
      outstanding_messages_.pop();
      timer_.reset();
      retransmission_times_=0;
      if(!outstanding_messages_.empty()){
        timer_.start();
      }
    }
    else{
      break;
    }
  }
  if ( outstanding_messages_.empty() ) {
    timer_.stop();
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  timer_.tick(ms_since_last_tick);
  if(timer_.is_expired()){
    queued_messages_.push(outstanding_messages_.front());
    if(window_size!=0){
      ++retransmission_times_;
      timer_.double_RTO();
    }
    timer_.start();
  }
}
