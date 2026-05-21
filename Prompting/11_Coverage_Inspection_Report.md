# Test coverage assessment for UnitConverter
_Exported on 5/21/2026 at 14:32:37 GMT+9 from Cursor (3.4.17)_

---

**User**

프로젝트: c:\DEV\UnitConverter_08

현재 테스트 커버리지를 점검해줘.
- 실행: # gcov / lcov && gcov UnitConverter.cpp
- 커버되지 않은 줄 중 Invariant 관련 분기 우선 식별
  (특히: meter↔feet/yard 비율 변환 분기, 음수 입력 처리, 없는 단위 처리)
- Domain Logic 95% 미달 시: 누락 테스트 케이스 제안 (구현 코드 추가 금지)
- Boundary 85% 미달 시: 누락 계약 테스트 케이스 제안

출력:
| 파일 | Stmts | Miss | Cover | 미달 원인 | 추가 테스트 제안 |


---

**Cursor**

프로젝트 구조와 커버리지 빌드 설정을 확인한 뒤 gcov/lcov를 실행합니다.


## 커버리지 점검 결과

**실행 환경**: `build-cov` (GREEN, `--coverage`), `ctest` 52/52 PASS, `lcov --capture` 후 `coverage_domain.info` / `coverage_boundary.info` / `coverage_unitconverter.info` 재추출.

**게이트 판정 (PRD §4.3 Line)**  
| 레이어 | Cover | 목표 | 판정 |
|--------|-------|------|------|
| Domain (`Converter.cpp` + `UnitRegistry.cpp`) | **95.0%** (57/60) | ≥ 95% | **달성** (한계치) |
| Boundary (`InputParser.cpp` + `OutputFormatter.cpp`) | **93.8%** (45/48) | ≥ 85% | **달성** |

**참고 (Branch)**: Domain **53.8%** (42/78), Boundary **53.6%** (59/110) — PRD Branch 목표(90%/80%) 미달. 아래 표는 Line 기준.

**Invariant 핵심 분기 점검**  
| 항목 | Line | Branch | 비고 |
|------|------|--------|------|
| meter↔feet/yard 비율 (허브 변환) | **100%** (`Converter.cpp`) | 정상 경로 충분 | `test_meter_to_feet_*`, `test_meter_to_yard_*`, `test_feet_to_meter_*`, `test_feet_to_yard_via_meter_hub_*` |
| 음수/0 입력 | Domain·Boundary **커버** | `Converter:21-22` 일부 분기 0회 | `test_convert_negative_*`, `test_parse_meter_negative_*`, `test_meter_to_feet_zero_*` |
| 없는 단위 | **커버** | throw 분기 실행됨 | `factorFor` L27, `test_convert_unknown_*`, `test_convert_unknown_unit_*` |
| 미커버 (Invariant 인접) | `UnitRegistry` L42, L55 | `registerUnit("")`, `setFactor` 무효 ratio | Domain 합산은 95%로 통과하나 파일 단위 갭 존재 |
| 미커버 (Boundary 계약) | `InputParser` L42, L45, L56 | **ERR-INPUT-005 미테스트** | L42=단위명 검증, L45=빈 값, L56=숫자 잔여 문자 |

`gcov UnitConverter.cpp`: Windows에서 `.gcno` 이름 불일치로 직접 실행 실패 → **lcov와 동일** (14/15, L14 `getline` EOF 미실행).

---

## 요약 표

| 파일 | Stmts | Miss | Cover | 미달 원인 | 추가 테스트 제안 |
|------|------:|-----:|------:|-----------|------------------|
| **Domain 합산** | 60 | 3 | **95.0%** | — (≥95% 달성, 여유 0) | 아래 `UnitRegistry.cpp` 3 line 보강 시 Branch·버퍼 확보 |
| `src/domain/Converter.cpp` | 21 | 0 | **100%** | — | — (Invariant 정상·예외 경로 충족) |
| `src/domain/UnitRegistry.cpp` | 39 | 3 | **92.3%** | L42 `registerUnit("")` throw, L55 `setFactor` 무효 hubFactor throw, L18 return epilogue(계측) | `test_register_unit_empty_name_throws`: `registerUnit("", 1.0)` → `invalid_argument`. `test_set_factor_zero_or_nan_throws`: `setFactor("inch", 0.0)` / `NaN` → `invalid_argument` |
| **Boundary 합산** | 48 | 3 | **93.8%** | — (≥85% 달성) | PRD **ERR-INPUT-005** prefix 스냅샷 부재 |
| `src/boundary/InputParser.cpp` | 33 | 3 | **90.9%** | L42 ERR-005, L45 빈 `valueToken`, L56 `consumed != size` | `test_parse_invalid_unit_name_005`: `"123meter:2.5"` 또는 `"meter$:1"` → `ERR-INPUT-005` prefix. `test_parse_empty_value_004`: `"meter:"` → `ERR-INPUT-004`. `test_parse_number_suffix_004`: `"meter:2.5abc"` → `ERR-INPUT-004`. (선택) `test_parse_double_colon_001`: `"meter:2:5"` → `ERR-INPUT-001` |
| `src/boundary/OutputFormatter.cpp` | 15 | 0 | **100%** | — | — |
| `src/UnitConverter.cpp` (baseline) | 15 | 1 | **93.3%** | L14 `!std::getline` → `return 1` (EOF) | Golden/통합: stdin EOF 파이프 → exit code 1, stderr 없음 |

