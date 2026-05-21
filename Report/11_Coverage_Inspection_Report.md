# 테스트 커버리지 점검 보고서 — gcov / lcov · Invariant 분기 · 테스트 갭

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-COV-001 |
| **문서 유형** | 커버리지 점검 (Line / Branch · Invariant 우선 분석) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **프로젝트** | UnitConverter_08 (C++17, CMake, Catch2 v3.5.4) |
| **저장소 경로** | `c:\DEV\UnitConverter_08` |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md) §4.3, [10_REFACTORING_Final_Verification_Report.md](10_REFACTORING_Final_Verification_Report.md) |
| **관련 산출물** | `build-cov/`, `build-cov/coverage*.info`, `UnitRegistry.cpp.gcov` (프로젝트 루트) |

---

## 요약 (Executive Summary)

GREEN 빌드(`UNIT_CONVERTER_RED_PHASE=OFF`)·커버리지 빌드(`build-cov`, `--coverage`)에서 **ctest 52/52 PASS** 후 **lcov**로 레이어별 Line 커버리지를 재측정하였다.

| 구분 | 결과 | PRD 목표 |
|------|------|----------|
| **Domain Line** | **95.0%** (57/60) | ≥ 95% ✅ |
| **Boundary Line** | **93.8%** (45/48) | ≥ 85% ✅ |
| **UnitConverter.cpp (baseline)** | **93.3%** (14/15) | 참고 |
| **Domain Branch** | **53.8%** (42/78) | ≥ 90% ❌ |
| **Boundary Branch** | **53.6%** (59/110) | ≥ 80% ❌ |

**Invariant 핵심 (meter↔feet/yard 허브, 음수 거부, unknown unit)** 은 `Converter.cpp` Line **100%** 및 기존 Domain/Boundary 테스트로 **정상·예외 경로가 실행**되었다. 미커버는 주로 **등록 API 예외**(`UnitRegistry`), **파서 계약 ERR-005·004 세부**(`InputParser`), **CLI stdin EOF**(`UnitConverter.cpp`)이다.

> 본 보고서의 **추가 테스트 제안**은 테스트 코드 추가만을 전제로 하며, **프로덕션 구현 변경은 포함하지 않는다.**

---

## 1. 검증 환경 및 실행 절차

### 1.1 환경

| 항목 | 값 |
|------|-----|
| OS | Windows 10 (26200) |
| 컴파일러 | GNU 15.2.0 (MinGW-w64) |
| gcov / lcov | 15.2.0 |
| 빌드 디렉터리 | `build-cov` (Debug, `--coverage`) |
| RED phase | **OFF** |

### 1.2 실행 명령

```powershell
cd c:\DEV\UnitConverter_08
cmake --build build-cov
ctest --test-dir build-cov --output-on-failure

cd build-cov
lcov --capture --directory . --output-file coverage.info --rc lcov_branch_coverage=1
lcov --extract coverage.info "*/src/domain/*" --output-file coverage_domain.info --rc lcov_branch_coverage=1
lcov --extract coverage.info "*/src/boundary/*" --output-file coverage_boundary.info --rc lcov_branch_coverage=1
lcov --extract coverage.info "*/src/UnitConverter.cpp" --output-file coverage_unitconverter.info --rc lcov_branch_coverage=1
```

### 1.3 gcov (`UnitConverter.cpp`)

Windows MinGW 산출물 이름(`UnitConverter.cpp.gcno`)과 `gcov` 기본 기대 파일명이 불일치하여, 프로젝트 루트에서 `gcov UnitConverter.cpp` 직접 실행은 **notes 파일 미오픈**으로 실패한다. **Line 수치는 lcov `coverage_unitconverter.info`와 동일**하다 (14/15, 93.3%).

Domain 상세 미실행 라인 확인 시 프로젝트 루트의 `UnitRegistry.cpp.gcov`(ctest 후 생성)를 참조하였다.

