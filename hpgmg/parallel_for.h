#pragma once

#include <agency/execution_policy.hpp>
#include "omp_executor.h"

template<class Size, class Function>
auto parallel_for(Size n, Function&& f)
{
  //return agency::bulk_invoke(agency::par(n), [&](agency::parallel_agent& self)
  
  omp_executor omp_exec;

  return agency::bulk_invoke(agency::par(n).on(omp_exec), [&](agency::parallel_agent& self)
  {
    Size i = self.index();

    std::forward<Function>(f)(i);
  });
}

