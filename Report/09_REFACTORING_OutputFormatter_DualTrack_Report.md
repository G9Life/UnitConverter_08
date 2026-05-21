# Dual-Track REFACTOR — OutputFormatter 추출 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-REF-001 |
| **문서 유형** | REFACTORING — Dual-Track UI/Logic 분리 (1커밋 단위) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **작성** | Dual-Track REFACTOR 스프린트 (R-U3, R-7) |
| **프로젝트** | UnitConverter_08 (C++17, CMake, Catch2 v3.5.4) |
| **저장소 경로** | `c:\DEV\UnitConverter_08` |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../docs/TODO.md](../docs/TODO.md) R-01, [../README.md](../README.md) §Golden Master |
| **선행 보고서** | [05_GREEN_CATCH2_Test_Suite_Report.md](05_GREEN_CATCH2_Test_Suite_Report.md) (RPT-GREEN-001), [06_REFACTORING_Golden_Master_Regression_Report.md](06_REFACTORING_Golden_Master_Regression_Report.md) (RPT-GM-001), [08_REFACTORING_README_Checklist_Report.md](08_REFACTORING_README_Checklist_Report.md) (RPT-GM-003) |
| **관련 산출물** | `include/boundary/OutputFormatter.hpp`, `src/boundary/OutputFormatter.cpp`, `src/UnitConverter.cpp`, `tests/boundary_tests.cpp` |

---

## 요약 (Executive Summary)

GREEN 단계 이후 **첫 REFACTOR 커밋**으로, `UnitConverter.cpp` `main()`에 남아 있던 **반올림(HALF_UP 4자리)·stdout 변환 테이블 포맷**을 Boundary(UI Track) `boundary::OutputFormatter`로 추출하였다. Domain(Logic Track)은 **변경 없음**. 외부 계약(입력 `단위:값`, 출력 `값 단위 = 변환값 단위`, 예외 타입) 및 Golden Master 기준은 **동일**하다.

| 구분 | 내용 |
|------|------|
| **리팩터 ID** | R-U3 (OutputFormatter 추출), R-7 (dead code 제거) |
| **사전 테스트** | CTest **50/50 PASS** |
| **사후 테스트** | CTest **52/52 PASS** (+Boundary 보호 2건) |
| **Golden Master** | `[golden][regression][r01]` **4/4 PASS** |
| **금지 준수** | 신규 기능 없음, `3.28084`/`1.09361` **신규 인라인 없음**, 테스트 삭제·완화 없음 |

> **제안 커밋 메시지**: `refactor(boundary): extract OutputFormatter from UnitConverter main`

---

## 1. REFACTOR 범위·절대 규칙

### 1.1 이번 커밋 목표 (선택 항목 ID)

| ID | 목표 | Track |
|----|------|-------|
| **R-U3** | `roundHalfUp4`, `formatConversionLine`, `writeConversionTable` → `boundary::OutputFormatter` | Boundary (UI) |
| **R-7** | `//refactoring` 주석, 미사용 `#include` 제거 | Control (`main`) |

### 1.2 의도적 제외 (다음 커밋 후보)

| ID | 내용 | 사유 |
|----|------|------|
| R-U2 | `ERR-INPUT-*` 상수 헤더화 | 1커밋 최소 단위 유지 |
| R-L3 | 비율 단일 출처 (`UnitRegistry` ↔ `Converter.hpp` 중복) | Domain/Data 영향 |
| R-L1 | `ConversionRule` VO | v1.1 확장 범위 |
| R-U2 + thin Application | `control::Application` 추출 | R-U3 선행 완료 후 |

### 1.3 절대 규칙 준수 체크

| 규칙 | 준수 |
|------|------|
| 새 기능 추가 금지 | ✅ 동작 동일, 구조만 분리 |
| 외부 계약 변경 금지 | ✅ 입력·출력·예외 타입 유지 |
| 테스트 삭제·완화 금지 | ✅ 기존 50건 유지 + 2건 추가 |
| GREEN 유지 (리팩터 전·후) | ✅ 50 → 52 PASS |
| `3.28084` / `1.09361` 인라인 **추가** 금지 | ✅ `UnitRegistry.cpp` 미변경 |

---

## 2. Dual-Track 분리 (변경 범위)

### 2.1 아키텍처 (변경 후)

