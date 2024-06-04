#pragma once
#include <gmp.h>

// Interval data structure
typedef struct {
  mpz_t lower;
  mpz_t upper;
} Interval;

void init_interval(Interval *interval);

void free_interval(Interval *interval);

int set_interval(Interval *interval, mpz_t a, mpz_t b);

int set_interval_ui(Interval *interval, int a, int b);

int reunion(Interval *res, Interval *a, Interval *b);

int in(Interval *interval, mpz_t val);

int overlap(Interval *a, Interval *b);

void print_interval(Interval interval);

// Interval set data structure
typedef struct {
  Interval *intervals;
  size_t size;
  size_t capacity;
} IntervalSet;

void init_interval_set(IntervalSet *set);

void ensure_capacity(IntervalSet *set);

void add_at_index(IntervalSet *set, Interval *interval);

void add_interval(IntervalSet *set, Interval *interval);

void print_intervalSet(IntervalSet *set);

void free_interval_set(IntervalSet *set);
