# PRD — 확장 가능한 C++ 단위 변환 학습 시스템

**문서 버전**: 1.0  
**기준**: Phase 4 Epic · User Journey · User Stories (US-01~06) · Gherkin Feature  
**범위**: README 기본·품질·추가 요구 및 Dual-Track/BCE 설계 정본과 정합

---

# 1. 프로젝트 개요

## 1.1 한 줄 목적문 (What / Who / Why)

**What**: meter 허브 기준 길이 단위 변환 CLI와 검증 가능한 계약·테스트 체계  
**Who**: C++와 클린 아키텍처(OCP/SRP, BCE)를 학습하는 개발자  
**Why**: 단위·비율·출력·설정 변경 시에도 **회귀 없이** 구조를 확장하는 실습 역량을 확보하기 위해

## 1.2 배경 및 문제 정의 (관찰 관점)

초기 산출물은 단일 진입점에 `meter` / `feet` / `yard` 분기와 stdout 출력이 결합되어 있다. 사용자는 `단위:값` 한 줄을 입력하면 등록된 모든 길이 단위로 환산 결과를 받는다. 단위가 하나 늘거나 비율이 바뀌거나 출력이 JSON·CSV로 바뀔 때마다 **동일 파일의 분기·출력 문자열을 함께 수정**해야 하며, 자동화된 계약 테스트가 없으면 “돌아가는 것 같음”만으로 변경을 승인하게 된다. 실습 목표는 상용 제품 출시가 아니라, **Domain(환산·등록)과 Boundary(파싱·직렬화·에러 문구)와 Data(비율 로드)를 분리**하고 Catch2로 불변식을 고정하는 것이다.

## 1.3 목표 (측정 가능)

| ID | 목표 | 측정 |
|----|------|------|
| G-01 | meter 기준 환산 정확도 고정 | Domain 테스트: 1m→3.28084ft, 1m→1.09361yd, ε=1e-9 이내 GREEN |
| G-02 | 입력·미지원·형식 오류 선행 거부 | US-01 AC 전부 GREEN; Gherkin 형식·unknown 시나리오 GREEN |
| G-03 | 단위 추가 시 변환 핵심 변경 최소화 | 신규 단위 1개 추가 PR에서 환산 알고리즘 핵심 diff 0줄(등록·설정·테스트만) |
| G-04 | 출력 3포맷 동일 의미 보장 | table·JSON·CSV 각각 Catch2 ≥3건; 동일 입력 시 target (unit,value) 집합 동일 |
| G-05 | 설정·동적 등록·실패 경로 검증 | US-05·US-06 AC 및 Data 실패 테스트 GREEN |

## 1.4 비목표 (Non-Goal)

| ID | 비목표 | 이유 |
|----|--------|------|
| NG-01 | GUI·웹 API·다중 사용자 서비스 | 콘솔 학습 범위 초과 |
| NG-02 | 온도·무게 등 비길이 차원 변환 | 도메인 범위 고정(meter 허브 길이만) |
| NG-03 | 6시간 워크숍 회고·발표·AI 설문을 PRD 인수 조건으로 포함 | 저장소 밖 활동; 본 PRD 인수는 **저장소 검증 가능 항목만** |

---

# 2. 사용자 및 이해관계자

## 2.1 타깃 사용자 (페르소나 1명)

| 항목 | 내용 |
|------|------|
| 이름(가칭) | 김학습 |
| 역할 | C++ 실무·전환 개발자, 사내/부트캠프 실습 참가 |
| 기술 | C++ 기초, 테스트 경험 0~1년, `if/else` 유지보수 부담 인지 |
| 목표 | 단위 추가·비율 변경·포맷 추가 시 **테스트가 허용하는 리팩터** 수행 |
| 성공 정의 | 저장소에서 Catch2 전부 GREEN + Level 5 체크리스트 미통과 0행 |

**이해관계자 (참고, 인수 권한 없음)**: 강사(요구 해석), 동료 리뷰어(구조·OCP 검증)

## 2.2 주요 사용 시나리오 (Phase 4 Journey 기반)

### 시나리오 A — 기본 변환 (Journey 1→2→6)

1. 학습자가 빌드·실행 후 `meter:2.5` 입력  
2. 시스템이 Registry의 모든 단위로 환산  
3. table 기본 포맷으로 3줄 출력, stderr 비어 있음, exit 0  

### 시나리오 B — 계약 실패 학습 (Journey 2→4)

1. `meter2.5` 또는 `mile:1` 입력  
2. Boundary가 형식 또는 unknown 판별  
3. 고정 ERR prefix stderr 출력, stdout 변환 줄 0, exit 1  

### 시나리오 C — 확장 (Journey 5→6)

