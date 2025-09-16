#!/bin/bash

# Start nginx in the background
echo "Starting nginx..."
nginx -g "daemon off;" &

# Function to run data fetch
run_data_fetch() {
    echo "$(date): Running data fetch..."
    cd /easymarry-data
    ./em-data
    echo "$(date): Data fetch completed"
}

# Run initial data fetch
run_data_fetch

# Schedule hourly runs using cron-like loop
while true; do
    # Sleep for 1 hour (3600 seconds)
    sleep 3600
    run_data_fetch
done