# To-Do 리스트 — UnitConverter (C++)

**기준 문서**: `docs/PRD.md` v1.0 (Phase 5)  
**판정 주체**: 학습자(실행·테스트) + 동료 리뷰어(구조·PR)  
**v1.0 정의**: AC-01~AC-03 필수 GREEN + R-01 회귀 세트 통과

---

## 🔴 필수 (Must-Have) — v1.0 릴리스 차단 항목

- [ ] **CMake 프로젝트 골격 추가** (실행 타깃 + Catch2 테스트 타깃 분리) | PRD §4.1 | **학습자**가 `cmake --build` 후 테스트·실행 바이너리 생성 성공, README 빌드 절차와 일치
- [ ] **Domain: UnitRegistry + meter 기본 3단위** | F-04, §5.1, Gherkin Background | **학습자**가 `[ratio]` 테스트에서 meter=1.0, feet=3.28084, yard=1.09361 로드·등록 GREEN
- [ ] **Domain: ConvertAll (전 단위 환산)** | F-03, G-01, AC-02 | **학습자**가 3단위 등록 시 결과 3건, 키 집합=Registry GREEN; ε=1e-9
- [ ] **Domain: 환산 정확도·자기환산·meter 경유** | §3.3, G-01 | **리뷰어**가 T-D-01~05 해당 Catch2 전부 GREEN; feet↔yard 독립 비율 없음 확인
- [ ] **Domain: LengthQuantity 양수·유한만 허용** | F-02, §3.3 | **학습자**가 0·음수·NaN·Inf 입력 시 Domain 예외/실패 테스트 GREEN
- [ ] **Boundary: `단위:값` 파싱·형식 검증** | F-01, AC-01, GH-02 | **학습자**가 `meter:2.5` 파싱 성공; `meter2.5`→ERR-INPUT-001 prefix 일치
- [ ] **Boundary: ERR-INPUT-001~005 prefix 고정** | §3.2 F-05, R-03 | **학습자**가 계약 스냅샷 테스트 5코드 각 GREEN; 임의 문구 변경 0건
- [ ] **Boundary: 미등록 단위 거부·부분 출력 금지** | F-05, GH-03, AC-01 | **학습자**가 `mile:1`→`Unknown unit: mile`, stdout 변환 줄 0, exit 1
- [ ] **Boundary: table 기본 출력 + round4** | §6.1, AC-02, GH-01 | **학습자**가 `meter:2.5` 시 3줄; `8.2021 feet`, `2.7340 yard` 포함; 성공 시 stderr空
- [ ] **Control: Boundary→Domain→Boundary 오케스트레이션** | §4.2 BCE | **리뷰어**가 Entity에 iostream/파일 API 없음; main은 위임만
- [ ] **통합: Gherkin Happy Path** | GH-01, AC-02 | **학습자**가 end-to-end `meter:2.5` exit 0, 3줄 수치 PRD §3.3과 일치
- [ ] **통합: Gherkin 실패 2건** | GH-02, GH-03, AC-01 | **학습자**가 형식·unknown 시나리오 exit 1, stderr prefix 일치
- [ ] **필수 Catch2 세트 (F-06)** | F-06, R-01, §4.3 Domain | **학습자**가 Domain line≥95%, branch≥90%, 실패 코드 8종 각 ≥1 테스트
- [ ] **초기 단일 파일 분기 제거** | G-03, §1.2, AC-03 | **리뷰어**가 환산 핵심 if/else 비율 하드코딩 0줄; 변환은 Registry 경유만
- [ ] **README ↔ PRD round4 정합** | §6.1, R-02 | **학습자**가 README 예시 출력을 8.2021/2.7340(4자리)로 갱신 또는 PRD 우선 주석 명시

---

## 🟡 권장 (Should-Have) — 품질 향상 항목

