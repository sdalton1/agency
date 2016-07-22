#pragma once

#include <agency/execution_categories.hpp>
#include <utility>
#include <type_traits>

class omp_executor
{
  public:
    using execution_category = agency::parallel_execution_tag;

    using index_type = int;

    using size_type = int;

    template<class Function>
    void execute(Function f, int n)
    {
      #pragma omp parallel for
      for(int i = 0; i < n; ++i)
      {
        f(i);
      }
    }
};