1. JSON 설정 로드(기본 3단위) 또는 `register:cubit=0.4572 meter`  
2. `cubit:1` 변환 후 JSON/CSV/table 중 선택 출력  
3. Domain 테스트·통합 테스트가 등록·비율·포맷 회귀 없음을 확인  

---

# 3. 기능 요구사항

## 3.1 핵심 기능 목록

| 우선순위 | ID | 기능 | Story |
|----------|-----|------|-------|
| **필수** | F-01 | `단위:값` 파싱·형식 검증 | US-01 |
| **필수** | F-02 | 양수 유한 값만 허용(0·음수·NaN·Inf 거부) | US-01 |
| **필수** | F-03 | Registry 기반 전 단위 환산(ConvertAll) | US-03 |
| **필수** | F-04 | meter/feet/yard 기본 Registry | US-03, Gherkin Background |
| **필수** | F-05 | 미등록 단위 거부(부분 출력 없음) | US-01, GH-03 |
| **필수** | F-06 | Catch2 Domain·Boundary·통합 테스트 | Epic SC |
| **권장** | F-07 | table / JSON / CSV 출력 선택 | US-04 |
| **권장** | F-08 | JSON 설정 파일 로드·검증 | US-05 |
| **권장** | F-09 | 런타임 동적 단위 등록 | US-06 |
| **선택** | F-10 | 등록 결과 JSON 파일 저장·재시작 로드 | US-06 확장 |

## 3.2 기능별 입·출력 계약 (문자열·스키마)

### F-01 / F-02 — 변환 입력

| 항목 | 계약 |
|------|------|
| 입력 채널 | stdin 1행 또는 동등 단일 라인 인자(구현 선택, 동작 동일) |
| 형식 | `{unit_token}:{value_token}` — 콜론 정확히 1개 |
| unit_token | trim 후 길이 1~32, 정규식 `^[a-zA-Z][a-zA-Z0-9_]*$` |
| value_token | 유한 실수 파싱 성공, **value > 0** |
| 성공 DTO | `source_unit`, `source_value` (double) |

### F-03 / F-04 — 변환 출력(공통 의미)

| 항목 | 계약 |
|------|------|
| 출력 집합 | Registry에 등록된 **모든** `target_unit` 각 1건 |
| 각 건 의미 | `source_value` + `source_unit` → `target_value` + `target_unit` |
| Domain 정밀도 | 비교 시 ε = **1e-9** |
| 표시 정밀도 | **HALF_UP, 소수 4자리** (Boundary 전용, Domain ε와 분리) |

### F-05 — 오류 출력

| error_code | stderr prefix (고정) | exit |
|------------|----------------------|------|
| ERR-INPUT-001 | `Invalid format. Use unit:value (ex: meter:2.5)` | 1 |
| ERR-INPUT-002 | `Unknown unit: {unit_token}` | 1 |
| ERR-INPUT-003 | `Value must be positive: {value_token}` | 1 |
| ERR-INPUT-004 | `Invalid number: {value_token}` | 1 |
| ERR-INPUT-005 | `Invalid unit name: {unit_token}` | 1 |

성공 시: **stderr 길이 0**, exit **0**.

### F-09 — 동적 등록 입력

| 항목 | 계약 |
|------|------|
| 형식 | `register:{unit_token}={ratio} meter` |
| ratio | 유한 실수, **ratio > 0** (의미: 1 `{unit_token}` = ratio meter) |
| 성공 | Registry에 unit 추가, 이후 F-03 대상에 포함 |
| 실패 | `Unit already registered: {unit_token}` / `Invalid ratio for unit: {unit_token}` |

## 3.3 제약 사항 (Gherkin Background 정합)

| 제약 | 규칙 | Gherkin 대응 |
|------|------|--------------|
| 기준 단위 | **meter** 단일 허브 | Background: base unit "meter" |
| 기본 비율 | 1 meter = 1.0 meter; 1 meter = 3.28084 feet; 1 meter = 1.09361 yard | Background Given 3줄 |
| feet↔yard | **meter 경유만**; 독립 교차 비율 저장 금지 | feet/yard는 meter 기반 계산 |
| 음수 정책 | **거부** (0 포함) | `meter:-1` → ERR-INPUT-003 |
| 미지원 단위 | **거부**, stdout 변환 줄 0 | `mile:1` → Unknown unit: mile |
| 입력 형식 | 콜론 필수 | `meter2.5` → ERR-INPUT-001 |
| 표시 반올림 | HALF_UP 4자리 | `2.5 meter = 8.2021 feet`, `2.5 meter = 2.7340 yard` |

**Gherkin Happy Path 고정 기대 (table, `meter:2.5`)**