- [ ] **JSON 출력 (format_version=1)** | F-07, §6.2, AC-04 | **학습자**가 `[json]` 테스트 ≥3 GREEN; conversions 길이=Registry 수
- [ ] **CSV 출력 (헤더·행 수·round4)** | F-07, §6.3, AC-04 | **학습자**가 `[csv]` 테스트 ≥3 GREEN; 헤더 exact match
- [ ] **table/json/csv 동형성 검증** | §6.4, G-04 | **학습자**가 동일 입력 시 (target_unit, round4 value) 집합 3포맷 동일 테스트 GREEN
- [ ] **JSON 설정 로드 (units.json)** | F-08, §5.2, AC-05 | **학습자**가 기본 JSON 로드 후 3단위 환산 GREEN
- [ ] **설정 로드 실패 경로** | F-08, US-05, ERR-DATA-* | **학습자**가 파일 없음·base_unit 오류·ratio≤0·malformed JSON 각 ERR prefix GREEN
- [ ] **동적 등록 `register:… meter`** | F-09, §5.3, AC-05 | **학습자**가 cubit 등록 후 `cubit:1`→0.4572m(ε); ConvertAll에 cubit 포함
- [ ] **등록 실패 (중복·ratio 0)** | F-09, §5.3 | **학습자**가 ERR-DOM-001/002 prefix 테스트 GREEN
- [ ] **Boundary Mock + Domain 분리 (Dual-Track)** | §4.2, §4.4 | **리뷰어**가 `[parse]`/`[json]` 테스트가 Domain Mock 사용; Domain 테스트 I/O 0
- [ ] **Data 레이어 커버리지** | §4.3 Data | **학습자**가 Data line≥90%, branch≥85% 리포트 첨부
- [ ] **Traceability 표 (docs)** | §7.1 Epic SC-06 | **학습자**가 US-01~06·GH 시나리오별 Test-ID 1:1 매핑표 커밋, 빈칸 0
- [ ] **회귀 필수 목록 문서화** | R-01, §7.2 | **학습자**가 `docs/` 또는 README에 pre-merge 필수 테스트 목록·실행 명령 기재
- [ ] **.cursorrules (Dual-Track 금지 규칙)** | §4.2 | **리뷰어**가 project~ai_behavior 8키 존재, Domain I/O forbidden 명시

---

## 🟢 선택 (Nice-to-Have) — v2.0 후보

- [ ] **등록 결과 JSON 저장·재시작 로드** | F-10 | 재실행 후 cubit 유지로 운영형 연습 가능
- [ ] **YAML 설정 호환** | §5.2 | JSON과 동등 스키마 로더 추가 시 설정 선택지 확대
- [ ] **출력 포맷 CLI 플래그 (`--format=json`)** | F-07 | 사용자가 변환 없이 표현만 전환하는 OCP 연습 강화
- [ ] **Property-based ratio 테스트 (100 random)** | §4.3 | 등록 단위 N≥3일 때 쌍별 D-INV-3 자동 검증
- [ ] **커버리지 CI 아티팩트 업로드** | §4.3 | PR마다 line/branch 리포트 자동 보관
- [ ] **Gherkin `.feature` 파일 + 테스트 링크** | GH-01~03 | 요구→실행 추적을 저장소 파일로 고정

---

## 🔵 기술 부채 (Tech Debt)

- [ ] **단일 파일에 파싱·환산·출력 결합** | §1.2 초기 산출물 | **학습자**가 BCE 분리 완료 시 `[x]`; until then v1.0 차단 항목과 중복 추적
- [ ] **README 8.2 feet vs PRD round4 8.2021 불일치** | §6.1, R-02 | **학습자**가 README 수치 갱신 또는 “표시는 PRD round4 우선” 한 줄 추가
- [ ] **g++ 단일 명령 빌드만 문서화** | README §빌드 | **학습자**가 CMake 절차로 README 교체; g++는 부록 또는 삭제
- [ ] **Catch2·테스트 디렉터리 부재** | F-06 | **학습자**가 tests/ + CTest 등록 후 R-01 자동화 가능
- [ ] **ERR-DATA 계약 미구현** | F-08 | **학습자**가 Data 레이어 도입 시 ERR-DATA-001~003 prefix 테스트 추가
- [ ] **비율 상수 코드 중복 (3.28084 다처)** | §5.1 | **학습자**가 JSON 단일 출처화 후 Domain은 스냅샷만 소비

