# Catch2 테스트 구현 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-IMPL-001 |
| **문서 유형** | 테스트 구현·실행 결과 보고서 (Test Implementation Report) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **작성** | 시니어 C++ QA |
| **프로젝트** | UnitConverter (C++17, CMake, Catch2 v3.5.4) |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../README.md](../README.md), [RED_QA_Test_Plan_Report.md](01_RED_QA_Test_Plan_Report.md) (RPT-QA-001), [RED_Todo_Checklist_Report.md](02_RED_Todo_Checklist_Report.md) (RPT-RED-001) |
| **관련 산출물** | `tests/`, `CMakeLists.txt`, `include/`, `src/` |

---

## 요약 (Executive Summary)

본 보고서는 사용자 요청 범위(**아이템 타입별 최소 5개 Catch2 테스트**, Given-When-Then 주석, 변환 비율 명시, `test_[타입]_[조건]_[기대결과]` 명명)에 따라 작성·구현된 테스트 스위트와 **빌드·실행 결과**를 정리한다.

| 구분 | 결과 |
|------|------|
| 테스트 케이스 | **50건** (assertion 72개) |
| 빌드 | `cmake -S . -B build` · `cmake --build build` — **성공** |
| 실행 | `unit_converter_tests.exe` — **All tests passed (GREEN)** |
| 카테고리별 최소 5건 | 정상 변환·경계값·예외·동적 등록·설정 로드 — **각 5건 이상 충족** |
| Dual-Track | Domain 22건 · Boundary 16건 · Data 12건 |

> **단계 참고**: RED 체크리스트(RPT-RED-001)는 “실패 테스트만 커밋”을 전제로 한다. 본 스프린트는 **테스트 + Domain/Boundary/Data 최소 구현**을 함께 반영하여 GREEN 상태이다. RED 전용 브랜치가 필요하면 구현을 스텁화하거나 assertion을 `[!mayfail]`·의도적 실패 기대로 분리하는 후속 작업이 필요하다.

---

## 1. 요청 범위 대비 충족 여부

| 요청 항목 | 요구 | 구현·검증 |
|-----------|------|-----------|
| 정상 변환 | meter→feet, meter→yard, feet→meter(역변환) | Domain 6건 + Boundary 파싱·연계 5건 |
| 경계값 | value=0, 매우 큰 수, 소수 6자리 | Domain 6건 + Boundary 5건 |
| 예외 | 잘못된 형식, 음수, 없는 단위 | Domain 5건 + Boundary 6건 |
| 동적 등록 | registerUnit 후 변환 | Domain 5건 (`cubit`, `inch` 포함) |
| 설정 로드 | JSON/YAML 정상·실패 | Data 12건 (JSON 6 + YAML 5 + dispatch 1) |
| 형식 | Catch2, GWT 주석, 비율 명시, 명명 규칙 | 전 테스트 공통 적용 |
| 빌드 Green | `cmake -S . -B build && cmake --build build` | 2026-05-21 로컬 빌드 성공 확인 |

---

## 2. 테스트 아키텍처

### 2.1 디렉터리·타깃

```text
UnitConverter_08/
├── CMakeLists.txt              # FetchContent Catch2, core lib, tests, CTest
├── include/
│   ├── domain/                 # UnitRegistry, Converter
│   ├── boundary/               # InputParser
│   └── data/                   # ConfigLoader (JSON/YAML)
├── src/                        # 위 헤더 구현
├── tests/
│   ├── domain_tests.cpp        # [ratio][domain][quantity][register]
│   ├── boundary_tests.cpp      # [parse][boundary]
│   ├── data_tests.cpp          # [data][json][yaml]
│   └── fixtures/               # units_valid/invalid JSON·YAML
├── config/units.json           # 기본 비율 스냅샷
└── build/                      # 생성물 (unit_converter_tests.exe)
```

### 2.2 Catch2 태그 매핑

| 태그 | 파일 | 용도 |
|------|------|------|
| `[ratio]` `[domain]` | domain_tests.cpp | 환산 정확도·meter 허브 |
| `[quantity]` `[domain]` | domain_tests.cpp | 0·NaN·Inf·대수·6자리 |
| `[register]` `[domain]` | domain_tests.cpp | 동적 단위 등록 |
| `[parse]` `[boundary]` | boundary_tests.cpp | `단위:값` 파싱 계약 |
| `[data]` `[json]` `[yaml]` | data_tests.cpp | 설정 로드 성공·실패 |

### 2.3 환산 불변식 (테스트 주석 공통)

| 비율 | 값 | 검증 예 |
|------|-----|---------|
| 1 meter = 3.28084 feet | hub factor `feet` = 3.28084 | `meter:2.5` → feet **8.2021** (ε 1e-5) |
| 1 meter = 1.09361 yard | hub factor `yard` = 1.09361 | `meter:2.5` → yard **2.734025** |
| 1 feet = 1/3.28084 meter | 역변환 | `feet:1.0` → meter **≈ 0.304799** |
| feet↔yard | **meter 허브만** | 직접 교차 비율 저장·사용 금지 |
| Domain ε | **1e-9** (내부), 테스트 허용 **1e-5~1e-6** | `nearlyEqual()` |
| 1 cubit = 0.4572 meter | `registerUnit("cubit", 0.4572)` | `cubit:1` → meter 0.4572 |

