#!/usr/bin/env bash
# Run dangerous MEOW tests in isolated Docker containers.
# Each terminal test case (shutdown, force_kill) runs in its own container.
# Usage: ./run_danger_tests.sh [--no-build]

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
IMAGE="meow-danger-test"
TIMEOUT_S=10  # seconds before a container is considered hung

pass=0
fail=0

# -------------------------------------------------------------------------
# Preflight
# -------------------------------------------------------------------------

if ! command -v docker &>/dev/null; then
    echo "ERROR: docker not found" >&2
    exit 1
fi

if [[ "${1:-}" != "--no-build" ]]; then
    echo "Building $IMAGE ..."
    docker build -t "$IMAGE" -f "$SCRIPT_DIR/Dockerfile" "$REPO_ROOT" || {
        echo "ERROR: docker build failed" >&2
        exit 1
    }
fi

# -------------------------------------------------------------------------
# Helpers
# -------------------------------------------------------------------------

# run_terminal CASE EXTRA_DOCKER_ARGS EXPECTED_EXIT
#   EXPECTED_EXIT: a number to match exactly, or "any" (just check != 2 and != 124)
run_terminal() {
    local name="$1"
    local extra="$2"
    local expected="$3"

    # shellcheck disable=SC2086
    timeout "$TIMEOUT_S" docker run --rm $extra "$IMAGE" /test/test_danger "$name"
    local code=$?

    if [[ $code -eq 124 ]]; then
        printf "  %-40s FAIL (timed out — container did not terminate)\n" "$name"
        ((fail++))
        return
    fi

    if [[ "$expected" == "any" ]]; then
        if [[ $code -eq 2 ]]; then
            printf "  %-40s FAIL (function returned instead of terminating)\n" "$name"
            ((fail++))
        else
            printf "  %-40s PASS (container exited with code %d)\n" "$name" "$code"
            ((pass++))
        fi
    elif [[ $code -eq $expected ]]; then
        printf "  %-40s PASS (exit %d)\n" "$name" "$code"
        ((pass++))
    else
        printf "  %-40s FAIL (expected exit %s, got %d)\n" "$name" "$expected" "$code"
        ((fail++))
    fi
}

# run_check CASE EXTRA_DOCKER_ARGS
#   The test binary exits 0 on pass, 2 on fail.
run_check() {
    local name="$1"
    local extra="$2"

    # shellcheck disable=SC2086
    timeout "$TIMEOUT_S" docker run --rm $extra "$IMAGE" /test/test_danger "$name"
    local code=$?

    if [[ $code -eq 124 ]]; then
        printf "  %-40s FAIL (timed out)\n" "$name"
        ((fail++))
    elif [[ $code -eq 0 ]]; then
        printf "  %-40s PASS\n" "$name"
        ((pass++))
    else
        printf "  %-40s FAIL (exit %d)\n" "$name" "$code"
        ((fail++))
    fi
}

# -------------------------------------------------------------------------
# Blacklist (no special capabilities needed)
# -------------------------------------------------------------------------

echo ""
echo "=== Shell blacklist ==="
run_check "blacklist_rm_rf"  ""
run_check "blacklist_rm_fr"  ""
run_check "blacklist_rm_fr_ws"  ""
run_check "blacklist_rm_slash_fr"  ""
run_check "blacklist_dd_dev" ""
run_check "blacklist_mkfs"   ""

# -------------------------------------------------------------------------
# Shutdown (CAP_SYS_BOOT + --init so PID 1 is tini, not the test binary)
# -------------------------------------------------------------------------

echo ""
echo "=== Shutdown ==="
BOOT="--cap-add SYS_BOOT --init"
run_check    "shutdown_invalid"  ""
run_terminal "shutdown_poweroff" "$BOOT" "any"
run_terminal "shutdown_halt"     "$BOOT" "any"
run_terminal "shutdown_reboot"   "$BOOT" "any"

# -------------------------------------------------------------------------
# Force kill
# -------------------------------------------------------------------------

echo ""
echo "=== Force kill ==="
run_check    "forcekill_inval"       ""
# --init: test binary must not be PID 1 so kill(-1,SIGKILL) can reach it
run_terminal "forcekill_exit"        "--init" 1
run_terminal "forcekill_unforgivable" "--init" 139
run_terminal "forcekill_abort"       "--init" 134
run_terminal "forcekill_sigkill"     "--init --privileged" 137
# sysrq: unprivileged — /proc/sysrq-trigger not writable, must return ERR_FAIL
run_check    "forcekill_sysrq"       ""

# -------------------------------------------------------------------------
# Summary
# -------------------------------------------------------------------------

echo ""
echo "Results: $pass passed, $fail failed"
[[ $fail -eq 0 ]] && exit 0 || exit 1
