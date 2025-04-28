#pragma once
// Licensed to Florent Guelfucci under one or more agreements.
// Florent Guelfucci licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
#include <exception>

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

protected:
  cancellation_token(const cancellation_token_source* source);

private:
  const cancellation_token_source* _source;

  void throw_operation_canceled_exception();
};

class cancellation_token_source
{
public:
  cancellation_token_source() : 
    _state(state::not_cancelled_sate)
  {
  }
  ~cancellation_token_source()  = default;

  cancellation_token_source(const cancellation_token_source&) = delete;
  cancellation_token_source& operator=(const cancellation_token_source&) = delete;

  bool is_cancellation_requested() const
  {
    return _state != state::not_cancelled_sate;
  }

  cancellation_token token() const
  {
    return cancellation_token(this);
  }

  void cancel()
  {
    _state = state::notifiy_state;
    // notify
    _state = state::notifiy_complete_state;
  }

private:
  enum state : unsigned
  {
    not_cancelled_sate = 0,
    notifiy_state = 1,
    notifiy_complete_state = 2,
  };

  state _state;
};

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

cancellation_token::cancellation_token(const cancellation_token_source* source) : 
  _source(source)
{
}

void cancellation_token::throw_operation_canceled_exception()
{
  throw operation_canceled_exception();
}

