# QA 테스트 계획 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-QA-001 |
| **문서 유형** | 테스트 계획 보고서 (Test Plan Report) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **작성** | 시니어 QA 리드 |
| **프로젝트** | UnitConverter (C++17) |
| **대상 기능** | meter → feet 변환 (meter 허브 `ConvertAll` 샘플) |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../README.md](../README.md), [../docs/requirment.md](../docs/requirment.md) |
| **인수 게이트** | PRD R-01 · AC-01~02 · Domain line ≥95% · Boundary line ≥85% |

---

## 요약 (Executive Summary)

본 보고서는 샘플 예제 **`meter:2.5` → `2.5 meter = 8.2021 feet`** 를 중심으로, Catch2 기반 Dual-Track 테스트 범위·우선순위, 경계값·예외 케이스, 커버리지 목표, gcov/lcov 측정 전략을 정의한다.

| 구분 | 핵심 결정 |
|------|-----------|
| 비즈니스 불변식 | `1 m = 3.28084 ft`, `1 m = 1.09361 yd`; feet↔yard 직접 비율 금지 |
| 표시 vs Domain | HALF_UP 소수 4자리(Boundary) / ε=1e-9(Domain) 분리 검증 |
| P0 테스트 | T-D-01, T-D-02, T-B-parse-01, T-I-01 |
| 실패 정책 | stdout 변환 0줄, ERR prefix 고정, exit 1 |
| 커버리지 게이트 | Domain ≥95% line, Boundary ≥85% line (미달 시 v1.0 인수 불가) |

---

## 1. 테스트 목표

1. **비즈니스 불변식 고정**: `1 meter = 3.28084 feet`, `1 meter = 1.09361 yard`, feet↔yard는 meter 경유만 허용.
2. **표시·Domain 분리 검증**: Domain 비교 ε = `1e-9`, Boundary 표시 HALF_UP **소수 4자리**.
3. **입력 계약 선행 거부**: 형식·숫자·단위·양수 정책 위반 시 stdout 변환 줄 0, stderr prefix 고정, exit `1`.
4. **회귀 방지**: v1.0 필수 Catch2 세트(R-01)와 커버리지 임계(Domain ≥95% line, Boundary ≥85% line) 충족 후 merge.

---

## 2. 샘플 예제 기준 계약 (기준선)

| 항목 | 값 |
|------|-----|
| 입력 | `meter:2.5` |
| README 요구사항 | 기본 요구사항 4번 (정확 환산·테스트) · 1번 (입출력 예시) |
| Domain 기대 (feet) | `2.5 × 3.28084 = 8.2021` (ε 이내) |
| table 기대 (feet 줄) | `2.5 meter = 8.2021 feet` |
| 전체 table (3줄) | `2.5 meter`, `8.2021 feet`, `2.7340 yard` |
| 성공 시 | stderr 비어 있음, exit `0` |

> 레거시 [../UnitConverter.cpp](../UnitConverter.cpp)는 round4·`meter:0` 거부·ERR 코드 체계가 없음. 본 보고서는 **목표 구현(BCE)** 기준이며, 프로토타입은 §8 gcov 베이스라인 측정용으로만 병행한다.

---

## 3. Catch2 단위 테스트 — 범위 및 우선순위

### 3.1 테스트 파일·태그 구조 (목표)

```text
tests/
  domain_tests.cpp      # [ratio] [domain] [registry] [quantity]
  boundary_tests.cpp    # [parse] [boundary] [error-contract] [table]
  integration_tests.cpp # [integration] [boundary]
```

| 우선순위 | ID | Catch2 태그 | 레이어 | 범위 (meter→feet 관련) | 선행 조건 |
|----------|-----|-------------|--------|------------------------|-----------|
| **P0** | T-D-01 | `[ratio]` | Domain | `1 meter` → `3.28084 feet` (ε); 비율 상수 Registry 출처 일치 | Registry 기본 3단위 로드 |
| **P0** | T-D-02 | `[ratio]` | Domain | `meter:2.5` → feet `8.2021` (ε); yard `2.7340` 동시 검증 | T-D-01 GREEN |
| **P0** | T-B-parse-01 | `[parse]` | Boundary | `meter:2.5` 파싱 DTO (`source_unit=meter`, `value=2.5`) | Domain Mock |
| **P0** | T-I-01 | `[integration]` | E2E | stdin `meter:2.5` → table 3줄, feet 줄 `8.2021`, exit 0, stderr空 | CMake·실행 파일 |
| **P1** | T-D-03 | `[ratio]` | Domain | feet→meter→feet 왕복; yard 경유 시 feet와 **독립 feet↔yard 비율 없음** | T-D-01 |
| **P1** | T-D-04 | `[ratio]` | Domain | `ConvertAll` 건수 = Registry 등록 수(기본 3) | Registry |
| **P1** | T-D-05 | `[quantity]` | Domain | `value ≤ 0`, NaN, Inf → Quantity 생성 거부 (I/O 없음) | — |
| **P1** | T-B-05 | `[table]` | Boundary | round4: `8.2021`, `2.7340` HALF_UP; 패턴 `{src} {unit} = {tgt} {unit}` | Domain 결과 주입 |
| **P1** | T-B-ERR-* | `[parse][boundary]` | Boundary | §4 경계값·§5 예외 케이스 stderr prefix·exit | Parser |
| **P2** | T-D-06 | `[register]` | Domain | 신규 단위 추가 시 환산 핵심 diff 0 (OCP) | v1.1 확장 |
| **P2** | T-B-06~07 | `[json][csv]` | Boundary | 동일 입력 시 (feet, 8.2021) 집합 동형성 | table GREEN 후 |

