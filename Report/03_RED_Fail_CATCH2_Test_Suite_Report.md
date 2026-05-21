# RED Catch2 테스트 스위트 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-RED-002 |
| **문서 유형** | RED 단계 테스트 스위트·실행 결과 보고서 |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **작성** | 시니어 C++ QA |
| **프로젝트** | UnitConverter (C++17, CMake, Catch2 v3.5.4) |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../README.md](../README.md), [01_RED_QA_Test_Plan_Report.md](01_RED_QA_Test_Plan_Report.md) (RPT-QA-001), [02_RED_Todo_Checklist_Report.md](02_RED_Todo_Checklist_Report.md) (RPT-RED-001), [../docs/defect_list.md](../docs/defect_list.md) |
| **관련 산출물** | `tests/domain_tests.cpp`, `tests/boundary_tests.cpp`, `tests/data_tests.cpp`, `tests/red_phase_stubs.cpp`, `CMakeLists.txt` |
| **선행 보고서** | [03_RED_CATCH2_Test_Implementation_Report.md](03_RED_CATCH2_Test_Implementation_Report.md) (RPT-IMPL-001, GREEN 50건 기준) |

---

## 요약 (Executive Summary)

본 보고서는 **RED 단계 전용** Catch2 테스트 스위트 작성 결과를 정리한다. 구현(`src/`) 변경 없이 계약을 고정하는 테스트만 추가·정비하였으며, CMake 옵션 `UNIT_CONVERTER_RED_PHASE=ON`(기본값)으로 **링크 스텁**을 사용해 의도적 실패를 보장한다.

| 구분 | 결과 |
|------|------|
| 테스트 케이스 | **45건** (`[red]` 태그) |
| 빌드 | `cmake -S . -B build` · `cmake --build build` — **성공** |
| 실행 (RED) | `unit_converter_tests.exe` — **44 failed, 1 passed** (의도적 RED) |
| 카테고리별 최소 5건 | 정상·경계·예외·동적등록·설정로드 — **각 5건 이상 충족** |
| Dual-Track | Domain 20건 · Boundary 15건 · Data 10건 |
| `src/` 구현 | **미수정** (GREEN 금지 준수) |

> **RPT-IMPL-001과의 관계**: 03번 보고서는 GREEN(50/50 PASS) 기준이다. 본 보고서는 동일 PRD 계약을 **실패하는 테스트**로 고정한 RED 스프린트 산출물이다. GREEN 전환 시 `-DUNIT_CONVERTER_RED_PHASE=OFF`로 `src/*.cpp`를 링크한다.

---

## 1. 요청 범위 대비 충족 여부

| 요청 항목 | 요구 | RED 구현 |
|-----------|------|----------|
| 정상 변환 | meter→feet, meter→yard, feet→meter(역변환) | Domain 5건 + Boundary 파싱·연계 5건 |
| 경계값 | value=0, 매우 큰 수, 소수 6자리 | Domain 5건 + Boundary 5건 |
| 예외 | 잘못된 형식, 음수, 없는 단위 | Domain 5건 + Boundary 5건 (ERR-INPUT prefix) |
| 동적 등록 | registerUnit 후 변환 | Domain 5건 (`cubit`, `inch`) |
| 설정 로드 | JSON/YAML 정상·실패 | Data 10건 (JSON 5 + YAML 5) |
| 형식 | Catch2, GWT 주석, 비율 명시, `test_[타입]_[조건]_[기대결과]` | 전 테스트 공통 |
| RED 빌드 | configure·build 성공, **테스트 실패** | 2026-05-21 확인 |
| 구현 금지 | GREEN/REFACTOR/`src/` 수정 없음 | `src/` 미변경, 스텁은 `tests/red_phase_stubs.cpp`만 |

---

## 2. RED 아키텍처

### 2.1 디렉터리·빌드 스위치

```text
UnitConverter_08/
├── CMakeLists.txt
│   └── UNIT_CONVERTER_RED_PHASE=ON  → tests/red_phase_stubs.cpp
│   └── UNIT_CONVERTER_RED_PHASE=OFF → src/domain|boundary|data/*.cpp
├── include/                    # API 계약 (변경 없음)
├── src/                        # GREEN 구현 (RED 스프린트에서 미사용)
├── tests/
│   ├── domain_tests.cpp        # 20건 [ratio|quantity|register][domain][red]
│   ├── boundary_tests.cpp      # 15건 [parse][boundary][red]
│   ├── data_tests.cpp          # 10건 [data][json|yaml][red]
│   ├── red_phase_stubs.cpp     # RED 링크 전용 (미구현 동작)
│   └── fixtures/               # units_valid/invalid JSON·YAML
└── docs/defect_list.md         # DEF-001~008 결함 추적
```

