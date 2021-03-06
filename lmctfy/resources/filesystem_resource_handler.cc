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

#include "lmctfy/resources/filesystem_resource_handler.h"

#include <vector>

#include "file/base/path.h"
#include "util/safe_types/unix_gid.h"
#include "util/safe_types/unix_uid.h"
#include "util/errors.h"
#include "util/task/codes.pb.h"

using ::file::Basename;
using ::file::JoinPath;
using ::util::UnixGid;
using ::util::UnixUid;
using ::std::vector;
using ::util::Status;
using ::util::StatusOr;
using ::util::error::INVALID_ARGUMENT;

namespace containers {
namespace lmctfy {

StatusOr<FilesystemResourceHandlerFactory *>
FilesystemResourceHandlerFactory::New(
    CgroupFactory *cgroup_factory, const KernelApi *kernel,
    EventFdNotifications *eventfd_notifications) {
  // RLimit hierarchy must be mounted.
  if (!cgroup_factory->IsMounted(RLimitControllerFactory::HierarchyType())) {
    return Status(::util::error::NOT_FOUND,
                  "Filesystem resource depends on the rlimit cgroup hierarchy");
  }

  // Create rlimit controller.
  RLimitControllerFactory *rlimit_controller = new RLimitControllerFactory(
      cgroup_factory, kernel, eventfd_notifications);

  return new FilesystemResourceHandlerFactory(rlimit_controller, cgroup_factory,
                                              kernel);
}

FilesystemResourceHandlerFactory::FilesystemResourceHandlerFactory(
    const RLimitControllerFactory *rlimit_controller_factory,
    CgroupFactory *cgroup_factory, const KernelApi *kernel)
  : CgroupResourceHandlerFactory(RESOURCE_FILESYSTEM, cgroup_factory, kernel),
  rlimit_controller_factory_(rlimit_controller_factory) {}

string FilesystemResourceHandlerFactory::GetEffectiveContainerName(
    const string &container_name) const {
  return JoinPath("/", file::Basename(container_name).ToString());
}

StatusOr<ResourceHandler *>
FilesystemResourceHandlerFactory::GetResourceHandler(
    const string &container_name) const {
  RLimitController *controller;
  const string effective_container_name =
      GetEffectiveContainerName(container_name);
  RETURN_IF_ERROR(rlimit_controller_factory_->Get(effective_container_name),
                  &controller);
  return new FilesystemResourceHandler(container_name, kernel_, controller);
}

StatusOr<ResourceHandler *>
FilesystemResourceHandlerFactory::CreateResourceHandler(
    const string &container_name, const ContainerSpec &spec) const {
  RLimitController *controller;
  const string effective_container_name =
      GetEffectiveContainerName(container_name);
  RETURN_IF_ERROR(rlimit_controller_factory_->Create(
      effective_container_name, UnixUid(spec.owner()),
      UnixGid(spec.owner_group())),
                  &controller);

  return new FilesystemResourceHandler(container_name, kernel_, controller);
}

FilesystemResourceHandler::FilesystemResourceHandler(
    const string &container_name, const KernelApi *kernel,
    RLimitController *rlimit_controller)
  : CgroupResourceHandler(container_name, RESOURCE_FILESYSTEM, kernel,
                          {rlimit_controller}),
  rlimit_controller_(rlimit_controller) {}


Status FilesystemResourceHandler::Stats(Container::StatsType type,
                                        ContainerStats *output) const {
  // TODO(kyurtsever) Expose FilesystemResource stats.
  return Status::OK;
}

Status FilesystemResourceHandler::Spec(ContainerSpec *spec) const {
  FilesystemSpec *filesystem = spec->mutable_filesystem();
  if (!filesystem->has_fd_limit()) {
    filesystem->set_fd_limit(
        XRETURN_IF_ERROR(rlimit_controller_->GetFdLimit()));
  }
  return Status::OK;
}

StatusOr<Container::NotificationId>
FilesystemResourceHandler::RegisterNotification(
    const EventSpec &spec, Callback1<Status> *callback) {
  ::std::unique_ptr<Callback1<Status>> callback_deleter(callback);
  return Status(::util::error::NOT_FOUND, "No handled event found");
}

Status FilesystemResourceHandler::DoUpdate(const ContainerSpec &spec) {
  const FilesystemSpec &filesystem = spec.filesystem();
  if (filesystem.has_fd_limit()) {
    RETURN_IF_ERROR(rlimit_controller_->SetFdLimit(filesystem.fd_limit()));
  }
  return Status::OK;
}

void FilesystemResourceHandler::RecursiveFillDefaults(
    ContainerSpec *spec) const {
  // TODO(kyurtsever) Define default value for fd_limit.
}

Status FilesystemResourceHandler::VerifyFullSpec(
    const ContainerSpec &spec) const {
  const FilesystemSpec &filesystem = spec.filesystem();
  if (!filesystem.has_fd_limit()) {
    return Status(INVALID_ARGUMENT, "FD limit has to be defined.");
  }
  return Status::OK;
}

}  // namespace lmctfy
}  // namespace containers
