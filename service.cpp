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

#include <os>
#include <net/inet4>
#include <sstream>
//#include <http>
#include "server/server.hpp"

std::unique_ptr<server::Server> acorn;

#include <memdisk>
#include <fs/fat.hpp> // FAT32 filesystem
using namespace fs;
using namespace std;

////// DISK //////
// instantiate disk with filesystem
auto disk = fs::new_shared_memdisk();

void recursive_fs_dump(vector<fs::Dirent> entries, int depth = 1) {
  auto& filesys = disk->fs();
  int indent = (depth * 3);
  for (auto entry : entries) {

    // Print directories
    if (entry.is_dir()) {
      // Normal dirs
      if (entry.name() != "."  and entry.name() != "..") {
        printf(" %*s-[ %s ]\n", indent, "+", entry.name().c_str());
        recursive_fs_dump(*filesys.ls(entry).entries, depth + 1 );
      } else {
        printf(" %*s  %s \n", indent, "+", entry.name().c_str());
      }

    }else {
      // Print files / symlinks etc.
      //printf(" %*s  \n", indent, "|");
      printf(" %*s-> %s \n", indent, "+", entry.name().c_str());
    }
  }
  printf(" %*s \n", indent, " ");
  //printf(" %*s \n", indent, "o");

}

///
/// Template based middleware (concept sketch)
///

using Resp_ptr = std::shared_ptr<http::Response>;

// Default, least evolved middleware

class Mediumware {

public:
  using Request = http::Request;
  using Next = delegate<void(Request, Resp_ptr)>;

  std::string name() { return "Mediumware"; }

  virtual void process(Request req, Resp_ptr resp, Next next) {
    printf("<Logger> Received vanilla request %s \n", req.to_string().c_str());
    next(req, resp);
  }
};


// Cookie parsers request subclass
template<typename REQ>
class Request_with_cookies : public REQ {
public:

  using Key = std::string;
  using Value = std::string;
  using CookieJar = map<Key,Value>;

  Value cookie(Key key) {
    return cookies_[key];
  }

  Request_with_cookies(REQ& base) :
    REQ(base),
    cookies_{ std::make_pair("username", "Rico"),
      std::make_pair("password","Unbreakable") }
  {}

private:
  CookieJar cookies_;
};

// Cookie parsers request subclass

template<typename REQ>
class Cookie_parser {

public:


  // Middleware types
  using Request = Request_with_cookies<REQ>;
  using Next = delegate<void(Request, Resp_ptr)>;

  std::string name() { return "Cookie_parser"; }
  void process(REQ req, Resp_ptr resp, Next next){

    printf("<Cookie_parser> Cookies parsed! Calling next with cookies \n");

    next(Request(req), resp);
  }
};


template <typename REQ>
class JSON_parsed_request : public REQ {
public:

  using JSON = std::string;

  JSON body() {
    return obj_;
  }

  JSON_parsed_request (REQ& r) :
    REQ(r),
    obj_{"{\"username\": \"Rico\" }"}
  {}

private:
  JSON obj_;
};



template<typename REQ>
class Body_parser {

public:

  using Request = JSON_parsed_request<REQ>;
  using JSON = typename Request::JSON;
  using Next = delegate<void(Request, Resp_ptr)>;

  std::string name() { return "Body_parser"; }
  void process(REQ req, Resp_ptr resp, Next next){

    printf("<Body_parser> Body parsed! Calling next with JSON \n");
    next(Request(req), resp);
  }


};


template<typename REQ>
class Async_delayer {
public:

  using Request = REQ;
  using Next = delegate<void(Request, Resp_ptr)>;

  std::string name() { return "Async_delayer"; }

  void process(REQ req, Resp_ptr resp, Next next) {

    printf("<Async_delayer> Calling next in a second \n");

    hw::PIT::on_timeout(1, [&req, resp, next](){
        printf("<Async_delayer> Next! \n");
        next(req, resp);
      });
  }
};


template<typename REQ>
class Dependant_middleware {

public:

  // Middleware types
  using Request = REQ;
  using Next = delegate<void(Request, Resp_ptr)>;

  std::string name() { return "Dependant_middleware"; }
  void process(REQ req, Resp_ptr resp, Next next){
    printf("Dependant middleware, expecting cookies and JSON-parsed body \n");

    printf("Username from JSON: %s\n", req.cookie("password").c_str());
    printf("JSON from body: %s\n", req.body().c_str());

    next(req, resp);
  }
};



///
/// Variadic middleware "use"-funcition
///

// Recursive part
template<typename Req, typename Current, typename Next, typename... Rest>
void use(Req req, Resp_ptr resp,
         Current curr, Next next_1, Rest... next_n) {
  printf("<use> Current: %s, Next: %s, then Rest... \n",
         curr.name().c_str(), next_1.name().c_str() );

  typename Current::Next next_callback =
    [next_1, next_n...] (typename Current::Request req, Resp_ptr resp) {
    use(req, resp, next_1, next_n...);
  };

  curr.process(req, resp, next_callback);

}

// Base case (last step)
template<typename Req, typename Last>
void use(Req req, Resp_ptr resp, Last last ){

  printf("<use> Last: %s \n", last.name().c_str());

  typename Last::Next last_callback =
    [ &last ] (typename Last::Request req, Resp_ptr resp) {
    printf("Last callback! Called by: %s \n", last.name().c_str());

    //resp.send();
  };

  last.process(req, resp, last_callback);
}


void Service::start() {

  // mount the main partition in the Master Boot Record
  disk->mount([](fs::error_t err) {

      if (err)  panic("Could not mount filesystem, retreating...\n");

      server::Router routes;

      ///
      /// Use Mediumware (currently in a route)
      ///
      routes.on_get("/mediumware/.*", [](auto req, auto res) {


          // Unfortunately we have to do the type nesting manually,
          // at least for now
          using M0 = Mediumware;
          using M1 = Cookie_parser<M0::Request>;
          using M2 = Body_parser<M1::Request>;
          //...
          using Mn = Dependant_middleware<M2::Request>;

          // The use call - stack up all middleware
          use(*req, res,
              M0(),
              M1(),
              M2(),
              //...
              Mn());

        });

      routes.on_get("/api/users/.*", [](auto req, auto res) {
          res->add_header(http::header_fields::Entity::Content_Type,
                          "text/JSON; charset=utf-8"s)
            .add_body("{\"id\" : 1, \"name\" : \"alfred\"}"s);

          res->send();
        });

      // initialize server
      acorn = std::make_unique<server::Server>();
      acorn->set_routes(routes).listen(8081);

      auto vec = disk->fs().ls("/").entries;

      printf("------------------------------------ \n");
      printf(" Memdisk contents \n");
      printf("------------------------------------ \n");
      recursive_fs_dump(*vec);
      printf("------------------------------------ \n");

      hw::PIT::instance().onRepeatedTimeout(15s, []{
          printf("%s\n", acorn->ip_stack().tcp().status().c_str());
        });

    });
}
