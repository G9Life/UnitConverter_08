# RED 단계 To-Do 체크리스트 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-RED-001 |
| **문서 유형** | RED 단계 진행 체크리스트 (Dual-Track TDD) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **프로젝트** | UnitConverter (C++17) |
| **정본 참조** | [../docs/test_plan.md](../docs/test_plan.md), [../docs/PRD.md](../docs/PRD.md), [../README.md](../README.md) |
| **연관 보고서** | [RED_QA_Test_Plan_Report.md](RED_QA_Test_Plan_Report.md) (RPT-QA-001) |

---

## 요약 (Executive Summary)

본 보고서는 **RED(실패 테스트 작성)** 단계에서 완료해야 할 항목을 Track A(Boundary/UI)와 Track B(Domain/Logic)로 나누어 체크리스트로 관리한다. 각 항목은 Catch2 테스트가 **의도적으로 실패**하는 상태로 커밋되었을 때 체크한다.

| 구분 | 항목 수 | RED 완료 조건 |
|------|---------|---------------|
| Track A — UI / Boundary | 7 | 파싱·에러·출력 계약 실패 테스트 존재 |
| Track B — Domain / Logic | 7 | 환산·Registry·설정 실패 테스트 존재 |
| 커버리지 목표 | 3 | 측정 스크립트·임계값 정의 (GREEN 후 충족) |
| 결함 목록 | 2 | `defect_list.md` 운영·회귀 확인 |

> **계약 정합 참고**: README 체크리스트는 `std::invalid_argument`를 명시하나, PRD v1.0 정본은 **ERR-INPUT-00x prefix + exit 1** 체계이다. RED 테스트 작성 시 Boundary 레이어는 PRD stderr 계약을 우선하고, Domain 레이어는 예외 없이 `Quantity`/Registry 거부로 검증한다.

---

## 1. RED 단계 정의

| 항목 | 내용 |
|------|------|
| **목표** | 구현 전 요구 계약을 **실패하는 테스트**로 고정 |
| **산출물** | Catch2 테스트 소스 + CMake/ctest 골격 |
| **금지** | GREEN 구현으로 테스트를 통과시키기 (RED 단계 종료 전) |
| **완료 신호** | `ctest` 실행 시 실패하며, 실패 원인이 “미구현”임이 명확함 |

---

## 2. Track A — UI / Boundary 테스트

Boundary·파싱·CLI 출력 계약. Mock Domain 또는 스텁으로 **I/O 계약만** 검증한다.

| ID | 상태 | 시나리오 | 입력 / 조건 | 기대 (RED 테스트 assertion) | PRD 매핑 |
|----|------|----------|-------------|------------------------------|----------|
| TC-A-01 | [ ] | Happy Path | `meter:2.5` | 변환 결과 반환(또는 table 3줄 DTO) | T-I-01, AC-01 |
| TC-A-02 | [ ] | 형식 오류 | `meter2.5` (`:` 없음) | `std::invalid_argument` 또는 ERR-INPUT-001, stdout 0줄, exit 1 | ERR-001 |
| TC-A-03 | [ ] | 음수 | `meter:-1.0` | 거부, ERR-INPUT-003 또는 동등 예외 | ERR-003 |
| TC-A-04 | [ ] | 미등록 단위 | `parsec:1.0` | 거부, ERR-INPUT-002 `Unknown unit: parsec` | ERR-002 |
| TC-A-05 | [ ] | 숫자 파싱 실패 | `meter:abc` | 거부, ERR-INPUT-004 | ERR-004 |
| TC-A-06 | [ ] | 출력 포맷 | `meter:2.5` 성공 시 | 원 입력 보존: `2.5 meter = ...` 패턴 | §6.1 table |
| TC-A-07 | [ ] | 영값 경계 | `meter:0` | 거부, 양수만 허용 (BV-01) | ERR-003 |

### Track A 진행 체크리스트

