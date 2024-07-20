# Deferred_array
Are you using std::vector<std::vector<std::vector<int\>\>\>? 
If dimension of your vector is determined at complie time, and size of each dimension is not but fixed. 
Deferred_array is what you need.

# requriement
c++17

# build test
g++ test.cpp -std=c++17 -Wall -o test

you can uncomment "assert(is_out_of_rangge(idx) == false);" to enable boundary check
# run test
./test

# example
Deferred_array<int, 3> arr; // declare a three dimensions array. 

int a = 3, b = 4, c = 5; 

arr = Deferred_array<int, 3>({a, b, c}); // arr[3][4][5] 

arr[1][2][3] = 10; 
