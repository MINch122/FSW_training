#!/bin/bash
# submodule_init.sh
#
# WSL2에서 git submodule update 후 워킹 트리가 비어있는 문제를 해결합니다.
# git이 객체를 fetch하지만 체크아웃이 실패하는 경우 이 스크립트를 실행하세요.
#
# 사용법:
#   cd FSW_Baseline
#   git submodule update
#   bash script/submodule_init.sh

set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "[1/2] 서브모듈 업데이트..."
git -C "$REPO_ROOT" submodule update

echo "[2/2] 각 서브모듈 워킹 트리 복원 (WSL2 체크아웃 문제 해결)..."
git -C "$REPO_ROOT" submodule foreach 'git checkout HEAD -- . && echo "  OK: $name"'

echo ""
echo "완료. 현재 서브모듈 상태:"
git -C "$REPO_ROOT" submodule status
