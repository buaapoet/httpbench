// Small humble program for benchmarking
// (C) 2012 Dipl.-Inform. (FH) Paul C. Buetow
// Contact: bench@mx.buetow.org
// See COPYING for license infos

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

#define VERSION "0.0.0-devel"

#define SUCCESS 0
#define E_WRONG_USAGE 1
#define E_MISSING_OPT_C 2
#define E_MISSING_OPT_I 3
#define E_OPT_ERROR 5
#define E_OPEN_FILE 8

struct data {
    // Read only for threads
    int i_duration_s;
    int i_concurrent;
    int i_timeout;
    double d_rps_wanted;
    char *c_urlist;
    int i_num_urls;
    char **pc_urls;
    char *c_expected;

    // Read/write for threads
    pthread_mutex_t mutex;
    int i_exit;
    double d_time_min;
    double d_time_max;
    double d_time_avg;
    unsigned int ui_curl_errors;
    unsigned int ui_parse_errors;
    unsigned int ui_timeout_exceeded;
    unsigned int ui_count;
    unsigned int ui_count_total;
    double d_sleep_us;
};

void usage(void) {
    fprintf(stdout, "Bench Version %s Usage:\n", VERSION);
    fprintf(stdout, "./bench\n");
    fprintf(stdout, "\t-u <urllistfile.txt>\n");
    fprintf(stdout, "\t-d <duration sec>\n");
    fprintf(stdout, "\t-c <concurrent>\n");
    fprintf(stdout, "\t-r <rps>\n");
    fprintf(stdout, "\t[-t <timeoutms>]\n");
    fprintf(stdout, "\t[-e <exptected response str>]\n");
}

void checkarg_c(char c_name, char *c_arg) {
    if (c_arg == NULL) {
        fprintf(stderr, "Missing mandatory value for option '%c'\n", c_name);
        usage();
        exit(E_MISSING_OPT_C);
    }
}

void checkarg_i(char c_name, int i_arg) {
    if (i_arg == -1) {
        fprintf(stderr, "Missing mandatory value for option '%c'\n", c_name);
        usage();
        exit(E_MISSING_OPT_I);
    }
}

void print_stats(
    unsigned int ui_count,
    int i_elapsed_time,
    double d_time_max,
    double d_time_min,
    double d_time_avg,
    unsigned int ui_timeout_exceeded,
    unsigned int ui_curl_errors,
    unsigned int ui_parse_errors) {

    fprintf(stdout, "(thread %d) count:%d elapsed:%d (%.1f rps), max:%0.6f min:%0.6f avg:%0.6f timeout_exceeded:%d (%.3f%%) curl_errors:%d (%.3f%%) parse_errors:%d (%.3f%%)\n",
            (int) pthread_self(),
            ui_count,
            i_elapsed_time,
            ((double) ui_count/(double) i_elapsed_time),
            d_time_max,
            d_time_min,
            d_time_avg / (double) ui_count,
            ui_timeout_exceeded,
            ((double) ui_timeout_exceeded)/(((double) ui_count)/100),
            ui_curl_errors,
            ((double) ui_curl_errors)/(((double) ui_count)/100),
            ui_parse_errors,
            ((double) ui_parse_errors)/(((double) ui_count)/100)
           );
}

// Very *very* magic algorithm!
double sleep_us(double d_sleep_us, double d_rps, double d_rps_wanted, double *pd_perc) {
    *pd_perc = d_rps / d_rps_wanted;

    if (*pd_perc < 0.1)
        return d_sleep_us * 0.1;
    else if (*pd_perc > 2.0)
        return d_sleep_us * 2;

    return *pd_perc * d_sleep_us;
}

size_t static write_data(void *p_buf, size_t s_size, size_t s_nmemb, void *p_val) {
    char **pc_response = (char**) p_val;

    *pc_response = strndup(p_buf, (size_t)(s_size * s_nmemb));

    return s_size * s_nmemb;
}

CURL* curl_init(char **c_response) {
    CURL *p_curl = curl_easy_init();

    curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, c_response);

    return p_curl;
}

