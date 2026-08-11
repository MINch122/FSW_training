#ifndef THRUST_MSG_H
#define THRUST_MSG_H

/**
 * @file
 * THRUST App Message Top-Level Include
 *
 * 외부 파일이 THRUST 메시지 관련 타입을 사용할 때
 * 이 파일 하나만 include 하면 모두 접근 가능하다.
 *
 *   thrust_msg.h
 *     ├─ thrust_mission_cfg.h  (공개 설정)
 *     ├─ thrust_msgdefs.h      (페이로드 struct)
 *     └─ thrust_msgstruct.h    (완전한 SB 메시지 struct)
 */
#include "thrust_mission_cfg.h"
#include "thrust_msgdefs.h"
#include "thrust_msgstruct.h"

#endif /* THRUST_MSG_H */
