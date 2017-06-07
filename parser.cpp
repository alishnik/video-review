#include <sstream>
#include <fstream>
#include <iostream>
#include <cassert>

typedef double Time;

struct Parameters
{
    Time res_period;
    Time arr_period;
    Time delay_bound;
    Time window_size;

    double det_per;
    double ran_per;
    int seed;
    Parameters ()
        : res_period(-1)
        , arr_period(-1)
        , delay_bound(-1)
    	, window_size(-1)
        , det_per(-1)
        , ran_per(-1)
    	, seed(-1)
        {}
        
    bool CheckValues()
    {
        assert (arr_period > 0);
        assert ((res_period > 0) && (res_period < arr_period));
        assert (delay_bound > 0);
        assert (window_size >= 0);
        assert ((det_per > 0) && (det_per < 1));
        assert ((ran_per > 0) && (ran_per < 1));
        assert(seed > 0);
        return true;
    }
};

std::ostream& operator<< (std::ostream& output, const Parameters& params) 
{
    output << "res_period = " << params.res_period << std::endl
                  << "arr_period = " << params.arr_period << std::endl
                  << "delay_bound = " << params.delay_bound << std::endl
                  << "det_per = " << params.det_per << std::endl
                  << "ran_per = " << params.ran_per << std::endl
                  << "window_size = " << params.window_size << std::endl
                  << "seed = " << params.seed;
    return output;
}

Parameters ReadParameters(std::istream& input)
{
    std::string name;
    Parameters params;
    while (!input.eof()) 
    {
        input >> name;
        if (name == "res_period") 
            input >> params.res_period;
        else if (name == "arr_period")
            input >> params.arr_period;
        else if (name == "delay_bound")
            input >> params.delay_bound;
        else if (name == "det_per")
            input >> params.det_per;
        else if (name == "ran_per")
            input >> params.ran_per;
        else if (name == "window_size")
			input >> params.window_size;
        else if (name == "seed")
			input >> params.seed;
        else
            assert(false);
    }
    return params;
}

int main(int argc, char** argv)
{
    // Откуда считывать?
    std::ifstream input;
    Parameters params;
    
    if (argc == 1) 
    {
        input.open("input.txt");
        assert(input.fail());
        params = ReadParameters(input);
    }
    else if (argc == 2)
    {
        input.open(argv[1]);
        assert(input.fail());
        params = ReadParameters(input);
    }
    else
    {
        std::stringstream ss;
        for (int i = 1; i < argc; ++i)
        {
            ss << argv[i];
            if (i != argc - 1) 
            {
                ss << " ";
            }
        }
        params = ReadParameters(ss);        
    }
    params.CheckValues(); // Проверка корректности


    std::cout << params << std::endl;
    return 0;
}

//./a.out res_period 50 arr_period 100 delay_bound 200 det_per 0.2 ran_per 0.5
