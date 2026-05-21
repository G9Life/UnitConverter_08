# Test case design for unit conversions
_Exported on 5/21/2026 at 09:57:43 GMT+9 from Cursor (3.4.17)_

---

**User**

[P] 테스트 설계에 강한 시니어 C++ QA입니다.
[C] C++17, CMake, Catch2
[T] 아이템 타입별 최소 5개 테스트를 작성해줘.

    대상 케이스:
    - 정상 변환: meter→feet, meter→yard, feet→meter (역변환)
    - 경계값: value=0 / 매우 큰 수 / 소수점 6자리 정확도
    - 예외: 잘못된 형식 / 음수 / 없는 단위
    - 동적 등록: registerUnit 후 변환
    - 설정 로드: JSON/YAML 정상·실패 케이스

    형식 규칙:
    - Catch2 사용
    - Given-When-Then 주석 구조
    - 각 테스트에 변환 비율 명시 (1 meter = 3.28084 feet)
    - 테스트 이름: test_[변환타입]_[조건]_[기대결과] 형식

[F] 완성된 테스트 코드. cmake -S . -B build && cmake --build build가 Green이 되게 작성.


---

**Cursor**

C++ 단위 변환기 코드베이스와 기존 테스트 구조를 먼저 파악하겠습니다.


Catch2 테스트와 GREEN 빌드를 위한 도메인·경계·데이터 레이어를 구현합니다.


Catch2 테스트와 GREEN 빌드를 위한 CMake·도메인·경계·데이터 레이어를 추가했습니다.

## 빌드 결과

```text
cmake -S . -B build
cmake --build build
→ 성공

.\build\unit_converter_tests.exe
→ 50 test cases, 72 assertions — All tests passed
```

## 테스트 구조 (50건)

| 카테고리 | 파일 | 테스트 수 | 예시 |
|----------|------|-----------|------|
| 정상 변환 | `tests/domain_tests.cpp` | 6 | `test_meter_to_feet_normal_returns_8_2021` |
| 경계값 | domain + boundary | 11 | `test_meter_to_feet_zero_value_throws`, `test_parse_meter_six_decimal_precision` |
| 예외 | domain + boundary | 11 | `test_parse_invalid_format_no_colon_throws`, `test_convert_unknown_unit_mile_throws` |
| 동적 등록 | `domain_tests.cpp` | 5 | `test_register_cubit_then_convert_to_meter` |
| JSON/YAML 설정 | `tests/data_tests.cpp` | 12 | `test_config_json_valid_loads_ratios`, `test_config_yaml_invalid_ratio_fails_data002` |

각 테스트는 **Given-When-Then** 주석과 변환 비율(예: `1 meter = 3.28084 feet`)을 포함하며, 이름은 `test_[타입]_[조건]_[기대결과]` 형식입니다.

## 환산 모델 (meter 허브)

```text
meterHub = sourceValue / factor(source)
targetValue = meterHub × factor(target)

기본 factor: meter=1.0, feet=3.28084, yard=1.09361
registerUnit("cubit", 0.4572) → 1 cubit = 0.4572 meter
```

## 추가된 주요 파일

- `CMakeLists.txt` — Catch2 FetchContent, `unit_converter_core` 라이브러리, `unit_converter_tests`
- `include/domain/`, `include/boundary/`, `include/data/` — 헤더
- `src/domain/`, `src/boundary/`, `src/data/` — 구현
- `tests/domain_tests.cpp`, `tests/boundary_tests.cpp`, `tests/data_tests.cpp`
- `tests/fixtures/` — JSON/YAML 정상·실패 fixture

## 실행

```powershell
ctest --test-dir build --output-on-failure
.\build\unit_converter_tests.exe "[ratio]"
.\build\UnitConverter.exe
```

현재는 **GREEN** 단계입니다. RED 단계로 전환하려면 구현을 의도적으로 제거하거나 테스트 기대값만 먼저 커밋하면 됩니다.

---

**User**

Report 폴더에 보고서로 작성해줘

---

**Cursor**

Report 폴더의 기존 보고서 형식을 확인한 뒤, 동일한 형식으로 테스트 작성 보고서를 작성합니다.


`Report/03_CATCH2_Test_Implementation_Report.md` (RPT-IMPL-001)에 Catch2 테스트 구현 보고서를 작성했고, `Report/README.md`에 목록을 반영했습니다.

## 보고서 위치

[Report/03_CATCH2_Test_Implementation_Report.md](Report/03_CATCH2_Test_Implementation_Report.md)

## 포함 내용

| 섹션 | 내용 |
|------|------|
| 요약 | 50 test cases, 72 assertions, GREEN 빌드·실행 결과 |
| 요청 범위 대비 | 정상·경계·예외·동적등록·JSON/YAML 각 5건 이상 |
| 아키텍처 | `tests/`, `include/`, `src/`, fixture, Catch2 태그 |
| 환산 불변식 | 1 m = 3.28084 ft, 허브 환산식, ε |
| 테스트 목록 | 50건 전체 표 (카테고리별) |
| RED 체크리스트 매핑 | TC-A-01~07, TC-B-01~07 대응 (TC-A-06 통합 미구현 표기) |
| 빌드·실행 | cmake/ctest 명령 및 실행 로그 |
| 후속 작업 | T-I-01, ERR prefix, table/JSON/CSV, 커버리지 |

RED 체크리스트와의 관계도 문서에 명시했습니다. 현재는 테스트와 최소 구현이 함께 있어 **GREEN** 상태이며, RED 전용 브랜치가 필요하면 구현 스텁 분리가 후속 작업으로 적혀 있습니다.

