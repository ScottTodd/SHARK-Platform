// Copyright 2024 Advanced Micro Devices, Inc
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef SHORTFIN_WORKER_H
#define SHORTFIN_WORKER_H

#include <functional>
#include <string>
#include <vector>

#include "iree/base/loop_sync.h"
#include "shortfin/support/iree_concurrency.h"

namespace shortfin {

// Cooperative worker.
class Worker {
 public:
  struct Options {
    iree_allocator_t allocator;
    std::string name;

    Options(iree_allocator_t allocator, std::string name)
        : allocator(allocator), name(name) {}
  };

  Worker(Options options);
  Worker(const Worker &) = delete;
  ~Worker();

  const std::string_view name() const { return options_.name; }

  void Start();
  void Kill();
  void WaitForShutdown();

  // Enqueues a callback on the worker.
  void EnqueueCallback(std::function<void()> callback);

 private:
  int Run();
  iree_status_t TransactLoop(iree_status_t signal_status);

  const Options options_;
  iree_slim_mutex mu_;
  iree_thread_ptr thread_;
  iree_event signal_transact_;
  iree_event signal_ended_;

  // State management. These are all manipulated both on and off the worker
  // thread.
  bool kill_ = false;
  std::vector<std::function<void()>> pending_thunks_;

  // Loop management. This is all purely operated on the worker thread.
  iree_loop_sync_scope_t loop_scope_;
  iree_loop_sync_t *loop_sync_;
  iree_loop_t loop_;
  std::vector<std::function<void()>> next_thunks_;
};

}  // namespace shortfin

#endif  // SHORTFIN_WORKER_H