/*
 * Copyright (c) 2016, Vladimir Strisovsky All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "Test.h"

namespace cppqc
{

template<typename Function>
struct CompactCheckFunction
{
    using type = Function;
    using FunctionT = Function;
    static const bool valid = true;

    template<typename ReturnType, typename ... T>
    static ReturnType apply(const Function& fnc, ReturnType, const T& ... v)
    {
        return fnc(v ...);
    }
};

template<>
struct CompactCheckFunction<void>
{
    using type = void;
    using FunctionT = void*;
    static const bool valid = false;

    template<typename Function, typename ReturnType, typename ... T>
    static ReturnType apply(const Function& fnc, ReturnType r, const T& ... v)
    {
        return r;
    }
};

template<typename CheckFunction, typename TrivialFunction, typename ClassifyFunction, typename ... T>
class CompactCheck : public Property<T ...>
{
    std::string m_name;

    template<typename _CheckFunction, typename _TrivialFunction, typename _ClassifyFunction, typename ... _T>
    friend class CompactCheck;

    using CheckFunctionT = CompactCheckFunction<CheckFunction>;
    typename CheckFunctionT::FunctionT m_checkFnc;

    using TrivialFunctionT = CompactCheckFunction<TrivialFunction>;
    typename TrivialFunctionT::FunctionT m_trivialFnc;

    using ClassifyFunctionT = CompactCheckFunction<ClassifyFunction>;
    typename ClassifyFunctionT::FunctionT m_classifyFnc;

    CompactCheck( std::string name,
                  typename CheckFunctionT::FunctionT&& checkFunction,
                  typename TrivialFunctionT::FunctionT&& trivialFunctions,
                  typename ClassifyFunctionT::FunctionT&& classifyFunction)
    : m_name(name)
    , m_checkFnc(checkFunction)
    , m_trivialFnc(trivialFunctions)
    , m_classifyFnc(classifyFunction)
    {}

      bool check(const T& ... v) const override
      {
          return CheckFunctionT::apply(m_checkFnc, true, v ...);
      }

      bool trivial(const T& ... v) const override
      {
        return TrivialFunctionT::apply(m_trivialFnc, false, v ...);
      }

      std::string classify(const T& ... v) const override
      {
        return ClassifyFunctionT::apply(m_classifyFnc, std::string(), v ...);
      }

      std::string name() const
      {
          return m_name.empty() ? "no-name" : m_name;
      }

public:
    CompactCheck()
    {}

    template<typename _CheckFunction>
    CompactCheck<_CheckFunction, typename TrivialFunctionT::type, typename ClassifyFunctionT::type, T ...> property(const std::string& name, _CheckFunction&& checkFnc)
    {
        static_assert(CheckFunctionT::valid == false, "Check function is already set");
        return CompactCheck<_CheckFunction, typename TrivialFunctionT::type, typename ClassifyFunctionT::type, T ...>(name, std::move(checkFnc), std::move(m_trivialFnc), std::move(m_classifyFnc));
    }

    template<typename _TrivialFunction>
    CompactCheck<typename CheckFunctionT::type, _TrivialFunction, typename ClassifyFunctionT::type, T ...> trivial(_TrivialFunction&& trivialFnc)
    {
        static_assert(TrivialFunctionT::valid == false, "Trivial function is already set");
        return CompactCheck<typename CheckFunctionT::type, _TrivialFunction, typename ClassifyFunctionT::type, T ...>(std::move(m_name), std::move(m_checkFnc), std::move(trivialFnc), std::move(m_classifyFnc));
    }

    template<typename _ClassifyFunction>
    CompactCheck<typename CheckFunctionT::type, typename TrivialFunctionT::type, _ClassifyFunction, T ...> classify(_ClassifyFunction&& classifyFnc)
    {
        static_assert(ClassifyFunctionT::valid == false, "Classsify function is already set");
        return CompactCheck<typename CheckFunctionT::type, typename TrivialFunctionT::type, _ClassifyFunction, T ...>(std::move(m_name), std::move(m_checkFnc), std::move(m_trivialFnc), std::move(classifyFnc));
    }

    Result test( std::size_t maxSuccess = 100, std::size_t maxDiscarded = 0, std::size_t maxSize = 0)
    {
        return quickCheck(*this, maxSuccess, maxDiscarded, maxSize);
    }

    Result testWithOutput( std::ostream &out = std::cout,  std::size_t maxSuccess = 100, std::size_t maxDiscarded = 0, std::size_t maxSize = 0)
    {
        return quickCheckOutput(*this, out, maxSuccess, maxDiscarded, maxSize);
    }
};

template<typename ... T>
CompactCheck<void, void, void, T ...> gen()
{
    return CompactCheck<void, void, void, T ...>();
}



}
