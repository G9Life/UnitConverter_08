# REFACTORING 최종 검증 보고서 — 전체 테스트·커버리지·인수 기준

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-VERIFY-001 |
| **문서 유형** | REFACTORING 완료 후 최종 검증 (테스트·커버리지·체크리스트) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **프로젝트** | UnitConverter_08 (C++17, CMake, Catch2 v3.5.4) |
| **저장소 경로** | `c:\DEV\UnitConverter_08` |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md) §4.3, [../README.md](../README.md), [02_RED_Todo_Checklist_Report.md](02_RED_Todo_Checklist_Report.md) |
| **선행 보고서** | [05_GREEN_CATCH2_Test_Suite_Report.md](05_GREEN_CATCH2_Test_Suite_Report.md) (RPT-GREEN-001), [09_REFACTORING_OutputFormatter_DualTrack_Report.md](09_REFACTORING_OutputFormatter_DualTrack_Report.md) (RPT-REF-001) |
| **관련 산출물** | `build/`, `build-cov/coverage*.info` |

---

## 요약 (Executive Summary)

리팩토링 커밋 완료 후 **GREEN 빌드**(`UNIT_CONVERTER_RED_PHASE=OFF`) 기준으로 전체 테스트·Golden Master·구조·커버리지를 검증하였다.

| 구분 | 결과 |
|------|------|
| **CTest** | **52/52 PASS** (실패 0) |
| **Golden Master** | **PASS** — `[golden][regression][r01]` 4 cases, 12 assertions |
| **TC-A-01~07** | 전항목 **PASS** |
| **TC-B-01~07** | 전항목 **PASS** |
| **if-else 단위 분기** | **제거 완료** (`UnitRegistry` map + meter 허브) |
| **매직 넘버 인라인** | **준수** (`src/*.cpp` 변환 로직에 `3.28084`/`1.09361` 없음) |
| **Domain / Boundary 분리** | **완료** |
| **Domain Line 커버리지** | **95.0%** (목표 ≥ 95%) ✅ |
| **Boundary Line 커버리지** | **93.8%** (목표 ≥ 85%) ✅ |
| **v1.0 인수 (line 기준)** | **충족** |

> **종합 판정**: PRD §4.3 line 커버리지 게이트·Track A/B 체크리스트·Golden Master 출력 불변을 **모두 충족**. Branch 커버리지는 PRD 목표( Domain ≥90%, Boundary ≥80% ) 미달 — 후속 스프린트 권장.

---

## 1. 검증 환경

| 항목 | 값 |
|------|-----|
| OS | Windows 10 (26200) |
| 컴파일러 | GNU 15.2.0 (MinGW-w64) |
| 빌드 생성기 | Ninja |
| CMake | 3.16+ |
| Catch2 | v3.5.4 (FetchContent) |
| RED phase | **OFF** (`-DUNIT_CONVERTER_RED_PHASE=OFF`) |
| 빌드 타입 | Debug |

---

## 2. 빌드·테스트 실행

### 2.1 표준 빌드·테스트 (기능 검증)

```bash
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build -V
```

> **참고**: `cmake --build build ctest`처럼 인자를 붙이면 `ctest`가 빌드 타깃으로 해석되어 실패한다. `build`와 `ctest`는 **별도 명령**으로 실행한다.

### 2.2 실행 결과

| 항목 | 결과 |
|------|------|
| 빌드 | **성공** (`unit_converter_core`, `UnitConverter`, `unit_converter_tests`) |
| CTest | **52/52 PASS** |
| 실패 | **0** |
| 총 소요 시간 (real) | 약 **35초** |

### 2.3 Golden Master

| 항목 | 내용 |
|------|------|
| CTest 이름 | `GoldenMaster` |
| 필터 | `[golden][regression][r01]` |
| 케이스 수 | 4 (`UnitConverter_meter_2_5`, `feet_1_0`, `yard_1_0`, `meter_0_0`) |
| assertion | 12 |
| 결과 | **PASS** |
| 검증 내용 | CLI stdout이 `tests/golden_master_expected.txt`와 **불변** (출력 회귀 없음) |

---

## 3. Track A — Boundary 체크리스트 (TC-A-01 ~ TC-A-07)

| ID | 시나리오 | 대응 Catch2 테스트 | 결과 |
|----|----------|-------------------|------|
| TC-A-01 | Happy Path `meter:2.5` | `test_parse_meter_colon_value_success`, `test_parse_then_meter_to_feet_chain`, `UnitConverter_meter_2_5` | ✅ PASS |
| TC-A-02 | `:` 없음 → ERR-INPUT-001 | `test_parse_invalid_format_no_colon_throws` | ✅ PASS |
| TC-A-03 | 음수 → ERR-INPUT-003 | `test_parse_meter_negative_throws` | ✅ PASS |
| TC-A-04 | 미등록 단위 → ERR-INPUT-002 | `test_convert_unknown_unit_parsec_throws` | ✅ PASS |
| TC-A-05 | 숫자 파싱 실패 → ERR-INPUT-004 | `test_parse_invalid_number_abc_throws` | ✅ PASS |
| TC-A-06 | 출력 포맷 `값 단위 = 변환값 단위` | `test_output_format_conversion_line_meter_to_feet`, Golden Master | ✅ PASS |
| TC-A-07 | `meter:0` 거부 (양수만) | `test_parse_meter_zero_throws_positive_required`, `UnitConverter_meter_0_0` | ✅ PASS |