void* timer_thread(void *p) {
    struct data* p_data = (struct data*) p;
    unsigned long int i;
    time_t t_start_total, t_start;

    t_start_total = t_start = time(NULL);

    for (i = 0;; ++i) {
        time_t t_now, t_elapsed, t_elapsed_total;
        double d_rps, d_perc;

        usleep(1000000);
        t_now = time(NULL);
        t_elapsed_total = t_now - t_start_total;

        if (p_data->i_duration_s <= t_elapsed_total) {
            pthread_mutex_lock(&p_data->mutex);
            p_data->i_exit = 1;
            p_data->i_duration_s = t_elapsed_total;
            pthread_mutex_unlock(&p_data->mutex);

            break;
        }

        t_elapsed = t_now - t_start;

        pthread_mutex_lock(&p_data->mutex);
        d_rps = ((double) p_data->ui_count)/((double) t_elapsed);

        if (p_data->d_rps_wanted > 0) {
            int flag = 0;

            if (d_rps > p_data->d_rps_wanted ) {
                double d_diff = (d_rps - p_data->d_rps_wanted);
                p_data->d_sleep_us = sleep_us(p_data->d_sleep_us, d_rps, p_data->d_rps_wanted, &d_perc);

                fprintf(stdout, "RPS - %d wanted:%d thread_sleep_us:%0.3f sleep_adjust:%0.3f time:%d/%d\n",
                        (int) d_rps,
                        (int) p_data->d_rps_wanted,
                        p_data->d_sleep_us, d_perc,
                        (int) t_elapsed_total,
                        p_data->i_duration_s);

                flag = 1;

            } else if (d_rps < p_data->d_rps_wanted) {
                double d_diff = (p_data->d_rps_wanted - d_rps);
                p_data->d_sleep_us = sleep_us(p_data->d_sleep_us, d_rps, p_data->d_rps_wanted, &d_perc);

                fprintf(stdout, "RPS + %d wanted:%d thread_sleep_us:%0.3f sleep_adjust:%0.3f time:%d/%d\n",
                        (int) d_rps,
                        (int) p_data->d_rps_wanted,
                        p_data->d_sleep_us, d_perc,
                        (int) t_elapsed_total,
                        p_data->i_duration_s);

                flag = 1;

            } else {
                fprintf(stdout, "RPS = %d wanted:%d thread_sleep_us:%0.3f time:%d/%d\n",
                        (int) d_rps,
                        (int) p_data->d_rps_wanted,
                        p_data->d_sleep_us,
                        (int) t_elapsed_total,
                        p_data->i_duration_s);
            }

            if (flag == 1) {
                p_data->ui_count = 0;
                t_start = time(NULL);
            }
        }

        pthread_mutex_unlock(&p_data->mutex);
    }

    return NULL;
}

void* request_thread(void *p) {
    struct data* p_data = (struct data*) p;
    char *c_response;
    CURL *p_curl = curl_init(&c_response);

    if (p_curl) {
        CURLcode res;
        time_t t1 = time(NULL);
        int i, i_which_url = -1;
        unsigned int d_sleep_us;
        double d_val, d_time_min = 9999999999, d_time_max = -1,
                      d_timeout = (double) p_data->i_timeout / (double) 1000,
                      d_time_avg = 0;
        unsigned int ui_count = 0, ui_curl_errors = 0, ui_timeout_exceeded = 0, ui_parse_errors = 0;

        for (;; ++ui_count) {
            i_which_url = (i_which_url+1) % p_data->i_num_urls;
            curl_easy_setopt(p_curl, CURLOPT_URL, p_data->pc_urls[i_which_url]);
            res = curl_easy_perform(p_curl);

            if (CURLE_OK == res) {
                if (NULL == strstr(c_response, p_data->c_expected)) {
                    fprintf(stderr, "While requesting %s:\n", p_data->pc_urls[i_which_url]);
                    fprintf(stderr, "Expected response %s but received %s", p_data->c_expected, c_response);
                    ++ui_parse_errors;
                }

                res = curl_easy_getinfo(p_curl, CURLINFO_TOTAL_TIME, &d_val);

                if ((CURLE_OK == res) && (d_val > 0)) {

                    if (d_timeout > 0 && d_val > d_timeout) {
                        ++ui_timeout_exceeded;
                    }

                    if (d_val > d_time_max)
                        d_time_max = d_val;

                    if (d_val < d_time_min)
                        d_time_min = d_val;

                    d_time_avg += d_val;
                } else {
                    char* c_error = calloc(1024, sizeof(char));
                    memset((void*)c_error, 0, 1024);
                    c_error[1023] = '\0';
                    fprintf(stdout, "(thread %d) cURL returned: %s\n",
                            (int) pthread_self(), c_error);
                    free(c_error);
                    ++ui_curl_errors;
                }
            } else {
                ++ui_curl_errors;
            }

            if (c_response != NULL) {
                free(c_response);
                c_response = NULL;
            }

            if (ui_count % 10000 == 0 && ui_count > 0)
                print_stats(ui_count, time(NULL) - t1, d_time_max, d_time_min, d_time_avg, ui_timeout_exceeded, ui_curl_errors, ui_parse_errors);
            pthread_mutex_lock(&p_data->mutex);
            ++p_data->ui_count;
            d_sleep_us = p_data->d_sleep_us;
            if (p_data->i_exit)
                break;
            pthread_mutex_unlock(&p_data->mutex);


            usleep((unsigned int)d_sleep_us);
        }

        if (p_data->d_time_min > d_time_min)
            p_data->d_time_min = d_time_min;
        if (p_data->d_time_max < d_time_max)
            p_data->d_time_max = d_time_max;

        p_data->ui_curl_errors += ui_curl_errors;
        p_data->ui_parse_errors += ui_parse_errors;
        p_data->ui_timeout_exceeded += ui_timeout_exceeded;
        p_data->ui_count_total += ui_count + 1;
        p_data->d_time_avg += d_time_avg;

        pthread_mutex_unlock(&p_data->mutex);

        curl_easy_cleanup(p_curl);
    }

    return NULL;
}

