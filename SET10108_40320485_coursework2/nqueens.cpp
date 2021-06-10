#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <omp.h>
#include <thread>
#include <fstream>
#include <string>
#include <cmath>
#include <mutex>
#include <vector>
#include <memory>
#include <cstdint>


#define MAX_N 16
std::ofstream myfile;
std::mutex mut;
bool exitprogram = false;

int isValid(int qr[], int n)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			// two queens in the same row - return false
			if (qr[i] == qr[j]) return 0;

			// two queens in the same diagonal - return false
			if (qr[i] - qr[j] == i - j ||
				qr[i] - qr[j] == j - i)
				return 0;
		}
	}
	//otherwise return true
	return 1;
}

int isValid_thread(int qr[], int n)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			// two queens in the same row - return false
			if (qr[i] == qr[j]) return 0;

			// two queens in the same diagonal - return false
			if (qr[i] - qr[j] == i - j ||
				qr[i] - qr[j] == j - i)
				return 0;
		}
	}
	//otherwise return true
	return 1;
}


void write_solutions(int qr[], int index, int n_number) {
	//printf("\n");
	myfile << "\n";
	for (index = 0; index < n_number; index++)
	{
		int j;
		for (j = 0; j < n_number; j++)
		{
			if (qr[index] == j)	myfile << "|Q";
			else myfile <<"| ";
		}
		myfile << "|\n";
	}
	myfile << "\n";
}

void write_solutions_thread(int qr[], int index, int n_number) {
	std::lock_guard<std::mutex> lock(mut);

	myfile << "\n";
	for (index = 0; index < n_number; index++)
	{
		int j;
		for (j = 0; j < n_number; j++)
		{
			if (qr[index] == j)	myfile << "|Q";
			else myfile << "| ";
		}
		myfile << "|\n";
	}
	myfile << "\n";
}

void increment(int64_t &sol) {
	std::lock_guard<std::mutex> lock(mut);
	sol++;
}



void solve_thread_loop(int64_t max_it, int num, int64_t &solutions_thread, int64_t it) {

	for (it = it; it < max_it; it++)
	{
		uint64_t tag = uint64_t(it);
		int i;
		int queen_rows[MAX_N];
		
		for (i = 0; i < num; i++)
		{			
			queen_rows[i] = tag % num;
			
			tag /= num;
		}
	
		if (isValid(queen_rows, num))
		{			
			increment(solutions_thread);

			write_solutions_thread(queen_rows, i, num);
						
		}
	}
}

int main(int argc, char* argv[])
{
	if (argv[1] == std::string("openmp")) {		
		uint64_t max_iterations = 1;
		double start_time, end_time;
		int64_t solutions = 0;
		auto num_threads = std::thread::hardware_concurrency();

		int n;
		printf("Enter the value of N: ");
		scanf_s("%d", &n);


		std::string filename = "openmp_solutions_";
		filename += std::to_string(n);
		filename += ".txt";



		myfile.open(filename);

		max_iterations = uint64_t(pow(n, n));


		omp_set_num_threads(num_threads);

		myfile << "-----------------------\n-----------------------\n   \n   \n   \n   \n   \n   \n   ";

		start_time = omp_get_wtime();

		int64_t iteration;
		#pragma omp parallel for
		for (iteration = 0; iteration < max_iterations; iteration++)
		{
			uint64_t tag = (uint64_t)iteration;
			int i;
			int queen_rows[MAX_N];

			//generate configurations where there's only one queen per collumn
			for (i = 0; i < n; i++)
			{
				queen_rows[i] = tag % n;
				tag /= n;
			}

			if (isValid(queen_rows, n))
			{
				#pragma omp atomic
				solutions++;

				#pragma omp critical //guard lock mutex for the printing function		
				write_solutions(queen_rows, i, n);
			}
		}

		end_time = omp_get_wtime();

		myfile.seekp(0, std::ios::beg);
		myfile << "Total execution time: " << end_time - start_time << " seconds" << std::endl;;
		myfile << "Number of solutions: " << solutions << std::endl;

		myfile.close();
		printf("Done.");
		
		
	}

	if (argv[1] == std::string("serial")) {
		uint64_t max_iterations = 1;
		double start_time, end_time;
		int64_t solutions = 0;

		int n;
		printf("Enter the value of N: ");

		scanf_s("%d", &n);

		std::string filename = "serial_solutions_";
		filename += std::to_string(n);
		filename += ".txt";



		myfile.open(filename);

		max_iterations = pow(n, n); //checking every single combination

		myfile << "-----------------------\n-----------------------\n   \n   \n   \n   \n   \n   \n   ";

		start_time = omp_get_wtime();

		int64_t iteration;
		for (iteration = 0; iteration < max_iterations; iteration++)
		{
			uint64_t tag = (uint64_t)iteration;
			int i;
			int queen_rows[MAX_N];
			
			//generate configs only where there's a single queen per column only
			for (i = 0; i < n; i++)
			{
				queen_rows[i] = tag % n;

				tag /= n;
			}

			if (isValid(queen_rows, n))
			{			
				solutions++;			
				write_solutions(queen_rows, i, n);
			}
		}

		end_time = omp_get_wtime();
		
		myfile.seekp(0, std::ios::beg);
		myfile << "Total execution time: " << end_time - start_time << " seconds" << std::endl;;
		myfile << "Number of solutions: " << solutions << std::endl;

		myfile.close();
		printf("Done.");
	}

	if (argv[1] == std::string("thread")) {
		uint64_t max_iterations_absolute = 1;
		double start_time, end_time;
		int64_t solutions = 0;

		int n;
		printf("Enter the value of N: ");
		scanf_s("%d", &n);

		std::string filename = "thread_solutions_";
		filename += std::to_string(n);
		filename += ".txt";



		myfile.open(filename);

		auto threads_num = std::thread::hardware_concurrency();

		max_iterations_absolute = uint64_t(pow(n, n));

		uint64_t max_iterations_current = uint64_t(max_iterations_absolute / threads_num);

		start_time = omp_get_wtime();


		std::vector<std::thread> threads;

		int64_t iteration = 0;

		myfile << "-----------------------\n-----------------------\n   \n   \n   \n   \n   \n   \n   ";
		
		for (int i = 0; i < threads_num; i++) {	
			
			threads.push_back(std::thread(solve_thread_loop, max_iterations_current, n, std::ref(solutions), iteration));
			iteration += uint64_t(max_iterations_absolute / threads_num);
			max_iterations_current += uint64_t(max_iterations_absolute / threads_num);

		}
			

		// Join threads
		for (auto& t : threads)
			t.join();


		end_time = omp_get_wtime();

		myfile.seekp(0, std::ios::beg);
		myfile << "Total execution time: " << end_time - start_time << " seconds" << std::endl;;
		myfile << "Number of solutions: " << solutions << std::endl;
		myfile.close();
		printf("Done.");
		
	}
	

	return 0;
}
