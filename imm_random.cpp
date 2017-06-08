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
    Time current_age;
    Time last_res_time; //will be used to write the time of last res_period in window
    Time last_arr_time; //will be used to write the time of last arr_period in window
    int current_size;
    int batch_index;
    int max_batch_size;
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
        , current_age(-1)
        , current_size(-1)
        , last_res_time(0)
        , last_arr_time(0)
        , batch_index(-1)
        , max_batch_size(-1)
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
        assert (mean_access_time > 0);
        assert (current_age >= 0);
        assert (current_size >=0);
        assert (batch_index >=0);
        assert (max_batch_size > 0);
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
                  << "mean_access_time = " << params.mean_access_time << std::endl
                  << "current_age = " << params.current_age << std::endl
                  << "current_size = " << params.current_size << std::endl
                  << "batch_index = " << params.batch_index << std::endl
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
        else if (name == "mean_access_time")
            input >> params.mean_access_time;
        else if (name == "current_age")
            input >> params.current_age;
        else if (name == "current_size")
            input >> params.current_size;
        else if (name == "batch_index")
            input >> params.batch_index;
        else if (name == "batchfile_path")
            input >> params.batchfile_path;
        else
            assert(false);
    }
    return params;
}

struct Event {
    Time time;
    int event_id;
    Event (Time t = -1, int e = -1)
        : time(t)
        , event_id(e)
    {}
    //ADDED CONDITION IN COMPARATOR
    bool operator< (const Event& a) const
    {
        if (time == a.time)
        {
            return (event_id > a.event_id);
        }
        else
            return time > a.time;
    }
};

struct Packet{
    Time time;
    Packet (Time x) : time(x) {}
    bool operator< (const Packet& a) const
    {
        return time < a.time;
    }
};