### 1.4 CTest 결과

| 항목 | 값 |
|------|-----|
| 총 테스트 | 52 |
| PASS | 52 |
| FAIL | 0 |
| Golden Master | PASS (`[golden][regression][r01]`) |

---

## 2. 레이어별 Line 커버리지 (PRD §4.3 게이트)

| 레이어 | 대상 | Stmts | Hit | Miss | Cover | 목표 | 판정 |
|--------|------|------:|----:|-----:|------:|------|------|
| **Domain** | `Converter.cpp` + `UnitRegistry.cpp` | 60 | 57 | 3 | **95.0%** | ≥ 95% | ✅ |
| **Boundary** | `InputParser.cpp` + `OutputFormatter.cpp` | 48 | 45 | 3 | **93.8%** | ≥ 85% | ✅ |
| Baseline | `UnitConverter.cpp` | 15 | 14 | 1 | **93.3%** | — | 참고 |

### 2.1 파일별 상세

| 파일 | Stmts | Miss | Cover | Functions |
|------|------:|-----:|------:|-----------|
| `src/domain/Converter.cpp` | 21 | 0 | **100%** | 100% (6/6) |
| `src/domain/UnitRegistry.cpp` | 39 | 3 | **92.3%** | 100% (9/9) |
| `src/boundary/InputParser.cpp` | 33 | 3 | **90.9%** | 100% (3/3) |
| `src/boundary/OutputFormatter.cpp` | 15 | 0 | **100%** | 100% (3/3) |
| `src/UnitConverter.cpp` | 15 | 1 | **93.3%** | 100% (1/1) |

---

## 3. 요약 표 (미달 원인 · 추가 테스트 제안)

| 파일 | Stmts | Miss | Cover | 미달 원인 | 추가 테스트 제안 |
|------|------:|-----:|------:|-----------|------------------|
| **Domain 합산** | 60 | 3 | **95.0%** | — (≥95% 달성, 여유 0) | `UnitRegistry.cpp` 3 line 보강 시 Branch·버퍼 확보 |
| `src/domain/Converter.cpp` | 21 | 0 | **100%** | — | — (Invariant 정상·예외 경로 충족) |
| `src/domain/UnitRegistry.cpp` | 39 | 3 | **92.3%** | L42 `registerUnit("")` throw, L55 `setFactor` 무효 hubFactor throw, L18 return epilogue(계측) | `test_register_unit_empty_name_throws`: `registerUnit("", 1.0)` → `invalid_argument`. `test_set_factor_zero_or_nan_throws`: `setFactor("x", 0.0)` / `NaN` → `invalid_argument` |
| **Boundary 합산** | 48 | 3 | **93.8%** | — (≥85% 달성) | PRD **ERR-INPUT-005** prefix 스냅샷 부재 |
| `src/boundary/InputParser.cpp` | 33 | 3 | **90.9%** | L42 ERR-005, L45 빈 `valueToken`, L56 `consumed != size` | `test_parse_invalid_unit_name_005`: `"123meter:2.5"` 또는 `"meter$:1"` → `ERR-INPUT-005`. `test_parse_empty_value_004`: `"meter:"` → `ERR-INPUT-004`. `test_parse_number_suffix_004`: `"meter:2.5abc"` → `ERR-INPUT-004`. (선택) `test_parse_double_colon_001`: `"meter:2:5"` → `ERR-INPUT-001` |
| `src/boundary/OutputFormatter.cpp` | 15 | 0 | **100%** | — | — |
| `src/UnitConverter.cpp` | 15 | 1 | **93.3%** | L14 `!std::getline` → `return 1` (EOF) | Golden/통합: stdin EOF 파이프 → exit code 1 |

---

## 4. Invariant 관련 분기 우선 분석

### 4.1 meter ↔ feet / yard 비율 변환 (허브)