### 2.2 RED 스텁 동작 (`red_phase_stubs.cpp`)

| API | RED 스텁 동작 | 실패 유도 테스트 예 |
|-----|---------------|---------------------|
| `Converter::convert` | 항상 `0.0` 반환 | `test_meter_to_feet_normal_returns_8_2021` |
| `InputParser::parse` | `{"", 0.0}` 반환 | `test_parse_meter_colon_value_success` |
| `UnitRegistry` | `hasUnit` → false, `registerUnit` no-op | `test_register_has_unit_after_registration` |
| `ConfigLoader::load*` | `success=false`, `ERR-DATA-001` | `test_config_json_valid_loads_ratios` |
| `nearlyEqual` | 항상 `false` | 부동소수 비교 전반 |

> 스텁은 **프로덕션 구현이 아님**. GREEN 단계에서 `UNIT_CONVERTER_RED_PHASE=OFF` 시 컴파일 대상에서 제외된다.

### 2.3 환산 불변식 (테스트 주석 공통)

| 비율 | 검증 예 |
|------|---------|
| 1 meter = 3.28084 feet | `meter:2.5` → **8.202100** feet (ε 1e-5) |
| 1 meter = 1.09361 yard | `meter:2.5` → **2.734025** yard |
| 1 feet = 1/3.28084 meter | `feet:1.0` → **0.304800** meter (역변환) |
| 1 cubit = 0.4572 meter | `registerUnit` 후 cubit→meter **0.457200** |
| 1 inch = 0.0254 meter | 100 inch → **2.540000** meter |

```text
meterHub = sourceValue / factor(source)
targetValue = meterHub × factor(target)
```

---

## 3. 테스트 케이스 목록 (45건)

### 3.1 Domain — 정상 변환 (5건)

| 테스트명 | Given-When-Then 요약 | 기대 |
|----------|----------------------|------|
| `test_meter_to_feet_normal_returns_8_2021` | 2.5 m, 1 m = 3.28084 ft | 8.202100 ft |
| `test_meter_to_yard_normal_returns_2_7340` | 2.5 m, 1 m = 1.09361 yd | 2.734025 yd |
| `test_feet_to_meter_reverse_returns_0_3048` | 1 ft → m 역변환 | 0.304800 m |
| `test_meter_to_feet_one_meter_returns_3_28084` | 1 m → ft | 3.28084 ft |
| `test_feet_to_yard_via_meter_hub_returns_1_09361` | 3.28084 ft → yd (허브) | 1.09361 yd |

### 3.2 Domain — 경계값 (5건)

| 테스트명 | 조건 | 기대 |
|----------|------|------|
| `test_meter_to_feet_zero_value_throws` | value=0 | `std::invalid_argument` |
| `test_meter_to_feet_large_value_finite` | 1e100 | finite, ratio preserved |
| `test_meter_to_feet_six_decimal_precision` | 1.123456 | ε 1e-6 |
| `test_meter_to_feet_min_positive_value` | 1e-10 | finite > 0 |
| `test_meter_to_feet_nan_value_throws` | NaN | throw |

### 3.3 Domain — 예외 (5건)

| 테스트명 | 조건 | 기대 |
|----------|------|------|
| `test_convert_negative_value_throws` | -1.0 | throw |
| `test_convert_unknown_source_unit_throws` | parsec | Unknown unit |
| `test_convert_unknown_target_unit_throws` | mile | throw |
| `test_register_unit_duplicate_throws` | cubit 중복 | throw |
| `test_register_unit_invalid_ratio_throws` | ratio 0 | throw |

### 3.4 Domain — 동적 등록 (5건)

| 테스트명 | 조건 | 기대 |
|----------|------|------|
| `test_register_cubit_then_convert_to_meter` | cubit 0.4572 | 0.457200 m |
| `test_register_cubit_convert_all_count_four` | +cubit | 4 entries |
| `test_register_cubit_to_feet_via_meter` | cubit→feet | 0.4572×3.28084 ft |
| `test_register_inch_small_ratio` | inch 0.0254 | 100 inch→2.54 m |
| `test_register_has_unit_after_registration` | hasUnit cubit | true, count 4 |

