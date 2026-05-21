# 결함 목록·분석 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-DEF-001 |
| **문서 유형** | 결함 추적·QA 분석 보고서 (Defect List Report) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **작성** | QA 리드 |
| **프로젝트** | UnitConverter (C++17, CMake, Catch2 v3.5.4) |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../docs/defect_list.md](../docs/defect_list.md), [../README.md](../README.md) |
| **연관 보고서** | [02_RED_Todo_Checklist_Report.md](02_RED_Todo_Checklist_Report.md) (RPT-RED-001), [03_RED_Fail_CATCH2_Test_Suite_Report.md](03_RED_Fail_CATCH2_Test_Suite_Report.md) (RPT-RED-002) |

---

## 요약 (Executive Summary)

본 보고서는 RED 스프린트·빌드·QA 분석 과정에서 발견된 결함 **18건(DEF-001~018)** 을 정리하고, 심각도·재현·수정 방향을 프로젝트 관점에서 요약한다. **운영 정본(마스터 테이블)** 은 [docs/defect_list.md](../docs/defect_list.md)이며, 본 문서는 리뷰·승인용 보고서이다.

| 구분 | 건수 | 비고 |
|------|------|------|
| **전체** | 18 | DEF-001 ~ DEF-018 |
| **Fixed** | 1 | DEF-001 (CMake 경로) |
| **Open** | 17 | RED 스텁·계약·환경 |
| **Critical** | 10 | 변환·파싱·설정 전면 오류 |
| **Major** | 5 | 경계·오류코드·환경 |
| **Minor** | 1 | ERR prefix |
| **Info** | 2 | RED 의도·문서 |

| 실행 기준 (2026-05-21) | 결과 |
|------------------------|------|
| 빌드 | `cmake -S . -B build && cmake --build build` — **성공** |
| 테스트 (RED) | **44 failed / 45** (`UNIT_CONVERTER_RED_PHASE=ON`) |
| README 결함 체크 | `defect_list.md` 기록 [x] · 회귀 통과 [ ] |

---

## 1. 결함 현황 대시보드

### 1.1 심각도별 분포

```text
Critical ██████████ 10
Major    █████       5
Minor    █           1
Info     ██          2
```

### 1.2 영역별 분류

| 영역 | ID | Open | Fixed |
|------|-----|------|-------|
| 빌드/환경 | DEF-001, 002, 017 | 2 | 1 |
| Domain 변환 | DEF-004, 005, 006, 016 | 4 | 0 |
| Boundary 파싱 | DEF-007, 008, 003, 009, 010 | 5 | 0 |
| 동적 등록 | DEF-011, 012 | 2 | 0 |
| Data 설정 | DEF-013, 014, 015 | 3 | 0 |
| 문서/계약 | DEF-018 | 1 | 0 |

---

## 2. 결함 테이블 (정본 요약)