### 3.2 Dual-Track 실행 순서 (RED → GREEN)

```text
1. Domain [ratio]     — T-D-01, T-D-02 (feet 비율·2.5 샘플)
2. Boundary [parse]   — 성공 파싱 + ERR-INPUT-001~005
3. Boundary [table]   — feet 줄 round4 스냅샷
4. Integration        — T-I-01 Gherkin Happy Path
5. Data / Control     — 설정 로드·유스케이스 (feet 비율 변경 회귀)
```

### 3.3 Catch2 실행 명령 (목표 CMake)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build
ctest --test-dir build --output-on-failure

./build/unit_converter_tests "[ratio]"
./build/unit_converter_tests "T-D-02"
./build/unit_converter_tests "[parse]"
./build/unit_converter_tests "[integration]"
```

---

## 4. 경계값 케이스 목록

meter→feet 변환 및 동일 입력 파이프라인 기준. **기대 ERR 코드·prefix는 PRD §3.2 F-05 정본.**

| # | 케이스 | 입력 예시 | 검증 레이어 | 기대 결과 |
|---|--------|-----------|-------------|-----------|
| BV-01 | **value = 0** (영값) | `meter:0` | Boundary | ERR-INPUT-003, `Value must be positive: 0`, stdout 0줄, exit 1 |
| BV-02 | **매우 큰 수** (오버플로) | `meter:1e308` | Domain + Boundary | `feet = value × 3.28084` finite 검증; Inf 시 정책·테스트 고정 |
| BV-03 | **value < 0** | `meter:-1` | Boundary | ERR-INPUT-003, `Value must be positive: -1`, exit 1 |
| BV-04 | **소수 파싱 실패** | `meter:abc` | Boundary | ERR-INPUT-004, `Invalid number: abc`, exit 1 |
| BV-05 | **":" 없음** | `meter2.5` | Boundary | ERR-INPUT-001, `Invalid format. Use unit:value (ex: meter:2.5)`, exit 1 |
| BV-06 | **없는 단위** | `parsec:1.0` | Boundary | ERR-INPUT-002, `Unknown unit: parsec`, exit 1 |
| BV-06b | PRD 정본 | `mile:1` | Boundary | ERR-INPUT-002, `Unknown unit: mile`, exit 1 |
| BV-07 | 최소 양수 | `meter:1e-10` | Domain | ε 검증; round4 표시 정책 문서·테스트 동시 고정 |
| BV-08 | 비율 경계 | `meter:1` | Domain | feet `3.28084` (ε); table `3.2808 feet` (HALF_UP) |
| BV-09 | Happy Path | `meter:2.5` | Domain + Integration | feet `8.2021`, stderr空, exit 0 |

### 4.1 BV-02 오버플로 판정 절차

1. Domain: 환산 후 `std::isfinite` assert.
2. Boundary: Domain 실패 시 ERR-INPUT-004 vs DomainError 중 계약 1종 확정.
3. 통합: stdout 0줄·exit 1·stderr prefix 단일화.

---

## 5. 예외·특이 케이스 목록

| # | 카테고리 | 입력 / 조건 | 기대 | 비고 |
|---|----------|-------------|------|------|
| EX-01 | NaN | `meter:nan` | ERR-INPUT-004 | Domain 미전달 |
| EX-02 | Inf | `meter:inf` | ERR-INPUT-004 | |
| EX-03 | 빈 입력 | `` | ERR-INPUT-001 | |
| EX-04 | 콜론 2개+ | `meter:2:5` | ERR-001 또는 004 (split 규칙 고정) | |
| EX-05 | 잘못된 unit | `1meter:2.5` | ERR-INPUT-005 | |
| EX-06 | trim | ` meter : 2.5 ` | 성공 → feet `8.2021` | |
| EX-07 | feet 소스 | `feet:3.28084` | meter 허브 역변환 검증 | |
| EX-08 | 부분 출력 금지 | `parsec:1` 실패 | stdout 0줄 | |
| EX-09 | 성공 stderr | `meter:2.5` | stderr 0, exit 0 | AC-01 |
| EX-10 | 프로토타입 갭 | UnitConverter.cpp | `meter:0` 통과·round4 없음 | v1.0 인수 불가 |

---

## 6. 테스트 케이스 ↔ PRD 추적

| 테스트 ID | 설명 | AC / Goal |
|-----------|------|-----------|
| T-D-01 | 1m → 3.28084ft, 1.09361yd | G-01, AC-02 |
| T-D-02 | `meter:2.5` → feet 8.2021 (ε) | AC-02, Gherkin |
| T-D-03 | meter 경유, 독립 feet↔yard 비율 없음 | §3.3 |
| T-D-04 | ConvertAll 건수 = 3 | F-03 |
| T-D-05 | 0, 음수, NaN, Inf 거부 | F-02 |
| T-D-06 | 단위 추가 시 핵심 diff 0 | G-03 |
| T-B-05 | table round4 feet 줄 | §6.1 |
| T-I-01 | CLI `meter:2.5` 통합 | 시나리오 A |

---

## 7. 커버리지 목표

| 레이어 | Line | Branch | meter→feet 관련 조건 |
|--------|------|--------|----------------------|
| **Domain** | **≥ 95%** | **≥ 90%** | Convert·RatioToBase·ConvertAll feet 분기 |
| **Boundary** | **≥ 85%** | **≥ 80%** | Parser, round4, ERR-001~005 각 ≥1 |
| Data | ≥ 90% | ≥ 85% | units.json feet=3.28084 |
| Control | ≥ 80% | ≥ 75% | parse→convert→render 1경로 |
| **전체** | **≥ 85%** | — | Domain <95% → 인수 불가 |

### 7.1 필수 커버 경로 체크리스트

- [ ] `value_in_meter = source × k(meter)`
- [ ] `target_feet = value_in_meter / k(feet)` (`k(feet)=3.28084`)
- [ ] HALF_UP 4자리 `8.2021`
- [ ] ERR-001, 003, 004, 002(parsec/mile) 각 1회 이상

---

## 8. gcov / lcov 측정 전략

### 8.1 도구·환경

| OS | 설정 | 리포트 |
|----|------|--------|
| Linux / WSL | `-DENABLE_COVERAGE=ON`, `--coverage` | lcov + genhtml |
| Windows | WSL 권장 | 공식 집계는 Linux CI |

### 8.2 측정 대상

| 단계 | 대상 | 목적 |
|------|------|------|
| Baseline | `UnitConverter.cpp` | 레거시 gcov 갭 분석 |
| v1.0 인수 | `src/domain/*`, `src/boundary/*` | Domain 95% / Boundary 85% 게이트 |

### 8.3 lcov 워크플로

```bash
cmake -S . -B build-cov -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build-cov
ctest --test-dir build-cov --output-on-failure

lcov --capture --directory build-cov --output-file coverage.info
lcov --extract coverage.info '*/UnitConverter.cpp' --output-file coverage_unitconverter.info
genhtml coverage_unitconverter.info --output-directory Report/coverage_baseline