| 항목 | 상태 | 근거 |
|------|------|------|
| Line | **100%** | `Converter.cpp` 전 라인 실행 |
| 데이터 경로 | **커버** | `UnitRegistry::loadDefaults()` L66–68 (`kFeetPerMeter`, `kYardPerMeter`) 68회 실행 |
| 테스트 | 충분 | `test_meter_to_feet_*`, `test_meter_to_yard_*`, `test_feet_to_meter_reverse_*`, `test_feet_to_yard_via_meter_hub_*` |

변환은 `if-else` 단위 분기 없이 `factorFor` + `toMeterHub` / `fromMeterHub`로 동작하므로, **비율 Invariant는 map 조회·허브 산술 경로가 반복 실행**되는 것으로 충족된다.

### 4.2 음수·0·비유한 입력

| 계층 | 위치 | 상태 | 테스트 |
|------|------|------|--------|
| Domain | `Converter::convert` L21–22 | Line 실행, Branch 일부 0회 | `test_convert_negative_value_throws`, `test_meter_to_feet_zero_value_throws`, `test_meter_to_feet_nan_value_throws` |
| Boundary | `InputParser::parse` L58–59 | Line 실행 | `test_parse_meter_negative_throws`, `test_parse_meter_zero_throws_positive_required`, `test_parse_meter_infinity_throws` |

**권장 보강 (선택)**: Domain `convert("meter", INFINITY, "feet")` — `!std::isfinite` 분기.

### 4.3 없는 단위 (unknown unit)

| 계층 | 위치 | 상태 | 테스트 |
|------|------|------|--------|
| Domain | `UnitRegistry::factorFor` L26–27 | throw 4회 (미등록 6% 분기) | `test_convert_unknown_source/target_unit_throws` |
| Boundary | parse 성공 후 convert | ERR-002 prefix | `test_convert_unknown_unit_parsec_throws`, `test_convert_unknown_unit_mile_throws` |

### 4.4 미커버 — Invariant 인접·계약

| 파일 | Line | 내용 | 영향 |
|------|------|------|------|
| `UnitRegistry.cpp` | 42 | `registerUnit` 빈 이름 | 등록 검증 Invariant |
| `UnitRegistry.cpp` | 55 | `setFactor` hubFactor ≤ 0 / non-finite | 비율 검증 Invariant |
| `InputParser.cpp` | 42 | `ERR-INPUT-005` 단위명 형식 | PRD prefix 계약 **미테스트** |
| `InputParser.cpp` | 45 | 빈 숫자 토큰 `meter:` | ERR-004 |
| `InputParser.cpp` | 56 | `stod` 부분 소비 `meter:2.5abc` | ERR-004 |
| `UnitConverter.cpp` | 14 | stdin EOF | Control/baseline |

---

## 5. Domain Logic ≥ 95% — 테스트 제안 (구현 코드 추가 없음)

**합산 95.0%로 Line 게이트는 달성**하였다. 다만 `UnitRegistry.cpp` 단독 92.3%이며, Domain 합산 여유가 **0 line**이므로 아래 테스트 추가를 권장한다.

| # | 제안 테스트 ID (안) | Given / When / Then | 대상 line |
|---|---------------------|---------------------|-----------|
| 1 | `test_register_unit_empty_name_throws` | `registerUnit("", 0.4572)` → `std::invalid_argument` | UR L42 |
| 2 | `test_set_factor_invalid_hub_factor_throws` | `setFactor("x", 0.0)` 및 `NaN` → `std::invalid_argument` | UR L55 |
| 3 | `test_convert_positive_infinity_throws` (선택) | `convert("meter", INFINITY, "feet")` → throw | CV L21 |
| 4 | `test_yard_to_feet_via_meter_hub` (선택) | `convert("yard", 1.09361, "feet")` ≈ 3.28084 | 비율 대칭 |

