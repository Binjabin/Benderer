#!/bin/bash

cd "$(dirname "$0")"

IMG="preview.ppm"
BIN="./cmake-build-debug/Benderer"
GEOMETRY="800x800"

trap "kill $FEH_PID 2>/dev/null" EXIT

# Start renderer
$BIN &
RENDER_PID=$!

# Wait for first valid file
while [ ! -s "$IMG" ]; do
    sleep 0.05
done

sleep 0.2

# Launch feh (use full path if needed)
/usr/bin/feh \
    --reload 0.1 \
    --geometry $GEOMETRY \
    --auto-zoom \
    --scale-down \
    --force-aliasing \
    "$IMG" &

FEH_PID=$!

wait $RENDER_PID
wait $FEH_PID
