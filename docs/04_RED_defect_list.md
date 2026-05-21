# 결함 목록 (defect_list.md)

## 문서 관리

| 항목 | 내용 |
|------|------|
| **프로젝트** | UnitConverter_08 (C++17, Catch2, CMake) |
| **작성** | QA 리드 |
| **작성일** | 2026-05-21 (갱신) |
| **정본 참조** | [PRD.md](PRD.md), [README.md](../README.md), [Report/02_RED_Todo_Checklist_Report.md](../Report/02_RED_Todo_Checklist_Report.md), [Report/03_RED_Fail_CATCH2_Test_Suite_Report.md](../Report/03_RED_Fail_CATCH2_Test_Suite_Report.md) |
| **현재 단계** | **RED** (`UNIT_CONVERTER_RED_PHASE=ON`, 45건 중 **44 failed**) |

### 상태 범례

| 상태 | 의미 |
|------|------|
| **Open** | 미수정·RED 실패 중 |
| **Fixed** | 수정 반영됨 |
| **Closed** | GREEN `src/` 구현으로 해소 (RED 스텁 제외 시) |

---

## 결함 테이블

| ID | Severity | 변환 타입 | 재현 절차 | 기대값 | 실제값 | 근본 원인 | 수정 요약 | 상태 |
|----|----------|-----------|-----------|--------|--------|-----------|-----------|------|
| DEF-001 | Critical | 빌드/CMake | 클린 트리 `cmake -S . -B build && cmake --build build` | configure·빌드 성공 | `Cannot find source file: UnitConverter.cpp` | 루트 `UnitConverter.cpp` 삭제, `src/UnitConverter.cpp`로 이동 후 CMake 경로 미갱신 | `add_executable(UnitConverter src/UnitConverter.cpp)` | **Fixed** |
| DEF-002 | Major | 환경/CMake | `build/` 잔존 후 generator 변경·`cmake -S . -B build` | 동일 generator 재configure 성공 | `Ninja` vs `MinGW Makefiles` 불일치 | `CMakeCache.txt`에 이전 generator 잔존 | `Remove-Item -Recurse build` 후 `-G Ninja` 고정 | **Open** |
| DEF-003 | Minor | 계약/stderr | `InputParser::parse("meter2.5")` 또는 `meter:-1` | `ex.what()`가 `ERR-INPUT-001`·`003` 등으로 **시작** | `Invalid format...` / `Value must be positive...` (prefix 없음) | `InputParser.cpp`·`UnitRegistry.cpp` throw에 PRD §5 코드 미부착 | throw 문자열 앞 `ERR-INPUT-00x ` 추가 (구조체 변경 없음) | **Open** |
| DEF-004 | Critical | meter→feet | `convert("meter", 2.5, "feet")` — `test_meter_to_feet_normal_returns_8_2021` | **8.202100** (ε 1e-5) | **0.000000** | RED 스텁 `Converter::convert`가 항상 0 반환 (`tests/red_phase_stubs.cpp:54`) | GREEN: `UNIT_CONVERTER_RED_PHASE=OFF` + `Converter.cpp` 허브식 `value/factor(from)×factor(to)` | **Open** |
| DEF-005 | Critical | meter→yard | `convert("meter", 2.5, "yard")` — `test_meter_to_yard_normal_returns_2_7340` | **2.734025** | **0.000000** | 동일 스텁, yard 분기·허브 미적용 | GREEN: `fromMeterHub(toMeterHub(...))` | **Open** |
| DEF-006 | Critical | feet→meter | `convert("feet", 1.0, "meter")` — `test_feet_to_meter_reverse_returns_0_3048` | **0.304800** | **0.000000** | 역변환 허브 미적용 (스텁 0.0) | GREEN: `Converter.cpp:20-25` meter 허브 경유 | **Open** |
| DEF-007 | Critical | 파싱/meter | `InputParser::parse("meter:2.5")` — `test_parse_meter_colon_value_success` | unit=`meter`, value=`2.5` | unit=`""`, value=`0.0` | RED 스텁 `parse` 고정 반환 (`red_phase_stubs.cpp:70`) | GREEN: `InputParser.cpp` 콜론 분리·`stod`·양수 검증 | **Open** |
| DEF-008 | Critical | 파싱 연계 | `parse("meter:2.5")` 후 feet 변환 — `test_parse_then_meter_to_feet_chain` | **8.202100** feet | **0.000000** | DEF-004 + DEF-007 복합 | GREEN: 파서 + Converter 허브식 | **Open** |
| DEF-009 | Major | 경계/value=0 | `convert("meter", 0.0, "feet")` — `test_meter_to_feet_zero_value_throws` | `std::invalid_argument` | 예외 없음, **0.000000** 반환 | 스텁이 `value<=0` 검사·throw 없음 | GREEN: `Converter.cpp:21-22` 양수·유한값 검사 | **Open** |
| DEF-010 | Major | 경계/parse zero | `InputParser::parse("meter:0")` — `test_parse_meter_zero_throws_positive_required` | throw + `ERR-INPUT-003` prefix | throw 없음, `{ "", 0.0 }` | 스텁 파서 미검증 | GREEN: `InputParser.cpp:58-59` + ERR prefix | **Open** |
| DEF-011 | Critical | 동적 등록 | `registerUnit("cubit",0.4572)` 후 `hasUnit("cubit")` | **true**, unitCount=4 | **false**, count=0 | 스텁 `registerUnit` no-op (`red_phase_stubs.cpp:35`) | GREEN: `UnitRegistry.cpp` `registerUnit`·`setFactor` | **Open** |
| DEF-012 | Critical | cubit→meter | `convert("cubit", 1.0, "meter")` — `test_register_cubit_then_convert_to_meter` | **0.457200** | **0.000000** | 등록·환산 미구현 (스텁) | GREEN: Registry 등록 + 허브 변환 | **Open** |
| DEF-013 | Critical | 설정/JSON | `ConfigLoader::loadJson(units_valid.json)` — `test_config_json_valid_loads_ratios` | `success=true`, 3 units | **success=false** | 스텁이 항상 `{false, ERR-DATA-001}` (`red_phase_stubs.cpp:81-82`) | GREEN: `ConfigLoader.cpp` JSON 파싱·registry 반영 | **Open** |
| DEF-014 | Major | 설정/JSON schema | `loadJson(units_invalid_schema.json)` — `test_config_json_invalid_schema_fails_data002` | **ERR-DATA-002** | **ERR-DATA-001** | 스텁이 오류 코드 구분 없음 | GREEN: 스키마 검증 분기·`ERR-DATA-002` | **Open** |
| DEF-015 | Critical | 설정/YAML | `ConfigLoader::loadYaml(units_valid.yaml)` — `test_config_yaml_valid_loads_ratios` | `success=true`, 8.202100 ft | **success=false** | DEF-013과 동일 스텁 | GREEN: `ConfigLoader.cpp` YAML 경로 | **Open** |
| DEF-016 | Major | 부동소수 비교 | `nearlyEqual(8.2021, 8.202100, 1e-5)` 등 전 테스트 | **true** (ε 이내) | **false** (항상) | RED 스텁 `nearlyEqual` → `false` (`red_phase_stubs.cpp:61-62`) | GREEN: `Converter.cpp:37-38` `fabs(a-b)<=epsilon` | **Open** |
| DEF-017 | Info | RED/CMake | `cmake -S . -B build` (기본 옵션) | 테스트 RED(미구현 실패) | **44 failed** (의도) | `UNIT_CONVERTER_RED_PHASE=ON` 기본값·`red_phase_stubs.cpp` 링크 | RED 유지: ON · GREEN 착수: `-DUNIT_CONVERTER_RED_PHASE=OFF` | **Open** (의도) |
| DEF-018 | Info | 계약/README | RED 체크리스트 vs PRD stderr | Boundary는 ERR prefix 우선 | README 일부 `invalid_argument`만 명시 | 문서 이중 계약 | DEF-003과 동시 정리·스냅샷 테스트 | **Open** |

