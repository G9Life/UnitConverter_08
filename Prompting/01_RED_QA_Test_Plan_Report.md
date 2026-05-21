# Test plan sample selection criteria
_Exported on 5/21/2026 at 09:36:28 GMT+9 from Cursor (3.4.17)_

---

**User**

@c:\DEV\UnitConverter_08/README.md 와 PRD 문서를 참고하여
테스트 플랜 작성을 위한 샘플 예제 1개를 선택해줘.

선택 기준:
1. README의 기본 요구사항에 명시된 핵심 변환 기능
2. 비즈니스 로직(1 meter = 3.28084 feet, 1 meter = 1.09361 yard)을 직접 검증
3. 정상 경로(happy path)와 경계값 모두 포함 가능한 기능

선택한 예제에 대해 다음을 명시하라:
- 선택한 기능 이름 (예: meter → feet 변환)
- README 요구사항 번호 (예: 기본 요구사항 4번)
- 입력 예시: "meter:2.5"
- 기대 출력: "2.5 meter = 8.202100 feet"
- 이 예제를 선택한 이유 2줄

코드 작성 금지. 선택 결과 문서만 출력.


---

**Cursor**

README와 PRD를 확인해 테스트 플랜용 샘플 예제를 선정합니다.


# 테스트 플랜 샘플 예제 선정

## 선택 결과

| 항목 | 내용 |
|------|------|
| **선택한 기능 이름** | **meter → feet 변환** (meter 허브 기준 `ConvertAll`의 대표 검증; 동일 입력으로 meter·yard 동시 확인 가능) |
| **README 요구사항 번호** | **기본 요구사항 4번** ([docs/requirment.md](docs/requirment.md) — *각 단위 간 변환이 정확히 계산되도록 테스트 코드를 작성할 것*) · README 정합: [RED §3 Domain](README.md), [통합 §5 `meter:2.5`](README.md), [PRD §3.3·§5.1·G-01](docs/PRD.md) |
| **입력 예시** | `meter:2.5` |
| **기대 출력** | `2.5 meter = 8.2021 feet` *(table 포맷, HALF_UP 소수 4자리; PRD §3.3·README 출력 포맷 계약)* |

**동일 입력의 전체 table 기대값 (통합·경계 포함 시 참고)**

```text
2.5 meter = 2.5 meter
2.5 meter = 8.2021 feet
2.5 meter = 2.7340 yard
```

- stderr: (비어 있음)  
- exit code: `0`

---

## 선택 이유 (2줄)

1. `meter:2.5`는 README·PRD Gherkin의 **고정 Happy Path**이며, `2.5 × 3.28084 = 8.2021`(feet)과 `2.5 × 1.09361 → 2.7340`(yard)로 **비즈니스 비율 두 개를 한 시나리오에서 직접 검증**할 수 있다.  
2. 같은 기능 축으로 **정상 경로**(양수 소수), **경계값**(`meter:1` → 정확히 `3.28084` / `1.09361`), **거부 경로**(`meter:0`, `meter:-1` → ERR-INPUT-003)를 Domain·Boundary·통합 테스트에 나눠 작성하기 좋다.

---

## 보조 검증 포인트 (테스트 플랜 확장용)

| 구분 | 입력 / 조건 | 기대 (feet 관점) |
|------|-------------|------------------|
| Happy path | `meter:2.5` | `8.2021 feet` (표시), Domain 내부 `8.2021` (ε=1e-9) |
| 비율 경계 | `meter:1` | `3.28084 feet` (1 m = 3.28084 ft 직접 검증) |
| 값 경계 (거부) | `meter:0`, `meter:-1` | 변환 없음, ERR-INPUT-003, exit 1 |

---

**User**