---

## Domain Logic 95% — 추가 제안 (합산은 달성, 구현 코드 추가 없음)

현재 **합산 95.0%로 미달 아님**. 다만 `UnitRegistry.cpp` 단독 92.3%이고 미실행 3 line이 등록 검증 분기이므로, **버퍼·PRD Domain 실패 8종** 관점에서 권장:

1. **`test_register_unit_empty_name_throws`** — `UnitRegistry r; r.registerUnit("", 0.4572);` → `std::invalid_argument` (L42).
2. **`test_set_factor_invalid_hub_factor_throws`** — `r.setFactor("x", 0.0)` 및 `NaN` (L55).
3. **(선택, Branch)** `test_convert_positive_infinity_throws` — `convert("meter", INFINITY, "feet")` — `Converter` L21 `!isfinite` 분기 보강.
4. **(선택, 비율 대칭)** `test_yard_to_feet_via_meter_hub` — `convert("yard", 1.09361, "feet")` ≈ 3.28084 (feet→yard는 이미 있음).

**이미 충분히 커버된 Invariant (추가 불필요)**  
- meter→feet/yard, feet→meter, feet→yard 허브: `domain_tests.cpp` `[ratio]`  
- 음수: `test_convert_negative_value_throws`, `test_parse_meter_negative_throws`  
- unknown unit: `test_convert_unknown_source/target_unit_throws`, boundary `parsec`/`mile` 체인  

---

## Boundary 85% — 추가 제안 (합산 달성, 계약 갭 중심)

Line 게이트는 충족. **PRD §4.3 “ERR-INPUT-001~005 각 prefix 테스트”** 기준으로 **005만 누락**:

| 코드 | 미커버 line | 제안 테스트 |
|------|-------------|-------------|
| ERR-INPUT-005 | L42 | `InputParser::parse("1meter:2.5")` — `requireErrPrefix(ex, "ERR-INPUT-005")` |
| ERR-INPUT-004 (보강) | L45 | `parse("meter:")` |
| ERR-INPUT-004 (보강) | L56 | `parse("meter:2.5x")` |
| ERR-INPUT-001 (선택) | L34 두 번째 `:` | `parse("meter:2:5")` |

---

## Branch 커버리지 (후속, 표 외 참고)

| 레이어 | Branch | PRD 목표 |
|--------|--------|----------|
| Domain | 53.8% (42/78) | ≥ 90% |
| Boundary | 53.6% (59/110) | ≥ 80% |
| UnitConverter.cpp | 54.2% (13/24) | — |

주요 0-hit 분기: `InputParser` `trim`/`isValidUnitName` 루프, `UnitRegistry::setFactor` 무효 분기, `Converter::convert` throw 경로 일부. 위 Line 보강 테스트가 Branch도 함께 올립니다.

---

**결론**: Line 게이트 **Domain 95.0% / Boundary 93.8% 모두 PASS**. Invariant 핵심(비율·음수·unknown unit)은 **Domain 변환 로직에서 Line 100%**. 남은 갭은 **등록 API 예외(`UnitRegistry`)**, **파서 계약 ERR-005·004 세부(`InputParser`)**, **CLI EOF(`UnitConverter`)** — 위 표의 테스트 케이스 추가만으로 보완 가능합니다 (구현 변경 없이 테스트만).

---

**User**

Report 폴더에 보고서로 작성해줘

---

**Cursor**

Report 폴더의 기존 보고서 형식을 확인한 뒤 커버리지 점검 보고서를 작성합니다.


`Report/11_Coverage_Inspection_Report.md`에 커버리지 점검 보고서를 작성했고, `Report/README.md` 목록에도 등록했습니다.

**문서 정보**
- **보고서 ID**: RPT-COV-001
- **파일**: [Report/11_Coverage_Inspection_Report.md](Report/11_Coverage_Inspection_Report.md)

**포함 내용**
1. 검증 환경·lcov/gcov 실행 절차·CTest 52/52 PASS  
2. 레이어별 Line/Branch 게이트 (Domain 95.0%, Boundary 93.8%)  
3. 요약 표 (파일별 Stmts / Miss / Cover / 미달 원인 / 테스트 제안)  
4. Invariant 우선 분석 (비율·음수·unknown unit)  
5. Domain ≥95%·Boundary ≥85% 기준 **테스트만** 제안 (구현 변경 없음)  
6. Branch 미달·lcov 산출물·결론·우선순위 권고 (P0: ERR-INPUT-005)

