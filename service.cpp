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
#include <regex>
//#include <http>
#include "base64.hpp"
#include "util/url.hpp"

// An IP-stack object
std::unique_ptr<net::Inet4<VirtioNet> > inet;

#include <memdisk>
#include <fs/fat.hpp> // FAT32 filesystem

#define HTTP_PARSER_DEBUG 1
#include <http_parser.h>



using namespace fs;
using namespace std;

////// DISK //////
// instantiate disk with filesystem
auto disk = fs::new_shared_memdisk();




void recursive_fs_dump(vector<fs::Dirent> entries, int depth = 1) {
  auto& filesys = disk->fs();
  int indent = (depth * 3);
  for (auto entry : entries) {

    // Print directories, special treatment for . / ..
    if (entry.is_dir()) {

      if (entry.name() != "."  and entry.name() != "..") {
        printf(" %*s-[ %s ]\n", indent, "+", entry.name().c_str());
        recursive_fs_dump(*filesys.ls(entry).entries, depth + 1 );
      } else {
        printf(" %*s  %s \n", indent, "+", entry.name().c_str());
      }

    }else {
      // Print files / symlinks etc.
      printf(" %*s- %s \n", indent, "|", entry.name().c_str());
    }
  }
  printf(" %*s \n", indent, " ");
}