[P] 시니어 QA 리드입니다.
[C] C++17, CMake, Catch2
[T] 선택한 샘플 예제(meter→feet 변환)를 기반으로 테스트 계획서를 작성해줘.

    포함 항목:
    - Catch2 기반 단위 테스트 범위/우선순위
    - 경계값 케이스 목록:
        · value = 0 (영값 변환)
        · value = 매우 큰 수 (오버플로 위험)
        · value < 0 (음수 입력 정책)
        · 소수점 파싱 실패 ("meter:abc")
        · ":"없는 입력 (형식 오류)
        · 없는 단위 ("parsec:1.0")
    - 예외/특이 케이스 목록
    - 커버리지 목표 (Domain 95%+ / Boundary 85%+)
    - # gcov / lcov / gcov UnitConverter.cpp 측정 전략

[F] Markdown 문서. test_plan.md로 저장.


---

**Cursor**

PRD·README 계약과 프로젝트 구조를 확인한 뒤 `test_plan.md`를 작성합니다.


계약 정본(PRD·README)에 맞춰 `test_plan.md`를 작성합니다.


`test_plan.md`를 프로젝트 루트에 저장했습니다.

## 문서 요약

**경로**: [test_plan.md](c:\DEV\UnitConverter_08\test_plan.md)

| 섹션 | 내용 |
|------|------|
| **§3 Catch2** | P0~P2 우선순위, T-D-01~06 / T-B-05~07 / T-I-01, `[ratio]`·`[parse]`·`[integration]` 태그, Dual-Track RED→GREEN 순서 |
| **§4 경계값** | 요청 6종(BV-01~06) + PRD 정본 `mile:1`, 최소 양수·`meter:1` 비율 경계, BV-02 오버플로 판정 절차 |
| **§5 예외** | NaN/Inf, 빈 입력, 콜론 중복, unit_token 오류, trim, feet 역변환, 부분 출력 금지, 프로토타입 갭 |
| **§7 커버리지** | Domain **≥95% line / ≥90% branch**, Boundary **≥85% / ≥80%**, Domain 미달 시 v1.0 인수 불가 |
| **§8 gcov/lcov** | Baseline(`UnitConverter.cpp`) vs v1.0(`src/domain`, `src/boundary`), CMake `-DENABLE_COVERAGE=ON`, lcov extract·genhtml, CI 게이트 |

**샘플 예제 고정값**: `meter:2.5` → table `2.5 meter = 8.2021 feet` (HALF_UP 4자리, PRD §3.3).

**참고**: 요청하신 `parsec:1.0`은 BV-06에 포함했고, PRD 정본 `mile:1`은 BV-06b로 병기했습니다. 레거시 프로토타입은 round4·`meter:0` 거부가 없어 §8 baseline 측정용으로만 구분했습니다.

---

**User**

Report 폴더에 보고서로 작성해줘

---

**Cursor**

Report 폴더 구조와 기존 테스트 계획서 위치를 확인합니다.


Report 폴더에 보고서 형식으로 작성하고, `docs/test_plan.md`는 Report 문서를 가리키도록 정리합니다.


`Report` 폴더에 보고서 형식으로 정리했습니다.

## 생성·변경 파일

| 경로 | 설명 |
|------|------|
| [Report/QA_Test_Plan_Report.md](Report/QA_Test_Plan_Report.md) | **정본** — RPT-QA-001, Executive Summary, 문서 관리, 승인란(부록 A) 포함 |
| [Report/README.md](Report/README.md) | Report 폴더 인덱스 |
| [docs/test_plan.md](docs/test_plan.md) | Report 보고서로 이관 안내(리다이렉트) |

## 보고서 구성 (추가된 보고서 전용 항목)

- **문서 관리**: 보고서 ID, 버전, 작성일, 인수 게이트
- **요약 (Executive Summary)**: P0 테스트·커버리지·실패 정책 한눈에 표기
- **상대 경로**: `Report/` 기준 `../docs/PRD.md`, `../UnitConverter.cpp` 등
- **lcov HTML 출력 경로**: `Report/coverage_baseline`, `Report/coverage_v1`
- **부록 A**: QA/개발/PM 승인 표

내용(경계값·예외·Catch2·gcov)은 기존 테스트 계획과 동일하며, 보고서 헤더·요약·승인란만 추가했습니다.

