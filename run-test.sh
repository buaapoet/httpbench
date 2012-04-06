#!/bin/bash -x

./bench -u request_test.txt -d 10 -c 10 -r 100 -t 20 -e SUMMARY