**허브 환산식**

```text
meterHub = sourceValue / factor(source)
targetValue = meterHub × factor(target)
```

---

## 3. 테스트 케이스 목록 (50건)

### 3.1 정상 변환 (11건)

| 테스트명 | 태그 | Given-When-Then 요약 | 기대 |
|----------|------|----------------------|------|
| `test_meter_to_feet_normal_returns_8_2021` | ratio | 2.5 m, 1 m = 3.28084 ft | 8.2021 |
| `test_meter_to_yard_normal_returns_2_7340` | ratio | 2.5 m, 1 m = 1.09361 yd | 2.734025 |
| `test_feet_to_meter_reverse_returns_0_3048` | ratio | 1 ft → m 역변환 | ≈ 0.304799 |
| `test_meter_to_feet_unit_ratio_one_meter` | ratio | 1 m → ft | 3.28084 |
| `test_feet_to_yard_via_meter_hub` | ratio | 3.28084 ft → yd (허브) | ≈ 1.09361 yd |
| `test_convert_all_meter_returns_three_units` | ratio | convertAll 1 m | 3건, feet=3.28084 |
| `test_parse_meter_colon_value_success` | parse | `meter:2.5` | unit/value DTO |
| `test_parse_feet_colon_value_success` | parse | `feet:3.28084` | 파싱 성공 |
| `test_parse_yard_colon_value_success` | parse | `yard:1.09361` | 파싱 성공 |
| `test_parse_then_meter_to_feet_chain` | parse | parse + convert | 8.2021 ft |
| `test_parse_trim_whitespace_success` | parse | trim 입력 | meter, 2.5 |

### 3.2 경계값 (11건)

| 테스트명 | 입력/조건 | 기대 |
|----------|-----------|------|
| `test_meter_to_feet_zero_value_throws` | value = 0 | `invalid_argument` |
| `test_meter_to_feet_large_value_finite` | 1e100 | finite, 비율 유지 |
| `test_meter_to_feet_six_decimal_precision` | 1.123456 | 6자리 × 3.28084 |
| `test_meter_to_feet_min_positive_value` | 1e-10 | 양수 finite |
| `test_meter_to_feet_nan_value_throws` | NaN | 거부 |
| `test_meter_to_feet_infinity_value_throws` | +Inf | 거부 |
| `test_parse_meter_zero_throws_positive_required` | `meter:0` | 양수만 |
| `test_parse_meter_large_value_success` | 1e50 | 파싱 성공 |
| `test_parse_meter_six_decimal_precision` | 1.123456 | 정밀 보존 |
| `test_parse_min_positive_value` | 1e-10 | 파싱 성공 |
| `test_parse_meter_negative_throws` | `meter:-1` | 거부 |

### 3.3 예외 (11건)

| 테스트명 | 입력/조건 | 기대 메시지·동작 |
|----------|-----------|------------------|
| `test_convert_unknown_source_unit_throws` | parsec | Unknown unit |
| `test_convert_unknown_target_unit_throws` | mile | Unknown unit |
| `test_convert_negative_value_throws` | -1.0 | Value must be positive |
| `test_register_unit_duplicate_throws` | cubit 중복 | Unit already registered |
| `test_register_unit_invalid_ratio_throws` | ratio 0 | Invalid ratio |
| `test_parse_invalid_format_no_colon_throws` | meter2.5 | Invalid format |
| `test_parse_invalid_number_abc_throws` | meter:abc | Invalid number |
| `test_parse_empty_unit_name_throws` | :2.5 | Invalid unit name |
| `test_parse_double_colon_throws` | meter:2:5 | Invalid format |
| `test_convert_unknown_unit_parsec_throws` | parsec:1.0 | Unknown unit |
| `test_convert_unknown_unit_mile_throws` | mile:1 | Unknown unit |

### 3.4 동적 등록 (5건)

| 테스트명 | 조건 | 기대 |
|----------|------|------|
| `test_register_cubit_then_convert_to_meter` | cubit 0.4572 m | 1 cubit → 0.4572 m |
| `test_register_cubit_convert_all_count_four` | +cubit | convertAll 4건 |
| `test_register_cubit_to_feet_via_meter` | cubit→feet | 0.4572 × 3.28084 ft |
| `test_register_inch_small_ratio` | inch 0.0254 m | 100 inch → 2.54 m |
| `test_register_has_unit_after_registration` | hasUnit | cubit 존재, count 4 |

### 3.5 설정 로드 JSON/YAML (12건)

