// Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once


// XXX nomerge
// XXX eliminate this file once we've eliminated execution_categories.hpp

#include <agency/detail/config.hpp>
#include <agency/execution/executor/properties/bulk_guarantee.hpp>
#include <agency/execution/execution_categories.hpp>


namespace agency
{
namespace detail
{


template<class T>
struct bulk_guarantee_to_execution_category;

template<class T>
using bulk_guarantee_to_execution_category_t = typename bulk_guarantee_to_execution_category<T>::type;


template<>
struct bulk_guarantee_to_execution_category<bulk_guarantee_t::sequenced_t>
{
  using type = sequenced_execution_tag;
};

template<>
struct bulk_guarantee_to_execution_category<bulk_guarantee_t::concurrent_t>
{
  using type = concurrent_execution_tag;
};

template<>
struct bulk_guarantee_to_execution_category<bulk_guarantee_t::parallel_t>
{
  using type = parallel_execution_tag;
};

template<>
struct bulk_guarantee_to_execution_category<bulk_guarantee_t::unsequenced_t>
{
  using type = unsequenced_execution_tag;
};

template<class OuterGuarantee, class InnerGuarantee>
struct bulk_guarantee_to_execution_category<bulk_guarantee_t::scoped_t<OuterGuarantee,InnerGuarantee>>
{
  using type = scoped_execution_tag<
    bulk_guarantee_to_execution_category_t<OuterGuarantee>,
    bulk_guarantee_to_execution_category_t<InnerGuarantee>
  >;
};


} // end detail
} // end agency

