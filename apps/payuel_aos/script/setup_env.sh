#!/bin/bash
# =============================================================================
# setup_env.sh
#
# FSW_Baseline 빌드 환경 설치 스크립트
# Ubuntu/Debian 계열 리눅스에서 실행하세요.
#
# 사용법:
#   cd FSW_Baseline
#   bash script/setup_env.sh
#
# 포함 항목:
#   - cFS 빌드 필수 패키지
#   - ARM 크로스 컴파일러
#   - Python3 / pyenv (CSP waf 빌드용)
#   - cspbuild.sh / gpiodbuild.sh 의존성
#   - Ground System 의존성 (PyQt5, PyZMQ)
# =============================================================================

set -e

echo "======================================"
echo " FSW_Baseline 빌드 환경 설치 시작"
echo "======================================"

# ──────────────────────────────────────────
# 헬퍼: dpkg 잠금 해제
#   unattended-upgrades 등 백그라운드 프로세스가
#   dpkg 잠금을 점유할 경우 해당 프로세스를 종료
# ──────────────────────────────────────────
release_dpkg_lock() {
    if sudo fuser /var/lib/dpkg/lock-frontend &>/dev/null; then
        echo "  dpkg 잠금 감지 — 점유 프로세스 종료 중..."
        sudo kill -9 $(sudo fuser /var/lib/dpkg/lock-frontend 2>/dev/null) 2>/dev/null || true
        sudo rm -f /var/lib/dpkg/lock-frontend /var/lib/dpkg/lock /var/cache/apt/archives/lock
        sudo dpkg --configure -a
    fi
}

# ──────────────────────────────────────────
# 1. 시스템 패키지 업데이트
#    Chrome 등 서드파티 저장소 GPG 오류는
#    빌드와 무관하므로 경고만 출력하고 계속 진행
# ──────────────────────────────────────────
echo ""
echo "[1/6] 시스템 패키지 업데이트..."
release_dpkg_lock
sudo apt-get update || echo "  [경고] 일부 저장소 업데이트 실패 (빌드에 영향 없음, 계속 진행)"

# ──────────────────────────────────────────
# 2. cFS 빌드 필수 패키지
#    cmake, make, gcc/g++  : cFS CMake 빌드
#    gdb                   : execute.sh 실행
#    libtool/autoconf 계열 : 서브모듈 빌드
#    libssl-dev            : 암호화 라이브러리
#    lcov                  : 코드 커버리지
# ──────────────────────────────────────────
echo ""
echo "[2/6] cFS 빌드 필수 패키지 설치..."
release_dpkg_lock
sudo apt-get install -y \
    cmake \
    build-essential \
    make \
    gcc \
    g++ \
    git \
    gdb \
    libtool \
    autotools-dev \
    autoconf \
    automake \
    pkg-config \
    libssl-dev \
    lcov

# ──────────────────────────────────────────
# 3. ARM 크로스 컴파일러
#    cspbuild.sh / gpiodbuild.sh에서
#    OBC(ARM) 타깃 크로스 컴파일에 사용
#    (실제 툴체인은 submodules/toolchain에 포함)
# ──────────────────────────────────────────
echo ""
echo "[3/6] ARM 크로스 컴파일러 설치..."
release_dpkg_lock
sudo apt-get install -y \
    gcc-arm-linux-gnueabi \
    g++-arm-linux-gnueabi \
    binutils-arm-linux-gnueabi

# ──────────────────────────────────────────
# 4. Python3 / pyenv
#    cspbuild.sh는 waf 빌드시스템을 사용하며
#    pyenv로 관리되는 Python 3.10.13 버전을 요구함
# ──────────────────────────────────────────
echo ""
echo "[4/6] Python3 및 관련 패키지 설치..."
release_dpkg_lock
sudo apt-get install -y \
    python3 \
    python3-pip \
    python3-dev \
    python3-venv \
    python3-setuptools

# pyenv가 없으면 설치
if ! command -v pyenv &> /dev/null; then
    echo ""
    echo "  pyenv가 설치되어 있지 않습니다. 설치를 진행합니다..."

    # pyenv 빌드 의존성
    release_dpkg_lock
    sudo apt-get install -y \
        curl \
        libffi-dev \
        libbz2-dev \
        libreadline-dev \
        libsqlite3-dev \
        libncursesw5-dev \
        xz-utils \
        tk-dev \
        libxml2-dev \
        libxmlsec1-dev

    curl https://pyenv.run | bash

    # 현재 쉘에 pyenv 적용 (이번 스크립트 실행 한정)
    export PATH="$HOME/.pyenv/bin:$PATH"
    eval "$(pyenv init --path)"
    eval "$(pyenv init -)"

    echo ""
    echo "  [주의] 터미널 재시작 후 pyenv를 사용하려면 ~/.bashrc에 아래 내용을 추가하세요:"
    echo '    export PATH="$HOME/.pyenv/bin:$PATH"'
    echo '    eval "$(pyenv init --path)"'
    echo '    eval "$(pyenv init -)"'
else
    echo "  pyenv 이미 설치됨: $(pyenv --version)"
fi

# Python 3.10.13 설치 (-s: 이미 설치된 경우 건너뜀)
if command -v pyenv &> /dev/null; then
    echo ""
    echo "  Python 3.10.13 설치 중 (이미 설치된 경우 건너뜀)..."
    pyenv install -s 3.10.13
fi

# ──────────────────────────────────────────
# 5. cspbuild.sh / gpiodbuild.sh 의존성
#
#    [cspbuild.sh]
#    libsocketcan-dev : CSP CAN 인터페이스 (호스트 x86 빌드)
#    can-utils        : CAN 유틸리티 (선택)
#
#    [gpiodbuild.sh]
#    autopoint        : libgpiod/autogen.sh 내부 호출
#    gettext          : autopoint 의존성
# ──────────────────────────────────────────
echo ""
echo "[5/6] cspbuild.sh / gpiodbuild.sh 의존성 설치..."
release_dpkg_lock
sudo apt-get install -y \
    libsocketcan-dev \
    can-utils \
    autopoint \
    gettext

# ──────────────────────────────────────────
# 6. Ground System 의존성
#    tools/cFS-GroundSystem/GroundSystem.py 실행에 필요
#    PyQt5            : GUI 프레임워크
#    pyzmq            : 텔레메트리 소켓 통신
#    libcanberra      : GTK 사운드 모듈 (경고 억제용)
# ──────────────────────────────────────────
echo ""
echo "[6/6] Ground System 의존성 설치..."
release_dpkg_lock
sudo apt-get install -y \
    python3-pyqt5 \
    libcanberra-gtk-module

pip3 install --user pyzmq 2>/dev/null || true

# ──────────────────────────────────────────
# 완료 메시지
# ──────────────────────────────────────────
echo ""
echo "======================================"
echo " 설치 완료!"
echo "======================================"
echo ""
echo "다음 단계:"
echo "  1. 서브모듈 초기화:  git submodule update --init --recursive"
echo "  2. CSP 빌드:         ./script/cspbuild.sh"
echo "  3. GPIO 빌드:        ./script/gpiodbuild.sh  (필요 시)"
echo "  4. cFS 빌드:         make SIMULATION=native prep && make && make install"
echo "  5. 실행:             ./script/execute.sh"
echo ""