**추가 불필요 (이미 충분)**  
meter→feet/yard, feet→meter, feet→yard 허브, 음수·unknown unit — `tests/domain_tests.cpp` `[ratio]`, `[quantity]`, `[domain]`.

---

## 6. Boundary ≥ 85% — 계약 테스트 제안 (구현 코드 추가 없음)

**합산 93.8%로 Line 게이트는 달성**하였다. PRD §4.3 **「ERR-INPUT-001~005 각 prefix 테스트」** 기준으로 **ERR-INPUT-005만 테스트 부재**이다.

| 코드 | 미커버 line | 제안 테스트 | 입력 예 |
|------|-------------|-------------|---------|
| ERR-INPUT-005 | L42 | `test_parse_invalid_unit_name_005` | `"123meter:2.5"`, `"meter$:1"` |
| ERR-INPUT-004 | L45 | `test_parse_empty_value_004` | `"meter:"` |
| ERR-INPUT-004 | L56 | `test_parse_number_suffix_004` | `"meter:2.5abc"` |
| ERR-INPUT-001 (선택) | L34 | `test_parse_double_colon_001` | `"meter:2:5"` |

기존 Boundary 테스트: 001 (`meter2.5`), 002 (`parsec`/`mile` convert), 003 (0·음수), 004 (`meter:abc`) — **PASS**.

---

## 7. Branch 커버리지 (후속 스프린트)

| 레이어 | Branch | PRD 목표 | 판정 |
|--------|--------|----------|------|
| Domain | **53.8%** (42/78) | ≥ 90% | ❌ |
| Boundary | **53.6%** (59/110) | ≥ 80% | ❌ |
| UnitConverter.cpp | **54.2%** (13/24) | — | 참고 |

주요 0-hit 분기:

- `InputParser`: `trim` / `isValidUnitName` 루프, 다중 콜론 분기
- `UnitRegistry`: `setFactor` 무효 ratio 분기 (L54–55)
- `Converter`: throw 경로 복합 조건 일부

§5·§6의 Line 보강 테스트가 Branch 커버리지도 함께 상승시킬 가능성이 높다.

---

## 8. lcov 산출물

| 파일 | 설명 |
|------|------|
| `build-cov/coverage.info` | 전체 캡처 |
| `build-cov/coverage_domain.info` | Domain extract |
| `build-cov/coverage_boundary.info` | Boundary extract |
| `build-cov/coverage_unitconverter.info` | `UnitConverter.cpp` baseline |

`genhtml` HTML 리포트는 Windows Perl 제약으로 미생성. Linux/WSL CI에서 `Report/coverage_v1/`, `Report/coverage_baseline/` 생성 권장 ([10_REFACTORING_Final_Verification_Report.md](10_REFACTORING_Final_Verification_Report.md) §6.5).

---

## 9. 결론 및 권고

| # | 항목 | 판정 |
|---|------|------|
| 1 | Domain Line ≥ 95% | ✅ **95.0%** |
| 2 | Boundary Line ≥ 85% | ✅ **93.8%** |
| 3 | Invariant (비율·음수·unknown) 핵심 로직 | ✅ `Converter.cpp` 100% |
| 4 | ERR-INPUT-001~005 prefix 각 1테스트 | ⚠️ **005 누락** |
| 5 | Branch PRD 목표 | ❌ 후속 권장 |

**권고 (우선순위)**  

1. **P0** — `tests/boundary_tests.cpp`에 ERR-INPUT-005 스냅샷 1건 추가  
2. **P1** — `meter:` / `meter:2.5abc` ERR-004 보강  
3. **P1** — Domain `registerUnit("")`, `setFactor` 무효 ratio  
4. **P2** — Branch 게이트: §7 테스트 묶음 + `genhtml` CI  

---

*본 문서는 2026-05-21 로컬 `build-cov` lcov 재측정 및 Invariant 우선 분석 결과를 정리한 커버리지 점검 보고서이다.*