---

## 4. Track B — Domain / Data 체크리스트 (TC-B-01 ~ TC-B-07)

| ID | 시나리오 | 대응 Catch2 테스트 | 기대값 | 결과 |
|----|----------|-------------------|--------|------|
| TC-B-01 | meter → feet | `test_meter_to_feet_normal_returns_8_2021` | 8.20210 (ε=1e-5) | ✅ PASS |
| TC-B-02 | meter → yard | `test_meter_to_yard_normal_returns_2_7340` | 2.734025 | ✅ PASS |
| TC-B-03 | 역변환 feet → meter | `test_feet_to_meter_reverse_returns_0_3048` | 0.30480 | ✅ PASS |
| TC-B-04 | convertAll | `test_register_cubit_convert_all_count_four` 등 | 등록 단위 수 | ✅ PASS |
| TC-B-05 | 동적 등록 cubit | `test_register_cubit_then_convert_to_meter` 등 | 환산 가능 | ✅ PASS |
| TC-B-06 | 설정 로드 성공 | `test_config_json_valid_loads_ratios` 등 | JSON 비율 반영 | ✅ PASS |
| TC-B-07 | 설정 로드 실패 fallback | `test_config_json_missing_file_fails_data001`, `test_config_yaml_load_defaults_on_failure` | 기본 비율 유지 | ✅ PASS |

---

## 5. 리팩토링 구조 검증

### 5.1 Domain / Boundary / Control 분리

```text
┌─────────────────────────────────────────────────────────────┐
│ Control: src/UnitConverter.cpp (thin main)                    │
│   prompt → getline → parse → convertAll → writeConversionTable│
└─────────────────────────────────────────────────────────────┘
         │                    │                      │
         ▼                    ▼                      ▼
┌─────────────────┐  ┌──────────────────┐  ┌──────────────────────┐
│ Boundary        │  │ Domain           │  │ Boundary             │
│ InputParser     │  │ Converter        │  │ OutputFormatter      │
│ src/boundary/   │  │ UnitRegistry     │  │ src/boundary/        │
└─────────────────┘  └──────────────────┘  └──────────────────────┘
                              │
                              ▼
                     ┌──────────────────┐
                     │ Data             │
                     │ ConfigLoader     │
                     │ src/data/        │
                     └──────────────────┘
```

| 레이어 | 소스 | 역할 |
|--------|------|------|
| **Domain** | `src/domain/Converter.cpp`, `UnitRegistry.cpp` | meter 허브 변환·단위 등록 |
| **Boundary** | `src/boundary/InputParser.cpp`, `OutputFormatter.cpp` | 파싱·HALF_UP 4자리·stdout 테이블 |
| **Control** | `src/UnitConverter.cpp` | I/O 조합만 (변환·포맷 로직 없음) |
| **Data** | `src/data/ConfigLoader.cpp` | JSON/YAML 비율 로드 |

### 5.2 if-else 단위 분기 체인 제거

| 검증 | 결과 |
|------|------|
| `Converter.cpp`에 `if (fromUnit == "feet")` 등 단위별 분기 | **없음** — `toMeterHub` / `fromMeterHub` + `registry_.factorFor()` |
| `UnitRegistry` 단위 조회 | `std::map` lookup (`factorFor`) — if-else 체인 **아님** |
| 레거시 if-else 체인 | **UnitRegistry로 대체 완료** |

### 5.3 매직 넘버 3.28084 / 1.09361

| 위치 | 형태 | 판정 |
|------|------|------|
| `src/domain/UnitRegistry.cpp` | `constexpr kFeetPerMeter`, `kYardPerMeter` | ✅ 단일 출처 (named constant) |
| `src/domain/Converter.cpp` | 인라인 리터럴 **없음** | ✅ |
| `src/boundary/*.cpp` | 인라인 리터럴 **없음** | ✅ |
| `include/domain/Converter.hpp` | `kDefaultFeetPerMeter` 등 (헤더 상수) | ✅ 변환 로직 미사용·문서용 |
| `config/units.json`, 테스트 fixture | 설정·기대값 | ✅ 허용 (테스트·설정 출처) |

**판정**: 변환 로직(`src/domain`, `src/boundary`)에 **인라인 하드코딩 없음** — R-L3 준수.

---

## 6. 커버리지 (gcov / lcov)

### 6.1 커버리지 빌드·실행

