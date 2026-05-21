# GREEN Catch2 테스트 스위트 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-GREEN-001 |
| **문서 유형** | GREEN 단계 구현·실행 결과 보고서 |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **작성** | TDD GREEN 스프린트 |
| **프로젝트** | UnitConverter (C++17, CMake, Catch2 v3.5.4) |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../docs/TODO.md](../docs/TODO.md), [01_RED_QA_Test_Plan_Report.md](01_RED_QA_Test_Plan_Report.md) (RPT-QA-001), [02_RED_Todo_Checklist_Report.md](02_RED_Todo_Checklist_Report.md) (RPT-RED-001) |
| **선행 보고서** | [03_RED_Fail_CATCH2_Test_Suite_Report.md](03_RED_Fail_CATCH2_Test_Suite_Report.md) (RPT-RED-002) |
| **관련 산출물** | `src/domain/Converter.cpp`, `src/domain/UnitRegistry.cpp`, `src/boundary/InputParser.cpp`, `src/data/ConfigLoader.cpp` |

---

## 요약 (Executive Summary)

본 보고서는 **RED → GREEN** 전환 스프린트 결과를 정리한다. CMake 옵션 `UNIT_CONVERTER_RED_PHASE=OFF`로 프로덕션 소스(`src/`)를 링크하고, Catch2 **45건 전부 GREEN**을 확인하였다. **REFACTOR(이름 정리·구조 분리)는 수행하지 않았다.**

| 구분 | RED (`ON`) | GREEN (`OFF`) |
|------|------------|---------------|
| 빌드 | `cmake -S . -B build` · `cmake --build build` — **성공** | 동일 + `-DUNIT_CONVERTER_RED_PHASE=OFF` — **성공** |
| `ctest` | **44 failed, 1 passed** (의도적 RED) | **45 passed, 0 failed** |
| `src/` 변경 | 없음 (스텁만) | Domain·Boundary 최소 구현 |
| 비율 상수 | — | `kFeetPerMeter` / `kYardPerMeter` / `kDefaultFeetPerMeter` (인라인 `3.28084` 금지 준수) |

> **제안 커밋 메시지**: `feat(green): implement convert meter to feet (Approx 1e-5)` — 실제 diff는 meter 허브 `convert()`·ERR prefix·Registry 연동을 포함한다.

---

## 1. TDD GREEN 범위·금지 사항

| 항목 | 내용 |
|------|------|
| **단계** | TDD **GREEN만** (REFACTOR 금지) |
| **대상 테스트** | `tests/domain_tests.cpp`, `tests/boundary_tests.cpp`, `tests/data_tests.cpp` (총 45 Catch2) |
| **계획 정본** | `docs/TODO.md`, `docs/PRD.md` §5.1 비율·§3.2 ERR prefix |
| **금지** | REFACTOR(이름·폴더·클래스 분리), 비율 `3.28084` **인라인** 리터럴 |
| **허용** | `src/` 최소 프로덕션 코드로 기존 `[red]` 테스트 통과 |

---

## 2. 실행 절차 및 결과

### 2.1 RED 확인 (Step 1)

```powershell
cd c:\DEV\UnitConverter_08
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=ON
cmake --build build
cd build
ctest
```

| 지표 | 결과 |
|------|------|
| 테스트 총계 | 45건 |
| 실패 | **44건** |
| 통과 | 1건 (`test_config_yaml_missing_file_fails` — 실패 경로 기대) |
| TC-B-01 | **FAILED** (`convert` → `0.0`, `nearlyEqual` → `false`) |

### 2.2 GREEN 구현 (Step 2)

프로덕션 링크 전환:

```powershell
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
```

### 2.3 GREEN 확인 (Step 3)

```powershell
ctest
```

| 지표 | 결과 |
|------|------|
| 테스트 총계 | 45건 |
| 통과 | **45건 (100%)** |
| 실패 | **0건** |
| 실행 시간 | 약 24초 (환경 의존) |

---

## 3. 구현 변경 요약

### 3.1 Domain — `Converter::convert` (meter 허브)

**파일**: `src/domain/Converter.cpp`

- 양수·유한 값 검증 후 `toMeterHub` → `fromMeterHub` 경유
- `meter` → `feet` 포함 모든 등록 단위 쌍은 **Registry factor**로 환산 (직접 if/else 비율 하드코딩 없음)

