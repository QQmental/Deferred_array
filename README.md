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