```bash
cmake -S . -B build-cov -DUNIT_CONVERTER_RED_PHASE=OFF -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_CXX_FLAGS=--coverage -DCMAKE_EXE_LINKER_FLAGS=--coverage ^
  -DCMAKE_SHARED_LINKER_FLAGS=--coverage
cmake --build build-cov
ctest --test-dir build-cov --output-on-failure
lcov --capture --directory build-cov --output-file build-cov/coverage.info --rc lcov_branch_coverage=1
```

### 6.2 레이어별 Line 커버리지 (PRD §4.3 게이트)

| 레이어 | 대상 파일 | Line | 목표 | 판정 |
|--------|-----------|------|------|------|
| **Domain** | `Converter.cpp`, `UnitRegistry.cpp` | **95.0%** (57/60) | ≥ 95% | ✅ **달성** |
| **Boundary** | `InputParser.cpp`, `OutputFormatter.cpp` | **93.8%** (45/48) | ≥ 85% | ✅ **달성** |
| **Baseline** | `UnitConverter.cpp` | **93.3%** (14/15) | — | 참고 |

### 6.3 파일별 상세

| 파일 | Line | Functions |
|------|------|-----------|
| `src/domain/Converter.cpp` | **100%** (21/21) | 100% (6/6) |
| `src/domain/UnitRegistry.cpp` | **92.3%** (36/39) | 100% (9/9) |
| `src/boundary/InputParser.cpp` | (합산) | (합산) |
| `src/boundary/OutputFormatter.cpp` | (합산) | (합산) |
| Boundary 합계 | **93.8%** (45/48) | 100% (6/6) |

**Domain 미커버 추정**: `UnitRegistry::clear()`, `registerUnit` 빈 이름 예외 등 저빈도 경로 (약 3 line).

### 6.4 Branch 커버리지 (참고 — PRD 목표 미달)

| 레이어 | Branch | PRD 목표 |
|--------|--------|----------|
| Domain | **53.8%** (42/78) | ≥ 90% |
| Boundary | **53.6%** (59/110) | ≥ 80% |
| UnitConverter.cpp | **54.2%** (13/24) | — |

> Line 게이트는 **충족**. Branch는 예외·경계 분기 테스트 보강 후속 권장.

### 6.5 lcov 산출물

| 파일 | 설명 |
|------|------|
| `build-cov/coverage.info` | 전체 캡처 |
| `build-cov/coverage_domain.info` | Domain extract |
| `build-cov/coverage_boundary.info` | Boundary extract |
| `build-cov/coverage_unitconverter.info` | `UnitConverter.cpp` baseline |

`genhtml` HTML 리포트는 Windows 환경에서 Perl 런타임 제약으로 **미생성**. Linux/WSL CI에서 `Report/coverage_v1/`, `Report/coverage_baseline/` 생성 권장.

---

## 7. 인수 기준 대조표

| # | 확인 항목 | 결과 |
|---|-----------|------|
| 1 | TC-A-01 ~ TC-A-07 전부 PASS | ✅ |
| 2 | TC-B-01 ~ TC-B-07 전부 PASS | ✅ |
| 3 | Golden Master PASS (출력 불변) | ✅ |
| 4 | if-else 체인 제거 (`UnitRegistry` 교체) | ✅ |
| 5 | 매직 넘버 3.28084/1.09361 인라인 없음 | ✅ |
| 6 | Domain(변환) / Boundary(파싱·출력) 분리 | ✅ |
| 7 | Domain Line ≥ 95% | ✅ 95.0% |
| 8 | Boundary Line ≥ 85% | ✅ 93.8% |

---

## 8. CTest 전체 목록 (52건 요약)

| 구분 | 건수 | 비고 |
|------|------|------|
| Golden Master (CLI) | 4 | `UnitConverter_*` |
| Boundary | 16 | parse, format, unknown unit |
| Domain | 20 | ratio, quantity, register |
| Data | 11 | JSON/YAML config |
| CTest `GoldenMaster` | 1 | 필터 `[golden][regression][r01]` |
| **합계** | **52** | **0 failed** |

---

## 9. 권장 후속 작업

| 우선순위 | 항목 | 사유 |
|----------|------|------|
| P1 | CI에 `UNIT_CONVERTER_RED_PHASE=OFF` + `ctest` + lcov 게이트 | 로컬 검증 재현 |
| P2 | Branch 커버리지 90%/80% 보강 테스트 | PRD §4.3 branch 목표 |
| P3 | `ENABLE_COVERAGE` CMake 옵션 정식화 | README 문서와 CMakeLists 정합 |
| P4 | `genhtml` → `Report/coverage_v1/` (Linux CI) | HTML 증빙 |

---

## 10. 승인

| 역할 | 성명 | 일자 | 서명 |
|------|------|------|------|
| 검증 수행 | (에이전트/로컬 빌드) | 2026-05-21 | |
| QA 리뷰 | | | |
| 인수 (v1.0) | | | |

---

*본 문서는 리팩토링 완료 후 로컬 `ctest -V` 및 `build-cov` lcov 측정 결과를 정리한 최종 검증 보고서이다.*
