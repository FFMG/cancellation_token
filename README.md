# cancellation_token

C++ Implementation of the .NET CancellationToken Struct

## What is it?

Very simple function to create a cancellation token similar to the .NET implementation.

## Example

```c++
#include <chrono>
#include <iostream>
#include <thread>
#include "../src/cancellation_token.h"

using namespace myoddweb;

void do_something(cancellation_token token)
{
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

  // do stuff
  std::thread t(do_something, cs.token());

  // do more stuff
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  cs.cancel();

  // wait for the thread to complete...
  t.join();
}
```

### Callback

You can gerister for callback when a cancellation is called.

```c++
using namespace myoddweb;

int main()
{
  // the source
  auto cs = cancellation_token_source();

  auto id = cs.register_callback([]{
    std::cout << "Callback main called!" << std::endl;
  });

  // do stuff

  // we don't want to be called anymore
  cs.unregister_callback(id);

  // cancel 
  cs.cancel();

  ...
}
```

### Compiling the example

Using g++ just call ... 

`g++ -Wall -Wextra -Werror -O3 ./examples/main.cpp -o main -std=c++20`

# What next?

I want to add notifications at some point.