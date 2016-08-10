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

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <regex>
#include <stdexcept>
#include <functional>

/* Local Dependencies */
#include "request.hpp"
#include "response.hpp"

namespace server {

  /**
   * @brief This class is used to describe
   * route resolution
   *
   * Its main function is to register information
   * relating request to response mappings
   */
  class Router {
  private:
    /* Internal Alias Section */

    /*
     * This entity is the pattern matching mechanism used to
     * map a requested resource to its prefered representation
     */
    using Route_expr = std::regex;

    /*
     * This entity is used to generate a representation of the
     * requested resource
     */
    using Generator = std::function<void(Request_ptr, Response_ptr)>;

    /*
     * An entity to encapsulate the pattern matching mechanism and
     * the representation generator into a single unit for the
     * purpose of registration
     */
    struct Route {
      Route_expr expr;
      Generator  generator;

      /*
       * Constructor
       *
       * |param| <ex> : A requested resource
       *
       * |param| <g> : A resource generator
       */
      Route(const std::string& ex, Generator g) : expr{ex}, generator{g} {}
    };

    /*
     * An entity that's used for registering request to response mappings
     */
    using Route_table = std::unordered_map<http::Method, std::vector<Route>>;

    /*
     * This entity is used as a latch that refer to external information which
     * implies that it always depends on an external entity
     */
    using Span = gsl::span<char>;

  public:
    /**
     * @brief Default constructor that creates an empty registration
     * table
     */
    explicit Router() = default;

    /**
     * @brief Default destructor
     */
    ~Router() noexcept = default;

    /**
     * @brief Default move constructor
     */
    Router(Router&&) = default;

    /**
     * @brief Default move assignment operator
     */
    Router& operator = (Router&&) = default;

    /**
     * @brief Register a route for the {OPTIONS} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_options(T&& route, Generator generator);

    /**
     * @brief Register a route for the {GET} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_get(T&& route, Generator generator);

    /**
     * @brief Register a route for the {HEAD} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_head(T&& route, Generator generator);

    /**
     * @brief Register a route for the {POST} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_post(T&& route, Generator generator);

    /**
     * @brief Register a route for the {PUT} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_put(T&& route, Generator generator);

    /**
     * @brief Register a route for the {DELETE} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_delete(T&& route, Generator generator);

    /**
     * @brief Register a route for the {TRACE} HTTP method
    *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_trace(T&& route, Generator generator);

    /**
     * @brief Register a route for the {CONNECT} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_connect(T&& route, Generator generator);

    /**
     * @brief Register a route for the {PATCH} HTTP method
     *
     * @tparam T route : A pattern for a requested resource
     *
     * @param generator : The representation generator
     *
     * @return The object that invoked this method
     */
    template
    <
      typename T,
      typename = std::enable_if_t
                 <std::is_same
                 <std::string, std::remove_reference_t
                 <std::remove_const_t<T>>>::value>
    >
    Router& on_patch(T&& route, Generator generator);

    /**
     * @brief Install a new route table for route resolutions
     *
     * @tparam {http::Router} new_routes : The new route table
     * to install
     *
     * @return The object that invoked this method
     */
    template <typename Routee_Table>
    Router& install_new_configuration(Routee_Table&& new_routes);

    /**
     * @brief Get the generator for the pattern macthed resource
     *
     * @param method  : An HTTP method
     * @param pattern : The pattern to match in the registry
     *
     * @note : This method is not declared |const| because it utilizes
     * the subscript operator from std::map
     **/
    Generator match(const http::Method method, const std::string& pattern);

  private:
    Route_table route_table_;

    /*
     * Prohibit copy operations
     */
    Router(const Router&) = delete;
    Router& operator = (const Router&) = delete;
  }; //< class Router

  /**
   * @brief This class is used to represent errors that occur while
   * using the provided {Router} operations
   */
  class Router_error : public std::runtime_error {
    using runtime_error::runtime_error;
  };

  /**--v----------- Implementation Details -----------v--**/

  template <typename T, typename>
  inline Router& Router::on_options(T&& route, Generator generator) {
    route_table_[http::OPTIONS].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_get(T&& route, Generator generator) {
    route_table_[http::GET].emplace_back(std::forward<T>(route),  generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_head(T&& route, Generator generator) {
    route_table_[http::HEAD].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_post(T&& route, Generator generator) {
    route_table_[http::POST].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_put(T&& route, Generator generator) {
    route_table_[http::PUT].emplace_back(std::forward<T>(route), generator);
    return *this;
  }


  template <typename T, typename>
  inline Router& Router::on_delete(T&& route, Generator generator) {
    route_table_[http::DELETE].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_trace(T&& route, Generator generator) {
    route_table_[http::TRACE].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_connect(T&& route, Generator generator) {
    route_table_[http::CONNECT].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename T, typename>
  inline Router& Router::on_patch(T&& route, Generator generator) {
    route_table_[http::PATCH].emplace_back(std::forward<T>(route), generator);
    return *this;
  }

  template <typename Routee_Table>
  inline Router& Router::install_new_configuration(Routee_Table&& new_routes) {
    route_table_ = std::forward<Routee_Table>(new_routes).route_table_;
    return *this;
  }

  inline Router::Generator Router::match(const http::Method method, const std::string& path) {
    auto routes = route_table_[method];

    if (routes.empty()) {
      throw Router_error("No routes for method " + http::method::str(method));
    }

    for (auto& route : routes) {
      if (std::regex_match(path.begin(), path.end(), route.expr))
        return route.generator;
    }

    throw Router_error("No matching route for " + http::method::str(method) + " " + path);
  }

  /**--^----------- Implementation Details -----------^--**/

} //< namespace server

#endif //< ROUTER_HPP
