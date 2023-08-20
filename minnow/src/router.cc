#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  route_table_.push_back( { route_prefix, prefix_length, next_hop, interface_num } );
}

void Router::route(){
  for(auto& current_interface : interfaces_){
    auto maybe_datagram = current_interface.maybe_receive();
    if(maybe_datagram.has_value()){
      auto& datagram_value = maybe_datagram.value();
      if(datagram_value.header.ttl>1){
        datagram_value.header.ttl--;
        datagram_value.header.compute_checksum();
        auto dst_ip = datagram_value.header.dst;
        auto route = longest_prefix_match_(dst_ip);
        if(route!=route_table_.end()){
          if(route->next_hop.has_value()){
            //cout<<"DEBUG: forwarding datagram to "<<route->next_hop->ip()<<" on interface "<<route->interface_num<<"\n";
            interface(route->interface_num).send_datagram(datagram_value, route->next_hop.value());
          }
          else{
            //cout<<"DEBUG: forwarding datagram to "<<Address::from_ipv4_numeric(dst_ip).ip()<<" on interface "<<route->interface_num<<"\n";
            interface(route->interface_num).send_datagram(datagram_value, Address::from_ipv4_numeric(dst_ip));
          }
        }
      }
    }
  }
}

int Router::longest_prefix_( uint32_t dst_ip, uint32_t route_prefix, uint8_t prefix_length ){
  //cout<<"prefix "<<Address::from_ipv4_numeric(route_prefix).ip()<<"\n";
  //cout<<"prefix length "<<static_cast<int>(prefix_length)<<"\n";
  if(prefix_length>32){
    return -1;
  }
  dst_ip = dst_ip >> (32-prefix_length);
  route_prefix = route_prefix >> (32-prefix_length);
  //cout<<"dst_ip "<<Address::from_ipv4_numeric(dst_ip).ip()<<"\n";
  //cout<<"route_prefix "<<Address::from_ipv4_numeric(route_prefix).ip()<<"\n";
 //cout<<endl;
  return (dst_ip==route_prefix) ? prefix_length : -1;
}


std::vector<Router::RouteTableEntry>::iterator Router::longest_prefix_match_( uint32_t dst_ip ){
  auto longest_prefix_route = route_table_.end();
  int longest_ = 0;
  for(auto& route : route_table_){
    auto prefix_length = longest_prefix_(dst_ip, route.route_prefix, route.prefix_length);
    if(prefix_length>longest_){
      longest_ = prefix_length;
      longest_prefix_route = route_table_.begin() + ( &route - &route_table_[0] );
    }
  }
  if(longest_prefix_route==route_table_.end()){
    //cout<<"DEBUG: no route found for "<<Address::from_ipv4_numeric(dst_ip).ip()<<"\n";
    return route_table_.end();
  }
  return longest_prefix_route;
}