> 전체 8열·상태 열: [docs/defect_list.md §결함 테이블](../docs/defect_list.md#결함-테이블)

| ID | Severity | 변환 타입 | 재현 절차 | 기대값 | 실제값 | 근본 원인 | 수정 요약 |
|----|----------|-----------|-----------|--------|--------|-----------|-----------|
| DEF-001 | Critical | 빌드/CMake | 클린 트리 `cmake -S . -B build` | 빌드 성공 | `Cannot find UnitConverter.cpp` | CMake 소스 경로 미갱신 | `src/UnitConverter.cpp` 경로 수정 |
| DEF-002 | Major | 환경/CMake | generator 변경 후 재configure | 성공 | Ninja vs MinGW 불일치 | `CMakeCache` 잔존 | `build/` 삭제 후 `-G Ninja` |
| DEF-003 | Minor | 계약/stderr | `parse("meter2.5")` | `ERR-INPUT-001`로 시작 | prefix 없음 | PRD §5 미반영 | throw에 `ERR-INPUT-00x ` 추가 |
| DEF-004 | Critical | meter→feet | `convert("meter",2.5,"feet")` | **8.202100** | **0.000000** | RED 스텁 `convert`→0 | GREEN 허브식 |
| DEF-005 | Critical | meter→yard | `convert("meter",2.5,"yard")` | **2.734025** | **0.000000** | 동일 스텁 | GREEN 허브식 |
| DEF-006 | Critical | feet→meter | `convert("feet",1.0,"meter")` | **0.304800** | **0.000000** | 역변환 미구현 | `Converter.cpp` 허브 |
| DEF-007 | Critical | 파싱/meter | `parse("meter:2.5")` | meter, 2.5 | `""`, 0.0 | 스텁 고정 반환 | `InputParser.cpp` 구현 |
| DEF-008 | Critical | 파싱 연계 | parse 후 feet 변환 | **8.202100** ft | **0.000000** | DEF-004+007 | 파서+Converter |
| DEF-009 | Major | 경계/value=0 | `convert("meter",0,"feet")` | throw | 0.0 반환 | 스텁 검증 없음 | `value<=0` throw |
| DEF-010 | Major | 경계/parse zero | `parse("meter:0")` | ERR-003+throw | throw 없음 | 스텁 | `InputParser` 양수 검증 |
| DEF-011 | Critical | 동적 등록 | `registerUnit("cubit",…)` | hasUnit true | false | no-op 스텁 | `UnitRegistry` 구현 |
| DEF-012 | Critical | cubit→meter | cubit→meter | **0.457200** | **0.000000** | 등록·환산 없음 | Registry+허브 |
| DEF-013 | Critical | 설정/JSON | `loadJson(valid)` | success | false | 스텁 항상 실패 | `ConfigLoader` JSON |
| DEF-014 | Major | 설정/schema | invalid schema | ERR-DATA-002 | ERR-DATA-001 | 코드 미구분 | 스키마 분기 |
| DEF-015 | Critical | 설정/YAML | `loadYaml(valid)` | success | false | 동일 스텁 | YAML 로드 |
| DEF-016 | Major | nearlyEqual | ε 비교 | true | 항상 false | 스텁 | `fabs(a-b)<=ε` |
| DEF-017 | Info | RED/CMake | 기본 RED 빌드 | 44 failed | 44 failed | RED 의도 | ON 유지 / GREEN 시 OFF |
| DEF-018 | Info | 문서 | README vs PRD | ERR prefix | 문서 불일치 | 이중 계약 | DEF-003 연동 정리 |

---

## 3. QA 분석 — 대표 결함 상세

### 3.1 DEF-004 — meter→feet (Critical)

| 항목 | 내용 |
|------|------|
| **기대/실제** | expected **8.202100**, got **0.000000** |
| **버그 위치** | `tests/red_phase_stubs.cpp:54` (RED) · GREEN 시 `src/domain/Converter.cpp:20-25` |
| **심각도 근거** | 핵심 변환 계약 전면 불일치 |
| **최소 수정** | Item/구조체 변경 없음. `meterHub = value/factor(from)`, `result = meterHub×factor(to)` |

```text
test_meter_to_feet_normal_returns_8_2021
  REQUIRE( nearlyEqual(result, 8.202100, 1e-5) )
  with expansion: false
```

### 3.2 DEF-007 — 파싱 (Critical)

| 항목 | 내용 |
|------|------|
| **기대/실제** | unit=`meter`, value=`2.5` → unit=`""`, value=`0.0` |
| **버그 위치** | `tests/red_phase_stubs.cpp:70` |
| **수정 요약** | `InputParser.cpp` — 콜론 분리, trim, `stod`, 양수 검증 |

### 3.3 DEF-003 — ERR prefix (Minor, GREEN 시)

| 항목 | 내용 |
|------|------|
| **기대/실제** | `ERR-INPUT-001` prefix → `Invalid format...` |
| **버그 위치** | `src/boundary/InputParser.cpp:35`, `src/domain/UnitRegistry.cpp:27` |
| **심각도 근거** | 수치는 맞을 수 있으나 PRD stderr 계약 불일치 |
| **최소 수정** | throw 문자열만 `"ERR-INPUT-001 Invalid format..."` 형태로 변경 |

### 3.4 DEF-001 — 빌드 (Fixed)

| 항목 | 내용 |
|------|------|
| **기대/실제** | configure 성공 → `Cannot find UnitConverter.cpp` |
| **버그 위치** | `CMakeLists.txt:36` (`src/UnitConverter.cpp`) |
| **상태** | **Fixed** |

---

## 4. RED vs GREEN 결함 해소 매트릭스

| ID | RED 원인 | GREEN 조치 | 구조체 변경 |
|----|----------|------------|-------------|
| DEF-004~006, 016 | `red_phase_stubs.cpp` | `-DUNIT_CONVERTER_RED_PHASE=OFF` → `src/domain/Converter.cpp` | 없음 |
| DEF-007~010, 003 | 스텁 / prefix | `src/boundary/InputParser.cpp` | 없음 |
| DEF-011~012 | 스텁 | `src/domain/UnitRegistry.cpp` | 없음 |
| DEF-013~015 | 스텁 | `src/data/ConfigLoader.cpp` | 없음 |
| DEF-017 | RED 의도 | 테스트 44 failed = RED 완료 신호 | — |

---

## 5. 재현·확인 절차

### 5.1 RED 확인 (현재)

```powershell
Set-Location c:\DEV\UnitConverter_08
cmake -S . -B build -G Ninja -DUNIT_CONVERTER_RED_PHASE=ON
cmake --build build
.\build\unit_converter_tests.exe
```

**기대:** `44 failed`, `1 passed`

### 5.2 GREEN 회귀 (결함 해소 후)

```powershell
cmake -S . -B build -G Ninja -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

**기대:** DEF-004~016 해소 시 **45/45 PASS** (DEF-003 prefix 포함 시 Boundary 6건 추가 검증)

---

## 6. RPT-RED-001 결함 체크리스트 연동

| 체크리스트 ID | 결함 ID | 상태 |
|---------------|---------|------|
| DEF-RED-01 `defect_list.md` 생성 | DEF-001~018 | [x] |
| DEF-RED-02 회귀 테스트 통과 | 전체 Open(RED) | [ ] |
| TC-B-01 meter→feet | DEF-004 | Open |
| TC-A-07 meter:0 | DEF-009, 010 | Open |
| TC-B-06/07 설정 로드 | DEF-013~015 | Open |
| ERR prefix (PRD §5) | DEF-003 | Open |

---

## 7. 후속 작업

| 우선순위 | 항목 | 담당 단계 |
|----------|------|-----------|
| P0 | `UNIT_CONVERTER_RED_PHASE=OFF` + Domain 허브 구현 | GREEN |
| P0 | InputParser·UnitRegistry ERR prefix | GREEN |
| P1 | ConfigLoader JSON/YAML | GREEN |
| P1 | CLI table 통합 테스트 (TC-A-06) | GREEN |
| P2 | `defect_list.md` 상태 Fixed/Closed 갱신 | QA |
| P2 | README 결함 체크 2번째 항목 [x] | QA |

---

## 8. 승인

| 역할 | 이름 | 서명 | 일자 |
|------|------|------|------|
| QA 리드 | | | |
| 개발 리드 | | | |

---

*결함 추가·상태 변경 시 [docs/defect_list.md](../docs/defect_list.md)를 먼저 갱신하고, 본 보고서 요약·대시보드를 동일 PR에서 맞춘다.*
