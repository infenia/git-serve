#!/bin/bash

# Start nginx in the background
echo "Starting nginx..."
nginx -g "daemon off;" &

# Start the em-data application
echo "Starting em-data application..."
cd /easymarry-data
./em-data

# Keep the container running
wait