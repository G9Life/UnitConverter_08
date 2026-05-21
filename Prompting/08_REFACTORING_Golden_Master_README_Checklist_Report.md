# Golden Master regression safety mechanism
_Exported on 5/21/2026 at 13:42:00 GMT+9 from Cursor (3.4.17)_

---

**User**

@c:\DEV\UnitConverter_08/README.md 파일에 아래 섹션을 추가해줘.
"## RED 단계 To-Do 리스트" 아래에 새 섹션으로 삽입.

## Golden Master 회귀 안전장치

> Refactoring 시작 전 구축. GREEN 완료 후 즉시 적용.

### 기준 파일 생성
- [ ] GM-01: golden_master_expected.txt 생성 (meter:2.5 기준 출력)
- [ ] GM-02: feet:1.0 / yard:1.0 / meter:0.0 시나리오 추가
- [ ] GM-03: git add tests/golden_master_expected.txt (버전 관리 포함)

### 테스트 코드
- [ ] GM-04: test_golden_master.cpp + golden_master_expected.txt 작성
- [ ] GM-05: approve 패턴 적용 (파일 없으면 생성, 있으면 비교)
- [ ] GM-06: CMake: add_test(NAME GoldenMaster COMMAND UnitConverter_test) → PASS 확인

### CI 연동
- [ ] GM-07: .github/workflows/golden_master.yml 작성
- [ ] GM-08: PR 머지 차단 (required status check) 설정
- [ ] GM-09: Refactoring 후 Golden Master 재실행 → PASS 확인


---

**Cursor**

README.md 구조를 확인한 뒤 지정한 위치에 섹션을 추가합니다.


목차에 새 섹션을 추가하고, RED To-Do 리스트 바로 아래에 Golden Master 섹션을 삽입합니다.


`README.md`에 **Golden Master 회귀 안전장치** 섹션을 추가했습니다.

**변경 내용**
1. **목차** — `RED 단계 To-Do 리스트`와 `빠른 시작` 사이에 링크 추가
2. **본문** — `## RED 단계 To-Do 리스트` 섹션 끝(결함 목록 연결 항목 다음)과 `## 빠른 시작` 사이에 요청하신 체크리스트 전체 삽입

GM-01~GM-09 항목(기준 파일 생성, 테스트 코드, CI 연동)은 요청하신 그대로 반영했습니다.

---

**User**

Report 폴더에 08_REFACTORING_ 시작되는 파일명으로 보고서를 작성해줘

---

**Cursor**

Report 폴더의 기존 보고서 형식과 Golden Master 관련 작업 현황을 확인합니다.


README Golden Master 체크리스트 반영 현황을 정리한 08번 보고서를 작성하고 `Report/README.md`를 갱신합니다.


`Report/08_REFACTORING_Golden_Master_README_Checklist_Report.md` (RPT-GM-003)를 작성했고, `Report/README.md` 목록도 갱신했습니다.

## 보고서 요약

**문서**: `Report/08_REFACTORING_Golden_Master_README_Checklist_Report.md`  
**ID**: RPT-GM-003

README에 추가한 **Golden Master 회귀 안전장치**(GM-01~09)와 저장소 실제 상태를 대조한 진행·갭 분석 보고서입니다.

| GM 항목 | 상태 |
|---------|------|
| GM-01, GM-02, GM-04, GM-06 | **완료** (기준 파일·4 시나리오 테스트·`ctest -R GoldenMaster` PASS) |
| GM-03 | **미완** — `git add` / 커밋 필요 |
| GM-05 | **부분** — 섹션 비교만 구현, approve 자동 생성은 미구현 (06번 설계와 차이) |
| GM-07~08 | **미완** — CI 워크플로·required check 없음 |
| GM-09 | **대기** — 리팩터링 PR 후 재실행 기록 예정 |

**실행 검증** (2026-05-21): `GoldenMaster` PASS (~3.7s), 전체 CTest **50/50 PASS**.

보고서에는 README 체크박스 권장 갱신안, GM-07용 GitHub Actions 초안, 06·07번 보고서와의 추적성 표도 포함했습니다.

`Report/README.md`에는 08번 항목을 추가했고, 07번 링크 파일명을 실제 파일(`07_REFECTORING_Golden_Master_Test_Code_Report.md`)과 맞춰 두었습니다.

