#include <chrono>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
using namespace std::chrono;
using namespace std;

namespace std
{
    namespace chrono
    {
        struct steady_clock1
        {
            typedef uint64_t rep;
            typedef ratio_multiply<ratio<_XTIME_NSECS_PER_TICK, 1>, nano> period;
            typedef chrono::duration<rep, period> duration;
            typedef chrono::time_point<steady_clock1> time_point;
            static const bool is_steady = true;
            static time_point now()
            {
                static rep t = 0;
                return time_point( duration(t++) );
            }
        };
    }
}
void test_system_clock()
{
    volatile int sink;
    for ( auto size = 1u; size < 1000000000u; size *= 100 )
    {
        // record start time
        auto start = std::chrono::steady_clock1::now();
        // do some work
        std::vector<int> v( size, 42u );
        sink = std::accumulate( v.begin(), v.end(), 0u ); // make sure it's a side effect
        // record end time
        auto end = std::chrono::steady_clock1::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "Time to fill and iterate a vector of "
            << size << " ints : " << diff.count() << " s\n";
    }
}

template <int v1,int v2> struct gcd{ const static int value = gcd< v2, v1%v2 >::value; };
template<int v1>struct gcd<v1,0>{ const static int value = v1; };


void test_chrono()
{
    std::string str = "Hello,World";
    std::transform( str.begin(), str.end(), str.begin(), tolower );
    cout<<gcd<7, 6>::value;
    //test_system_clock();
}