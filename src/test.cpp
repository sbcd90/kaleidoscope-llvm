#include <iostream>

extern "C" {
double average(double, double);
}

int main() {
    std::cout << "average of 7.0 and 11.0: " << average(7.0, 11.0) << std::endl;
}