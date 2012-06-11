// Small humble program for benchmarking
// (C) 2012 Dipl.-Inform. (FH) Paul C. Buetow
// For ./debian/copyright for License

#include <ctype.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "version.h"

#define SUCCESS 0
#define E_WRONG_USAGE 1
#define E_MISSING_OPT_C 2
#define E_MISSING_OPT_I 3
#define E_OPT_ERROR 5
#define E_OPEN_FILE 8

int QUIT;

struct data {
    // Read only for threads
    int i_duration_s;
    int i_concurrent;
    int i_timeout;
    double d_rps_wanted;
    char *c_urlparam;
    int i_num_urls;
    char **pc_urls;
    char *c_expected;

    // Read/write for threads
    pthread_mutex_t mutex;
    int i_exit;
    double d_time_min;
    double d_time_max;
    double d_time_avg;
    unsigned int ui_time_avg_count;
    unsigned int ui_curl_errors;
    unsigned int ui_parse_errors;
    unsigned int ui_timeout_exceeded;
    unsigned int ui_count;
    unsigned int ui_count_total;
    double d_sleep_us;
};

void usage(void);
void checkarg_c(char c_name, char *c_arg);
void checkarg_i(char c_name, int i_arg);
void handle_signal();
int is_http_url(char *c_str);
void print_stats(
    unsigned int ui_count,
    int i_elapsed_time,
    int i_wanted_time,
    double d_rps_wanted,
    double d_time_max,
    double d_time_min,
    double d_time_avg_total,
    unsigned int ui_timeout_exceeded,
    unsigned int ui_curl_errors,
    unsigned int ui_parse_errors);
double sleep_us(double d_sleep_us, double d_rps, double d_rps_wanted, double *pd_perc);
size_t static write_data(void *p_buf, size_t s_size, size_t s_nmemb, void *p_val);
CURL* curl_init(char **c_response);
void* timer_thread(void *p);
void* request_thread(void *p);
