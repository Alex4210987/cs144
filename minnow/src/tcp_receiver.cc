#include "tcp_receiver.hh"
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  int64_t zero_point_=0;
  if(!syn){
    if(!message.SYN){
      return;
    }
    zero_point=message.seqno;
    zero_point_=Wrap32{zero_point}.unwrap(zero_point,1);
    syn=true;
  }
  if(message.FIN){
    fin=true;
  }
  int64_t insert_index=Wrap32(message.seqno).unwrap(zero_point,inbound_stream.writer().bytes_pushed()+1);
  if(insert_index+message.SYN-1<zero_point_){
    return;
  }
  reassembler.insert(insert_index+message.SYN-1,message.payload, message.FIN,inbound_stream);
  return;
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage message;
  message.window_size = (inbound_stream.available_capacity() > 65535) ? 65535 : inbound_stream.available_capacity();
  if(!syn){
    message.ackno=std::nullopt;
  }else{
    uint64_t pushed_bytes_=inbound_stream.bytes_pushed();
    if(fin&&inbound_stream.is_closed()){
      pushed_bytes_++;
    }
    message.ackno=Wrap32{zero_point}+pushed_bytes_+1;
  }
  return message;
}