- 줄 수 = 등록 단위 수(기본 3)  
- `2.5 meter = 2.5 meter`  
- `2.5 meter = 8.2021 feet`  
- `2.5 meter = 2.7340 yard`  

---

# 4. 비기능 요구사항

## 4.1 기술 스택

| 항목 | 버전·도구 | 용도 |
|------|-----------|------|
| 언어 | **C++17** | Domain·Boundary·Control 구현 |
| 빌드 | **CMake** | 실행 파일·Catch2 테스트 타깃 분리 |
| 테스트 | **Catch2** | RED→GREEN, 태그 `[ratio]` `[parse]` `[json]` 등 |

## 4.2 아키텍처 원칙

| 원칙 | 요구 | 검증 |
|------|------|------|
| **SRP** | Entity=환산·등록; Boundary=파싱·렌더·에러문구; Data=스냅샷 I/O; Control=유스케이스 순서 | Entity 파일에 iostream·파일 API 없음 |
| **OCP** | 신규 단위=등록·설정·테스트 추가; 변환 그래프 핵심 수정 0 | PR diff 규칙 |
| **BCE** | Boundary→Control→Entity; Data→Entity 포트 | 역방향 include 금지 |
| **Dual-Track** | Domain 테스트는 I/O 없음; Boundary는 Domain Mock | 태그·Mock 테스트 ≥1 |

## 4.3 테스트 커버리지 목표

| 레이어 | Line | Branch | 추가 조건 |
|--------|------|--------|-----------|
| Domain | ≥ 95% | ≥ 90% | Domain 실패 코드 8종 각 ≥1 테스트 |
| Boundary | ≥ 85% | ≥ 80% | ERR-INPUT-001~005 각 prefix 테스트 |
| Data | ≥ 90% | ≥ 85% | 로드 성공·실패 fixture 전부 |
| Control | ≥ 80% | ≥ 75% | US-01~06 유스케이스 각 1 통합 경로 |
| 전체 | ≥ 85% | — | Domain 미달 시 인수 불가 |

## 4.4 확장성 원칙

1. 새 `unit_token` 추가 시 **RatioToBase(meter 기준)** 등록만으로 ConvertAll 대상 자동 포함  
2. 출력 포맷 추가 시 **Renderer 추가**; ConversionResult 구조 변경 없음  
3. 비율 변경 시 **설정 파일 + Domain ratio 테스트 + Gherkin Background 문서** 동시 갱신(단독 변경 금지)  

---

# 5. 데이터 요구사항

## 5.1 단위 비율 상수 (meter 허브)

| unit_token | meters per 1 unit (1 unit = k meter) | 출처 |
|------------|--------------------------------------|------|
| meter | 1.0 | 기준 |
| feet | 3.28084 | README·Gherkin Background |
| yard | 1.09361 | README·Gherkin Background |

환산: `value_in_meter = source_value × k(source)`; `target_value = value_in_meter / k(target)` (Domain ε 검증).

## 5.2 설정 외부화

| 항목 | 규칙 |
|------|------|
| 권장 형식 | **JSON** (YAML은 선택 호환) |
| 최소 스키마 | `base_unit` 문자열 `"meter"`; `units` 객체: `{ "meter":1.0, "feet":3.28084, "yard":1.09361 }` |
| 로드 실패 | ERR-DATA-001(파일/파싱), ERR-DATA-002(스키마·ratio), exit 1, 변환 미실행 |
| 테스트 | InMemory Store를 테스트 더블로 사용 가능; **인수는 File 로드 경로 포함** |

## 5.3 동적 단위 등록 계약

| 항목 | 값 |
|------|-----|
| 입력 예 | `register:cubit=0.4572 meter` |
| 의미 | 1 cubit = 0.4572 meter |
| 사후 조건 | `cubit:1` → meter 표시값 0.4572 (ε); ConvertAll에 cubit 키 존재 |
| 금지 | 동일 unit_token 중복 등록; ratio ≤ 0 |

---

# 6. 출력 요구사항

## 6.1 콘솔 기본 포맷 (Table)

| 항목 | 규칙 |
|------|------|
| 기본 모드 | table (명시 없을 때) |
| 1줄 패턴 | `{source_value} {source_unit} = {target_value} {target_unit}` |
| 줄 순서 | Registry `unit_token` 사전순(구현 고정 후 문서화, **테스트로 고정**) |
| target_value | **소수 4자리**, HALF_UP |
| 줄 수 | = Registry 등록 수 |

## 6.2 JSON (권장)

| 필드 | 타입 | 규칙 |
|------|------|------|
| format_version | string | `"1"` 고정 |
| source.unit | string | 입력 unit_token |
| source.value | number | 입력 value (반올림 전) |
| conversions[] | array | 길이 = Registry 수 |
| conversions[i].unit | string | |
| conversions[i].value | number | **4자리** HALF_UP |

