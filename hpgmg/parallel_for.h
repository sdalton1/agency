#pragma once

#include <agency/agency.hpp>
#include "omp_executor.h"

template<class Size, class Function>
auto parallel_for(Size n, Function&& f)
{
#if defined(USE_AGENCY_OMP_EXECUTOR)
  omp_executor exec;
#else
  agency::parallel_executor exec;
#endif

  return agency::bulk_invoke(agency::par(n).on(exec), [&](agency::parallel_agent& self)
  {
    Size i = self.index();

    std::forward<Function>(f)(i);
  });
}

