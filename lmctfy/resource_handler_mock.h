// Copyright 2013 Google Inc. All Rights Reserved.
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

#ifndef SRC_RESOURCE_HANDLER_MOCK_H_
#define SRC_RESOURCE_HANDLER_MOCK_H_

#include "lmctfy/resource_handler.h"

#include "gmock/gmock.h"

namespace containers {
namespace lmctfy {

class MockResourceHandlerFactory : public ResourceHandlerFactory {
 public:
  explicit MockResourceHandlerFactory(ResourceType type)
      : ResourceHandlerFactory(type) {}
  virtual ~MockResourceHandlerFactory() {}

  MOCK_METHOD1(Get, ::util::StatusOr<ResourceHandler *>(
      const string &container_name));
  MOCK_METHOD2(Create, ::util::StatusOr<ResourceHandler *>(
      const string &container_name,
      const ContainerSpec &spec));
  MOCK_METHOD1(InitMachine, ::util::Status(const InitSpec &spec));
};

typedef ::testing::NiceMock<MockResourceHandlerFactory>
    NiceMockResourceHandlerFactory;
typedef ::testing::StrictMock<MockResourceHandlerFactory>
    StrictMockResourceHandlerFactory;

class MockResourceHandler : public ResourceHandler {
 public:
  MockResourceHandler(const string &container_name, ResourceType type)
      : ResourceHandler(container_name, type) {}
  virtual ~MockResourceHandler() {}

  MOCK_METHOD2(Update, ::util::Status(const ContainerSpec &spec,
                                      Container::UpdatePolicy policy));
  MOCK_METHOD0(Destroy, ::util::Status());
  MOCK_METHOD1(CreateResource, ::util::Status(const ContainerSpec &spec));
  MOCK_METHOD1(Enter, ::util::Status(const ::std::vector<pid_t> &tids));
  MOCK_CONST_METHOD2(Stats, ::util::Status(Container::StatsType type,
                                           ContainerStats *output));
  MOCK_CONST_METHOD1(Spec, ::util::Status(ContainerSpec *spec));
  MOCK_METHOD2(RegisterNotification,
               ::util::StatusOr<Container::NotificationId>(
                   const EventSpec &spec,
                   Callback1< ::util::Status> *callback));
};

typedef ::testing::NiceMock<MockResourceHandler> NiceMockResourceHandler;
typedef ::testing::StrictMock<MockResourceHandler> StrictMockResourceHandler;

}  // namespace lmctfy
}  // namespace containers

#endif  // SRC_RESOURCE_HANDLER_MOCK_H_
