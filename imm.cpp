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

#define SIZE    1000

/*int DPackets[SIZE];
int SPackets[SIZE];
int APackets[SIZE];*/

int OFFSET[SIZE];
double T_RES[SIZE];
double PLR[SIZE];

struct Event {
	double time;
	int event_id;  
	Event (double t, int e) : time(t), event_id(e) {}
	bool operator< (Event a){
	return time < a.time;
	}
};

struct Packet{
	double time;
	double p_dead;
	Packet (double x) : time(x) {}
	bool operator< (Packet a){
	return time < a.time;
	}
};

double success(double q){
	return ((double) rand() / (RAND_MAX)) > q; //Подбрасывание монетки, где q --- вероятность неудачи
}

int main(int argc, char **argv) {

double T_in = 500;
double T_res = 10; 
double D = 30; 
double q = 0.2;
int M = 1000000;
int offset = 0;
int size = 0;
std::string file;
std::string infolder;
int seed;
std::string str_seed;
std::string Tres_file = "";
double alpha = 1;

if (argc < 100){

std::ifstream input;
input.open("input.txt");

if (input.fail())	// checks to see if file opended 
{ 
	std::cout << "error" << std::endl; 
	return 1;	// no point continuing if the file didn't open...
}

int num = 0;

while(!input.eof())	// reads file to end of *file*, not line
	{ 
		input >> T_in >> D >> q >> offset >> size >> infolder;	// read first column number
		// you can also do it on the same line like this:
		// infile >> exam1[num] >> exam2[num] >> exam3[num]; ++num;
	} 

input.close();

}

std::cout << "yo\t" << T_in << '\t' << D << '\t' << q << '\t' << offset << '\t' << size << '\t' << infolder << std::endl;

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
int nPackets = 0; //all packets
int sPackets = 0; //successful packets

//T_res = (int)T_res;




std::list<Event> events;
std::list<Packet> packets;
std::string out_s;
int j = 0;
int i = 0;
std::vector<int> d0;
std::vector<int> d;                                      // вектор размеров пачек
int iter = 0; //number of different lengths of batches

if (file.empty()){
    assert(false);
}
if (T_in < 0){
    assert(false);
}
if (T_res < 0){
    assert(false);
}
if (q < 0){
    assert(false);
}



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
int number_of_iterations = n_batches + 1;//n_batches + 1;                //количество итераций по циклу
    
//for(int i = 0; i < n_batches; i++)
	//printf("%d\n", d[i]);
    
/*
	std::getline(ii, out_s);
	if(!ii.eof()){
		nPackets += atoi(out_s.c_str());225:26: err
		d[iter++] = atoi(out_s.c_str());
	}
	else {
		break;
	}
}*/

//int n_iter = 0;
//int n_reserve = 0;	
Event income(0 * T_in, 0);  //приход пачки
events.push_front(income);
Event reserve(0 * T_in, 1); //резервирование для передачи
events.push_back(reserve);  
/*while (n_reserve < d[n_iter]){
	Event reserve(n_iter * T_in + offset + n_reserve * T_res, 1);
	events.push(transmit);
	n_reserve++;
}*/

int cnt = 0; //счётчик событий поступления пачек
int cnt_res = 1;    //счётчик событий резервирования
int count = 0;      //индикатор того, что на этом моменте добавились пачки
int buff = 0;

int T_res_iter = 0;
//T_res = T_RES[T_res_iter];
double PLR;
std::cout << number_of_iterations << std::endl;

/*std::cout << "events: ";
	for (std::list<Event>::iterator it = events.begin(); it != events.end(); it++)
		std::cout << it->time << ',' << it->event_id << "\t";
	std::cout << '\n';
	
std::cout << "packets: ";
	for (std::list<Packet>::iterator it = packets.begin(); it != packets.end(); it++)
		std::cout << it->time << "\t";
	std::cout << '\n';*/

file = std::string("disk/outputs/6000/0.0001/") + file + std::string(".may/") + file + std::string(".sim_out.dat");

std::ofstream myfile(file.c_str(), std::ofstream::app);

//std::cout << file << "\n";

while (cnt < number_of_iterations){
	Event tmp = events.front();

	std::cout << std::fixed << "the time is " << tmp.time << '\n';
	std::cout << "events: ";
	for (std::list<Event>::iterator it = events.begin(); it != events.end(); it++)
		std::cout << it->time << ',' << it->event_id << "\t";
	std::cout << '\n';

	events.pop_front();

	std::cout << "packets: ";
	for (std::list<Packet>::iterator it = packets.begin(); it != packets.end(); it++)
		std::cout << it->time << "\t";
	std::cout << '\n';

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
		for(i = 0; i < d[cnt % n_batches]; i++){
			//std::cout << i << std::endl;
			packets.push_back (Packet(tmp.time));}
		cnt++;              //переход к следующей пачке на новой итерации
		Event buf = events.front();
		Event azaz(cnt * T_in, 0);  //прибытие пачки
		if (azaz.time > buf.time)   
			events.push_back(azaz);
		else
			events.push_front(azaz);
		count = 0;
		buff = d[cnt % n_batches];
	}
	else{ //если событие --- приход резервированного времени
		for (std::list<Packet>::iterator it = packets.begin(); it != packets.end() && tmp.time - it->time > D;){	
//		while (((tmp.time - (*it).time) > D) || (it != packets.end())){
			it = packets.erase(it);
			dPackets++;
			//DPackets[cnt_res - 1]++;
		}
		
		//if (count == 0)
		    //APackets[cnt_res] = buff;
		
		if (!packets.empty()){
			if (success(q)){
				packets.pop_front();
				sPackets++;
				//SPackets[cnt_res]++;
			}
		}
		Event fresh(tmp.time + T_res, 1);
		Event buf = events.front();
		if (fresh.time > buf.time)
			events.push_back(fresh);
		else
			events.push_front(fresh);
		
		cnt_res++;
		count++;
	}
	
	//std::cout << cnt  << std::endl;
	std::cout << "dropped: " << dPackets << "\tSuccess:\t" << sPackets << std::endl << std::endl;
}
	

double ans = (double) dPackets / (sPackets + dPackets);

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
//std::cout << "TOTAL  : " << nPackets << "\n";

return 0;

}
