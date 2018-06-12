/*
 * Copyright (c) 2017-2018, Rauli Laine
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
#ifndef PLORTH_VALUE_HPP_GUARD
#define PLORTH_VALUE_HPP_GUARD

#include <plorth/memory.hpp>
#include <plorth/unicode.hpp>

namespace plorth
{
  class context;
  class object;

  /**
   * Abstract base class for everything that acts as an value in Plorth.
   */
  class value : public memory::managed
  {
  public:
    /**
     * Enumeration of different supported value types.
     */
    enum type
    {
      /** Value for null. */
      type_null,
      /** Boolean values. */
      type_boolean,
      /** Number (floating point) values. */
      type_number,
      /** String (Unicode) values. */
      type_string,
      /** Array values. */
      type_array,
      /** Other type of objects. */
      type_object,
      /** Symbols. */
      type_symbol,
      /** Quotes. */
      type_quote,
      /** Words. */
      type_word,
      /** Errors. */
      type_error
    };

    /**
     * Returns type of given value.
     */
    static inline enum type type(const std::shared_ptr<value>& value)
    {
      return value ? value->type() : type_null;
    }

    /**
     * Returns type of the value.
     */
    virtual enum type type() const = 0;

    /**
     * Returns textual description of type of the value.
     */
    unistring type_description() const;

    /**
     * Returns textual description of given value type.
     */
    static unistring type_description(enum type type);

    /**
     * Tests whether reference of a value is of given type.
     */
    static inline bool is(const std::shared_ptr<value>& value, enum type type)
    {
      return value::type(value) == type;
    }

    /**
     * Determines prototype object of given value, based on it's type. If the
     * value is an object, property called "__proto__" will be used instead,
     * with the runtime's object prototype acting as a fallback.
     *
     * \param runtime Script runtime to use for prototype retrieval.
     * \param value   Reference to value of which prototype to determine.
     * \return        Reference to the prototype object of given value.
     */
    static std::shared_ptr<object> prototype_of(
      const std::shared_ptr<runtime>& runtime,
      const std::shared_ptr<value>& value
    );

    /**
     * Tests whether two values are equal.
     *
     * \param that Other value to test this one against.
     */
    virtual bool equals(const std::shared_ptr<value>& that) const = 0;

    /**
     * Executes value as part of compiled quote. Default implementation
     * evaluates the value and pushes result into the context.
     *
     * \param ctx Execution context to execute the value in.
     * \param val Value to execute.
     * \return    Boolean flag telling whether the execution was successfull or
     *            whether an error was encountered.
     */
    static bool exec(const std::shared_ptr<context>& ctx,
                     const std::shared_ptr<value>& val);

    /**
     * Evaluates value as element of an array or value of object's property.
     * Default implementation just returns the value itself.
     *
     * \param ctx  Execution context to evaluate the value in.
     * \param val  Value to evaluate.
     * \param slot Where result of the evaluation will be placed into.
     * \return     Boolean flag telling whether the execution was successful or
     *             whether an error was encountered.
     */
    static bool eval(const std::shared_ptr<context>& ctx,
                     const std::shared_ptr<value>& val,
                     std::shared_ptr<value>& slot);

    /**
     * Constructs string representation of the value.
     */
    virtual unistring to_string() const = 0;

    /**
     * Constructs a string that resembles as accurately as possible what this
     * value would look like in source code.
     */
    virtual unistring to_source() const = 0;
  };

  bool operator==(const std::shared_ptr<value>&, const std::shared_ptr<value>&);
  bool operator!=(const std::shared_ptr<value>&, const std::shared_ptr<value>&);

  std::ostream& operator<<(std::ostream&, enum value::type);
  std::ostream& operator<<(std::ostream&, const value*);
}

#endif /* !PLORTH_VALUE_HPP_GUARD */
