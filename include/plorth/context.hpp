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
#ifndef PLORTH_CONTEXT_HPP_GUARD
#define PLORTH_CONTEXT_HPP_GUARD

#include <plorth/runtime.hpp>
#include <plorth/value-error.hpp>

#include <deque>

namespace plorth
{
  class word;

  /**
   * Represents program execution state.
   */
  class context : public memory::managed
  {
  public:
    using container_type = std::deque<ref<value>>;
    using dictionary_type = std::unordered_map<unistring, ref<quote>>;

    /**
     * Constructs new context.
     *
     * \param runtime Runtime associated with this context.
     */
    explicit context(const ref<class runtime>& runtime);

    /**
     * Returns the runtime associated with this context.
     */
    inline const ref<class runtime>& runtime() const
    {
      return m_runtime;
    }

    /**
     * Returns the currently uncaught error in this context or null reference
     * if this context has no error.
     */
    inline const ref<class error>& error() const
    {
      return m_error;
    }

    /**
     * Sets the current uncaught error to one given as argument.
     *
     * \param error Error instance to set as the currently uncaught error.
     */
    inline void error(const ref<class error>& error)
    {
      m_error = error;
    }

    /**
     * Constructs new error instance with given error code and error message
     * and replaces this execution state's currently uncaught error with it.
     *
     * \param code     Error code
     * \param message  Textual description of the error
     * \param position Optional position in the source code where the error
     *                 occurred
     */
    void error(enum error::code code,
               const unistring& message,
               const struct position* position = nullptr);

    /**
     * Removes currently uncaught error in the context.
     */
    inline void clear_error()
    {
      m_error.release();
    }

    /**
     * Returns the dictionary used by this context to store words.
     */
    inline dictionary_type& dictionary()
    {
      return m_dictionary;
    }

    /**
     * Returns the dictionary used by this context to store words.
     */
    inline const dictionary_type& dictionary() const
    {
      return m_dictionary;
    }

    /**
     * Compiles given source code into a quote.
     *
     * \param source   Source code to compile into quote.
     * \param filename Optional file name information from which the source
     *                 code was read from.
     * \return         Reference the quote that was compiled from given source,
     *                 or null reference if syntax error was encountered.
     */
    ref<quote> compile(const unistring& source,
                       const unistring& filename = U"");

    /**
     * Provides direct access to the data stack.
     */
    inline container_type& data()
    {
      return m_data;
    }

    /**
     * Provides direct access to the data stack.
     */
    inline const container_type& data() const
    {
      return m_data;
    }

    /**
     * Returns true if the data stack is empty.
     */
    inline bool empty() const
    {
      return m_data.empty();
    }

    /**
     * Returns the number of values currently in the data stack.
     */
    inline std::size_t size() const
    {
      return m_data.size();
    }

    /**
     * Removes all values from the data stack.
     */
    inline void clear()
    {
      m_data.clear();
    }

    /**
     * Pushes given value into the data stack.
     */
    inline void push(const ref<class value>& value)
    {
      m_data.push_back(value);
    }

    /**
     * Pushes null value into the data stack.
     */
    void push_null();

    /**
     * Pushes boolean value into the data stack.
     */
    void push_boolean(bool value);

    /**
     * Pushes integer number value into the data stack.
     */
    void push_int(number::int_type value);

    /**
     * Pushes real number value into the data stack.
     */
    void push_real(number::real_type value);

    /**
     * Pushes either integer or real number into stack, based on the given text
     * input which is parsed into a number.
     */
    void push_number(const unistring& value);

    /**
     * Pushes string value into the data stack.
     */
    void push_string(const unistring& value);

    /**
     * Constructs string value from given array of Unicode code points and
     * pushes it onto data stack.
     */
    void push_string(string::const_pointer chars, string::size_type length);

    /**
     * Constructs array from given sequence of values and pushes it into the
     * data stack.
     */
    void push_array(array::const_pointer elements, array::size_type size);

    /**
     * Constructs object from given properties and pushes it into the data
     * stack.
     */
    void push_object(const object::container_type& properties);

    /**
     * Constructs symbol from given identifier and pushes it onto the data
     * stack.
     */
    void push_symbol(const unistring& id);

    /**
     * Constructs quote from given sequence of values and pushes it onto the
     * data stack.
     */
    void push_quote(const std::vector<ref<value>>& values);

    /**
     * Constructs word from given pair of symbol and quote and pushes it onto
     * the data stack.
     */
    void push_word(const ref<class symbol>& symbol,
                   const ref<class quote>& quote);

    /**
     * Pops value from the data stack and discards it. If the stack is empty,
     * range error will be set.
     *
     * \return Boolean flag that tells whether the operation was successfull or
     *         not.
     */
    bool pop();

    /**
     * Pops value of certain type from the data stack and discards it. If the
     * stack is empty, range error will be set. If the top value of the stack
     * is different type than expected, type error will be set.
     *
     * \param type Value type to be expected to be at the top of the stack.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop(enum value::type type);

    /**
     * Pops value from the data stack and places it into given reference slot.
     * If the stack is empty, range error will be set.
     *
     * \param slot Reference where the value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop(ref<value>& slot);

    /**
     * Pops value of certain type from the data stack and places it into given
     * reference slot. If the stack is empty, range error will be set. If the
     * top value of the stack is different type than expected, type error will
     * be set.
     *
     * \param slot Reference where the value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop(ref<value>& slot, enum value::type type);

    /**
     * Pops boolean value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * boolean is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the boolean value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_boolean(bool& slot);

    /**
     * Pops number value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * number is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the number value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_number(ref<number>& slot);

    /**
     * Pops string value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * string is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the string value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_string(ref<string>& slot);

    /**
     * Pops array value from the data stack and places it into given slot. If
     * the stack is empty, range error will be set. If something else than
     * array is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the array value will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_array(ref<array>& slot);

    /**
     * Pops object from the data stack and places it into given slot. If the
     * stack is empty, range error will be set. If something else than object
     * is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the object will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_object(ref<object>& slot);

    /**
     * Pops symbol from the data stack and places it into given slot. If the
     * stack is empty, range error will be set. If something else than symbol
     * is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the symbol will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_symbol(ref<symbol>& slot);

    /**
     * Pops quote from the data stack and places it into given slot. If the
     * stack is empty, range error will be set. If something else than quote
     * is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the quote will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_quote(ref<quote>& slot);

    /**
     * Pops word from the data stack and places it into given slot. If the
     * stack is empty, range error will be set. If something else than word
     * is as top-most value of the stack, type error will be set.
     *
     * \param slot Where the word will be placed into.
     * \return     Boolean flag that tells whether the operation was
     *             successfull or not.
     */
    bool pop_word(ref<word>& slot);

#if PLORTH_ENABLE_MODULES
    /**
     * Returns optional filename of the context, when the context is executed as
     * module.
     */
    inline const unistring& filename() const
    {
      return m_filename;
    }

    /**
     * Sets optional filename of the context, when the context is executed as
     * module.
     */
    inline void filename(const unistring& fn)
    {
      m_filename = fn;
    }
#endif

  private:
    /** Runtime associated with this context. */
    const ref<class runtime> m_runtime;
    /** Currently uncaught error in this context. */
    ref<class error> m_error;
    /** Data stack used for storing values in this context. */
    container_type m_data;
    /** Container for words associated with this context. */
    dictionary_type m_dictionary;
#if PLORTH_ENABLE_MODULES
    /** Optional filename of the context, when executed as module. */
    unistring m_filename;
#endif
  };
}

#endif /* !PLORTH_CONTEXT_HPP_GUARD */
