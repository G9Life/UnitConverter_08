# Golden Master file generation strategy
_Exported on 5/21/2026 at 13:18:44 GMT+9 from Cursor (3.4.17)_

---

**User**

[P] 회귀 테스트(Approval/Golden Master) 설계 전문가입니다.
[C] C++17, CMake, Catch2
[T] UnitConverter 출력 기반 Golden Master 기준 파일을 생성해줘.

    1) 기준 출력(tests/golden_master_expected.txt) 생성/보관 전략
       입력 시나리오 4개:
         "meter:2.5" / "feet:1.0" / "yard:1.0" / "meter:0.0"
       캡처 방식: stdout 리디렉션: ./UnitConverter < input.txt > actual.txt
       생성 후: git add tests/golden_master_expected.txt  (버전 관리 필수)

    2) approve 패턴 적용:
       - 기준 파일 없으면 → 현재 출력을 기준으로 자동 생성
       - 기준 파일 있으면 → actual vs expected 문자열 비교
       - 불일치 시: diff 출력 후 테스트 FAIL

    3) 기준 파일 구조:
       [meter:2.5]
       2.5 meter = 8.202100 feet
       2.5 meter = 2.734025 yard
       ---
       [feet:1.0]
       ...

[F] tests/golden_master_expected.txt 파일 내용 + 생성 스크립트


---

**Cursor**

Golden Master 기준 파일과 approve 패턴을 추가하기 위해 프로젝트 구조와 UnitConverter 출력 형식을 확인합니다.


실제 출력을 캡처해 기준 파일 형식을 확정합니다.


실제 출력(4자리, 3줄)을 기준으로 Golden Master 파일·Catch2 테스트·생성 스크립트를 추가합니다.


프롬프트와 첫 변환 줄이 한 줄에 붙어 있어 첫 줄이 누락됩니다. 프롬프트 접두어만 제거하도록 수정합니다.


Golden Master 회귀 테스트를 추가했습니다. **46건 전부 통과**했고, 기준 파일은 `git add`까지 반영해 두었습니다.

## 산출물

| 파일 | 역할 |
|------|------|
| `tests/golden_master_expected.txt` | 버전 관리되는 기준 출력 |
| `tests/golden_master_tests.cpp` | Catch2 approve 비교 (`[golden][regression][r01]`) |
| `scripts/generate_golden_master.ps1` | Windows 기준 파일 재생성 |
| `scripts/generate_golden_master.sh` | Unix 기준 파일 재생성 |
| `CMakeLists.txt` | GREEN 빌드 시에만 golden 테스트 링크 |

## Approve 동작

1. **기준 없음** → 현재 stdout을 `tests/golden_master_expected.txt`에 자동 생성 후 `WARN` (리뷰 후 커밋)
2. **기준 있음** → 4개 시나리오 stdout을 캡처해 문자열 비교
3. **불일치** → `--- expected` / `+++ actual` 형태 diff 출력 후 `FAIL`

프롬프트와 첫 변환 줄이 한 줄에 붙는 경우(`Insert value...: 2.5000 meter = ...`)를 위해 `): ` 이후만 추출합니다.

## 기준 파일 (`tests/golden_master_expected.txt`)

```
[meter:2.5]
2.5000 meter = 8.2021 feet
2.5000 meter = 2.5000 meter
2.5000 meter = 2.7340 yard
---
[feet:1.0]
1.0000 feet = 1.0000 feet
1.0000 feet = 0.3048 meter
1.0000 feet = 0.3333 yard
---
[yard:1.0]
1.0000 yard = 3.0000 feet
1.0000 yard = 0.9144 meter
1.0000 yard = 1.0000 yard
---
[meter:0.0]
---
```

`meter:0.0`은 stderr에 `ERR-INPUT-003`만 나가고 stdout 변환 줄은 없습니다 (PRD BV-01).

예시의 `2.5` / `8.202100` 형식과 달리, 실제 바이너리는 **round4·setprecision(4)** 기준(`2.5000`, `8.2021`)입니다.

## 생성 스크립트

```powershell
# GREEN 빌드 후
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
.\scripts\generate_golden_master.ps1
git add tests/golden_master_expected.txt
```

```bash
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
./scripts/generate_golden_master.sh
git add tests/golden_master_expected.txt
```

## 테스트 실행

```powershell
cd build
ctest -R "Golden Master" --output-on-failure
```

RED 빌드(`UNIT_CONVERTER_RED_PHASE=ON`)에서는 golden 테스트가 **SKIP**됩니다.

---

**User**

Report 폴더에 보고서로 작성해줘

---

**Cursor**

Report 폴더의 기존 보고서 형식을 확인한 뒤 Golden Master 회귀 테스트 보고서를 작성합니다.


`Report/06_Golden_Master_Regression_Report.md` (RPT-GM-001)를 추가했고, `Report/README.md` 인덱스도 갱신했습니다.

## 보고서 구성

| 섹션 | 내용 |
|------|------|
| **요약** | 4 시나리오, approve 패턴, 46건 PASS, RED 시 SKIP |
| **§1 목적·범위** | R-01 회귀, PRD/TODO 추적 |
| **§2 설계** | 캡처 방식, approve 상태 전이(mermaid), 프롬프트·stderr 처리 |
| **§3 시나리오** | GM-01~04 표, 기준 파일 구조, round4 정합, `meter:0.0` |
| **§4 구현** | 파일 맵, CMake, Catch2 태그 |
| **§5 운영** | `generate_golden_master.ps1/.sh`, git add, 갱신 워크플로 |
| **§6 실행 결과** | Golden Master PASS, 46/46 |
| **§7~9** | 추적성, 제한·향후, 결론 |
| **부록 A/B** | 전체 `golden_master_expected.txt`, 스크립트 요약 |

문서 경로: [Report/06_Golden_Master_Regression_Report.md](Report/06_Golden_Master_Regression_Report.md)

