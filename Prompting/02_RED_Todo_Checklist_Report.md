# RED 단계 To-Do 리스트 추가
_Exported on 5/21/2026 at 09:42:50 GMT+9 from Cursor (3.4.17)_

---

**User**

@c:\DEV\UnitConverter_08/README.md 파일에 아래 섹션을 추가해줘.
기존 내용은 유지하고, "## RED 단계 To-Do 리스트" 섹션만 새로 삽입.

추가할 섹션 형식:

## RED 단계 To-Do 리스트

> 이 체크리스트는 test_plan.md 기반으로 생성되었습니다.
> 각 항목은 RED(실패 테스트 작성) 완료 시 체크합니다.

### Track A — UI / Boundary 테스트
- [ ] TC-A-01: 정상 입력 "meter:2.5" → 변환 결과 반환 (Happy Path)
- [ ] TC-A-02: ":" 없는 입력 → std::invalid_argument 발생
- [ ] TC-A-03: 음수 입력 "meter:-1.0" → std::invalid_argument 발생
- [ ] TC-A-04: 없는 단위 "parsec:1.0" → std::invalid_argument 발생
- [ ] TC-A-05: 소수점 파싱 실패 "meter:abc" → std::invalid_argument 발생
- [ ] TC-A-06: 출력 포맷에 원 입력 단위·값 보존 ("2.5 meter = ...")
- [ ] TC-A-07: value=0 경계값 처리 확인

### Track B — Domain / Logic 테스트
- [ ] TC-B-01: convert("meter", 2.5, "feet") == 8.20210 (오차 1e-5)
- [ ] TC-B-02: convert("meter", 1.0, "yard") == 1.09361 (오차 1e-5)
- [ ] TC-B-03: convert("feet", 1.0, "meter") == 0.30480 (역변환)
- [ ] TC-B-04: convertAll("meter", 1.0) → 모든 등록 단위 변환 반환
- [ ] TC-B-05: registerUnit("cubit", 0.4572) 후 변환 가능
- [ ] TC-B-06: loadConfig(유효한 경로) → 비율 정상 로드
- [ ] TC-B-07: loadConfig(없는 경로) → 기본값(3.28084/1.09361) 유지

### 커버리지 목표
- [ ] Domain Logic: 95%+ (# gcov / lcov)
- [ ] Boundary Layer: 85%+
- [ ] 전체 TOTAL: 90%+

### 결함 목록 연결
- [ ] defect_list.md 생성 및 발견 결함 기록
- [ ] 모든 결함 수정 후 회귀 테스트 통과 확인

코드 작성은 금지

---

**Cursor**

README.md 구조를 확인한 뒤 적절한 위치에 섹션을 삽입합니다.


README.md에 새 섹션을 삽입하고 목차에 항목을 추가합니다.


`README.md`에 **## RED 단계 To-Do 리스트** 섹션을 추가했습니다.

**변경 사항**
- **위치**: `## RED 단계에서 해야 할 일` 바로 다음, `## 빠른 시작 (Quick Start)` 앞
- **목차**: `[RED 단계 To-Do 리스트](#red-단계-to-do-리스트)` 항목 추가
- **기존 내용**: 그대로 유지

Track A/B, 커버리지 목표, 결함 목록 연결 체크리스트는 요청하신 형식 그대로 반영했습니다. 코드는 수정하지 않았습니다.

---

**User**

Report 폴더에 보고서로 작성해줘

---

**Cursor**

Report 폴더 구조와 기존 보고서 형식을 확인한 뒤 보고서를 작성합니다.


README의 RED To-Do를 기존 QA 보고서 형식에 맞춰 Report 폴더에 작성합니다.


`Report` 폴더에 RED 단계 체크리스트 보고서를 추가했습니다.

## 생성된 보고서

**[Report/RED_Todo_Checklist_Report.md](Report/RED_Todo_Checklist_Report.md)** (RPT-RED-001)

| 섹션 | 내용 |
|------|------|
| 문서 관리 | 버전, 정본 참조, RPT-QA-001 연계 |
| 요약 | Track A/B 7건 + 커버리지 3 + 결함 2 |
| Track A/B | TC별 입력·기대값·PRD 매핑 테이블 + README와 동일 체크리스트 |
| 커버리지·결함 | gcov/lcov 목표, `defect_list.md` 운영 |
| 추적 매트릭스 | TC-A/B ↔ PRD ID |
| DoD | RED 완료 조건 19항목 |
| 진행 현황 | 0/19 (0%) 요약표 |

## 함께 갱신한 문서

- **Report/README.md** — 보고서 목록에 RPT-RED-001 추가, QA 보고서 파일명 정정
- **docs/test_plan.md** — RED To-Do 보고서 링크 추가
- **README.md** — RED To-Do 섹션에 상세 보고서 링크 1줄 추가

README 체크리스트와 보고서 항목은 동일하며, 보고서에는 PRD 매핑·Catch2 태그·계약 정합(`std::invalid_argument` vs ERR-INPUT) 안내가 추가되어 있습니다.

