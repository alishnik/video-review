#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <queue>
#include <list>
#include <cmath>
#include <random>

int main(){    
    typedef double Time;
    Time mean_access_time = 0.0001;
    
    std::default_random_engine generator;
    std::exponential_distribution<Time> rand_tx(1 / mean_access_time);        

    for (int i = 0; i < 1000; i++)
        std::cout << rand_tx(generator) << std::endl;
    return 0;
        }