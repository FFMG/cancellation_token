#pragma once
// Licensed to Florent Guelfucci under one or more agreements.
// Florent Guelfucci licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
#include <exception>
#include <functional>
#include <map>
#include <mutex>

class operation_canceled_exception : public std::exception
{
};

class cancellation_token_source;
class cancellation_token
{
  friend cancellation_token_source;
public:
  cancellation_token(const cancellation_token& src);
  cancellation_token& operator=(const cancellation_token& src);
  ~cancellation_token() = default;

  static cancellation_token none();

  bool is_cancellation_requested() const;
  bool can_be_cancelled() const;
  void throw_if_cancellation_requested();

  unsigned register_callback(std::function<void()> callback);
  bool unregister_callback(unsigned id);

protected:
  cancellation_token(cancellation_token_source* source);

private:
  cancellation_token_source* _source;

  void throw_operation_canceled_exception();
};

class cancellation_token_source
{
public:
  cancellation_token_source();
  ~cancellation_token_source()  = default;

  cancellation_token_source(const cancellation_token_source&) = delete;
  cancellation_token_source& operator=(const cancellation_token_source&) = delete;

  bool is_cancellation_requested() const;

  cancellation_token token();

  void cancel();

  unsigned register_callback(std::function<void()> callback);
  bool unregister_callback(unsigned id);

private:
  enum state : unsigned
  {
    not_cancelled_sate = 0,
    notifiy_state = 1,
    notifiy_complete_state = 2,
  };

  state _state;
  std::mutex _notify_mutex;
  std::mutex _callback_mutex;
  std::map<unsigned, std::function<void()>> _callbacks;
};

/// cancellation token source implementation
cancellation_token_source::cancellation_token_source() : 
  _state(state::not_cancelled_sate)
{
}

bool cancellation_token_source::is_cancellation_requested() const
{
  return _state != state::not_cancelled_sate;
}

cancellation_token cancellation_token_source::token()
{
  return cancellation_token(this);
}

void cancellation_token_source::cancel()
{
  // make sure only one cancel is called.
  std::unique_lock<std::mutex> lock_notify(_notify_mutex);
  if(is_cancellation_requested())
  {
    return;
  }
  
  _state = state::notifiy_state;

  // we can now release the lock as the state was set already.
  lock_notify.unlock();

  // notify
  std::unique_lock<std::mutex> callback_lock(_callback_mutex);
  for( auto& callback : _callbacks)
  {
    callback.second();
  }
  callback_lock.unlock();
  _state = state::notifiy_complete_state;
}

unsigned cancellation_token_source::register_callback(std::function<void()> callback)
{
  std::unique_lock<std::mutex> lock_notify(_notify_mutex);
  if(is_cancellation_requested())
  {
    return 0;
  }
  lock_notify.unlock();

  std::lock_guard<std::mutex> callback_lock(_callback_mutex);
  auto id = static_cast<unsigned>(_callbacks.size());
  _callbacks[id] = callback;
  return id;
}

bool cancellation_token_source::unregister_callback(unsigned id)
{
  std::unique_lock<std::mutex> lock_notify(_notify_mutex);
  if(is_cancellation_requested())
  {
    return false;
  }
  lock_notify.unlock();

  std::lock_guard<std::mutex> callback_lock(_callback_mutex);
  for( auto& callback : _callbacks)
  {
    if(callback.first == id)
    {
      _callbacks.erase(id);
      return true;
    }
  }

  // we did not find it.
  return false;
}

/// cancellation token implementation
cancellation_token::cancellation_token(const cancellation_token& src) :
    _source(src._source)
{
}

cancellation_token& cancellation_token::operator=(const cancellation_token& src)
{
  if(this != &src)
  {
    _source = src._source;
  }
  return *this;
}

cancellation_token cancellation_token::none() 
{
  return cancellation_token(nullptr);
}

bool cancellation_token::is_cancellation_requested() const
{
  return _source != nullptr && _source->is_cancellation_requested();
}

bool cancellation_token::can_be_cancelled() const
{
  return _source != nullptr;
}

void cancellation_token::throw_if_cancellation_requested()
{
  if(is_cancellation_requested())
  {
    throw_operation_canceled_exception();
  }
}

cancellation_token::cancellation_token(cancellation_token_source* source) : 
  _source(source)
{
}

void cancellation_token::throw_operation_canceled_exception()
{
  throw operation_canceled_exception();
}

unsigned cancellation_token::register_callback(std::function<void()> callback)
{
  //  if we have no source then it will never be called.
  if(_source == nullptr)
  {
    return 0;
  }
  return _source->register_callback(callback);
}

bool cancellation_token::unregister_callback(unsigned id)
{
  //  if we have no source then it will never be called.
  if(_source == nullptr)
  {
    return false;
  }
  return _source->unregister_callback(id);
}
