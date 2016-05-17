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
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "connection.hpp"
#include "server.hpp"

using namespace server;

Connection::Connection(Server& serv, Connection_ptr conn, const size_t idx) noexcept
  : server_{serv}
  , conn_{conn}
  , idx_{idx}
{
  conn_->read(BUFSIZE, OnData::from<Connection, &Connection::on_data>(this));
  conn_->onDisconnect(OnDisconnect::from<Connection, &Connection::on_disconnect>(this));
}

void Connection::on_data(buffer_t buf, size_t n) {
  request_ = std::make_shared<Request>(buf, n);

  response_ = std::make_shared<Response>(conn_);

  printf("<Acorn[Connection]> OnData: %s\n", std::string{static_cast<char*>(buf.get()), n}.c_str());

  server_.process(request_, response_);

}

void Connection::on_disconnect(Connection_ptr, Disconnect) {
  close();
}

void Connection::close() {
  conn_->close();
  server_.close(idx_);
}
