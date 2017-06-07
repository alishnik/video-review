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
    Time rand_tx_duration;
	Time det_tx_duration;
	Time mean_access_time;

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
    	, rand_tx_duration(-1)
    	, det_tx_duration(-1)
    	, mean_access_time(-1)
        {}

    bool CheckValues()
    {
        assert (arr_period > 0);
        assert ((res_period > 0) && (res_period < arr_period));
        assert (delay_bound > 0);
        assert (window_size >= 0);
        assert ((det_per > 0) && (det_per < 1));
        assert ((ran_per > 0) && (ran_per < 1));
        assert (seed > 0);
        assert (rand_tx_duration > 0);
        assert (det_tx_duration > 0);
        assert (rand_tx_duration + det_tx_duration <= res_period);
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

    if (argc == 1) //default file
    {
        input.open("input.txt");
        assert(input.fail());
        params = ReadParameters(input);
    }
    else if (argc == 2)	//specified file
    {
        input.open(argv[1]);
        assert(input.fail());
        params = ReadParameters(input);
    }
    else	//read from line
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

    std::ifstream state;
    state.open("state.txt");
    if(input.fail()){	// checks to see if file opended
		std::cout << "error" << std::endl;
		return 1;	// no point continuing if the file didn't open...
	}

    Time last_res_time = 0; //will be used to write the time of last res_period in window
    Time last_arr_time = 0; //will be used to write the time of last arr_period in window
    Time h = 0;
    int current_size = 0;
    int number_of_batch = 0;

    while(!state.eof())	// reads file to end of *file*, not line
    	{
    		input >>
    		h >>
    		current_size >>
    		number_of_batch >>
    		last_res_time >>
    		last_arr_time;
    	}
	state.close();



    std::cout << params << std::endl;
    return 0;
}

//./a.out res_period 50 arr_period 100 delay_bound 200 det_per 0.2 ran_per 0.5
