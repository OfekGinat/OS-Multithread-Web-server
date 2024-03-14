#!/bin/bash

# Loop 10 times
for ((i=1; i<=20; i++)); do
    ./client localhost 8003 output.cgi?5 &
done