## 6.3 CSV (권장)

| 항목 | 규칙 |
|------|------|
| 1행 | `source_unit,source_value,target_unit,target_value` (exact) |
| 데이터 행 | 등록 단위 수만큼 |
| target_value | 4자리 HALF_UP |

## 6.4 Table / JSON / CSV 동형성

동일 입력·동일 Registry에서 **(target_unit, round4(target_value)) 집합**이 세 포맷에서 **완전 일치** (순서는 table/CSV 정렬 규칙 따름, 집합은 동일).

---

# 7. 성공 지표

## 7.1 인수 기준 (체크박스, Phase 4 Story 정합)

- [ ] **AC-01 (US-01, GH-02, GH-03)**: `meter:2.5` 성공 stderr空; `meter2.5`→ERR-INPUT-001; `mile:1`→`Unknown unit: mile`; `meter:0`·`meter:-1`→ERR-INPUT-003; exit 코드 일치  
- [ ] **AC-02 (US-03, GH-01)**: Gherkin Happy 3줄 수치 일치(2.5/8.2021/2.7340); Domain ε 테스트 GREEN  
- [ ] **AC-03 (US-02)**: cubit 등록 후 ConvertAll N+1건; 환산 핵심 파일 diff 0줄  
- [ ] **AC-04 (US-04)**: table·JSON·CSV 각 Catch2 ≥3 GREEN; 포맷 변경 시 Domain 테스트 diff 0  
- [ ] **AC-05 (US-05, US-06)**: JSON 로드·깨진 설정·`register:cubit=0.4572 meter`→`cubit:1` 통합 GREEN  

## 7.2 회귀 보호 규칙 (계약 변경 금지)

| 규칙 ID | 내용 | 위반 시 |
|---------|------|---------|
| R-01 | 필수 테스트 세트(T-D-01~06, T-B-05~07, T-I-01, Gherkin 3시나리오) CI 또는 pre-merge 스크립트 **전부 GREEN** | merge 금지 |
| R-02 | Background 비율·Gherkin 기대 수치 변경 시 **JSON 설정 + Domain 테스트 + PRD §5.1** 동시 변경 | 단독 PR 거부 |
| R-03 | ERR-* stderr **prefix** 변경 시 의도된 스냅샷 테스트 업데이트만 허용 | 리뷰 2인 승인 |
| R-04 | Entity→stdout·파일 직접 의존 | 인수 불가 |
| R-05 | 신규 unit_token 추가 시 ConvertAll 기대 **건수 +1** 테스트 갱신 필수 | 누락 시 인수 불가 |

---

# 8. 용어 정의 (Glossary)

| 용어 | 정의 |
|------|------|
| **meter 허브** | 모든 환산이 meter 값을 중간 표현으로 거치는 단일 기준 체계 |
| **UnitId / unit_token** | 단위 식별 문자열; 파싱·Registry 키로 사용 |
| **RatioToBase** | 1 unit = k meter를 나타내는 양수 유한 실수 k |
| **Registry** | UnitId↔RatioToBase 등록 집합; ConvertAll의 대상·출력 완전성 기준 |
| **LengthQuantity** | 유효한 (UnitId, value), value>0且 finite |
| **ConversionResult** | 한 번의 ConvertAll이 반환하는 (target_unit, target_value) 목록 |
| **Boundary** | 파싱·표시 반올림·ERR prefix·table/JSON/CSV 직렬화 담당 레이어 |
| **Control** | 입력 해석→Domain/Data 호출→출력 포맷 선택 오케스트레이션 |
| **ε (epsilon)** | Domain 수치 비교 허용 오차 **1e-9** |
| **round4** | 표시·JSON·CSV·table용 **HALF_UP 소수 4자리** (Domain ε와 별개) |
| **Dual-Track TDD** | Domain RED/GREEN과 Boundary(Mock) 계약 테스트를 병렬·독립 유지하는 방식 |

---

## 부록 — Phase 4 추적 매트릭스 (요약)

| PRD 절 | Phase 4 산출물 |
|--------|----------------|
| §1, §7 | Epic SC-01~06 (저장소 검증 가능분) |
| §2.2 | Journey 7단계 A/B/C |
| §3, §7.1 | US-01~06 Acceptance Criteria |
| §3.3, §6.1 | Gherkin Background + 3 Scenario |
| §7.2 | Level 5 회귀 체크리스트 R-01~R-05 |

---

*본 PRD는 구현·클래스·빌드 스크립트 본문을 포함하지 않는다. README 예시(8.2 feet)와 Gherkin(8.2021 feet) 표시 차이는 **round4 정책(§3.3, §6)** 을 우선한다.*