| 테스트명 | Fixture / 조건 | 기대 |
|----------|------------------|------|
| `test_config_json_valid_loads_ratios` | units_valid.json | success, 3 units |
| `test_config_json_valid_convert_meter_to_yard` | valid JSON | 1.09361 yd |
| `test_config_json_missing_file_fails_data001` | 없는 경로 | ERR-DATA-001, 기본 비율 유지 |
| `test_config_json_invalid_schema_fails_data002` | base_unit mile | ERR-DATA-002 |
| `test_config_json_load_keeps_default_on_failure` | missing path | feet 3.28084, yard 1.09361 |
| `test_config_json_feet_to_meter_reverse_after_load` | valid JSON | 역변환 ≈ 0.304799 m |
| `test_config_yaml_valid_loads_ratios` | units_valid.yaml | 2.5 m → 8.2021 ft |
| `test_config_yaml_valid_meter_to_yard` | valid YAML | 2.734025 yd |
| `test_config_yaml_missing_file_fails` | 없는 yaml | ERR-DATA-001 |
| `test_config_yaml_invalid_ratio_fails_data002` | meter: 0 | ERR-DATA-002 |
| `test_config_yaml_load_defaults_on_failure` | invalid yaml | 기본 factor 유지 |
| `test_config_load_dispatch_json_format` | ConfigFormat::Json | load() 성공 |

**Fixture 경로**: `tests/fixtures/` — CMake `TEST_FIXTURE_DIR` 매크로로 테스트 바이너리에 주입.

---

## 4. RED 체크리스트(RPT-RED-001) 매핑

### Track A — Boundary

| ID | 상태 | 대응 테스트 |
|----|------|-------------|
| TC-A-01 | [x] | `test_parse_then_meter_to_feet_chain`, `test_parse_meter_colon_value_success` |
| TC-A-02 | [x] | `test_parse_invalid_format_no_colon_throws` |
| TC-A-03 | [x] | `test_parse_meter_negative_throws` |
| TC-A-04 | [x] | `test_convert_unknown_unit_parsec_throws` |
| TC-A-05 | [x] | `test_parse_invalid_number_abc_throws` |
| TC-A-06 | [ ] | table/CLI 출력 스냅샷 통합 테스트 **미구현** (Boundary 단위만) |
| TC-A-07 | [x] | `test_parse_meter_zero_throws_positive_required` |

### Track B — Domain / Data

| ID | 상태 | 대응 테스트 |
|----|------|-------------|
| TC-B-01 | [x] | `test_meter_to_feet_normal_returns_8_2021` |
| TC-B-02 | [x] | `test_meter_to_yard_normal_returns_2_7340` |
| TC-B-03 | [x] | `test_feet_to_meter_reverse_returns_0_3048` |
| TC-B-04 | [x] | `test_convert_all_meter_returns_three_units` |
| TC-B-05 | [x] | `test_register_cubit_then_convert_to_meter` 등 5건 |
| TC-B-06 | [x] | `test_config_json_valid_loads_ratios` 등 JSON 6건 |
| TC-B-07 | [x] | `test_config_json_load_keeps_default_on_failure` |

---

## 5. 빌드·실행 절차 및 결과

### 5.1 명령

```powershell
cmake -S . -B build
cmake --build build
.\build\unit_converter_tests.exe
ctest --test-dir build --output-on-failure
```

### 5.2 태그 필터 예

```powershell
.\build\unit_converter_tests.exe "[ratio]"
.\build\unit_converter_tests.exe "[parse]"
.\build\unit_converter_tests.exe "[data]"
```

### 5.3 실행 로그 (2026-05-21)

```text
Randomness seeded to: 1912929417
===============================================================================
All tests passed (72 assertions in 50 test cases)
```

---

## 6. 명명·주석 규칙 (준수 샘플)

### 6.1 테스트명

```text
test_[변환타입]_[조건]_[기대결과]
예: test_meter_to_feet_normal_returns_8_2021
```

### 6.2 Given-When-Then 주석 (예: domain_tests.cpp)

```cpp
TEST_CASE("test_meter_to_feet_normal_returns_8_2021", "[ratio][domain]") {
    // Given: 1 meter = 3.28084 feet, source 2.5 meter
    // When: convert meter → feet
    // Then: 2.5 × 3.28084 = 8.2021 (ε = 1e-9)
    ...
}
```

---

## 7. 미구현·후속 작업

| 항목 | 우선순위 | 설명 |
|------|----------|------|
| T-I-01 통합 테스트 | P0 | stdin `meter:2.5` → table 3줄, stderr空, exit 0 |
| ERR-INPUT prefix 스냅샷 | P1 | PRD §3.2 F-05 prefix 문자열·exit code (현재 `std::invalid_argument` what()) |
| table / JSON / CSV Renderer | P1 | T-B-05~07, AC-04 |
| gcov/lcov 커버리지 게이트 | P1 | Domain ≥95%, Boundary ≥85% 측정·`coverage_v1/` 산출 |
| RED 전용 브랜치 | P2 | 구현 제거·의도적 실패만 커밋 시 RPT-RED-001 완료 처리 |
| defect_list.md | P2 | 결함 추적 문서 생성 |

---

## 8. 승인

| 역할 | 이름 | 서명 | 일자 |
|------|------|------|------|
| QA 리드 | | | |
| 개발 리드 | | | |

---

*본 보고서는 `tests/*.cpp` 및 `cmake --build build` 실행 결과를 기준으로 작성되었으며, 계약 변경 시 [PRD.md](../docs/PRD.md) §3.2·§3.3과 동일 PR에서 갱신한다.*
