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
	Time h;
	Time last_res_time; //will be used to write the time of last res_period in window
	Time last_arr_time; //will be used to write the time of last arr_period in window
	int current_size;
	int number_of_batch;
	std::string batchfile_path;

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
    	, h(-1)
    	, last_res_time(-1)
    	, last_arr_time(-1)
    	, current_size(-1)
    	, number_of_batch(-1)
    	, batchfile_path("")
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
        assert (h >= 0);
        assert (last_res_time >= 0);
        assert (last_arr_time >= 0);
        assert (current_size >=0);
        assert (number_of_batch >=0);
        assert (batchfile_path != "");
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
                  << "seed = " << params.seed << std::endl
                  << "rand_tx_duration = " << params.rand_tx_duration << std::endl
                  << "det_tx_duration = " << params.det_tx_duration << std::endl
                  << "h = " << params.h << std::endl
                  << "last_res_time = " << params.last_res_time << std::endl
                  << "last_arr_time = " << params.last_arr_time << std::endl
                  << "current_size = " << params.current_size << std::endl
                  << "number_of_batch = " << params.number_of_batch << std::endl
                  << "batchfile_path = " << params.batchfile_path
                  ;
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
        else if (name == "rand_tx_duration")
			input >> params.rand_tx_duration;
        else if (name == "det_tx_duration")
			input >> params.det_tx_duration;
        else if (name == "h")
			input >> params.h;
        else if (name == "last_res_time")
			input >> params.last_res_time;
        else if (name == "last_arr_time")
			input >> params.last_arr_time;
        else if (name == "current_size")
			input >> params.current_size;
        else if (name == "number_of_batch")
			input >> params.number_of_batch;
        else if (name == "batchfile_path")
			input >> params.batchfile_path;
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

	int dPackets = 0; //dropped packets
	int sPackets = 0; //successful packets
	int srandPackets = 0; //successful packets in random access

	srand(seed);
	std::default_random_engine generator;
	std::exponential_distribution<Time> rand_tx(1 / mean_access_time);

	std::priority_queue<Event> events;
	std::list<Packet> packets;
	std::vector<int> batch_stream;                                      // вектор размеров пачек

	std::ifstream ii(bacthfile_path.c_str());
	int batch_size = 0;
	int MAX_BATCH_SIZE = 100000000;
	while(ii >> batch_size) {
	    if (batch_size <= MAX_BATCH_SIZE){
	    	batch_stream.push_back(batch_size);
	        //std::cout << batch_size << '\n';
	        }
	    else{
	    	batch_stream.push_back(MAX_BATCH_SIZE);
	        //std::cout << M << '\n';
	        }
	}

	int n_batches = d.size();                           //число пачек

	int cnt = 0; 		//счётчик событий поступления пачек
	int cnt_res = 1;    //счётчик событий резервирования
	int count = 0;      //индикатор того, что на этом моменте добавились пачки

	std::ofstream myfile(file.c_str(), std::ofstream::app);

	Time max_rand_time = 0;
	Time current_time = 0;

	if ((h == 0) && (number_of_batch == 0) && (current_size == 0)){
		current_time = 0;
		Event income(0 , 0);  //приход пачки
		Event reserve(0 , 1); //начало передачи в зарезервированном интервале
		events.push(income);
		events.push(reserve);
	}
	else{
		if (h >= 0){
			current_time = h + number_of_batch * T_in;
			Packet packet(current_time - h);
			for (i = 0; i < current_size; i++)
				packets.push_back(packet);
			Time h1 = h - T_in;
			Time h0 = T_in;
			int number = number_of_batch;
			while (h0 <= h1){
				number++;
				Packet packet(current_time - h1);
				for (i = 0; i < d[number]; i++){
					std::cout << i << '\t' << number << '\t';
					packets.push_back(packet);
				}
				h0 += T_in;
				std::cout << std::endl;
			}
			cnt = floor(current_time / T_in);
			Event income(last_arr_time + T_in, 0);
			Event reserve(last_res_time + T_res, 1);
			events.push(income);
			events.push(reserve);
		}
		else{
			cnt = last_arr_time / T_in;
			current_time = h + number_of_batch * T_in;
			Event income(last_arr_time + T_in, 0);
			Event reserve(current_time + T_res, 1);
			events.push(income);
			events.push(reserve);
		}
	}

	simtime = window_size + current_time;

	Time CURRENT_TIME = 0;	//Memorize the last done event




	double PLR;











    return 0;
}

//./a.out res_period 50 arr_period 100 delay_bound 200 det_per 0.2 ran_per 0.5