```cpp
double Converter::convert(const std::string& fromUnit, double value, const std::string& toUnit) const {
    if (value <= 0.0 || !std::isfinite(value)) {
        throw std::invalid_argument("Value must be positive");
    }
    const double meterHub = toMeterHub(value, fromUnit);
    return fromMeterHub(meterHub, toUnit);
}
```

### 3.2 Domain — `UnitRegistry` 비율 상수

**파일**: `src/domain/UnitRegistry.cpp`

| 상수 | 값 | 용도 |
|------|-----|------|
| `kMeterFactor` | `1.0` | 기준 단위 |
| `kFeetPerMeter` | `3.28084` | 1 m = 3.28084 ft |
| `kYardPerMeter` | `1.09361` | 1 m = 1.09361 yd |

**파일**: `include/domain/Converter.hpp` — `kDefaultFeetPerMeter`, `kDefaultYardPerMeter`, `kDomainEpsilon`

### 3.3 Boundary — ERR-INPUT prefix (PRD §3.2)

**파일**: `src/boundary/InputParser.cpp`

| 코드 | throw 메시지 prefix |
|------|---------------------|
| ERR-INPUT-001 | `ERR-INPUT-001 Invalid format. Use unit:value (ex: meter:2.5)` |
| ERR-INPUT-003 | `ERR-INPUT-003 Value must be positive: {token}` |
| ERR-INPUT-004 | `ERR-INPUT-004 Invalid number: {token}` |
| ERR-INPUT-005 | `ERR-INPUT-005 Invalid unit name: {unit}` |

**파일**: `src/domain/UnitRegistry.cpp`

| 코드 | throw 메시지 prefix |
|------|---------------------|
| ERR-INPUT-002 | `ERR-INPUT-002 Unknown unit: {unit}` |

### 3.4 Data — `ConfigLoader`

**파일**: `src/data/ConfigLoader.cpp` — RED 스프린트 이전부터 구현됨. JSON/YAML fixture 로드·`ERR-DATA-001`/`002`·기본 Registry fallback 검증 **GREEN**.

### 3.5 미변경·의도적 유지

| 항목 | 상태 |
|------|------|
| `tests/red_phase_stubs.cpp` | RED 전용, GREEN 빌드에서 **미링크** |
| 테스트 소스 (`tests/*.cpp`) | **수정 없음** |
| REFACTOR | **미수행** (BCE 추가 분리·이름 정리 없음) |

---

## 4. Track A / Track B 체크리스트 매핑 (GREEN)

출처: [02_RED_Todo_Checklist_Report.md](02_RED_Todo_Checklist_Report.md)

### 4.1 Track A — Boundary (TC-A-01 ~ TC-A-07)

| ID | Catch2 대표 테스트 | GREEN |
|----|-------------------|-------|
| TC-A-01 | `test_parse_meter_colon_value_success`, `test_parse_then_meter_to_feet_chain` | [x] |
| TC-A-02 | `test_parse_invalid_format_no_colon_throws` | [x] |
| TC-A-03 | `test_parse_meter_negative_throws` | [x] |
| TC-A-04 | `test_convert_unknown_unit_parsec_throws` | [x] |
| TC-A-05 | `test_parse_invalid_number_abc_throws` | [x] |
| TC-A-06 | (table 출력 통합 — 본 스위트는 파싱·연계 위주) | [x] 파싱·convert 연계 |
| TC-A-07 | `test_parse_meter_zero_throws_positive_required` | [x] |

### 4.2 Track B — Domain / Data (TC-B-01 ~ TC-B-07)

| ID | Catch2 대표 테스트 | 기대 | GREEN |
|----|-------------------|------|-------|
| TC-B-01 | `test_meter_to_feet_normal_returns_8_2021` | `8.20210` (ε=1e-5) | [x] |
| TC-B-02 | `test_meter_to_yard_normal_returns_2_7340` | `1.09361` / `2.734025` | [x] |
| TC-B-03 | `test_feet_to_meter_reverse_returns_0_3048` | `0.30480` | [x] |
| TC-B-04 | `test_register_cubit_convert_all_count_four` 등 | `convertAll` 건수 | [x] |
| TC-B-05 | `test_register_cubit_then_convert_to_meter` 등 | cubit 등록 후 환산 | [x] |
| TC-B-06 | `test_config_json_valid_loads_ratios` 등 | JSON 비율 로드 | [x] |
| TC-B-07 | `test_config_json_missing_file_fails_data001` 등 | 실패 시 기본 비율 | [x] |

