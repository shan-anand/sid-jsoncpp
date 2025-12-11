#include "utils.h"
#include <stdexcept>
#include <limits>

std::string json::to_string(bool _value)
{
  return _value ? "true" : "false";
}

bool json::to_bool(const std::string& _str)
{
  if (_str == "true")
    return true;
  else if (_str == "false")
    return false;
  throw std::invalid_argument("Invalid boolean string: " + _str);
}

bool json::to_bool(const std::string& _str, bool& _out, std::string* _pstrError/* = nullptr*/)
{
  if (_str == "true")
    _out = true;
  else if (_str == "false")
    _out = false;
  else
  {
    if (_pstrError)
      *_pstrError = "Invalid boolean string: " + _str;
    return false;
  }
  return true;
}

bool json::to_num(const std::string& _str, uint32_t& _out, std::string* _pstrError/* = nullptr*/)
{
  try
  {
    size_t idx = 0;
    unsigned long long val = std::stoul(_str, &idx);
    if (idx != _str.length())
    {
      if (_pstrError)
        *_pstrError = "Extra characters found after number: " + _str.substr(idx);
      return false;
    }
    if (val > static_cast<unsigned long long>(std::numeric_limits<uint32_t>::max()))
    {
      if (_pstrError)
        *_pstrError = "Value out of range for uint32_t: " + _str;
      return false;
    }
    _out = static_cast<uint32_t>(val);
    return true;
  }
  catch (const std::invalid_argument& e)
  {
    if (_pstrError)
      *_pstrError = "Invalid argument: " + std::string(e.what());
    return false;
  }
  catch (const std::out_of_range& e)
  {
    if (_pstrError)
      *_pstrError = "Out of range: " + std::string(e.what());
    return false;
  }
}

bool json::to_num(const std::string& _str, long double& _out, std::string* _pstrError/* = nullptr*/)
{
  try
  {
    size_t idx = 0;
    _out = std::stold(_str, &idx);
    if (idx != _str.length())
    {
      if (_pstrError)
        *_pstrError = "Extra characters found after number: " + _str.substr(idx);
      return false;
    }
    return true;
  }
  catch (const std::invalid_argument& e)
  {
    if (_pstrError)
      *_pstrError = "Invalid argument: " + std::string(e.what());
    return false;
  }
  catch (const std::out_of_range& e)
  {
    if (_pstrError)
      *_pstrError = "Out of range: " + std::string(e.what());
    return false;
  }
}

bool json::to_num(const std::string& _str, int64_t& _out, std::string* _pstrError/* = nullptr*/)
{
  try
  {
    size_t idx = 0;
    _out = std::stoll(_str, &idx);
    if (idx != _str.length())
    {
      if (_pstrError)
        *_pstrError = "Extra characters found after number: " + _str.substr(idx);
      return false;
    }
    return true;
  }
  catch (const std::invalid_argument& e)
  {
    if (_pstrError)
      *_pstrError = "Invalid argument: " + std::string(e.what());
    return false;
  }
  catch (const std::out_of_range& e)
  {
    if (_pstrError)
      *_pstrError = "Out of range: " + std::string(e.what());
    return false;
  }
}

bool json::to_num(const std::string& _str, uint64_t& _out, std::string* _pstrError/* = nullptr*/)
{
  try
  {
    size_t idx = 0;
    _out = std::stoull(_str, &idx);
    if (idx != _str.length())
    {
      if (_pstrError)
        *_pstrError = "Extra characters found after number: " + _str.substr(idx);
      return false;
    }
    return true;
  }
  catch (const std::invalid_argument& e)
  {
    if (_pstrError)
      *_pstrError = "Invalid argument: " + std::string(e.what());
    return false;
  }
  catch (const std::out_of_range& e)
  {
    if (_pstrError)
      *_pstrError = "Out of range: " + std::string(e.what());
    return false;
  }
}

std::string json::get_sep(size_t _number)
{
  std::string out = std::to_string(_number);
  for ( int i = out.length()-3; i > 0; i -= 3 )
    out.insert(i, ",");
  return out;
}

size_t json::split(
  std::vector<std::string>& _out,
  const std::string&        _str,
  const char                _delimiter,
  const uint32_t            _options /*= 0*/
)
{
  _out.clear();
  const size_t strLen = _str.length();
  const bool skipEmpty = (_options & SPLIT_TRIM_SKIP_EMPTY) != 0;
  const bool trimSpaces = (_options & SPLIT_TRIM) != 0;

  for (size_t start = 0, end = 0; end <= strLen; end++ )
  {
    if ( _str[end] == _delimiter || end == strLen)
    {
      std::string token = _str.substr(start, end - start);
      if (trimSpaces)
      {
        size_t first = token.find_first_not_of(" \t\n\r");
        size_t last = token.find_last_not_of(" \t\n\r");
        if (first != std::string::npos && last != std::string::npos)
          token = token.substr(first, last - first + 1);
        else
          token.clear();
      }
      if (!(skipEmpty && token.empty()))
        _out.push_back(token);
      start = end + 1;
    }
  }
  return _out.size();
}
