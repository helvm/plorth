/*
 * Copyright (c) 2017, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <plorth/context.hpp>
#include <plorth/unicode.hpp>

#include "./utils.hpp"

#include <algorithm>
#include <cstring>

namespace plorth
{
  namespace
  {
    class simple_string : public string
    {
    public:
      explicit simple_string(const unichar* chars, size_type length)
        : m_length(length)
        , m_chars(m_length > 0 ? new unichar[m_length] : nullptr)
      {
        if (m_length > 0)
        {
          std::memcpy(m_chars, chars, sizeof(unichar) * m_length);
        }
      }

      ~simple_string()
      {
        if (m_length > 0)
        {
          delete[] m_chars;
        }
      }

      size_type length() const
      {
        return m_length;
      }

      value_type at(size_type offset) const
      {
        return m_chars[offset];
      }

    private:
      const size_type m_length;
      unichar* m_chars;
    };

    class concat_string : public string
    {
    public:
      explicit concat_string(const ref<string>& left, const ref<string>& right)
        : m_left(left)
        , m_right(right) {}

      size_type length() const
      {
        return m_left->length() + m_right->length();
      }

      value_type at(size_type offset) const
      {
        const size_type left_length = m_left->length();

        if (offset < left_length)
        {
          return m_left->at(offset);
        } else {
          return m_right->at(offset - left_length);
        }
      }

    private:
      const ref<string> m_left;
      const ref<string> m_right;
    };

    class substring : public string
    {
    public:
      explicit substring(const ref<string>& original,
                         size_type offset,
                         size_type length)
        : m_original(original)
        , m_offset(offset)
        , m_length(length) {}

      size_type length() const
      {
        return m_length;
      }

      value_type at(size_type offset) const
      {
        return m_original->at(m_offset + offset);
      }

    private:
      const ref<string> m_original;
      const size_type m_offset;
      const size_type m_length;
    };
  }

  bool string::equals(const ref<class value>& that) const
  {
    const size_type len = length();
    ref<string> str;

    if (!that || !that->is(type_string))
    {
      return false;
    }
    str = that.cast<string>();
    if (len != str->length())
    {
      return false;
    }
    for (size_type i = 0; i < len; ++i)
    {
      if (at(i) != str->at(i))
      {
        return false;
      }
    }

    return true;
  }

  unistring string::to_string() const
  {
    const size_type len = length();
    unistring result;

    result.reserve(len);
    for (size_type i = 0; i < len; ++i)
    {
      result.append(1, at(i));
    }

    return result;
  }

  unistring string::to_source() const
  {
    return json_stringify(to_string());
  }

  ref<class string> runtime::string(const unistring& input)
  {
    return string(input.c_str(), input.length());
  }

  ref<class string> runtime::string(string::const_pointer chars,
                                    string::size_type length)
  {
    return new (*m_memory_manager) simple_string(chars, length);
  }

  /**
   * Word: length
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - number
   *
   * Returns the length of the string.
   */
  static void w_length(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      ctx->push(str);
      ctx->push_int(str->length());
    }
  }

  /**
   * Word: space?
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the string contains only whitespace characters. Empty strings
   * return false.
   */
  static void w_is_space(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();

      ctx->push(str);
      if (!length)
      {
        ctx->push_boolean(false);
        return;
      }
      for (string::size_type i = 0; i < length; ++i)
      {
        if (!unichar_isspace(str->at(i)))
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * Word: lower-case?
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the string contains only lower case characters. Empty strings
   * return false.
   */
  static void w_is_lower_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();

      ctx->push(str);
      if (!length)
      {
        ctx->push_boolean(false);
        return;
      }
      for (string::size_type i = 0; i < length; ++i)
      {
        if (!unichar_islower(str->at(i)))
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * Word: upper-case?
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - boolean
   *
   * Tests whether the string contains only upper case characters. Empty strings
   * return false.
   */
  static void w_is_upper_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();

      ctx->push(str);
      if (!length)
      {
        ctx->push_boolean(false);
        return;
      }
      for (string::size_type i = 0; i < length; ++i)
      {
        if (!unichar_isupper(str->at(i)))
        {
          ctx->push_boolean(false);
          return;
        }
      }
      ctx->push_boolean(true);
    }
  }

  /**
   * Word: chars
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts characters from the string and returns them in an array of
   * substrings.
   */
  static void w_chars(const ref<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      std::vector<ref<value>> output;

      output.reserve(length);
      for (string::size_type i = 0; i < length; ++i)
      {
        const unichar c = str->at(i);

        output.push_back(runtime->string(&c, 1));
      }
      ctx->push(str);
      ctx->push_array(output.data(), length);
    }
  }

  /**
   * Word: runes
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts Unicode code points from the string and returns them in an array
   * of numbers.
   */
  static void w_runes(const ref<context>& ctx)
  {
    const auto& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      std::vector<ref<value>> output;

      output.reserve(length);
      for (string::size_type i = 0; i < length; ++i)
      {
        output.push_back(runtime->number(static_cast<std::int64_t>(str->at(i))));
      }
      ctx->push(str);
      ctx->push_array(output.data(), length);
    }
  }

  /**
   * Word: words
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts white space separated words from the string and returns them in
   * an array.
   */
  static void w_words(const ref<context>& ctx)
  {
    const ref<class runtime>& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      string::size_type begin = 0;
      string::size_type end = 0;
      std::vector<ref<value>> result;

      for (string::size_type i = 0; i < length; ++i)
      {
        if (unichar_isspace(str->at(i)))
        {
          if (end - begin > 0)
          {
            result.push_back(runtime->value<substring>(str, begin, end - begin));
          }
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(runtime->value<substring>(str, begin, end - begin));
      }

      ctx->push(str);
      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: lines
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   * - array
   *
   * Extracts lines from the string and returns them in an array.
   */
  static void w_lines(const ref<context>& ctx)
  {
    const ref<class runtime>& runtime = ctx->runtime();
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      string::size_type begin = 0;
      string::size_type end = 0;
      std::vector<ref<value>> result;

      for (string::size_type i = 0; i < length; ++i)
      {
        const unichar c = str->at(i);

        if (i + 1 < length && c == '\r' && str->at(i + 1) == '\n')
        {
          result.push_back(runtime->value<substring>(str, begin, end - begin));
          begin = end = ++i + 1;
        }
        else if (c == '\n' || c == '\r')
        {
          result.push_back(runtime->value<substring>(str, begin, end - begin));
          begin = end = i + 1;
        } else {
          ++end;
        }
      }
      if (end - begin > 0)
      {
        result.push_back(runtime->value<substring>(str, begin, end - begin));
      }

      ctx->push(str);
      ctx->push_array(result.data(), result.size());
    }
  }

  /**
   * Word: reverse
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Reverses the string.
   */
  static void w_reverse(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      unichar result[length];

      for (string::size_type i = length; i > 0; --i)
      {
        result[length - i] = str->at(i - 1);
      }
      ctx->push_string(result, length);
    }
  }

  /*
   * Word: upper-case
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Converts the string into upper case.
   */
  static void w_upper_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      unichar result[length];

      for (string::size_type i = 0; i < length; ++i)
      {
        result[i] = unichar_toupper(str->at(i));
      }
      ctx->push_string(result, length);
    }
  }

  /**
   * Word: lower-case
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Converts the string into lower case.
   */
  static void w_lower_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      unichar result[length];

      for (string::size_type i = 0; i < length; ++i)
      {
        result[i] = unichar_tolower(str->at(i));
      }
      ctx->push_string(result, length);
    }
  }

  /**
   * Word: swap-case
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Turns lower case characters in the string into upper case and vice versa.
   */
  static void w_swap_case(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      unichar output[length];

      for (string::size_type i = 0; i < length; ++i)
      {
        unichar c = str->at(i);

        if (unichar_islower(c))
        {
          c = unichar_toupper(c);
        } else {
          c = unichar_tolower(c);
        }
        output[i] = c;
      }
      ctx->push_string(output, length);
    }
  }

  /**
   * Word: capitalize
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Converts the first character of the string into upper case and the
   * remaining characters into lower case.
   */
  static void w_capitalize(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const auto length = str->length();
      unichar output[length];

      for (string::size_type i = 0; i < length; ++i)
      {
        unichar c = str->at(i);

        if (i == 0)
        {
          c = unichar_toupper(c);
        } else {
          c = unichar_tolower(c);
        }
        output[i] = c;
      }
      ctx->push_string(output, length);
    }
  }

  /**
   * Word: trim
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Strips whitespace from the begining and the end of the string.
   */
  static void w_trim(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const string::size_type length = str->length();
      string::size_type i, j;

      for (i = 0; i < length; ++i)
      {
        if (!unichar_isspace(str->at(i)))
        {
          break;
        }
      }
      for (j = length; j != 0; --j)
      {
        if (!unichar_isspace(str->at(j - 1)))
        {
          break;
        }
      }
      if (i != 0 || j != length)
      {
        ctx->push(ctx->runtime()->value<substring>(str, i, j - i));
      } else {
        ctx->push(str);
      }
    }
  }

  /**
   * Word: trim-left
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Strips whitespace from the begining of the string.
   */
  static void w_trim_left(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const string::size_type length = str->length();
      string::size_type i;

      for (i = 0; i < length; ++i)
      {
        if (!unichar_isspace(str->at(i)))
        {
          break;
        }
      }
      if (i != 0)
      {
        ctx->push(ctx->runtime()->value<substring>(str, i, length - i));
      } else {
        ctx->push(str);
      }
    }
  }

  /**
   * Word: trim-right
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - string
   *
   * Strips whitespace from the end of the string.
   */
  static void w_trim_right(const ref<context>& ctx)
  {
    ref<string> str;

    if (ctx->pop_string(str))
    {
      const string::size_type length = str->length();
      string::size_type i;

      for (i = length; i != 0; --i)
      {
        if (!unichar_isspace(str->at(i - 1)))
        {
          break;
        }
      }
      if (i != length)
      {
        ctx->push(ctx->runtime()->value<substring>(str, 0, i));
      } else {
        ctx->push(str);
      }
    }
  }

  /**
   * Word: >number
   * Prototype: string
   *
   * Takes:
   * - string
   *
   * Gives:
   * - number
   *
   * Converts string into a floating point decimal number, or throws a value
   * error if the number cannot be converted into one.
   */
  static void w_to_number(const ref<context>& ctx)
  {
    ref<string> a;

    if (ctx->pop_string(a))
    {
      const unistring str = a->to_string();

      if (is_number(str))
      {
        ctx->push_number(str);
      } else {
        ctx->error(error::code_value, U"Could not convert string to number.");
      }
    }
  }

  /**
   * Word: +
   * Prototype: string
   *
   * Takes:
   * - string
   * - string
   *
   * Gives:
   * - string
   *
   * Concatenates the contents of the two strings and returns the result.
   */
  static void w_concat(const ref<context>& ctx)
  {
    ref<string> a;
    ref<string> b;

    if (ctx->pop_string(a) && ctx->pop_string(b))
    {
      if (a->empty())
      {
        ctx->push(b);
      }
      else if (b->empty())
      {
        ctx->push(a);
      } else {
        ctx->push(ctx->runtime()->value<concat_string>(b, a));
      }
    }
  }

  /**
   * Word: *
   * Prototype: string
   *
   * Takes:
   * - number
   * - string
   *
   * Gives:
   * - string
   *
   * Repeats the string given number of times.
   */
  static void w_repeat(const ref<context>& ctx)
  {
    ref<string> str;
    ref<number> num;

    if (ctx->pop_string(str) && ctx->pop_number(num))
    {
      const auto length = str->length();
      std::int64_t count = num->as_int();
      unistring result;

      if (count < 0)
      {
        count = -count;
      }

      result.reserve(length * count);

      while (count > 0)
      {
        --count;
        for (string::size_type i = 0; i < length; ++i)
        {
          result.append(1, str->at(i));
        }
      }

      ctx->push_string(result);
    }
  }

  /**
   * Word: @
   * Prototype: string
   *
   * Takes:
   * - number
   * - string
   *
   * Gives:
   * - string
   * - string
   *
   * Retrieves a character at given index. Negative indices count backwards
   * from the end of the string. If given index is out of bounds, a range error
   * will be thrown.
   */
  static void w_get(const ref<context>& ctx)
  {
    ref<string> str;
    ref<number> num;

    if (ctx->pop_string(str) && ctx->pop_number(num))
    {
      const auto length = str->length();
      std::int64_t index = num->as_int();
      unichar c;

      if (index < 0)
      {
        index += length;
      }

      ctx->push(str);

      if (index < 0 || index > static_cast<std::int64_t>(length))
      {
        ctx->error(error::code_range, U"String index out of bounds.");
        return;
      }

      c = str->at(index);
      ctx->push(ctx->runtime()->string(&c, 1));
    }
  }

  namespace api
  {
    runtime::prototype_definition string_prototype()
    {
      return
      {
        { U"length", w_length },
        { U"chars", w_chars },
        { U"runes", w_runes },
        { U"words", w_words },
        { U"lines", w_lines },

        // Tests.
        // TODO: includes?
        // TODO: index-of
        // TODO: starts-with?
        // TODO: ends-with?
        { U"space?", w_is_space },
        { U"lower-case?", w_is_lower_case },
        { U"upper-case?", w_is_upper_case },

        // Conversions.
        { U"reverse", w_reverse },
        { U"upper-case", w_upper_case },
        { U"lower-case", w_lower_case },
        { U"swap-case", w_swap_case },
        { U"capitalize", w_capitalize },
        { U"trim", w_trim },
        { U"trim-left", w_trim_left },
        { U"trim-right", w_trim_right },
        // TODO: pad-left
        // TODO: pad-right
        // TODO: substring
        // TODO: split
        // TODO: replace
        // TODO: normalize
        { U">number", w_to_number },

        { U"+", w_concat },
        { U"*", w_repeat },
        { U"@", w_get }
      };
    }
  }
}
