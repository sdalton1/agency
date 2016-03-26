//------------------------------------------------------------------------------------------------------------------------------
// Samuel Williams
// SWWilliams@lbl.gov
// Lawrence Berkeley National Lab
//------------------------------------------------------------------------------------------------------------------------------
#ifndef TIMER_H
#define TIMER_H

#include<stdint.h>
#ifndef _OPENMP
#define CALIBRATE_TIMER
#endif
// user must provide a function getTime and include it in timers.c
// if calibration is necesary, then the user must #define CALIBRATE_TIMER
double getTime() {
  #ifdef _OPENMP
    return(omp_get_wtime());
  #else
    uint64_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return( 1e-9*((double)( (((uint64_t)hi) << 32) | ((uint64_t)lo) )) ); // timers are in units of seconds;  assume 1GHz cycle counter and convert later
  #endif
}

#endif
