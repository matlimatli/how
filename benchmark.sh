#!/usr/bin/env bash
# Benchmark cost-effective models across all providers.
# Usage: bash benchmark.sh [number_of_runs]
#
# Measures total round-trip time per model. Runs each model N times
# (default 3) and reports the median.

set -euo pipefail

RUNS="${1:-3}"
QUERY="to list files in current directory"
BINARY="./build/how"

if [[ ! -x "$BINARY" ]]; then
    echo "Build first: mkdir -p build && cd build && cmake .. && make" >&2
    exit 1
fi

# Models to benchmark: "provider:model:cost"
MODELS=(
    "openai:gpt-5-nano:\$0.05"
    "openai:gpt-5.4-nano:\$0.20"
    "openai:gpt-5-mini:\$0.25"
    "google:gemini-2.5-flash-lite:\$0.10"
    "google:gemini-3.1-flash-lite-preview:\$0.25"
    "anthropic:claude-haiku-4-5-20251001:\$0.80"
    "anthropic:claude-sonnet-4-6:\$1.00"
    "mistral:mistral-small-latest:\$0.15"
    "mistral:mistral-medium-latest:\$0.40"
    "mistral:mistral-large-latest:\$0.50"
)

# Get median of an array of numbers
median() {
    local sorted
    sorted=($(printf '%s\n' "$@" | sort -n))
    local count=${#sorted[@]}
    echo "${sorted[$((count / 2))]}"
}

# Print header
printf "\n"
printf "%-35s  %8s  %8s  %s\n" "Model" "Median" "Best" "Cost/MTok"
printf "%-35s  %8s  %8s  %s\n" "-----------------------------------" "--------" "--------" "---------"

for entry in "${MODELS[@]}"; do
    IFS=':' read -r provider model cost <<< "$entry"

    times=()
    failed=0

    for ((i = 1; i <= RUNS; i++)); do
        start=$(python3 -c 'import time; print(time.time())')
        if HOW_PROVIDER="$provider" HOW_MODEL="$model" "$BINARY" $QUERY > /dev/null 2>&1; then
            end=$(python3 -c 'import time; print(time.time())')
            elapsed=$(python3 -c "print(f'{${end} - ${start}:.3f}')")
            times+=("$elapsed")
        else
            failed=1
            break
        fi
    done

    if [[ $failed -eq 1 ]]; then
        printf "%-35s  %8s  %8s  %s\n" "$model" "FAILED" "-" "$cost"
    else
        med=$(median "${times[@]}")
        best=$(printf '%s\n' "${times[@]}" | sort -n | head -1)
        printf "%-35s  %8ss  %7ss  %s\n" "$model" "$med" "$best" "$cost"
    fi
done

printf "\n"