---

## 심각도 정의

| 등급 | 기준 |
|------|------|
| **Critical** | 변환·파싱·설정 결과가 전면 오류(0, 빈 문자열, load 실패) |
| **Major** | 경계 throw 누락, 오류 코드 불일치, 환경 빌드 차단 |
| **Minor** | stderr prefix·포맷 등 계약 표면 불일치 (수치 로직은 GREEN 시 정상 가능) |
| **Info** | RED 단계 의도·문서·환경 |

---

## 재현·확인 절차

### RED (현재 기본)

```powershell
Set-Location c:\DEV\UnitConverter_08
cmake -S . -B build -G Ninja -DUNIT_CONVERTER_RED_PHASE=ON
cmake --build build
.\build\unit_converter_tests.exe
```

| 일자 | 결과 | 비고 |
|------|------|------|
| 2026-05-21 | **44 failed / 45** | RED 계약 고정·미구현 확인 |

### GREEN (미착수 — DEF-004~016 해소 후)

```powershell
cmake -S . -B build -G Ninja -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## 추적 매핑 (요약)

| 결함 ID | 대표 테스트 | 위치 |
|---------|-------------|------|
| DEF-004~006 | `test_meter_to_feet_*`, `test_feet_to_meter_*` | `tests/red_phase_stubs.cpp:54` / GREEN `src/domain/Converter.cpp` |
| DEF-007~008 | `test_parse_meter_*`, `test_parse_then_meter_to_feet_chain` | `red_phase_stubs.cpp:70` / `src/boundary/InputParser.cpp` |
| DEF-009~010 | `test_meter_to_feet_zero_*`, `test_parse_meter_zero_*` | 스텁 / GREEN Converter·InputParser |
| DEF-011~012 | `test_register_*` | `red_phase_stubs.cpp:35` / `src/domain/UnitRegistry.cpp` |
| DEF-013~015 | `test_config_json_*`, `test_config_yaml_*` | `red_phase_stubs.cpp:77-86` / `src/data/ConfigLoader.cpp` |
| DEF-003 | `test_parse_invalid_format_*` 등 6건 (GREEN 시) | `InputParser.cpp:35`, `UnitRegistry.cpp:27` |

---

*결함 추가·상태 변경 시 본 파일과 [README.md](../README.md) 「RED 단계 To-Do 리스트 > 결함 목록 연결」을 동일 PR에서 갱신한다.*
