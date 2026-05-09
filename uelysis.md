# BEE1012 Change Log

## 2026-04-24 14:21 KST
- `apps/sch_lab`에 지상국 명령 처리 경로를 추가했다.
- `apps/sch_lab/fsw/inc/` 아래에 `sch_lab_app.h`, `sch_lab_cmds.h`, `sch_lab_dispatch.h`, `sch_lab_eventids.h`를 추가했다.
- `apps/sch_lab/fsw/src/` 아래에 `sch_lab_cmds.c`, `sch_lab_dispatch.c`를 추가하고, 기존 `sch_lab_app.c`를 새 명령/디스패치 구조에 맞게 연결했다.
- `sch_lab`용 command/config header를 추가했다.
  `default_sch_lab_fcncodes.h`, `default_sch_lab_msgids.h`, `default_sch_lab_msgstruct.h`, `default_sch_lab_topicids.h`
- `sch_lab` 테이블 포맷은 바꾸지 않고, 런타임 state에서 엔트리별 enable/disable이 가능하도록 구현했다.
  엔트리 index 기준으로 on/off 하며, disable은 테이블을 지우지 않고 "주석 처리한 것처럼" 송신만 막는다.
- `sch_lab` 빌드 설정을 갱신했다.
  `apps/sch_lab/CMakeLists.txt`, `mission_build.cmake`, `arch_build.cmake`
- 검증:
  기존 generated include를 사용해 `sch_lab_app.c`, `sch_lab_cmds.c`, `sch_lab_dispatch.c` 소스 단위 컴파일을 통과했다.
- 참고:
  현재 전체 `make prep`는 mission config/serial-config 입력 파일 경로 불일치로 별도 환경 이슈가 있다.

