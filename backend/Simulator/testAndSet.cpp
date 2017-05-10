
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <atomic>


int locked;
int sum = 0;


void threadFunc(){
	int done = 0;
	while(!done){
		if(__sync_bool_compare_and_swap (&locked, 0, 1)){
			done = 1;
			sum += sum + 1;
			std::cout << "thread " << std::this_thread::get_id() << "has added to sum and sum = " << sum << "\n";
			locked = 0;
		}
	}
}

int main(){

	locked = 0;

	std::cout << "spawning all threads\n";

	std::thread first(threadFunc);
	std::thread second (threadFunc);
	std::thread third (threadFunc);
	std::thread fourth (threadFunc);

	// synchronize threads:
  	first.join();                
  	second.join();               
  	third.join();
  	fourth.join();

  	std::cout << "all threads completed and sum is " << sum << "\n";

  return 0;

}