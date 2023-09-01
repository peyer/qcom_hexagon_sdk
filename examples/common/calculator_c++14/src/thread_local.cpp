#include <stdlib.h>
#include <iostream>
#include <thread>
#include <mutex>
#include "calculator.h"

#define FARF_ERROR 1
#include "HAP_farf.h"

const int SEED_VALUE = 7;
const int MEM_SIZE  = 1024;

const unsigned char data[MEM_SIZE] = {
#include "data.h"
};

const int MAX_THREADS = 5;
const int start[MAX_THREADS] = {
 0,  1,  2,  3,  4, 
};

const int end[MAX_THREADS] = {
5, 6, 7, 8, 9, 
};

std::mutex cout_mutex;

static void check(const int id, const int value)
{
  int sum = SEED_VALUE;
  for (int i = start[id]; i < end[id]; i++)
    sum += data[i];
  if (sum != value) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    FARF(ERROR, "PASS: id = %d, sum (%d) == value (%d)", id, sum, value);
    exit(1);
  }
#ifdef DEBUG
  else {
    std::lock_guard<std::mutex> lock(cout_mutex);
    FARF(HIGH, "PASS: id = %d, sum (%d) == value (%d)", id, sum, value);
  }
#endif
}

class Count {
public:
  Count();
  operator int() { return count; };
  Count& operator += (int x) { count += x; return *this; }
private:
  int count;
};

std::atomic<unsigned int> numConstructorCalls(0);

Count::Count()
{
#ifdef DEBUG
  std::lock_guard<std::mutex> lock(cout_mutex);
  FARF(HIGH, "Count::Count() called");
#endif
  numConstructorCalls++;
  count = SEED_VALUE;
}

// README This tests uses a non-static TLS variable with a constructor
thread_local Count count;

void thread_func(const int id, const int start, const int end)
{
  for (int i = start; i < end; i++)
    count += data[i];
  check(id, count);
}

int calculator_plus_test_tls(int64 *res)
{
  FARF(ALWAYS, "test_tls start for threads %d", MAX_THREADS);
  std::thread *threads[MAX_THREADS];
  for (int i = 0; i < MAX_THREADS; i++)
    threads[i] = new std::thread(thread_func, i, start[i], end[i]);
  for (int i = 0; i < MAX_THREADS; i++) {
    threads[i]->join();
    delete threads[i];
  }
  if (numConstructorCalls != MAX_THREADS) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    FARF(ERROR, "FAIL: numConstructorCalls (%d) != MAX_THREADS (%d)", (int)numConstructorCalls, MAX_THREADS);
    //exit(1);
    return -1;
  }
  FARF(ALWAYS, "PASS");
  if (res)
     *res = 0;
  return 0;
}

