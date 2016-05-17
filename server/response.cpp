// This file is a part of the IncludeOS unikernel - www.includeos.org
//
// Copyright 2015-2016 Oslo and Akershus University College of Applied Sciences
// and Alfred Bratterud
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distribut ed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "response.hpp"

using namespace server;

Response::Response(Connection_ptr conn)
  : http::Response{}
  , conn_{conn}
{
  add_header(http::header_fields::Response::Server, "IncludeOS/Acorn");
  // screw keep alive
  add_header(http::header_fields::Response::Connection, "close");
}

void Response::send(const bool close) const {
  write_to_conn(close);
  end();
}

void Response::write_to_conn(const bool close_on_written) const {
  auto res = to_string();

  if(not close_on_written) {
    conn_->write(res.data(), res.size());
  }
  else {
    conn_->write(res.data(), res.size(), [conn_](size_t) {
      conn->close();
    });
  }
}

void Response::send_code(const Code code) {
  set_status_code(code);
  send();
}

void Response::send_file(const File& file) {
  add_header(http::header_fields::Entity::Content_Length, file.size_str());
  add_header(http::header_fields::Entity::Content_Type, file.mime());

  /* Send header */
  auto res = to_string();
  conn_->write(res.data(), res.size());

  /* Send file over connection */
  printf("<Response::send_file> Asking to send %llu bytes.\n", file.size());
  Async::upload_file(file.disk, file.entry, conn_,
    [conn_](fs::error_t err, bool good)
  {
      if(good) {
        printf("<Response::send_file> %s - Success!\n",
          conn->to_string().c_str());
        //conn->close();
      }
      else {
        printf("<Response::send_file> %s - Error: %s\n",
          conn->to_string().c_str(), err.to_string().c_str());
      }
  });

  end();
}

void Response::end() const {
  // Response ended, signal server?
}
