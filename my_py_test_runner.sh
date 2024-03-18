#!/bin/bash

# Loop for 10 iterations
for ((i=11; i<=30; i++))
do
    # Run the command and redirect output to a file
    python3 -m pytest -n auto > my_test_output_number_${i}.txt
done