lcov --extract coverage.info '*/src/domain/*' '*/src/boundary/*' \
  --output-file coverage_layers.info
genhtml coverage_layers.info --output-directory Report/coverage_v1
```

### 8.4 gcov 해석 (feet 샘플)

- **UnitConverter.cpp**: `meter:2.5` 성공·Unknown unit만 커버 가능; BV-01·BV-04·round4 미커버 → 갭 리스트.
- **Domain**: feet 환산·ε 라인 95% 미만 → T-D-01/02 보강.
- **Boundary**: ERR 분기 80% 미만 → §4·§5 케이스 추가.

### 8.5 CI 게이트

```text
pre-merge: ctest R-01 GREEN → lcov Domain ≥95%, Boundary ≥85% → 실패 시 merge 차단
```

---

## 9. Gherkin 시나리오 매핑

| 시나리오 | When | Then (feet) |
|----------|------|-------------|
| Happy Path | `meter:2.5` | `2.5 meter = 8.2021 feet` (3줄 중 1줄) |
| Invalid format | `meter2.5` | ERR-001, no conversion lines |
| Unknown unit | `mile:1` / `parsec:1.0` | ERR-002, no conversion lines |

---

## 10. 리스크·미결정

| ID | 내용 | QA 조치 |
|----|------|---------|
| RISK-01 | BV-02 Inf 시 에러 코드 미정 | PRD §3.2 보완 + Catch2 1건 |
| RISK-02 | `8.2` vs `8.2021` 표기 | round4(8.2021) 우선 |
| RISK-03 | Windows gcov | WSL/CI Linux 공식 집계 |

---

## 11. 완료 정의 (DoD)

- [ ] P0: T-D-01, T-D-02, T-B-parse-01, T-I-01 GREEN
- [ ] BV-01~06, BV-06b Catch2 GREEN
- [ ] EX-01~09 계약·테스트 GREEN
- [ ] Domain line ≥95%, Boundary line ≥85% (lcov 증빙)
- [ ] `meter:2.5` → feet `8.2021`, stderr空, exit 0
- [ ] R-01 필수 세트 GREEN → v1.0 인수 검토

---

## 부록 A — 승인

| 역할 | 이름 | 서명 | 일자 |
|------|------|------|------|
| QA 리드 | | | |
| 개발 리드 | | | |
| PM / 강사 | | | |

---

*계약 변경 시 [../docs/PRD.md](../docs/PRD.md) §3.2·§3.3과 동일 PR에서 본 보고서(RPT-QA-001)를 갱신한다.*
