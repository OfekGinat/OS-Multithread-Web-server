#!/bin/bash

# Loop 10 times
for ((i=1; i<=12; i++)); do
    ./client localhost 8003 output.cgi?10 &
done