---

## ✅ 완료 항목 (Done)

- [x] **Phase 5 PRD 작성** | 2026-05-20 | docs: PRD v1.0 (Epic/Story/Gherkin/인수/회귀 반영)
- [x] **초기 프로토타입 (단일 파일 3단위 변환)** | 2026-05-20 | baseline: meter/feet/yard if/else 동작 (리팩터 전)
- [x] **README 기본 요구·비율·실습 Activities 정의** | 2026-05-20 | baseline: 요구사항 출처 고정

---

## 📋 회귀 방지 체크리스트 (PRD §7.2 기반)

**v1.0 태그·merge 전 — 학습자 실행, 리뷰어 확인**

| # | 확인 항목 | 통과 조건 | PRD 규칙 |
|---|-----------|-----------|----------|
| 1 | 필수 테스트 세트 | T-D-01~06, T-B-05~07, T-I-01, GH 3시나리오 **전부 GREEN** | R-01 |
| 2 | ERR prefix 계약 | ERR-INPUT-001~005 스냅샷 **diff 0** (의도 변경만 예외) | R-03 |
| 3 | Gherkin 수치 | `meter:2.5` → 2.5 / 8.2021 / 2.7340 (table, round4) | R-02, §3.3 |
| 4 | Domain 커버리지 | line ≥ 95%, branch ≥ 90% | §4.3 |
| 5 | Boundary 커버리지 | line ≥ 85%, branch ≥ 80% | §4.3 |
| 6 | Entity 순수성 | Domain 소스에 iostream/파일 API **0건** | R-04 |
| 7 | 신규 단위 시 테스트 갱신 | ConvertAll 기대 건수 **N→N+1** 테스트 포함 | R-05 |
| 8 | 비율 변경 동시성 | `units.json` + ratio 테스트 + PRD §5.1 **동일 PR** | R-02 |
| 9 | README 실행성 | 문서 명령만으로 build·test·run **성공** | §2.2 시나리오 A |
| 10 | AC 체크리스트 | PRD §7.1 AC-01~03 (v1.0) **전부 [x]** | §7.1 |

**v1.1+ (권장 완료 후 추가)**

| # | 확인 항목 | 통과 조건 |
|---|-----------|-----------|
| 11 | AC-04 | table·json·csv 각 ≥3 Catch2 GREEN |
| 12 | AC-05 | JSON 로드·실패·cubit 등록 통합 GREEN |
| 13 | Data 커버리지 | line ≥ 90%, branch ≥ 85% |
| 14 | Traceability | US-01~06 미매핑 Test-ID **0건** |

---

## 🗓️ 마일스톤

| 마일스톤 | 포함 항목 (PRD) | 목표일 | 상태 |
|----------|-----------------|--------|------|
| **M0 — 문서·베이스라인** | PRD, README, 초기 프로토타입 | 2026-05-20 | ✅ Done |
| **M1 — v1.0 Domain+Table** | F-02, F-03, F-04, F-06(Domain), §4.3 Domain, G-01, AC-02, GH-01 | 2026-05-27 | 🔴 Not started |
| **M2 — v1.0 Boundary+인수** | F-01, F-05, F-06(Boundary/통합), AC-01, GH-02, GH-03, R-01, §6.1 | 2026-06-03 | 🔴 Blocked by M1 |
| **M3 — v1.1 출력·설정** | F-07, F-08, AC-04, AC-05, G-04, §5.2, §6.2~6.4 | 2026-06-10 | 🟡 Planned |
| **M4 — v1.1 등록·품질** | F-09, §5.3, §4.3 Data/Control, Traceability, .cursorrules | 2026-06-17 | 🟡 Planned |
| **M5 — v2.0 후보** | F-10, YAML, property tests | TBD | 🟢 Backlog |

---

*상태 범례: 🔴 차단/미착수 · 🟡 진행/계획 · ✅ 완료 · v1.0 = Must-Have 전부 + 회귀 #1~#10 통과*