```text
┌─────────────────────────────────────────────────────────────┐
│ Control: src/UnitConverter.cpp (thin main)                    │
│   prompt → getline → parse → convertAll → writeConversionTable│
└─────────────────────────────────────────────────────────────┘
         │                    │                      │
         ▼                    ▼                      ▼
┌─────────────────┐  ┌──────────────────┐  ┌──────────────────────┐
│ Boundary (UI)   │  │ Domain (Logic)   │  │ Boundary (UI)        │
│ InputParser     │  │ Converter        │  │ OutputFormatter      │
│ parse()         │  │ convertAll()     │  │ roundHalfUp4()       │
│ ERR-INPUT-*     │  │ toMeterHub/...   │  │ formatConversionLine │
└─────────────────┘  │ UnitRegistry     │  │ writeConversionTable │
                     └──────────────────┘  └──────────────────────┘
```

### 2.2 Track별 변경 요약

| Track | 변경 전 | 변경 후 |
|-------|---------|---------|
| **Boundary (UI)** | 포맷·반올림이 `main`에 혼재 | `OutputFormatter`가 전담 |
| **Domain (Logic)** | `Converter`, `UnitRegistry` | **변경 없음** |
| **Data** | `ConfigLoader` | **변경 없음** |
| **Control** | 포맷 루프·`setprecision` in `main` | orchestration만 |

### 2.3 변경 전 문제점 → 변경 후 개선점

| 문제 (Before) | 개선 (After) |
|---------------|--------------|
| BCE 위반: `main`이 파싱·변환·포맷·I/O를 모두 인지 | SRP: 포맷 책임이 Boundary로 이동 |
| Golden Master 회귀 시 `main` 전체가 영향권 | `OutputFormatter` 단위 + GM 이중 보호 |
| `//refactoring` 잔여·불필요 include | 진입점 가독성 향상 |

---

## 3. 구현 상세

### 3.1 신규 API (`boundary::OutputFormatter`)

| 함수 | 책임 |
|------|------|
| `roundHalfUp4(double)` | 표시용 HALF_UP 소수 4자리 (`std::round(v * 10000) / 10000`) |
| `formatConversionLine(...)` | `"2.5000 meter = 8.2021 feet"` 한 줄 생성 |
| `writeConversionTable(...)` | `convertAll` 결과를 stdout에 줄 단위 출력 |

### 3.2 `main()` (변경 후)

```cpp
const boundary::ParsedInput parsed = boundary::InputParser::parse(input);
const domain::Converter converter(domain::UnitRegistry::defaultRegistry());
const auto results = converter.convertAll(parsed.unit, parsed.value);
boundary::writeConversionTable(std::cout, parsed.value, parsed.unit, results);
```

### 3.3 수정·추가 파일 목록

| 파일 | 작업 |
|------|------|
| `include/boundary/OutputFormatter.hpp` | 신규 |
| `src/boundary/OutputFormatter.cpp` | 신규 |
| `src/UnitConverter.cpp` | thin `main`, dead code 제거 |
| `CMakeLists.txt` | `OutputFormatter.cpp` → `unit_converter_core` |
| `tests/boundary_tests.cpp` | R-U3 보호 테스트 2건 추가 |

---

## 4. 테스트 게이트 (Step 0 → Step 4)

### 4.1 실행 환경

```powershell
cmake -S C:\DEV\UnitConverter_08 -B C:\DEV\UnitConverter_08\build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build C:\DEV\UnitConverter_08\build
ctest --test-dir C:\DEV\UnitConverter_08\build --output-on-failure
```

### 4.2 결과 요약

| 단계 | 시점 | CTest | 비고 |
|------|------|-------|------|
| Step 0 | 리팩터 **전** | **50/50 PASS** | GREEN baseline |
| Step 2 | 보호 테스트 보강 | +2 Boundary | `[format][boundary]` |
| Step 4 | 리팩터 **후** | **52/52 PASS** | 0 failed |

### 4.3 레이어별 스모크

| 필터 | 건수 | 결과 |
|------|------|------|
| `[domain]` | 20 | PASS |
| `[boundary]` | 17 | PASS (기존 15 + 신규 2) |
| `[data]` | 10 | PASS |
| `[golden][regression][r01]` | 4 시나리오 | PASS |
| CTest `GoldenMaster` | 1 | PASS (~3.3s) |

