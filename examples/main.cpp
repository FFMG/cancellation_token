#include <chrono>
#include <iostream>
#include <thread>
#include "../src/cancellation_token.h"

using namespace myoddweb;

void do_something(cancellation_token token)
{
  token.register_callback([]{
    std::cout << "Callback in thread called!" << std::endl;
  });

  std::cout << "do_something is starting ... " << std::endl;
  std::chrono::milliseconds timespan(1000);
  while(!token.is_cancellation_requested())
  {
    std::cout << "do_something is waiting ..." << std::endl;
    // wait a little.
    std::this_thread::sleep_for(timespan);
  }
  std::cout << "do_something is done" << std::endl;
}

int main()
{
  // the source
  auto cs = cancellation_token_source();

  cs.register_callback([]{
    std::cout << "Callback main called!" << std::endl;
  });

  // do stuff
  std::thread t1(do_something, cs.token());
  std::thread t2(do_something, cs.token());
  std::thread t3(do_something, cs.token());

  // do more stuff
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  cs.cancel();

  // wait for the thread to complete...
  t1.join();
  t2.join();
  t3.join();
}