double success(double q){
    return ((double) rand() / (RAND_MAX)) > q; //Подбрасывание монетки, где q --- вероятность неудачи
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
    else if (argc == 2)    //specified file
    {
        input.open(argv[1]);
        assert(input.fail());
        params = ReadParameters(input);
    }
    else    //read from line
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
    while(ii >> batch_size) {
        if (batch_size <= max_batch_size){
            batch_stream.push_back(batch_size);
        }
        else
            batch_stream.push_back(max_batch_size);
    }

    int n_batches = batch_stream.size();                           //число пачек

    if ((current_age == 0) && (batch_index == 0) && (current_size == 0)){
        current_time = 0;
        Event income(0 , 0);  //приход пачки
        Event reserve(0 , 1); //начало передачи в зарезервированном интервале
        events.push(income);
        events.push(reserve);
    }
    else{
        int curr_batch_size = current_size;
        int curr_batch_index = batch_index;
        Time curr_batch_age = current_age;
        trace_ended = false;
        while( curr_batch_age >= 0){
            for (int i = 0; i < curr_batch_size; ++i)
                packets.push_back(Packet(-curr_batch_age));

            curr_batch_age -= arr_period;
            curr_batch_index += 1;

            if (curr_batch_index < batch_stream.size() - 1)
                curr_batch_size = batch_stream[curr_batch_index];
            else{
                curr_batch_size = -1;
                trace_ended = true;
            }
            if (trace_ended)
                break;
        }
        if (curr_batch_index >= batch_stream.size() - 1)
            trace_ended = true;

        Event reserve(0, 1);
        events.push(reserve);

        if (!trace_ended){
            Event income(-curr_batch_age, 0);
            events.push(income);
        }
        }
    }

    simtime = window_size;
    Time max_rand_time = 0;
    Time current_time = 0;
    while (true){
        Event tmp = events.top();        //Текущее событие
        if (tmp.time > simtime)            //Событие наступает за пределами окна
            break;
        current_time = tmp.time;        //Текущее время, на выходе из цикла будет записано время последнего события

        //FOR DEBUG
        std::cout << std::fixed << "the time is " << current_time << std::endl;
        std::cout << "events: ";
        std::priority_queue<Event> copy_events(events);
        while(!copy_events.empty()){
            std::cout << copy_events.top().time << ',' << copy_events.top().event_id << "\t";
            copy_events.pop();
        }
        std::cout << '\n';

        events.pop();

        std::cout << "packets: ";
        for (auto it = packets.begin(); it != packets.end(); it++)
            std::cout << it->time << "\t";
        std::cout << '\n';
        //End FOR DEBUG

        //Processing of IMITATION
        if(tmp.event_id == 0){  //If event is INCOME
            if (!trace_ended){
                if (batch_index < batch_stream.size() - 1){
                    Packet packet(tmp.time);
                    for(int i = 0; i < batch_stream[batch_index]; i++){
                        packets.push_back(packet);
                    }
                    batch_index++;              //переход к следующей пачке на новой итерации

                    Event income(current_time + arr_period, 0);  //прибытие пачки
                    events.push(income);
                    last_arr_time = tmp.time;
                }
                else
                    trace_ended = true;
            }
        }
        else if (tmp.event_id == 1){ //If event is RES TRANSMISION
            last_res_time = tmp.time;    //Memorizing of last res event
            //DROPPING OLD PACKETS
            for (auto it = packets.begin(); it != packets.end() && tmp.time - it->time > delay_bound;){
                it = packets.erase(it);
                dPackets++;
            }
            //TRANSMISION IN RESERVED INTERVAL
            if (!packets.empty()){
                if (success(q)){
                    packets.pop_front();
                    sPackets++;
                }
            }
            //SCHEDULING RAND ACCESS IF THERE IS PACKET IN QUEUE WITH AGE current_age > D - T_res
            Event rand_event(current_time + det_tx_duration + rand_tx(generator) , 2); //EVENT OF TX IN RANDOM ACCESS
            Event reserve(current_time + res_period, 1);    //EVENT OF NEW RESERVATION PERIOD
            events.push(reserve);    //ADDING EVENT OF NEW RESERVATION PERIOD
            max_rand_time = reserve.time;            //THRESHOLD OF RANDOM TX END
            //1) IF TX IN RANDOM ACCESS ENDS EARLIER THAN START OF RESERVATION PERIOD, WE WILL DO IT
            //2) THERE IS PACKET WHICH WILL DIE TILL NEXT RESERVATION PERIOD
            if ((rand_event.time + rand_tx_duration <= max_rand_time) && (current_time - packets.front().time + res_period - delay_bound > 0))
                events.push(rand_event);
        }
        else //If event is RAND TRANSMISION
        {
            assert(tmp.event_id == 2);
            //DROPPING OLD PACKETS
            for (auto it = packets.begin(); it != packets.end() && tmp.time - it->time > delay_bound;){
                it = packets.erase(it);
                dPackets++;
            }
            if (!packets.empty()){    //IF QUEUE OF PACKETS ISN'T EMPTY
                Packet random_packet = packets.front();    //THE EXPECTED TRANSMITTED PACKET
                if (last_res_time + res_period - random_packet.time - delay_bound > 0){    //IF IT WILL DIE TILL NEXT RESERVATION PERIOD
                    if (success(q_rand)){
                        packets.pop_front();
                        srandPackets++;
                    }
                    Event rand_event(current_time + rand_tx_duration + rand_tx(generator) , 2); //EVENT OF TX IN RANDOM ACCESS
                    if (rand_event.time + rand_tx_duration < max_rand_time)
                        events.push(rand_event); //IF TX IN RANDOM ACCESS ENDS EARLIER THAN START OF RESERVATION PERIOD, WE WILL DO IT
                }
            }
        }
        //FOR DEBUG
        std::cout << "dropped: " << dPackets << "\tSuccess:\t" << sPackets + srandPackets<< std::endl << std::endl;
    }

    double PLR = static_cast<double>(dPackets) / (srandPackets + sPackets + dPackets);

    if (!packets.empty()){
        age = packets.front().time;
        packets.pop_front();
        current_size = 1;

        while (1){
            if (packets.front().time == age){
                packets.pop_front();
                current_size++;
            }
            else
                break;
        }

        Time young_age = packets.back().time;
        batch_index -= 1 + (young_age - age) / arr_period;
        current_age = last_res_time - current_age + res_period;
    }
    else{
        current_age = last_res_time - last_arr_time - arr_period + res_period;
        batch_index = last_arr_time / arr_period + 1;
        if (batch_index <= n_batches)
            current_size = batch_stream[batch_index];
        else
            current_size = 0;
    }


    std::cout << "res_period = " << params.res_period << std::endl
              << "arr_period = " << params.arr_period << std::endl
              << "delay_bound = " << params.delay_bound << std::endl
              << "det_per = " << params.det_per << std::endl
              << "ran_per = " << params.ran_per << std::endl
              << "window_size = " << params.window_size << std::endl
              << "seed = " << params.seed << std::endl
              << "rand_tx_duration = " << params.rand_tx_duration << std::endl
              << "det_tx_duration = " << params.det_tx_duration << std::endl
              << "mean_access_time = " << params.mean_access_time << std::endl
              << "batchfile_path = " << params.batchfile_path << std::endl
              << "current_age = " << params.current_age << std::endl
              << "current_size = " << params.current_size << std::endl
              << "batch_index = " << params.batch_index
              ;

    return 0;
}

//./a.out res_period 50 arr_period 100 delay_bound 200 det_per 0.2 ran_per 0.5