- [ ] TC-A-01: 정상 입력 "meter:2.5" → 변환 결과 반환 (Happy Path)
- [ ] TC-A-02: ":" 없는 입력 → std::invalid_argument 발생
- [ ] TC-A-03: 음수 입력 "meter:-1.0" → std::invalid_argument 발생
- [ ] TC-A-04: 없는 단위 "parsec:1.0" → std::invalid_argument 발생
- [ ] TC-A-05: 소수점 파싱 실패 "meter:abc" → std::invalid_argument 발생
- [ ] TC-A-06: 출력 포맷에 원 입력 단위·값 보존 ("2.5 meter = ...")
- [ ] TC-A-07: value=0 경계값 처리 확인

### Track A 권장 Catch2 태그

```text
[parse] [boundary] [error-contract] [table]
```

---

## 3. Track B — Domain / Logic 테스트

콘솔·stdout 없이 순수 환산·Registry·설정 로드만 검증한다.

| ID | 상태 | 시나리오 | 호출 / 조건 | 기대값 (ε = 1e-5 unless noted) | PRD 매핑 |
|----|------|----------|-------------|--------------------------------|----------|
| TC-B-01 | [ ] | meter → feet | `convert("meter", 2.5, "feet")` | `8.20210` | T-D-02, G-01 |
| TC-B-02 | [ ] | meter → yard | `convert("meter", 1.0, "yard")` | `1.09361` | T-D-01 |
| TC-B-03 | [ ] | 역변환 | `convert("feet", 1.0, "meter")` | `0.30480` | T-D-03 |
| TC-B-04 | [ ] | 전 단위 환산 | `convertAll("meter", 1.0)` | 등록 단위 수만큼 결과 반환 | T-D-04, F-03 |
| TC-B-05 | [ ] | 동적 등록 | `registerUnit("cubit", 0.4572)` 후 변환 | cubit 환산 가능 | F-09 |
| TC-B-06 | [ ] | 설정 로드 성공 | `loadConfig(유효 경로)` | JSON 비율 정상 반영 | US-05 |
| TC-B-07 | [ ] | 설정 로드 실패 | `loadConfig(없는 경로)` | 기본값 `3.28084` / `1.09361` 유지 | Data fallback |

### Track B 진행 체크리스트

- [ ] TC-B-01: convert("meter", 2.5, "feet") == 8.20210 (오차 1e-5)
- [ ] TC-B-02: convert("meter", 1.0, "yard") == 1.09361 (오차 1e-5)
- [ ] TC-B-03: convert("feet", 1.0, "meter") == 0.30480 (역변환)
- [ ] TC-B-04: convertAll("meter", 1.0) → 모든 등록 단위 변환 반환
- [ ] TC-B-05: registerUnit("cubit", 0.4572) 후 변환 가능
- [ ] TC-B-06: loadConfig(유효한 경로) → 비율 정상 로드
- [ ] TC-B-07: loadConfig(없는 경로) → 기본값(3.28084/1.09361) 유지

### Track B 권장 Catch2 태그

```text
[ratio] [domain] [registry] [quantity]
```

---

## 4. 커버리지 목표

RED 단계에서는 **측정 파이프라인 정의**까지 완료하고, 임계값 충족은 GREEN 이후에 검증한다.

| 목표 | 임계값 | 도구 | RED 단계 할 일 |
|------|--------|------|----------------|
| Domain Logic | Line **95%+** | gcov / lcov | `ENABLE_COVERAGE` CMake 옵션·리포트 경로 확정 |
| Boundary Layer | Line **85%+** | gcov / lcov | ERR 분기별 ≥1 테스트 매핑표 작성 |
| 전체 TOTAL | **90%+** | lcov 합산 | 레이어별 extract 스크립트 초안 |

### 커버리지 진행 체크리스트

