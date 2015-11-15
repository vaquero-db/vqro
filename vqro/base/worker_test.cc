#include <thread>
#include <chrono>

#include "vqro/base/base.h"
#include "vqro/base/worker.h"
#include "gtest/gtest.h"


namespace {

using namespace vqro;


TEST(WorkerTest, WorkerDoesWorkAndStops) {
  WorkerThread worker {};
  bool work_got_done = false;

  worker.Start().wait();
  worker.Do([&work_got_done] {
    work_got_done = true;
  }).wait();
  worker.Stop().wait();

  EXPECT_TRUE(work_got_done);
  EXPECT_FALSE(worker.Alive());
}


TEST(WorkerTest, WorkerRefusesTooMuchWork) {
  WorkerThread worker {};
  std::promise<void> will_start;
  std::future<void> work_started = will_start.get_future();
  bool keep_working = true;

  auto some_work = [&] {
    will_start.set_value();
    while (keep_working)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  };

  FLAGS_worker_task_queue_limit = 2;
  worker.Start().wait();

  ASSERT_EQ(worker.TasksQueued(), 0);
  worker.Do(some_work);
  work_started.wait();  // so we know our blocking task is not queued
  ASSERT_EQ(worker.TasksQueued(), 0);

  worker.Do(some_work);
  ASSERT_EQ(worker.TasksQueued(), 1);
  auto last_queued = worker.Do(some_work);
  ASSERT_EQ(worker.TasksQueued(), 2);

  EXPECT_THROW(worker.Do(some_work), WorkerThreadTooBusy);

  keep_working = false;
  last_queued.wait();
  EXPECT_EQ(worker.TasksQueued(), 0);
  worker.Stop().wait();

  EXPECT_FALSE(worker.Alive());
}


}  // namespace
