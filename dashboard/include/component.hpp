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

#pragma once
#ifndef DASHBOARD_COMPONENT_HPP
#define DASHBOARD_COMPONENT_HPP

#include "common.hpp"

namespace dashboard {

/**
 * This is an interface used to recognize an entity that can be
 * added to the dashboard
 *
 * To create a new dashboard component simply inherit this class
 * and override the abstract methods
 */
class Component {
public:
  /**
   * Provide a component identifier which will be used as a key
   * within the dashboard to locate the component
   *
   * @return The component identifier as a {std::string} object
   */
  virtual std::string key() const = 0;

  /**
   * Serialize the component to the specified writer as JSON
   *
   * @param
   * The writer to serialize the component to
   */
  virtual void serialize(Writer&) const = 0;

  /**
   * Default destructor
   */
  virtual ~Component() = default;
}; //< class Component

} //< namespace dashboard

#endif //< DASHBOARD_COMPONENT_HPP
