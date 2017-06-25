#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <queue>
#include <list>
#include <cassert>
#include <cmath>
#include <random>

#define SIZE  1000

/*int DPackets[SIZE];
int SPackets[SIZE];
int APackets[SIZE];*/

typedef double Time;

int OFFSET[SIZE];
double T_RES[SIZE];
double PLR[SIZE];

/* Standard C Function: Greatest Common Divisor */
int gcd ( int a, int b )
{
    int c;
    while ( a != 0 ) {
        c = a; a = b%a;  b = c;
    }
    return b;
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

int main(int argc, char **argv) {

	double T_in = 20;
	double T_res = 10;
	double D = 30;
	double q = 0.2;
	double q_rand = 0.5;
	int M = 1000000;
	int offset = 0;
	int size = 0;
	int window_size = 0;
	std::string file;
	std::string infolder;
	int seed;
	std::string str_seed;
	std::string Tres_file = "";
	std::string State_file = "";
	double alpha = 1;
	Time rand_tx_duration = 15; // duration of rand_tx
	Time det_tx_duration = 10;
	double mean_access_time = 1000000;

	if (argc < 100)
	{
		std::ifstream input;
		input.open("input.txt");

		if (input.fail())	// checks to see if file opended
		{
			std::cout << "error" << std::endl;
			return 1;	// no point continuing if the file didn't open...
		}

		while(!input.eof())	// reads file to end of *file*, not line
		{
			input >> T_in >> D >> q >> offset >> size >> infolder;	// read first column number
			// you can also do it on the same line like this:
			// infile >> exam1[num] >> exam2[num] >> exam3[num]; ++num;
		}

		input.close();
	}

	//std::cout << "yo\t" << T_in << '\t' << D << '\t' << q << '\t' << offset << '\t' << size << '\t' << infolder << std::endl;

for (int i = 1; i < argc; i+=2) { /* We will iterate over argv[] to get the parameters stored inside.
                                  * Note that we're starting on 1 because we don't need to know the 
                                  * path of the program, which is stored in argv[0] */
    if ((i + 1) != argc){ // Check that we haven't finished parsing already
        if (std::string(argv[i]) == "--seed") {
            seed = atoi(argv[i + 1]); // value for pseudorandom generator
            str_seed = std::string(argv[i + 1]);}
        else if (std::string(argv[i]) == "--res_period") {
            T_res = atof(argv[i + 1]); // interval between reservations
        }
        else if (std::string(argv[i]) == "--file") {
            file = std::string(argv[i + 1]); // batch
        }
        else if (std::string(argv[i]) == "--MAX") {
            M = atof(argv[i + 1]); // interval between reservations
        }
        else if (std::string(argv[i]) == "--Tres_file") {
            Tres_file = std::string(argv[i + 1]); // adress of file with T_res
        }
        else if (std::string(argv[i]) == "--State_file") {
                    State_file = std::string(argv[i + 1]); // adress of file with T_res
                }
        else if (std::string(argv[i]) == "--window_size") {
                    window_size = atoi(argv[i + 1]); // adress of file with T_res
                }
        /*if (std::string(argv[i]) == "--arr_period") {// We know the next argument *should* be the filename:
            T_in = atof(argv[i + 1]); // interval between incomings of packs
        } else if (std::string(argv[i]) == "--res_period") {
            T_res = atof(argv[i + 1]); // interval between reservations
        } else if (std::string(argv[i]) == "--MAX") {
            M = atof(argv[i + 1]); // interval between reservations
        } else if (std::string(argv[i]) == "--delay_bound") {
            D = atof(argv[i + 1]); // packet deadline	
		} else if (std::string(argv[i]) == "--alpha") {
            alpha = atof(argv[i + 1]); // packet deadline	
		}else if (std::string(argv[i]) == "--per") {
            q = atof(argv[i + 1]); // the probability of failure
		} else if (std::string(argv[i]) == "--file") {
            file = std::string(argv[i + 1]); // batch
        } else if (std::string(argv[i]) == "--infolder") {
            infolder = std::string(argv[i + 1]); // folder
        } else if (std::string(argv[i]) == "--offset") {
            offset = atoi(argv[i + 1]); // offset of window
        } else if (std::string(argv[i]) == "--size") {
            size = atoi(argv[i + 1]); // size of window
        } else if (std::string(argv[i]) == "--seed") {
            seed = atoi(argv[i + 1]); // value for pseudorandom generator
            str_seed = std::string(argv[i + 1]);
        } else if (std::string(argv[i]) == "--Tres_file") {
            Tres_file = std::string(argv[i + 1]); // adress of file with T_res
        }*/
	}
	else {
        std::cout << "Not enough or invalid arguments, please try again.\n";
        //Sleep(2000); 
        exit(0);
	}
}

int num = 0;

Time h = 0;
int current_size = 0;
int number_of_batch = 0;
Time last_res_time = 0; //will be used to write the time of last res_period in window
Time last_arr_time = 0; //will be used to write the time of last arr_period in window

std::ifstream input;
if (State_file != ""){
	input.open(State_file.c_str());
if(input.fail())	// checks to see if file opended
    {
		std::cout << "error" << std::endl;
		return 1;	// no point continuing if the file didn't open...
    }
	while(!input.eof())	// reads file to end of *file*, not line
	{
		input >>
		h >>
		current_size >>
		number_of_batch >>
		last_res_time >>
		last_arr_time;
	}
	input.close();
}

if (Tres_file != ""){
	std::ifstream fin;
	fin.open(Tres_file.c_str());
	
	if(fin.fail())	// checks to see if file opended 
    { 
		std::cout << "error" << std::endl; 
		return 1;	// no point continuing if the file didn't open...
    } 
	while(!fin.eof())	// reads file to end of *file*, not line
	{ 
		fin >> OFFSET[num];	// read first column number
		fin >> T_RES[num];	// read second column number
		T_RES[num] *= alpha;
		fin >> PLR[num];	// read third column number

		num++;	// go to the next number
		// you can also do it on the same line like this:
		// infile >> exam1[num] >> exam2[num] >> exam3[num]; ++num;
	} 
	fin.close();
}

srand(seed);
int dPackets = 0; //dropped packets
//int nPackets = 0; //all packets
int sPackets = 0; //successful packets
int srandPackets = 0; //successful packets in random access

//T_res = (int)T_res;


std::default_random_engine generator;
std::exponential_distribution<Time> rand_tx(1 / mean_access_time);


std::priority_queue<Event> events;
std::list<Packet> packets;
std::string out_s;

int i = 0;
std::vector<int> d0;
std::vector<int> d;                                      // вектор размеров пачек


assert(!file.empty());
assert(T_in >= 0);
assert(T_res >= 0);
assert(q >= 0);



std::string file1 = infolder + file + std::string(".batch.dat");

std::cout << infolder << '\t' << file1 << '\n';

std::ifstream ii(file1.c_str());
int batch_size;
while(ii >> batch_size) {
    if (batch_size <= M){
        d0.push_back(batch_size);
        //std::cout << batch_size << '\n';
        }
    else{
        d0.push_back(M);
        //std::cout << M << '\n';
        }
}

//std::cout << M << '\n';

if (size != 0){
    for(int i = offset; i < offset + size; i++)
        d.push_back(d0[i]);
}
else
    d = d0;        
    
//printf("%d, %d\n", offset, size);



int n_batches = d.size();                           //число пачек
//slot added
/*int tau = gcd(Tin, Tres);
int tin = Tin / tau;
int tres = Tres / tau;*/
    
//for(int i = 0; i < n_batches; i++)
	//printf("%d\n", d[i]);

int cnt = 0; //счётчик событий поступления пачек
int cnt_res = 1;    //счётчик событий резервирования
int count = 0;      //индикатор того, что на этом моменте добавились пачки


int T_res_iter = 0;
//T_res = T_RES[T_res_iter];
double PLR;
//std::cout << number_of_iterations << std::endl;

file = std::string("disk/outputs/6000/0.0001/") + file + std::string(".may/") + file + std::string(".sim_out.dat");

std::ofstream myfile(file.c_str(), std::ofstream::app);

//std::cout << file << "\n";

//ADDED parameters for random access
Time max_rand_time = 0;
Time current_time;

if ((State_file == "") || ((h == 0) && (number_of_batch == 0) && (current_size == 0))){
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

Time simtime = d.size() * T_in;
if (window_size != 0)
	simtime = window_size + current_time;

Time CURRENT_TIME = 0;	//will be used to determine time when sim will stop
while (true){
	Event tmp = events.top();
	CURRENT_TIME = current_time;
	current_time = tmp.time;
	if (current_time > simtime)
		break;

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
	for (std::list<Packet>::iterator it = packets.begin(); it != packets.end(); it++)
		std::cout << it->time << "\t";
	std::cout << '\n';

    if(tmp.time == current_time){    //если настало время одного из событий
    	//std::cout << "id " << tmp.event_id << std::endl;
	    if(tmp.event_id == 0){  //если событие --- поступление пачек
		    if (cnt > 0 and Tres_file != ""){
				if ((cnt % size) == 0){
					offset = cnt - size;
					PLR = (double) dPackets / (sPackets + dPackets);

					myfile << offset << '\t' <<  std::scientific << PLR << '\t' << std::fixed << T_res << '\t' << seed << std::endl;

					T_res_iter++;
					if (T_res_iter < num - 1)
						T_res = T_RES[T_res_iter];
					sPackets = 0;
					dPackets = 0;
				}
		    }
	    	Packet packet(tmp.time);
		    for(i = 0; i < d[cnt % n_batches]; i++){
		    	packets.push_back(packet);
		    }

		    cnt++;              //переход к следующей пачке на новой итерации

		    Event azaz(cnt * T_in, 0);  //прибытие пачки

		    events.push(azaz);

		    last_arr_time = tmp.time;

		    /*if (azaz.time > buf.time)
			    events.push(azaz);
		    else
			    events.push_front(azaz);
		    count = 0;*/
	    }
	    else if (tmp.event_id == 1){ //если событие --- приход резервированного времени
	    	//DROPPING OLD PACKETS
	    	for (std::list<Packet>::iterator it = packets.begin(); it != packets.end() && tmp.time - it->time > D;){
	    	//		while (((tmp.time - (*it).time) > D) || (it != packets.end())){
	    				std::cout << "bled ";
	    				it = packets.erase(it);
	    				dPackets++;
	    				//DPackets[cnt_res - 1]++;
	    			}
	    	//TRANSMISION IN DET INTERVAL
		    if (!packets.empty()){
			    if (success(q)){
				    packets.pop_front();
				    sPackets++;
				    //SPackets[cnt_res]++;
			    }
		    }
		    //SCHEDULING RAND ACCESS IF THERE IS PACKET IN QUEUE WITH AGE h > D - T_res
		    Event rand_event(current_time + det_tx_duration + rand_tx(generator) , 2); //EVENT OF TX IN RANDOM ACCESS
		    Event fresh(tmp.time + T_res, 1);	//EVENT OF NEW RESERVATION PERIOD
		    max_rand_time = fresh.time;			//THRESHOLD OF RANDOM TX END
		    //1) IF TX IN RANDOM ACCESS ENDS EARLIER THAN START OF RESERVATION PERIOD, WE WILL DO IT
			//2) THERE IS PACKET WHICH WILL DIE TILL NEXT RESERVATION PERIOD
		    if ((rand_event.time + rand_tx_duration < max_rand_time) && (current_time - packets.front().time + T_res - D > 0))
				events.push(rand_event);
		    
		    events.push(fresh);	//ADDING EVENT OF NEW RESERVATION PERIOD
		    
		    /*Event buf = events.front();
		    if (fresh.time > buf.time)
			    events.push_back(fresh);
		    else
			    events.push_front(fresh);*/
		
		    cnt_res++;
		    count++;
		    last_res_time = tmp.time;
	    }
	    else
	    {
	    	assert(tmp.event_id == 2);
	    	//если событие --- начало передачи в случайном доступе
	        if (!packets.empty()){	//IF QUEUE OF PACKETS ISN'T EMPTY
	            Packet random_packet = packets.front();	//THE EXPECTED TRANSMITTED PACKET
	        	if (current_time - random_packet.time + T_res - D > 0){	//IF IT WILL DIE TILL NEXT RESERVATION PERIOD CORRECT!
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
	}
	//std::cout << cnt  << std::endl;
    std::cout << "dropped: " << dPackets << "\tSuccess:\t" << sPackets << std::endl << std::endl;
}
	
double ans = static_cast<double>(dPackets) / (srandPackets + sPackets + dPackets);

Time age = 0;
current_size = 1;
number_of_batch = 0;

if (!packets.empty()){
	age = packets.front().time;
	packets.pop_front();

	while (1){
		if (packets.front().time == age){
			packets.pop_front();
			current_size++;
		}
		else
			break;
	}

	number_of_batch = age / T_in;
	age = last_res_time - age;
}
else{
	age = last_res_time - last_arr_time - T_in;
	number_of_batch = last_arr_time / T_in + 1;
	current_size = d[number_of_batch];
}

std::string output_file = "state.txt";
std::ofstream output(output_file.c_str(), std::ofstream::out);
if ((output.is_open()) && (State_file != "")){
	output <<
			age << std::endl <<
			current_size << std::endl <<
			number_of_batch << std::endl <<
			last_res_time << std::endl <<
			last_arr_time;
	output.close();
}

//T_res = (double)T_res / scaling_factor;

//std::cout << T_res << '\t' << std::scientific << ans << '\t' << std::fixed << seed << "\n";

/*if ((myfile.is_open()) & (Tres_file == ""))
{
    //for (i = 0; i < cnt_res; i++)
    myfile << T_res << '\t' << std::scientific << ans << '\t' << std::fixed << seed << "\n";

}
else
    std::cout << "Unable to open file";
    
myfile.close();*/

std::cout << "T_in = " << T_in << "\n";
std::cout << "T_res = " << T_res << "\n";
std::cout << "D = " << D << "\n";
std::cout << "q = " << q << "\n";

std::cout << "PLR =	" << ans << "\n";
std::cout << "dropped: " << dPackets << "\n";
std::cout << "success: " << sPackets << "\n";
std::cout << "success in random: " << srandPackets << "\n";
//std::cout << "TOTAL  : " << nPackets << "\n";

return 0;

}


