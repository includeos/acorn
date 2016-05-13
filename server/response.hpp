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

#ifndef SERVER_RESPONSE_HPP
#define SERVER_RESPONSE_HPP

#include <net/tcp.hpp>
#include <fs/filesystem.hpp>
#include <utility/async.hpp>

#include "http/inc/response.hpp"
#include "http/inc/mime_types.hpp"

struct File {

  explicit File(fs::Disk_ptr dptr, const fs::Dirent& ent)
    : disk_{dptr}
  {
    Expects(ent.is_file());
    entry_ = ent;
  }

  uint64_t size() const noexcept
  { return entry_.size(); }

  std::string size_str() const
  { return std::to_string{entry_.size()}; }

  const http::Mime_Type& mime() const noexcept {
    const auto& fname = entry_.fname;
    const auto ext_i  = fname.find_last_of(".");
    if(ext_i not_eq std::string::npos) {
      return http::extension_to_type(fname.substr(ext_i + 1));
    } else {
      return http::extension_to_type("txt");
    }
  }

  fs::Dirent   entry_;
  fs::Disk_ptr disk_;
};

namespace server {

class Response;
using Response_ptr = std::shared_ptr<Response>;

class Response : public http::Response {
private:
  using Code           = http::status_t;
  using Connection_ptr = net::TCP::Connection_ptr;

public:

  Response(Connection_ptr conn);

  /*
    Send only status code
  */
  void send_code(const Code);

  /*
    Send the Response
  */
  void send(const bool close = false) const;

  /*
    Send a file
  */
  void send_file(const File&);

  /*
    "End" the response
  */
  void end() const;

private:
  Connection_ptr conn_;

  void write_to_conn(const bool close_on_written) const;

}; //< server::Response

} //< server

#endif //< SERVER_RESPONSE_HPP
