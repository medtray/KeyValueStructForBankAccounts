// CSE 375/475 Assignment #1
// Fall 2019
//
// Description: This file implements a function 'run_custom_tests' that should be able to use
// the configuration information to drive tests that evaluate the correctness
// and performance of the map_t object.

#include <iostream>
#include <ctime>
#include "config_t.h"
#include "tests.h"
#include <thread>
#include "simplemap.h"
#include <chrono>
using namespace std::chrono;

	    void printer(int k, float v) {
			std::cout<<"<"<<k<<","<<v<<">"<< std::endl;
	}



	void run_custom_tests(config_t& cfg) {
		// Step 1
		// Define a simplemap_t of types <int,float>
		// this map represents a collection of bank accounts:
		// each account has a unique ID of type int;
		// each account has an amount of fund of type float.

		//simplemap_t<int,float> map;
		int max_key=cfg.key_max;

		float money_by_person=(float)100000/(float)max_key;

		//const size_t tableSize = 100;
		simplemap<int, float> map(max_key);

		float prob_deposit=0.8;

		int total=100000;

	

		// Step 2
		// Populate the entire map with the 'insert' function
		// Initialize the map in a way the sum of the amounts of
		// all the accounts in the map is 100000

		for (int i=0;i<max_key;i++)
		{
			map.insert(i,money_by_person);
		}

		/*
		map.insert(10,1000.0);
		map.insert(2,6);
		map.insert(3,5);
		map.insert(4,9);
		map.insert(5,60);
		map.insert(10,70);

		map.update(10,70);
		map.remove(4);

		std::pair<float, bool> read1;
		std::pair<float, bool> read2;

		read1=map.lookup(2);
		read2=map.lookup(200);
		*/

		// Step 3
		// Define a function "deposit" that selects two random bank accounts
		// and an amount. This amount is subtracted from the amount
		// of the first account and summed to the amount of the second
		// account. In practice, give two accounts B1 and B2, and a value V,
		// the function performs B1-=V and B2+=V.
		// The execution of the whole function should happen atomically:
		// no operation should happen on B1 and B2 (or on the whole map?)
		// while the function executes.

		map.deposit(1,2,10);
		map.deposit(2,1,10);

		std::pair<float, bool> read1;
		read1=map.lookup(0);

		map.balance();

		// Step 4
		// Define a function "balance" that sums the amount of all the
		// bank accounts in the map. In order to have a consistent result,
		// the execution of this function should happen atomically:
		// no other deposit operations should interleave.

		// Step 5
		// Define a function 'do_work', which has a for-loop that
		// iterates for config_t.iters times. In each iteration,
		// the function 'deposit' should be called with 80% of the probability;
		// otherwise (the rest 20%) the function 'balance' should be called.
		// The function 'do_work' should measure 'exec_time_i', which is the
		// time needed to perform the entire for-loop. This time will be shared with
		// the main thread once the thread executing the 'do_work' joins its execution
		// with the main thread.

		int nb_iter=cfg.iters;
		int n=cfg.threads;

		std::vector<double> proc_time(n, 0.0);

		std::vector<std::vector<double>> bal_thread(n);

			    // Define a Lambda Expression 
    auto do_work = [&](int iter, int max_key,int thread_id) { 
        float d=10;
		float r;
		auto start = high_resolution_clock::now();
			for (int i=0;i<iter;i++)
			{
				r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

				if (r<=prob_deposit)
				{
				
				//cout << i << "\n";
				int acc1 = rand() % max_key;
				int acc2 = rand() % max_key;

				while(acc1==acc2)
				{
					acc2 = rand() % max_key;
				}

				map.deposit(acc1,acc2,10);
				//cout <<"done from thread "<<thread_id << "\n";

				}

				else
				{
					bal_thread[thread_id].push_back(map.balance());
				}
				

			}

			auto stop = high_resolution_clock::now();

			auto elapsed_time=duration_cast<milliseconds>(stop - start);
			std::cout<<"<"<<elapsed_time.count()<<","<<thread_id<<">"<< std::endl;

		proc_time[thread_id]=elapsed_time.count();
    }; 

	auto start = high_resolution_clock::now();

		 std::vector<thread> threads(n);
    // spawn n threads:
    for (int i = 0; i < n; i++) {
        threads[i] = thread(do_work, nb_iter,max_key,i);
    }

    for (auto& th : threads) {
        th.join();
    }

	auto stop = high_resolution_clock::now();

	float elapsed_time=duration_cast<milliseconds>(stop - start).count();

	cout <<elapsed_time<< "\n";

	float sum=map.sum_map();

	int all_balances_verified=1;

	for (int i=0;i<n;i++)
	{
		std::vector<double> inter(bal_thread[i]);
		for (auto j = inter.begin(); j != inter.end(); ++j)
		{
			if ((int)(*j)!=total)
			{
				all_balances_verified=0;
				break;
			}
				
		}
	}

	cout <<all_balances_verified<< "\n";

	
	//float bal=map.balance();
		

		// Step 6
		// The evaluation should be performed in the following way:
		// - the main thread creates #threads threads (as defined in config_t)
		//   << use std:threds >>
		// - each thread executes the function 'do_work' until completion
		// - the (main) spawning thread waits for all the threads to be executed
		//   << use std::thread::join() >>
		//	 and collect all the 'exec_time_i' from each joining thread
		// - once all the threads have joined, the function "balance" must be called

		// WHAT IS THE OUTPUT OF this call of "balance"?
		// DOES IT MATCH WHAT YOU EXPECT?
		// WHAT DO YOU EXPECT?
		// WHAT ARE THE OUTCOMES OF ALL THE "balance" CALLS DURING THE EXECUTION?
		// IS THAT WHAT YOU EXPECT?

		// Step 7
		// Now configure your application to perform the same total amount
		// of iterations just executed, but all done by a single thread.
		// Measure the time to perform them and compare with the time
		// previously collected.
		// Which conclusion can you draw?
		// Which optimization can you do to the single-threaded execution in
		// order to improve its performance?

		auto start_seq = high_resolution_clock::now();

		int max_iter_seq=n*nb_iter;
		float r;

		for(int i=0;i<max_iter_seq;i++)
		{
			r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			if(r<=prob_deposit)
			{
			int acc1 = rand() % max_key;
			int acc2 = rand() % max_key;

			while(acc1==acc2)
			{
				acc2 = rand() % max_key;
			}

			map.one_thread_deposit(acc1,acc2,10);

			}

			else
			{
				map.sum_map();
			}
			
		}

		auto stop_seq = high_resolution_clock::now();

	float elapsed_time_seq=duration_cast<milliseconds>(stop_seq - start_seq).count();

	cout <<elapsed_time_seq<< "\n";

		// Step 8
		// Remove all the items in the map by leveraging the 'remove' function of the map
		// Destroy all the allocated resources (if any)
		// Execution terminates.
		// If you reach this stage happy, then you did a good job!

		
		for (int i=0;i<max_key;i++)
		{
			map.remove(i);
		}
		
		

		// Final step: Produce plot
        // I expect each submission to include a plot in which
        // the x-axis is the concurrent threads used {1;2;4;8}
        // the y-axis is the application execution t ime.
        // The performance at 1 thread must be the sequential
        // application without atomic execution

        // You might need the following function to print the entire map.
        // Attention if you use it while multiple threads are operating
        map.apply(printer);

		cout <<"done"<< "\n";

	}

	

void test_driver(config_t &cfg) {
	run_custom_tests(cfg);
}
