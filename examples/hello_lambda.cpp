#include <agency/agency.hpp>
#include <iostream>

int main()
{
  // create 10 sequenced_agents to greet us in bulk
  agency::bulk_invoke(agency::seq(10), [](agency::sequenced_agent& self)
  {
    std::cout << "Hello, world from agent " << self.index() << std::endl;
  });

  return 0;
}

