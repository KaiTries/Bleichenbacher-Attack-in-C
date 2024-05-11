#include "interval.h"
#include <time.h>
#include <stdio.h>


int main() {
    // initialize variables
    mpz_t a, b, c, d;
    mpz_init_set_ui(a, 1);
    mpz_init_set_ui(b, 10);
    mpz_init_set_ui(c, 20);
    mpz_init_set_ui(d, 30);

    // set intervals with mpz_t structs
    Interval intervalA;
    set_interval(&intervalA,a,b);
    Interval intervalB;
    set_interval(&intervalB,c,d);
    // set intervals with ints
    Interval intervalC;
    set_interval_ui(&intervalC,19,22);
    Interval intervalD;
    set_interval_ui(&intervalD,5,11);

    // init set
    IntervalSet set;
    init_interval_set(&set);

    // add intervalA
    add_interval(&set,&intervalA);
    // change intervalA after -> should not affect the set
    set_interval_ui(&intervalA, 100,200);

    add_interval(&set,&intervalB);
    add_interval(&set,&intervalC);
    add_interval(&set,&intervalD);
    
    // print interval should be -> [[1, 11], [19, 30]]
    // -> Intervals get correctly merged and sorted
    printf("print interval should be -> [[1, 11], [19, 30]]\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");
    free_interval_set(&set);
    init_interval_set(&set);

    Interval testInterval;
    for (size_t i = 0; i < 5; i++)
    {
        set_interval_ui(&testInterval,i*10,i*10 + 2);
        add_interval(&set, &testInterval);
    }
    printf("print interval should be -> [(0,2),(10,12),(20,22),(30,32),(40,42)]\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");

    set_interval_ui(&testInterval, 25,27);
    add_interval(&set, &testInterval);
    printf("Adding interval (25,27) should be placed correctly in middle\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");
    
    
    set_interval_ui(&testInterval, 22,32);
    add_interval(&set, &testInterval);
    printf("adding interval (22,32) should correctly fuse the intervals together\n");
    printf("=========================\n");
    print_intervalSet(&set);
    printf("=========================\n");
    
    
    return 0;   
}