void Service::start() {
  hw::Nic<VirtioNet>& eth0 = hw::Dev::eth<0,VirtioNet>();
  inet = std::make_unique<net::Inet4<VirtioNet> >(eth0);

  // Static IP configuration, until we (possibly) get DHCP
  // @note : Mostly to get a robust demo service that it works with and without DHCP
  inet->network_config( { 10,0,0,42 },      // IP
                        { 255,255,255,0 },  // Netmask
                        { 10,0,0,1 },       // Gateway
                        { 8,8,8,8 } );      // DNS


  // mount the main partition in the Master Boot Record
  disk->mount([](fs::error_t err) {

      if (err)  panic("Could not mount filesystem\n");

      auto vec = *disk->fs().ls("/").entries;


      printf("------------------------------------ \n");
      printf(" Memdisk contents \n");
      printf("------------------------------------ \n");
      recursive_fs_dump(vec);
      printf("------------------------------------ \n");


      std::string to_encode = "Før vi møtes bør vi sjekke at x = y & at y = z.";

      std::cout << "Base64: \n";
      std::cout << "Original: " << to_encode << "\n";

      auto encoded = url::encode(to_encode);

      std::cout << "Encoded: " << encoded << "\n";

      auto decoded = url::decode(encoded);
      std::cout << "Decoded: " << decoded << "\n";

      if (decoded == to_encode)
        std::cout << "SUCCESS \n";
      else
        std::cout << "FAIL \n";


      std::string to_decode = "Copyright%20Notice%0A%0A%20%20%20Copyright%20%28C%29%20The%20Internet%20Society%20%282005%29.%0A%0AAbstract%0A%0A%20%20%20A%20Uniform%20Resource%20Identifier%20%28URI%29%20is%20a%20compact%20sequence%20of%0A%20%20%20characters%20that%20identifies%20an%20abstract%20or%20physical%20resource.%20%20This%0A%20%20%20specification%20defines%20the%20generic%20URI%20syntax%20and%20a%20process%20for%0A%20%20%20resolving%20URI%20references%20that%20might%20be%20in%20relative%20form%2C%20along%20with%0A%20%20%20guidelines%20and%20security%20considerations%20for%20the%20use%20of%20URIs%20on%20the%0A%20%20%20Internet.%20%20The%20URI%20syntax%20defines%20a%20grammar%20that%20is%20a%20superset%20of%20all%0A%20%20%20valid%20URIs%2C%20allowing%20an%20implementation%20to%20parse%20the%20common%20components%0A%20%20%20of%20a%20URI%20reference%20without%20knowing%20the%20scheme-specific%20requirements%0A%20%20%20of%20every%20possible%20identifier.%20%20This%20specification%20does%20not%20define%20a%0A%20%20%20generative%20grammar%20for%20URIs%3B%20that%20task%20is%20performed%20by%20the%20individual%0A%20%20%20specifications%20of%20each%20URI%20scheme.%0A%0ASource%3A%20https%3A//tools.ietf.org/html/rfc3986%23section-2.4";

      decoded = url::decode(to_decode);

      cout << "Decoded: ------------------------------------ \n"
           << decoded
           << "\n------------------------------------ \n";

      auto reencoded = url::encode(decoded);

      cout << "Re-encoded: ------------------------------------ \n"
           << reencoded
           << "\n------------------------------------ \n";



      if (reencoded == to_decode)
        std::cout << "SUCCESS \n";
      else
        std::cout << "FAIL \n";


      // Static IP configuration, until we (possibly) get DHCP
      // @note : Mostly to get a robust demo service that it works with and without DHCP
      inet->network_config( { 10,0,0,42 },      // IP
                            { 255,255,255,0 },  // Netmask
                            { 10,0,0,1 },       // Gateway
                            { 8,8,8,8 } );      // DNS

      srand(OS::cycles_since_boot());

      // Set up a TCP server on port 80
      auto& server = inet->tcp().bind(80);

      hw::PIT::instance().onRepeatedTimeout(30s, []{
          printf("<Service> TCP STATUS:\n%s \n", inet->tcp().status().c_str());
        });

      // Add a TCP connection handler - here a hardcoded HTTP-service
      server.onConnect([](auto conn) {

          conn->read(1024, [conn](net::TCP::buffer_t buf, size_t n){

              printf("<Acorn> Pre settings \n");
              http_parser_settings settings;


              printf("------------ RAW HEADER ------------ \n%.*s\n---------------------\n",
                     n, buf.get());

              settings.on_message_begin = [](http_parser*){
                printf("<PARSE> Msg. begin \n");
                return 0;
              };

              settings.on_header_value = [](http_parser*, const char *at, size_t length) -> int {
                printf("<PARSE> Header value \n");
                return 0;
              };

              settings.on_body = [](http_parser*, const char *at, size_t length) -> int {
                printf("<PARSE> BODY \n");
                return 0;
              };

              settings.on_message_complete = [](http_parser*){
                printf("<PARSE> Msg. complete \n");
                return 0;
              };


              /* When on_chunk_header is called, the current chunk length is stored
               * in parser->content_length.
               */
              settings.on_chunk_header = [](http_parser*){
                printf("<PARSE> chunck header \n");
                return 0;
              };

              settings.on_chunk_complete = [](http_parser*){
                printf("<PARSE> chunk complete \n");
                return 0;
              };

              settings.on_headers_complete = [](http_parser*) -> int {
                printf("<PARSE> Headers complete \n");
                return 0;
              };

              settings.on_header_field = [](http_parser*, const char *at, size_t length) -> int {
                printf("<PARSE> Headers field \n");
                return 0;
              };


              settings.on_url = [](http_parser*, const char *at, size_t length) -> int {
                printf("<PARSE> URL: %s \n", std::string(at, length).c_str());

                http_parser_url url_res;
                http_parser_parse_url(at, length, 0, &url_res);

                if (url_res.field_set & (1 << UF_PATH)) {
                  auto path_start = url_res.field_data[UF_PATH].off;
                  auto path_len = url_res.field_data[UF_PATH].len;
                  printf("<PARSE> URL parsed. Path: (%.*s) \n", path_len, &at[path_start]);
                }


                //std::string s{ at, length };
                std::regex e { "([^=&?]+)=([^=&?]+)" };
                std::cregex_iterator rit ( at, at + length, e );
                std::cregex_iterator rend;

                while (rit!=rend) {
                  std::cmatch match = *rit;
                  std::string match_str = match.str();
                  auto key = match[1].str();
                  auto value = match[2].str();
                  std::cout << "<RE Match> " << match_str << '\n';
                  std::cout << "Key: " << url::decode(key)
                            << " Value: " << url::decode(value)
                            << "\n";

                  ++rit;
                }

                return 0;
              };



              /* ... */
              printf("<Acorn> Pre init \n");
              http_parser *parser = (http_parser*) malloc(sizeof(http_parser));
              http_parser_init(parser, HTTP_REQUEST);
              //parser->data = my_socket;

              printf("<Acorn> Pre execute \n");
              http_parser_execute(parser,&settings, (const char*) buf.get(), n);
              printf("<Acorn> Execute done \n");

            });
        });


      /*
      http::Router routes;

      routes.on_get("/"s, [](const auto&, auto& res){
          disk->fs().readFile("/index.html", [&res] (fs::error_t err, fs::buffer_t buff, size_t len) {
              if(err) {
                res.set_status_code(http::Not_Found);
              } else {
                // fill Response with content from index.html
                printf("<Server> Responding with index.html. \n");
                res.add_header(http::header_fields::Response::Server, "IncludeOS/Acorn")
                  .add_header(http::header_fields::Entity::Content_Type, "text/html; charset=utf-8"s)
                  .add_header(http::header_fields::Response::Connection, "close"s)
                  .add_body(std::string{(const char*) buff.get(), len});
              }
            });

        }); // << fs().readFile

      http::createServer().set_routes(routes).listen(8081);
      */
    }); // < disk*/
}
