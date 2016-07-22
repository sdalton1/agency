#pragma once

#include <agency/agency.hpp>
#include <functional>
#include <numeric>
#include <limits>

#include "omp_executor.h"


template<int N, class Function, class T, class BinaryFunction>
T parallel_reduce(Function&& f, T init, BinaryFunction&& binary_op)
{
  using intermediate_type = typename std::result_of<Function(int)>::type;

  std::array<intermediate_type, N> partials{};

#if defined(USE_AGENCY_OMP_EXECUTOR)
  omp_executor exec;
#else
  agency::parallel_executor exec;
#endif

  agency::bulk_invoke(agency::par(N).on(exec), [&](agency::parallel_agent& self)
  {
    int i = self.index();

    partials[i] = std::forward<Function>(f)(i);
  });

  return std::accumulate(partials.begin(), partials.end(), init, binary_op);
}


template<int N, class Function>
typename std::result_of<Function(int)>::type
  parallel_reduce(Function&& f)
{
  using result_type = typename std::result_of<Function(int)>::type;
  return parallel_reduce<N>(std::forward<Function>(f), result_type{}, std::plus<result_type>());
}

template<int N, class Function>
typename std::result_of<Function(int)>::type
  parallel_max(Function&& f)
{
  using T = typename std::result_of<Function(int)>::type;

  return parallel_reduce<N>(std::forward<Function>(f), std::numeric_limits<T>::min(), [](T a, T b)
  {
    return std::max(a, b);
  });
}

