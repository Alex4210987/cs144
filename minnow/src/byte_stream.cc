#include "byte_stream.hh"
#include <iostream>
#include <stdexcept>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  // Your code here.
  size_t len = min( data.size(), available_capacity() );
  buffer_.insert( buffer_.end(), data.begin(), data.begin() + len );
  bytes_pushed_ += len;
}

void Writer::close()
{
  // Your code here.
  is_closed_ = true;
}

void Writer::set_error()
{
  // Your code here.
  has_error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return capacity_ - buffer_.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  // Your code here.
  return { std::string_view( &buffer_.front(), 1 ) };
}

bool Reader::is_finished() const
{
  // Your code here.
  return is_closed_ && buffer_.empty();
}

bool Reader::has_error() const
{
  // Your code here.
  return has_error_;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  const std::size_t size = min( len, buffer_.size() );
  buffer_.erase( buffer_.begin(), buffer_.begin() + size );
  bytes_popped_ += size; // Update bytes_popped_
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return bytes_popped_;
}