### 4.4 신규 보호 테스트 (Step 2)

| TEST_CASE | 검증 |
|-----------|------|
| `test_output_round_half_up_4_meter_to_feet` | `roundHalfUp4(8.202100) == 8.2021` |
| `test_output_format_conversion_line_meter_to_feet` | `"2.5000 meter = 8.2021 feet"` 문자열 고정 |

### 4.5 Golden Master 계약 (변경 없음)

기준 파일 `tests/golden_master_expected.txt` — diff **0**.

| 시나리오 | 대표 출력 (1줄) |
|----------|-----------------|
| `meter:2.5` | `2.5000 meter = 8.2021 feet` |
| `feet:1.0` | `1.0000 feet = 0.3048 meter` |
| `yard:1.0` | `1.0000 yard = 0.9144 meter` |
| `meter:0.0` | stderr `ERR-INPUT-003`, stdout 변환 줄 없음 |

---

## 5. 위험 요소·롤백

| 위험 | 완화 | 롤백 포인트 |
|------|------|-------------|
| HALF_UP 4자리 반올림 drift | GM 4건 + `test_output_*` 2건 | `OutputFormatter.*` 삭제, `UnitConverter.cpp` 복원 |
| `fixed`/`setprecision(4)` 포맷 drift | `formatConversionLine` 문자열 assertion | 동일 |
| CMake 소스 누락 | 빌드 실패 즉시 발견 | `CMakeLists.txt` 한 줄 revert |

**한 줄 롤백:** 이 커밋만 `git revert` — Domain·InputParser·ConfigLoader 무영향.

---

## 6. REFACTOR 후보 로드맵 (잔여)

| 우선순위 | ID | 내용 | 선행 테스트 |
|----------|-----|------|-------------|
| 높음 | R-U2 | `boundary/ErrorCodes.hpp` | `[boundary]` ERR prefix |
| 중간 | R-L3 | 비율 단일 출처 (`loadDefaults` ↔ 상수) | `[domain]` `[data]` |
| 낮음 | R-L1 | `ConversionRule` VO | v1.1 |
| 낮음 | — | `control::Application` thin main 2차 | GM + Boundary |

---

## 7. 커밋 준비

### 7.1 제안 커밋 메시지 (Conventional Commit)

```
refactor(boundary): extract OutputFormatter from UnitConverter main

Move roundHalfUp4 and "value unit = converted unit" formatting into
boundary::OutputFormatter. Thin main delegates to writeConversionTable.
Add boundary format tests; Golden Master unchanged (52/52 GREEN).
```

### 7.2 스테이징 대상 (권장)

```text
include/boundary/OutputFormatter.hpp
src/boundary/OutputFormatter.cpp
src/UnitConverter.cpp
CMakeLists.txt
tests/boundary_tests.cpp
Report/09_REFACTORING_OutputFormatter_DualTrack_Report.md
Report/README.md
```

`build/` 산출물은 **커밋 제외**.

---

## 8. 결론

| 항목 | 상태 |
|------|------|
| Dual-Track R-U3 | **완료** — UI 포맷 책임 Boundary 분리 |
| Domain 무손상 | **확인** — `Converter`/`UnitRegistry` diff 없음 |
| 회귀 | **52/52 GREEN**, Golden Master diff 0 |
| 다음 REFACTOR | R-U2 또는 R-L3 (별도 1커밋 권장) |

---

## 부록 A. 관련 보고서 인덱스

| ID | 문서 |
|----|------|
| RPT-GREEN-001 | [05_GREEN_CATCH2_Test_Suite_Report.md](05_GREEN_CATCH2_Test_Suite_Report.md) |
| RPT-GM-001 | [06_REFACTORING_Golden_Master_Regression_Report.md](06_REFACTORING_Golden_Master_Regression_Report.md) |
| RPT-GM-002 | [07_REFECTORING_Golden_Master_Test_Code_Report.md](07_REFECTORING_Golden_Master_Test_Code_Report.md) |
| RPT-GM-003 | [08_REFACTORING_README_Checklist_Report.md](08_REFACTORING_README_Checklist_Report.md) |
| **RPT-REF-001** | **본 문서** |

---

*본 보고서는 Dual-Track REFACTOR 1커밋(R-U3) 실행 결과의 정본이며, 후속 R-U2·R-L3 리팩터는 별도 보고서로 추적한다.*
