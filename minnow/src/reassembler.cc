#include "reassembler.hh"
#include <iostream>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  // not eof
  if(!data.empty())
  {
    preprocess(first_index, data, output);
  }
  while(!buffer.empty()&&buffer.begin()->first==first_unassembled_index)
  {
    output.push(buffer.begin()->second);  
    unassembled_bytes-=buffer.begin()->second.size();
    first_unassembled_index = output.bytes_pushed();
    buffer.erase(buffer.begin());
  }
  //eof then close
  if (is_last_substring)
  {
    eof = true;
    eof_index = first_index + data.size();
    //output.end_input();
  }
  if(eof&&output.bytes_pushed()>=eof_index)
  {
    output.close();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return unassembled_bytes;
}

void Reassembler::preprocess(uint64_t first_index, std::string data, Writer& output)
{
  first_unassembled_index = output.bytes_pushed();
  capacity = output.available_capacity();
  if (first_index < first_unassembled_index)
  {
    if (first_index + data.size() <= first_unassembled_index)
    {
      return;
    }
    else
    {
      data = data.substr(first_unassembled_index - first_index);
      first_index = first_unassembled_index;
    }
  }
  if (first_index + data.size() > first_unassembled_index + capacity)
  {
    if (first_index >= first_unassembled_index + capacity)
    {
      return;
    }
    else
    {
      data = data.substr(0, first_unassembled_index + capacity - first_index);
    }
  }
  if (buffer.empty())
  {
    buffer.insert(pair<uint64_t, string>(first_index, data));
    unassembled_bytes += data.size();
    return;
  }
  store(first_index, data);
}
//read the data and cut head/tail if needed; or throw the data if out of range. init the buffer when empty

void Reassembler::store(uint64_t first_index, std::string data)
{
    for (auto it = buffer.begin(); it != buffer.end();)
    {
        size_t data_end = first_index + data.size() - 1;
        size_t buffer_end = it->first + it->second.size() - 1;
        if(first_index>=it->first&&data_end<=buffer_end)
        {
            return;
        }//data within buffer
        else if(first_index<=it->first&&data_end>=buffer_end)
        {
            unassembled_bytes-=it->second.size();
            buffer.erase(it++);
        }//buffer within data
        else if(first_index<=it->first&&data_end>=it->first&&data_end<=buffer_end)
        {
            data=data.substr(0,it->first-first_index)+it->second;
            unassembled_bytes-=it->second.size();
            buffer.erase(it++);
        }//data head < buffer head< data tail < buffer tail
        else if(first_index>=it->first&&first_index<=buffer_end&&data_end>=buffer_end)
        {
            data=it->second+data.substr(buffer_end-first_index+1);
            unassembled_bytes-=it->second.size();
            first_index=it->first;
            buffer.erase(it++);
        }//buffer head < data head < buffer tail < data tail
        else
        {
            it++;
        }
    }
    // If the data can't be merged with existing buffer elements, insert a new element
    buffer.insert({first_index, data});
    unassembled_bytes+=data.size();
}
    
