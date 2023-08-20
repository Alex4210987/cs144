#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

//#include<iostream>

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  if(arp_cache_.find(next_hop.ipv4_numeric()) != arp_cache_.end()) {
    // Send to known host
    EthernetFrame frame;
    frame.header.dst = arp_cache_[next_hop.ipv4_numeric()].first;
    frame.header.src = ethernet_address_;
    frame.header.type=EthernetHeader::TYPE_IPv4;
    frame.payload=serialize(dgram);
    frame_queue_.push(std::move(frame));
    //cout<<"case 1"<<endl;
  }else if(time_cache_.find(next_hop.ipv4_numeric()) == time_cache_.end()) {
    EthernetFrame frame;
    frame.header.dst = ETHERNET_BROADCAST;
    frame.header.src = ethernet_address_;
    frame.header.type=EthernetHeader::TYPE_ARP;
    ARPMessage arp;
    arp.opcode = ARPMessage::OPCODE_REQUEST;
    arp.sender_ethernet_address = ethernet_address_;
    arp.sender_ip_address = ip_address_.ipv4_numeric();
    arp.target_ip_address = next_hop.ipv4_numeric();
    frame.payload=serialize(arp);
    frame_queue_.push(std::move(frame));
    time_cache_.insert({next_hop.ipv4_numeric(),0});
    datagram_queue_.push(std::pair(dgram,next_hop));
    //cout<<"case 2"<<endl;
  }else{
    datagram_queue_.push(std::pair(dgram,next_hop));
    //cout<<"case 3"<<endl;
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  //return if self is not dest and not broadcast
  if(frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST) {
    //cout<<"not dest"<<endl;
    return {};
  }else if(frame.header.type ==EthernetHeader::TYPE_IPv4) {
    InternetDatagram data;
    if(parse(data,frame.payload)) {
      //cout<<"IPv4"<<endl;
      return data;
    }
  }else if(frame.header.type == EthernetHeader::TYPE_ARP) {
    ARPMessage arp;
    if(parse(arp,frame.payload)) {
      //if reply update arp cache and time cache; send possible datagram
      if(arp.opcode == ARPMessage::OPCODE_REPLY) {
        arp_cache_.insert({arp.sender_ip_address,{arp.sender_ethernet_address,0}});
        //traverse datagram queue and send if ip matches
        std::queue<std::pair<InternetDatagram, Address>> temp;
        while(!datagram_queue_.empty()) {
          std::pair<InternetDatagram, Address> pair = datagram_queue_.front();
          datagram_queue_.pop();
          if(pair.second.ipv4_numeric() == arp.sender_ip_address) {
            send_datagram(pair.first,pair.second);
          }else{
            temp.push(pair);
          }
        }
      }else if(arp.opcode == ARPMessage::OPCODE_REQUEST) {
        //if request and ip matches, send reply
        if(arp.target_ip_address == ip_address_.ipv4_numeric()) {
          EthernetFrame _frame;
          _frame.header.dst = frame.header.src;
          //cout<<"reply dst "<<to_string(_frame.header.dst)<<endl;
          _frame.header.src = ethernet_address_;
          _frame.header.type=EthernetHeader::TYPE_ARP;
          ARPMessage arp_;
          arp_.opcode = ARPMessage::OPCODE_REPLY;
          arp_.target_ethernet_address = frame.header.src;
          arp_.target_ip_address = arp.sender_ip_address;
          arp_.sender_ethernet_address = ethernet_address_;
          //cout<<"sender ether"<<to_string(arp_.sender_ethernet_address)<<endl;
          arp_.sender_ip_address = ip_address_.ipv4_numeric();
          _frame.payload=serialize(arp_);
          frame_queue_.push(_frame);
          //cout<<"reply arp"<<endl;
        }
        //and update arp cache
        arp_cache_.insert({arp.sender_ip_address,{arp.sender_ethernet_address,0}});
      }      
    }
  } 
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick ){
  static constexpr size_t IP_MAP_TTL = 30000;     // 30s
  static constexpr size_t ARP_REQUEST_TTL = 5000; // 5s

  for ( auto it = time_cache_.begin(); it != time_cache_.end(); ) {
    it->second += ms_since_last_tick;
    if ( it->second >= ARP_REQUEST_TTL ) {
      it = time_cache_.erase( it );
    } else {
      ++it;
    }
  }

  for ( auto it = arp_cache_.begin(); it != arp_cache_.end(); ) {
    it->second.second += ms_since_last_tick;
    if ( it->second.second >= IP_MAP_TTL ) {
      it = arp_cache_.erase( it );
    } else {
      ++it;
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send(){
  if(!frame_queue_.empty()) {
    EthernetFrame frame = frame_queue_.front();
    frame_queue_.pop();
    return frame;
  }
  return {};
}