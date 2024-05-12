#include "interval.h"

void init_interval(Interval* interval) {
    mpz_init(interval->lower);
    mpz_init(interval->upper);
}

void free_interval(Interval* interval) {
    mpz_clear(interval->lower);
    mpz_clear(interval->upper);
}

int set_interval(Interval* interval, mpz_t a, mpz_t b) {
    if (mpz_cmp(a,b) > 0) return 0;
    mpz_init_set(interval->lower, a);
    mpz_init_set(interval->upper, b);
    return 1;
}

int set_interval_ui(Interval* interval, int a, int b) {
    if (a > b) return 0;
    mpz_init_set_ui(interval->lower, a);
    mpz_init_set_ui(interval->upper, b);
    return 1;
}

int reunion(Interval* res, Interval* a, Interval* b) {
    // lowr end of b is in a
    int flagA = in(a, b->lower);
    // upper end of b is in a
    int flagB = in(a, b->upper);


    // lower end of a is in b
    int flagC = in(b, a->lower);
    // upper end of b is in a
    int flagD = in(b, a->upper);

    if(!flagA && !flagB && !flagC && !flagD) return 0;
    
    if (flagA && flagB) {
        set_interval(res, a->lower, a->upper);
    }

    if (flagA && !flagB) {
        set_interval(res, a->lower, b->upper);
    }

    if (!flagA && flagB) {
        set_interval(res, b->lower, a->upper);
    }

    if (flagC && flagD) {
        set_interval(res, b->lower, b->upper);
    }

    return 1;
}

int in(Interval* interval, mpz_t val) {
    if (mpz_cmp(interval->lower, val) > 0) return 0;
    if (mpz_cmp(interval->upper, val) < 0) return 0;
    return 1; 
}

int overlap(Interval* a, Interval* b) {
    if(!in(a, b->lower) && !in(a, b->upper) && !in(b, a->lower) && !in(b, a->upper)) 
        return 0;
    return 1;
}

void print_interval(Interval interval) {
    gmp_printf("[%Zd, %Zd]\n", interval.lower, interval.upper);
}

void init_interval_set(IntervalSet* set) {
    set->size = 0;
    set->capacity = 10; 
    set->intervals = malloc(set->capacity * sizeof(Interval));
}

void ensure_capacity(IntervalSet *set) {
    if (set->size >= set->capacity) {
        size_t new_capacity = set->capacity * 2;
        Interval *new_intervals = realloc(set->intervals, new_capacity * sizeof(Interval));
        if (new_intervals == NULL) {
            // Handle realloc failure; for now, we just exit.
            exit(EXIT_FAILURE);
        }
        set->intervals = new_intervals;
        set->capacity = new_capacity;
    }
}

void add_at_index(IntervalSet *set, Interval* interval) {
    int index;

    for (index = set->size - 1; index >= 0; index--) {
        if (mpz_cmp(set->intervals[index].upper, interval->lower) < 0) break;
    }

    for(int i = set->size; i > index; i--) {
        set->intervals[i] = set->intervals[i - 1];
    }
    set->intervals[++index] = *interval;
    set->size++;
}


void add_interval(IntervalSet* set, Interval* interval) {
    ensure_capacity(set);
    if (set->size == 0) {
        set->intervals[0] = *interval;
        set->size++;
        return;
    }

    int i;
    int left = -1;
    int right = -1;

    for (i = 0; i < set->size; i++)
    {   
        if (in(&set->intervals[i], interval->lower) && in(&set->intervals[i], interval->upper)) return;

        if(left == -1 && overlap(&set->intervals[i], interval)) {
            left = i;
            right = i;
        } else if (overlap(&set->intervals[i], interval)) {
            right = i;
        }

    }    
    Interval temp;
    init_interval(&temp);
    if(left != -1 && right != left) {
        reunion(&temp,&set->intervals[left],interval);
        reunion(&set->intervals[left],&temp,&set->intervals[right]);
        for(int i = 0; i < set->size - right; i++) {
            set->intervals[left+1+i] = set->intervals[right+1+i];
        }
        set->size -= (right - left);
    }
    else if(left != -1) {
        reunion(&temp,&set->intervals[left],interval);
        set->intervals[left] = temp;   
    } else {
        add_at_index(set, interval);
    }
}

void print_intervalSet(IntervalSet* set) {
    for (int i = 0; i < set->size; i++)
    {
        print_interval(set->intervals[i]);
    }
}

void free_interval_set(IntervalSet *set) {
    for (size_t i = 0; i < set->size; i++) {
        mpz_clear(set->intervals[i].lower);
        mpz_clear(set->intervals[i].upper);
    }
    free(set->intervals);
    set->size = 0;
}