- [ ] Domain Logic: 95%+ (# gcov / lcov)
- [ ] Boundary Layer: 85%+
- [ ] 전체 TOTAL: 90%+

상세 측정 절차는 [RED_QA_Test_Plan_Report.md](RED_QA_Test_Plan_Report.md) §8을 따른다.

---

## 5. 결함 목록 연결

RED 실행 중 발견된 **프로토타입 갭·계약 충돌**은 `defect_list.md`에 기록하고, GREEN 단계에서 수정·회귀한다.

| ID | 상태 | 활동 |
|----|------|------|
| DEF-RED-01 | [ ] | `defect_list.md` 생성 및 발견 결함 기록 |
| DEF-RED-02 | [ ] | 모든 결함 수정 후 회귀 테스트 통과 확인 |

### 결함 목록 진행 체크리스트

- [ ] defect_list.md 생성 및 발견 결함 기록
- [ ] 모든 결함 수정 후 회귀 테스트 통과 확인

**예상 초기 결함 (프로토타입 대비)**

| 결함 요약 | 관련 TC | 비고 |
|-----------|---------|------|
| `meter:0` 통과 | TC-A-07 | UnitConverter.cpp 갭 |
| round4 미적용 | TC-A-06 | HALF_UP 4자리 없음 |
| ERR prefix 미정 | TC-A-02~05 | if/else 메시지 비표준 |

---

## 6. Track A ↔ Track B ↔ PRD 추적 매트릭스

| TC ID | Track | PRD / QA ID | README RED To-Do |
|-------|-------|-------------|------------------|
| TC-A-01 | A | T-I-01 | ✓ |
| TC-A-02 | A | ERR-001, BV-05 | ✓ |
| TC-A-03 | A | ERR-003, BV-03 | ✓ |
| TC-A-04 | A | ERR-002, BV-06 | ✓ |
| TC-A-05 | A | ERR-004, BV-04 | ✓ |
| TC-A-06 | A | T-B-05, §6.1 | ✓ |
| TC-A-07 | A | BV-01 | ✓ |
| TC-B-01 | B | T-D-02 | ✓ |
| TC-B-02 | B | T-D-01 | ✓ |
| TC-B-03 | B | T-D-03 | ✓ |
| TC-B-04 | B | T-D-04 | ✓ |
| TC-B-05 | B | F-09 | ✓ |
| TC-B-06 | B | US-05 | ✓ |
| TC-B-07 | B | Data fallback | ✓ |

---

## 7. RED 완료 정의 (DoD)

아래 **전부** 체크 시 RED 단계 종료, GREEN 착수 가능.

### 7.1 테스트 존재·실패

- [ ] CMake + Catch2 + `ctest` 골격 동작
- [ ] Track A 7건 실패 테스트 커밋
- [ ] Track B 7건 실패 테스트 커밋
- [ ] 실패 원인이 “미구현 Domain/Boundary”로 명확

### 7.2 문서·정합

- [ ] README [RED 단계 To-Do 리스트](../README.md#red-단계-to-do-리스트)와 본 보고서 항목 일치
- [ ] PRD 수치(3.28084 / 1.09361)와 테스트 기대값 일치
- [ ] `defect_list.md` 최소 1건 이상 기록(프로토타입 갭)

### 7.3 다음 단계 (GREEN)

1. Domain T-D-01, T-D-02 GREEN → Track B TC-B-01~04 순차 통과  
2. Boundary 파서·ERR GREEN → Track A TC-A-01~07 순차 통과  
3. lcov Domain ≥95%, Boundary ≥85% → v1.0 인수 검토  

---

## 8. 진행 현황 요약

| 영역 | 완료 | 전체 | 진행률 |
|------|------|------|--------|
| Track A | 0 | 7 | 0% |
| Track B | 0 | 7 | 0% |
| 커버리지 목표 | 0 | 3 | 0% |
| 결함 목록 | 0 | 2 | 0% |
| **합계** | **0** | **19** | **0%** |

> 진행률은 체크리스트 항목 기준 수동 갱신. GREEN 전환 시 본 표를 매 스프린트마다 업데이트한다.

---

## 부록 A — 승인

| 역할 | 이름 | 서명 | 일자 |
|------|------|------|------|
| QA 리드 | | | |
| 개발 리드 | | | |
| PM / 강사 | | | |

---

*체크리스트 변경 시 [../README.md](../README.md) RED To-Do 섹션, [../docs/test_plan.md](../docs/test_plan.md), [RED_QA_Test_Plan_Report.md](RED_QA_Test_Plan_Report.md)를 동일 PR에서 갱신한다.*
