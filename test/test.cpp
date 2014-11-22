#include <iostream>

void f(const int &a)
{
    a = 10;
}

int main()
{
    const int a = 0;
    f(a);
    std::cout << a << std::endl;
    return 0;
}
