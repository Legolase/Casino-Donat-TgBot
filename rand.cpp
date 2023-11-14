#include "rand.h"
#include <random>

int random(int begin, int end)
{
    static std::mt19937 gen { std::random_device{}() };
    //static std::mt19937 gen{4};
    static std::uniform_int_distribution<int> dist(0, std::numeric_limits<int>::max());
    return (dist(gen) % (end - begin + 1)) + begin;
}
