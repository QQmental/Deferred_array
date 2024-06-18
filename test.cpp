#include <initializer_list>
#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "deferred_array.hpp"




int main()
{
    std:: size_t a = 4, b = 5;

    // size of the two dimension is set at run time
    Deferred_array<int, 3> arr({a, b, 6});
    

    for(std::size_t i = 0 ; i < arr.data_len() ; i++)
        arr.data()[i] = i+10;
    
    Deferred_array<int, 3> arr2;
    arr2 = arr; // copy all element

    #define indexies [4][5][6]
    
    int (*p)[5][6] = reinterpret_cast<int(*)[5][6]>(arr2.data());
   
    for(std::size_t i = 0 ; i < a ; i++)
    {
        for(std::size_t j = 0 ; j < b ; j++)
        {
            for(std::size_t k = 0 ; k < 6 ; k++)
            {
                auto item1 = arr[i][j][k];
                auto item2 = arr2[i][j][k];
                assert(item1 == item2 && item1 == p[i][j][k]); 
            }
        }
    }
}
