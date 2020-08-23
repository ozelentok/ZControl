#pragma once
#include <string>
#include <system_error>

class GAIErrorCategory : public std::error_category {
private:
  int _errno;

public:
  GAIErrorCategory(int system_errno);
  virtual const char *name() const noexcept override final;
  virtual std::string message(int error_code) const override final;
};
