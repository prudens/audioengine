#include <valarray>
#include <iostream>

void test_array()
{
    using namespace std;
    valarray<float> arr;
    float audio[100];
    for ( int i = 0; i < 100; i++ )
    {
        audio[i] = i;
    }
    arr = valarray<float>( audio, 100 );
    arr += 1;
    auto newarr = arr.cshift( 10 );
    for (auto v:newarr)
    {
        std::cout << v << " ";
    }
    std::cout << arr.sum() << endl;
    std::cout << arr.min() << endl;
    std::cout << arr.max() << endl;
    arr  = newarr[slice{0,10,2}];
    newarr = arr.apply( [] ( float v ) { return v*v; } );
}