---

## 5. Catch2 45건 실행 매트릭스 (GREEN)

| # | 레이어 | 테스트명 | 결과 |
|---|--------|----------|------|
| 1–10 | Data | `test_config_json_*`, `test_config_yaml_*` | PASS |
| 11–25 | Boundary | `test_parse_*`, `test_convert_unknown_*` | PASS |
| 26–45 | Domain | `test_meter_*`, `test_feet_*`, `test_register_*`, `test_convert_*` | PASS |

**실패 0건** — RPT-RED-002(44 failed) 대비 **전량 GREEN**.

---

## 6. 핵심 수치 검증 (PRD §3.3)

| 시나리오 | 입력 | Domain 기대 | Catch2 검증 |
|----------|------|-------------|-------------|
| meter → feet | `convert("meter", 2.5, "feet")` | `8.202100` (ε=1e-5) | `test_meter_to_feet_normal_returns_8_2021` |
| meter → feet (1m) | `convert("meter", 1.0, "feet")` | `3.28084` (ε=1e-9) | `test_meter_to_feet_one_meter_returns_3_28084` |
| meter → yard | `convert("meter", 2.5, "yard")` | `2.734025` | `test_meter_to_yard_normal_returns_2_7340` |
| feet → meter | `convert("feet", 1.0, "meter")` | `0.304800` | `test_feet_to_meter_reverse_returns_0_3048` |
| feet → yard (허브) | `convert("feet", 3.28084, "yard")` | `1.09361` | `test_feet_to_yard_via_meter_hub_returns_1_09361` |

---

## 7. RED 결함(DEF) 대비 GREEN 상태

| DEF ID | RED 증상 | GREEN 조치 | 상태 |
|--------|----------|------------|------|
| DEF-001 | `convert` 항상 0 | meter 허브 `convert` 구현 | **Closed** |
| DEF-002 | `nearlyEqual` 항상 false | `std::fabs` 비교 구현 | **Closed** |
| DEF-003 | ERR-INPUT prefix 없음 | InputParser·UnitRegistry throw prefix | **Closed** |
| DEF-010 | `meter:0` 미거부 | InputParser `value <= 0` 검증 | **Closed** |

상세: [04_RED_Defect_List_Report.md](04_RED_Defect_List_Report.md)

---

## 8. 빌드·회귀 명령 (README·PRD 정합)

```powershell
# GREEN 일상 빌드·테스트
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
cd build
ctest --output-on-failure

# RED 회귀 확인 (의도적 실패)
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=ON
cmake --build build
ctest
```

| CMake 옵션 | 링크 대상 | 용도 |
|------------|-----------|------|
| `UNIT_CONVERTER_RED_PHASE=ON` (기본) | `tests/red_phase_stubs.cpp` | RED·미구현 검증 |
| `UNIT_CONVERTER_RED_PHASE=OFF` | `src/domain`, `src/boundary`, `src/data` | GREEN·인수 |

---

## 9. 후속 작업 (본 스프린트 범위 외)

| 항목 | 상태 | 비고 |
|------|------|------|
| REFACTOR | **미착수** | 사용자 지시에 따라 이번 커밋 제외 |
| 커버리지 95%/85% | **미측정** | `ENABLE_COVERAGE`·lcov는 별도 스프린트 |
| CLI 통합·table round4 | **부분** | Domain/Boundary/Data 단위 GREEN; E2E `UnitConverter` main 별도 |
| `docs/TODO.md` Must-Have | **부분 GREEN** | 본 보고서는 Catch2 45건 기준 |

---

## 10. 결론

1. **RED**: `UNIT_CONVERTER_RED_PHASE=ON` — 45건 중 44건 실패, TC-B-01 포함 미구현 확인.  
2. **GREEN**: `UNIT_CONVERTER_RED_PHASE=OFF` — `src/` 최소 구현(환산·ERR prefix·Registry 상수)으로 **45/45 PASS**.  
3. **비율**: PRD §5.1 상수를 `constexpr`로 추출·사용, 변환 로직에 인라인 `3.28084` 없음.  
4. **REFACTOR**: 수행하지 않음.

---

*본 보고서는 GREEN 스프린트 실행 결과의 정본이며, 이후 REFACTOR·커버리지·E2E 인수는 별도 보고서로 추적한다.*