### 3.5 Boundary — 파싱·연계 (5건)

| 테스트명 | 입력 | 기대 |
|----------|------|------|
| `test_parse_meter_colon_value_success` | meter:2.5 | unit/value DTO |
| `test_parse_feet_colon_value_success` | feet:3.28084 | 파싱 성공 |
| `test_parse_yard_colon_value_success` | yard:1.09361 | 파싱 성공 |
| `test_parse_then_meter_to_feet_chain` | parse+convert | 8.202100 ft |
| `test_parse_trim_whitespace_success` | trim 입력 | meter, 2.5 |

### 3.6 Boundary — 경계·예외 (10건)

| 테스트명 | 조건 | 기대 |
|----------|------|------|
| `test_parse_meter_zero_throws_positive_required` | meter:0 | **ERR-INPUT-003** prefix |
| `test_parse_meter_large_value_success` | 1e50 | 값 보존 |
| `test_parse_meter_six_decimal_precision` | 1.123456 | 정밀 파싱 |
| `test_parse_min_positive_value` | 1e-10 | success |
| `test_parse_meter_infinity_throws` | meter:inf | throw |
| `test_parse_invalid_format_no_colon_throws` | meter2.5 | **ERR-INPUT-001** |
| `test_parse_meter_negative_throws` | meter:-1.0 | **ERR-INPUT-003** |
| `test_parse_invalid_number_abc_throws` | meter:abc | **ERR-INPUT-004** |
| `test_convert_unknown_unit_parsec_throws` | parsec:1.0 | **ERR-INPUT-002** |
| `test_convert_unknown_unit_mile_throws` | mile:1 | **ERR-INPUT-002** |

### 3.7 Data — JSON/YAML (10건)

| 테스트명 | 형식 | 기대 |
|----------|------|------|
| `test_config_json_valid_loads_ratios` | valid JSON | success, 3 units |
| `test_config_json_valid_convert_meter_to_yard` | valid JSON | 1.09361 yd |
| `test_config_json_missing_file_fails_data001` | 없는 파일 | ERR-DATA-001 + default |
| `test_config_json_invalid_schema_fails_data002` | invalid schema | ERR-DATA-002 |
| `test_config_json_feet_to_meter_reverse_after_load` | valid JSON | 0.304800 m |
| `test_config_yaml_valid_loads_ratios` | valid YAML | 8.202100 ft |
| `test_config_yaml_valid_meter_to_yard` | valid YAML | 2.734025 yd |
| `test_config_yaml_missing_file_fails` | 없는 yaml | ERR-DATA-001 |
| `test_config_yaml_invalid_ratio_fails_data002` | meter:0 | ERR-DATA-002 |
| `test_config_yaml_load_defaults_on_failure` | invalid yaml | default factors |

---

## 4. RED 체크리스트(RPT-RED-001) 매핑

### Track A — Boundary

| ID | RED 테스트 | 비고 |
|----|------------|------|
| TC-A-01 | `test_parse_then_meter_to_feet_chain` 등 | 파싱·연계 |
| TC-A-02 | `test_parse_invalid_format_no_colon_throws` | ERR-INPUT-001 prefix |
| TC-A-03 | `test_parse_meter_negative_throws` | ERR-INPUT-003 |
| TC-A-04 | `test_convert_unknown_unit_parsec_throws` | ERR-INPUT-002 |
| TC-A-05 | `test_parse_invalid_number_abc_throws` | ERR-INPUT-004 |
| TC-A-06 | — | CLI table 스냅샷 **미포함** (GREEN 후속) |
| TC-A-07 | `test_parse_meter_zero_throws_positive_required` | ERR-INPUT-003 |

### Track B — Domain / Data

| ID | RED 테스트 | 비고 |
|----|------------|------|
| TC-B-01 | `test_meter_to_feet_normal_returns_8_2021` | |
| TC-B-02 | `test_meter_to_yard_normal_returns_2_7340` | |
| TC-B-03 | `test_feet_to_meter_reverse_returns_0_3048` | |
| TC-B-04 | `test_register_cubit_convert_all_count_four` | convertAll 4건 |
| TC-B-05 | `test_register_cubit_then_convert_to_meter` 등 | |
| TC-B-06 | `test_config_json_valid_loads_ratios` 등 | |
| TC-B-07 | `test_config_yaml_load_defaults_on_failure` | fallback |

---

## 5. 빌드·실행 절차 및 결과