int main(int i_argc, char **c_argv) {
    struct data d;
    pthread_t *pt_threads;
    pthread_t t_timer;
    int i_threads;
    FILE *fh;
    char c_buff[4096];
    int i_opt;

    d.c_expected = "";
    d.c_urlist = NULL;
    d.d_rps_wanted = -1;
    d.i_concurrent = -1;
    d.i_duration_s = -1;
    d.i_timeout = -1;

    while ((i_opt = getopt(i_argc, c_argv, "e:u:r:c:d:t:h")) != -1) {
        switch (i_opt) {
        case 'e':
            d.c_expected = optarg;
            break;
        case 'u':
            d.c_urlist = optarg;
            break;
        case 'r':
            d.d_rps_wanted = (double) atoi(optarg);
            break;
        case 'c':
            d.i_concurrent = atoi(optarg);
            break;
        case 'd':
            d.i_duration_s = atoi(optarg);
            break;
        case 't':
            d.i_timeout = atoi(optarg);
            break;
        case 'h':
            usage();
            exit(SUCCESS);
            break;
        case '?':
            if (optopt == 'e' ||
                    optopt == 'u' ||
                    optopt == 'r' ||
                    optopt == 'c' ||
                    optopt == 'd' ||
                    optopt == 't'
               )

                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option -%c'.\n", optopt);

            else
                fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);

            exit(E_OPT_ERROR);
        default:
            usage();
            exit(E_WRONG_USAGE);
            break;
        }
    }

    checkarg_c('u', d.c_urlist);
    checkarg_i('r', d.d_rps_wanted);
    checkarg_i('c', d.i_concurrent);
    checkarg_i('d', d.i_duration_s);

    d.i_exit = 0;
    d.d_time_min = 999999;
    d.d_time_max = 0;
    d.ui_timeout_exceeded = 0;
    d.ui_curl_errors = 0;
    d.ui_parse_errors = 0;
    d.ui_count = 0;
    d.ui_count_total = 0;
    d.d_sleep_us = 1000000;

    d.pc_urls = calloc(1, sizeof(char*));

    pthread_mutex_init(&d.mutex, NULL);

    // Open urllistfile
    fh = fopen(d.c_urlist, "r");
    if (fh == NULL) {
        fprintf(stdout, "Coult not open file %s\n", d.c_urlist);
        exit(E_OPEN_FILE);
    }

    d.i_num_urls = 0;
    while (fgets(c_buff,sizeof(c_buff),fh) != NULL) {
        int i_len = strlen(c_buff);
        d.pc_urls[d.i_num_urls] = calloc(i_len, sizeof(char));
        strncpy(d.pc_urls[d.i_num_urls], c_buff, i_len-1);
        d.pc_urls[d.i_num_urls][i_len-1] = '\0';

        ++d.i_num_urls;
        d.pc_urls = realloc(d.pc_urls, (d.i_num_urls + 1) * sizeof(char*));
    }

    pt_threads = (pthread_t*) calloc(d.i_concurrent, sizeof(pthread_t));

    pthread_create(&t_timer, NULL, timer_thread, (void*) &d);

    for (i_threads = 0; i_threads < d.i_concurrent; ++i_threads) {
        // Add a sleep so threads are mixed up from the beginning
        usleep((unsigned int) i_threads * 10 + 1);
        fprintf(stdout, "Starting thread %d\n", i_threads);
        pthread_create(&pt_threads[i_threads], NULL, request_thread, (void*) &d);
    }

    for (i_threads = 0; i_threads < d.i_concurrent; ++i_threads)
        pthread_join(pt_threads[i_threads], NULL);

    fprintf(stdout, "\nTOTAL STATS using duration:%d concurrent:%d timeout:%d urllist:%s\n",
            d.i_duration_s,
            d.i_concurrent,
            d.i_timeout,
            d.c_urlist);
    print_stats(d.ui_count_total,
                d.i_duration_s,
                d.d_time_max,
                d.d_time_min,
                d.d_time_avg,
                d.ui_timeout_exceeded,
                d.ui_curl_errors,
                d.ui_parse_errors);

    // Cleaning up stuff
    for (; 0 < d.i_num_urls; --d.i_num_urls)
        free(d.pc_urls[d.i_num_urls-1]);

    free(d.pc_urls);

    pthread_mutex_destroy(&d.mutex);

    return SUCCESS;
}
