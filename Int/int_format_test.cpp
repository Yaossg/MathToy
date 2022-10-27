#include "int_format.cpp"

#include <iostream>

int main()
{
    using namespace yao_math::int_format;
    using namespace std;
    cout<<meta_int_format<false, char>(18446744073709551615ull, {})<<endl;

}