### 5.1 RED (기본)

```powershell
cmake -S . -B build -G Ninja -DUNIT_CONVERTER_RED_PHASE=ON
cmake --build build
.\build\unit_converter_tests.exe
```

### 5.2 GREEN 전환 (구현 단계 — 본 스프린트 범위 외)

```powershell
cmake -S . -B build -G Ninja -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

### 5.3 태그 필터

```powershell
.\build\unit_converter_tests.exe "[red]"
.\build\unit_converter_tests.exe "[ratio]"
.\build\unit_converter_tests.exe "[parse]"
.\build\unit_converter_tests.exe "[data]"
```

### 5.4 실행 로그 (2026-05-21, RED)

```text
test cases:  45 |  1 passed | 44 failed
assertions:  52 |  8 passed | 44 failed
```

**대표 실패 (의도적 RED)**

```text
test_meter_to_feet_normal_returns_8_2021
  REQUIRE( nearlyEqual(result, 8.202100, 1e-5) )
  with expansion: false    // result == 0.0 (stub)

test_parse_meter_colon_value_success
  REQUIRE( parsed.unit == "meter" )
  with expansion: "" == "meter"
```

**1건 통과 사유**: `test_config_yaml_missing_file_fails` 등 — 스텁이 `success=false`, `errorCode=ERR-DATA-001`을 반환하여 **실패 경로 assertion 일부**만 충족하는 경우. Registry `hasUnit` 등 후속 assertion은 RED에서 계속 실패한다.

---

## 6. 명명·주석 규칙

### 6.1 테스트명

```text
test_[변환타입]_[조건]_[기대결과]
예: test_meter_to_feet_normal_returns_8_2021
```

### 6.2 Given-When-Then 샘플

```cpp
TEST_CASE("test_meter_to_feet_normal_returns_8_2021", "[ratio][domain][red]") {
    // Given: 1 meter = 3.28084 feet, source value 2.5 meter
    // When: convert("meter", 2.5, "feet")
    // Then: 2.5 × 3.28084 = 8.202100 feet (ε = 1e-5)
    ...
}
```

### 6.3 PRD stderr prefix (Boundary)

```cpp
// Then: ERR-INPUT-001 prefix (PRD §5)
requireErrPrefix(ex, "ERR-INPUT-001");
```

GREEN 구현 시 `InputParser`·`UnitRegistry` throw 메시지에 prefix 부착 필요 ([defect_list.md](../docs/defect_list.md) DEF-003).

---

## 7. 결함·후속 (GREEN 로드맵)

| ID | 항목 | RED 상태 | GREEN 시 조치 |
|----|------|----------|---------------|
| DEF-003 | ERR-INPUT prefix | 테스트만 존재, **실패** | `InputParser.cpp` 메시지 prefix |
| DEF-004 | meter→feet 허브 | **실패** (0.0) | `Converter.cpp` 허브식 |
| DEF-005 | meter:0 거부 | **실패** (throw 없음) | 양수 검증 |
| — | `UNIT_CONVERTER_RED_PHASE=OFF` | — | `src/*.cpp` 링크 |
| — | CLI table T-I-01 | 미포함 | 통합 테스트 추가 |
| — | 커버리지 게이트 | 미측정 | gcov/lcov |

상세: [docs/defect_list.md](../docs/defect_list.md)

---

## 8. RED 완료 정의 (DoD) — 본 스프린트

| 항목 | 상태 |
|------|------|
| 아이템 타입별 최소 5개 Catch2 테스트 | [x] |
| Given-When-Then + 비율 주석 + 명명 규칙 | [x] |
| `cmake` configure·build 성공 | [x] |
| `unit_converter_tests` 실행 **RED** (미구현으로 실패) | [x] |
| `src/` GREEN 구현·REFACTOR | [ ] (의도적 제외) |
| [defect_list.md](../docs/defect_list.md) 연동 | [x] |

---

## 9. 승인

| 역할 | 이름 | 서명 | 일자 |
|------|------|------|------|
| QA 리드 | | | |
| 개발 리드 | | | |

---

*본 보고서는 RED 스프린트(`UNIT_CONVERTER_RED_PHASE=ON`) 기준이다. GREEN 결과는 [03_RED_CATCH2_Test_Implementation_Report.md](03_RED_CATCH2_Test_Implementation_Report.md)를 참조한다. 계약 변경 시 PRD §3.2·§5와 동일 PR에서 갱신한다.*
