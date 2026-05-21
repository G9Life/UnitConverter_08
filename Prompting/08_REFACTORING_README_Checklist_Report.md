# UnitConverter.cpp test analysis
_Exported on 5/21/2026 at 14:05:21 GMT+9 from Cursor (3.4.17)_

---

**User**

아직 코드는 수정하지 말고 분석만 해줘.
@UnitConverter.cpp 를 보고
아래 두 가지만 먼저 답해줘:
1. 현재 테스트 파일(test_*.cpp)이 존재하는지 확인하고,
   없다면 어느 파일에 대한 테스트가 필요한지 알려줘
2. 테스트 없이 리팩토링을 시작하면 안 되는 이유를 한 줄로 설명해줘


---

**Cursor**

`UnitConverter.cpp`와 테스트 파일 존재 여부를 확인 중입니다.


## 1. `test_*.cpp` 존재 여부

**존재합니다.** `test_` 접두 패턴에 맞는 파일은 1개입니다.

| 파일 | 역할 |
|------|------|
| `tests/test_golden_master.cpp` | `UnitConverter` 실행 파일의 stdout 회귀 테스트 (Golden Master, 4 시나리오) |

같은 `tests/` 폴더에 이름 패턴은 다르지만 Catch2 테스트도 있습니다.

| 파일 | 대상 |
|------|------|
| `tests/domain_tests.cpp` | `src/domain/Converter.cpp`, `UnitRegistry.cpp` |
| `tests/boundary_tests.cpp` | `src/boundary/InputParser.cpp` |
| `tests/data_tests.cpp` | `src/data/ConfigLoader.cpp` |

`UnitConverter.cpp`는 루트가 아니라 **`src/UnitConverter.cpp`** 에 있습니다. 이 파일은 `main()`에서 파싱·변환·출력을 묶고, 파일 안에만 있는 `roundHalfUp4()`로 소수 4자리 반올림을 합니다.

```13:37:c:\DEV\UnitConverter_08\src\UnitConverter.cpp
double roundHalfUp4(double value) {
    return std::round(value * 10000.0) / 10000.0;
}
// ...
        for (const auto& entry : results) {
            const double display = roundHalfUp4(entry.value);
            std::cout << std::fixed << std::setprecision(4)
                      << parsed.value << " " << parsed.unit << " = "
                      << display << " " << entry.unit << std::endl;
```

**`UnitConverter.cpp` 관점 정리**

- **의존 로직** (`InputParser`, `Converter`, `UnitRegistry`): 위 3개 단위 테스트로 이미 커버됩니다.
- **CLI 자체** (프롬프트, 출력 포맷, `roundHalfUp4`, exit code): `test_golden_master.cpp`가 E2E로 커버합니다.
- **주의**: Golden Master는 CMake에서 `UNIT_CONVERTER_RED_PHASE=OFF`(GREEN 빌드)일 때만 `unit_converter_tests`에 포함됩니다 (`CMakeLists.txt` 47–49행).

`test_*.cpp`가 **없었다면** 우선 필요한 테스트는 `src/UnitConverter.cpp`(또는 리팩터 후 thin `main`)용 Golden Master/통합 테스트와, 분리한다면 `roundHalfUp4`·출력 포맷용 Boundary `[table]` 테스트입니다. 지금은 Golden Master가 그 역할을 일부 담당합니다.

---

## 2. 테스트 없이 리팩토링을 시작하면 안 되는 이유 (한 줄)

**동작을 고정하는 자동 테스트가 없으면 구조만 바꿔도 변환·반올림·출력 형식 같은 회귀를 바로 알 수 없기 때문입니다.**

---

**User**

@UnitConverter.cpp 의 코드 스멜을 분석해줘.
코드는 절대 수정하지 말고, 문제 있는 부분만 아래 표 형식으로:
| 파일명 | 줄번호 | 스멜 종류 | 문제 설명 | 우선순위 |

점검 항목:
- 단일 main() 함수에 파싱·검증·변환·출력이 혼재 (SRP 위반)
- if-else 체인 (새 단위 추가 시 기존 코드 수정 필요, OCP 위반)
- 매직 넘버 3.28084 / 1.09361 하드코딩
- 예외 처리 없음 또는 불완전한 예외 처리
- 긴 함수 (20줄 초과)
- 사용하지 않는 변수·함수


---

**Cursor**

`UnitConverter.cpp`를 다시 읽고 점검 항목별로 스멜을 분류합니다.


## `src/UnitConverter.cpp` 코드 스멜

점검 항목 중 **이 파일에 해당하지 않는 것**은 표에 넣지 않았습니다.

- **if-else 단위 분기 / OCP**: 이 파일에 없음 (`Converter`·`UnitRegistry` 쪽 책임)
- **비율 매직 넘버 `3.28084` / `1.09361`**: 이 파일에 없음

| 파일명 | 줄번호 | 스멜 종류 | 문제 설명 | 우선순위 |
|--------|--------|-----------|-----------|----------|
| `UnitConverter.cpp` | 19–44 | SRP 위반 (God `main`) | `main()`이 프롬프트(20), stdin 입력(22–25), 파싱·변환 오케스트레이션(28–30), 표시 반올림·포맷 출력(32–37), 예외 응답(38–40)을 한곳에서 처리함 | 높음 |
| `UnitConverter.cpp` | 13–15, 32–36 | SRP 위반 (표시 책임 혼재) | `roundHalfUp4()`와 `setprecision(4)` 출력 포맷이 `main`과 같은 TU에 있어, Boundary/표시 계층으로 분리하기 전 상태 | 높음 |
| `UnitConverter.cpp` | 23–24 | 불완전한 예외·오류 처리 | `getline` 실패(EOF 등) 시 사용자 메시지 없이 `return 1`만 수행해, 파싱 오류(38–40) 경로와 동작·UX가 불일치 | 중간 |
| `UnitConverter.cpp` | 38–40 | 불완전한 예외 처리 | `std::invalid_argument`만 포착; 현재 하위 모듈은 대부분 이 타입이지만, 향후 다른 예외·비예외 실패 시 처리·종료 코드 정책이 정의되지 않음 | 낮음 |
| `UnitConverter.cpp` | 19–44 | 긴 함수 | `main()` 본문이 약 26줄로, 점검 기준(20줄 초과)을 넘김 | 중간 |
| `UnitConverter.cpp` | 14, 34 | 매직 넘버 | `10000.0`(소수 4자리 반올림), `4`(`setprecision`)가 의미 없이 리터럴로 박혀 있음 (비율 `3.28084`/`1.09361`과는 별개) | 중간 |
| `UnitConverter.cpp` | 4 | 사용하지 않는 include | `<sstream>`을 include하지만 파일 내에서 미사용 | 낮음 |
| `UnitConverter.cpp` | 46 | 기타 (작업 잔여물) | `//refactoring` 주석이 소스에 남아 있어, 미완료 작업 흔적·의도 불명확 | 낮음 |

**요약:** 리팩터링 관점에서 가장 큰 이슈는 **역할이 섞인 `main()`(SRP)** 이고, 점검 항목으로 적어주신 **if-else·비율 하드코딩은 이 파일이 아니라 `src/domain/` 등 다른 파일**에서 봐야 합니다.

---

**User**

@UnitConverter.cpp 를 ECB(Entity-Control-Boundary) 패턴으로 분석해줘.
코드는 수정하지 말고 다음만 답해줘:

1. 현재 main() 함수에서 ECB 역할이 어떻게 혼재하는가?
   - Entity: 변환 비율 데이터 (3.28084, 1.09361)
   - Control: 변환 로직 (if-else 체인)
   - Boundary: 입력 파싱, 출력 포맷
2. 어느 코드를 어느 클래스/모듈로 이동해야 하는가?
3. 새 단위 추가 시 변경이 최소화되는 구조를 제안하라.


---

**Cursor**

`UnitConverter.cpp`와 이미 분리된 모듈 구조를 함께 확인합니다.


## ECB 관점: `src/UnitConverter.cpp`

이 프로젝트는 **이미 부분적으로 ECB가 분리**되어 있습니다. `UnitConverter.cpp`의 `main()`은 옛날 모놀리식 파일이 아니라 **오케스트레이터 + Boundary 잔여물**에 가깝습니다. 아래는 **현재 `main()` 기준**과 **이미 다른 모듈로 나간 부분**을 함께 설명합니다.

---

## 1. `main()`에서 ECB 역할이 어떻게 혼재하는가?

### 전체 그림 (참고)

| ECB | 현재 위치 | `UnitConverter.cpp` `main()` 안에 있는가? |
|-----|-----------|------------------------------------------|
| **Entity** (비율 `3.28084`, `1.09361`) | `UnitRegistry::loadDefaults()` (`src/domain/UnitRegistry.cpp` 10–11, 64–68행) | **없음** (이미 Entity로 이동됨) |
| **Control** (변환 로직, if-else 대신 허브) | `Converter` (`src/domain/Converter.cpp`) | **없음** (호출만 함) |
| **Boundary** (입력 파싱) | `InputParser` (`src/boundary/InputParser.cpp`) | **일부만** (`parse` 호출, 28행) |
| **Boundary** (출력·I/O) | **`main()` 안** (20–25, 32–40행) | **여전히 혼재** |

### `main()` 안에서의 혼재 (줄 단위)

```19:44:c:\DEV\UnitConverter_08\src\UnitConverter.cpp
int main() {
    std::cout << "Insert value for converting (ex: meter:2.5): ";  // Boundary: 프롬프트
    // ...
    if (!std::getline(std::cin, input)) { ... }                    // Boundary: stdin I/O
    // ...
    boundary::InputParser::parse(input);                           // Boundary 위임 (OK)
    domain::Converter(...).convertAll(...);                      // Control 위임 (OK)
    for (...) {
        roundHalfUp4(...);                                         // Boundary: 표시 정책
        std::cout << std::fixed << std::setprecision(4) << ...;      // Boundary: 출력 포맷
    }
    catch (...) { std::cerr << ...; return 1; }                    // Boundary: 에러 표시
}
```

| ECB | `main()`에 남아 있는 것 | 문제 |
|-----|-------------------------|------|
| **Boundary** | 프롬프트, `getline`, `cout`/`cerr`, `roundHalfUp4`, `setprecision(4)`, 출력 문장 `"X unit = Y unit"` | I/O·표시 계층이 진입점에 묶여 있음 |
| **Control** | `Converter` 생성 + `convertAll` 호출 (29–30행) | 로직 자체는 아님. 다만 **Registry 생성·주입**까지 `main`이 담당 |
| **Entity** | 없음 | 비율 상수는 `main`에 없음 |
| **Control (if-else)** | 없음 | `main`에 단위별 if-else 없음. `Converter`는 meter 허브 + `registry_.allUnits()` 루프 |

**정리:** `main()`은 **Control/Entity를 호출하는 thin shell**에 가깝지만, **Boundary(입·출력·반올림 표시)가 아직 `main()`과 같은 파일(`roundHalfUp4`)에 남아** ECB가 완전히 분리되지 않았습니다. 질문에 적으신 **Entity·Control(if-else) 혼재는 레거시 `UnitConverter.cpp` 관점**이며, **현재 코드베이스에서는 이미 `domain/`으로 이전된 상태**입니다.

---

## 2. 어느 코드를 어느 클래스/모듈로 이동해야 하는가?

| 현재 코드 | 줄 | 이동 대상 (제안) | ECB |
|-----------|-----|------------------|-----|
| `3.28084`, `1.09361` | (이미 `UnitRegistry.cpp` 10–11, 67–68) | **`domain::UnitRegistry`** 또는 **`data::ConfigLoader` + JSON** | Entity |
| `Converter::convert` / `convertAll` | (이미 `Converter.cpp`) | **`domain::Converter`** 유지 | Control |
| `InputParser::parse` | (이미 `InputParser.cpp`) | **`boundary::InputParser`** 유지 | Boundary |
| `roundHalfUp4` | 13–15 | **`boundary::OutputFormatter`** (또는 `DisplayPolicy::roundHalfUp4`) | Boundary |
| `setprecision(4)` + 변환 결과 한 줄 포맷 | 34–36 | **`boundary::ConversionTableWriter`** / `formatLine(src, tgt, value)` | Boundary |
| 프롬프트 + `getline` | 20–25 | **`boundary::CliReader`** 또는 `Application::readCommand()` | Boundary |
| `catch` → `cerr` + exit code | 38–40 | **`boundary::CliErrorReporter`** 또는 Control의 `Application::run()` | Boundary (표시) + 진입점 (종료 코드만) |
| `defaultRegistry()` + `Converter` 조립 | 29–30 | **`control::ConversionService`** 또는 `Application` (Registry 주입) | Control (오케스트레이션) |
| `main()` 본문 전체 | 19–44 | **`src/main.cpp`**: 5~10줄 — `Application app; return app.run();` | 진입점만 |

**이미 잘 된 분리**

- **Entity**: `UnitRegistry` (`factors_` map, `registerUnit`)
- **Control**: `Converter` (허브 변환, `convertAll`이 등록 단위 전부 순회)
- **Boundary (입력)**: `InputParser`

**아직 `main()`에 남은 Boundary**만 추가로 빼면 ECB 분리가 완성됩니다.

---

## 3. 새 단위 추가 시 변경이 최소화되는 구조 (제안)

현재 `Converter::convertAll`은 **등록된 모든 단위를 순회**하므로, 단위별 if-else는 이미 제거된 상태입니다.

```28:34:c:\DEV\UnitConverter_08\src\domain\Converter.cpp
std::vector<ConversionEntry> Converter::convertAll(...) const {
    const double meterHub = toMeterHub(value, fromUnit);
    for (const auto& entry : registry_.allUnits()) {
        results.push_back({entry.first, fromMeterHub(meterHub, entry.first)});
    }
    return results;
}
```

### 목표 구조 (OCP)

```
[Boundary]  CliReader → InputParser → ParsedInput
                ↓
[Control]   ConversionService.convertAll(parsed)
                ↓ uses
[Entity]    UnitRegistry (단위명 → hub factor)
                ↓ returns
[Control]   vector<ConversionEntry>  (순수 double, ε 정밀)
                ↓
[Boundary]  OutputFormatter → ConversionTableWriter → stdout
```

### 새 단위 추가 시 변경 파일 (최소)

| 작업 | 변경 위치 | `Converter` / `main` / `InputParser` 수정 |
|------|-----------|---------------------------------------------|
| 기본 3단위 유지 | `UnitRegistry::loadDefaults()` 또는 `units.json` | **불필요** |
| 런타임 등록 | `UnitRegistry::registerUnit("inch", 0.0254)` | **불필요** |
| 설정 파일 로드 | `data::ConfigLoader::loadJson()` → Registry 주입 | **불필요** (주입만) |
| CLI 출력 형식 동일 | `boundary` 출력 모듈 | **불필요** (`convertAll` 결과만 순회) |

**변경이 필요한 곳:** Entity 데이터 **한 곳** (`loadDefaults` 한 줄, JSON 항목 추가, 또는 `registerUnit` 호출).

**변경이 필요 없는 곳:** `Converter`, `InputParser`, `main`, 출력 루프(제네릭 `for (entry : results)`).

### 권장 데이터 흐름 (확장 시)

1. **Entity**: 비율은 코드 상수가 아니라 `tests/fixtures/units_valid.json` 같은 설정 → `ConfigLoader` → `UnitRegistry`.
2. **Control**: `ConversionService`가 `Registry` + `Converter`를 캡슐화; `main`은 서비스만 호출.
3. **Boundary**: 파싱·표시·I/O만 담당; **Domain double**과 **표시용 round4**를 분리 (프로젝트 `.cursorrules`의 ε vs HALF_UP 4자리 정책과 일치).

이렇게 하면 **inch 추가 = Registry(또는 JSON) 1건 추가**이고, Control/Boundary/CLI는 그대로 둡니다.

---

### 한 줄 요약

`UnitConverter.cpp`의 `main()`은 **Control·Entity는 이미 위임**했지만, **Boundary(I/O·반올림·출력 포맷)와 애플리케이션 조립이 아직 `main()`에 남아** ECB가 완전히 분리되지 않았고, **새 단위는 `UnitRegistry`(Entity)만 바꾸면 되는 구조**가 이미 `Converter::convertAll` 쪽에 갖춰져 있습니다.

---

**User**

앞서 분석한 내용을 바탕으로 리팩토링 계획서를 작성해줘.
코드는 아직 수정하지 말고:

## 리팩토링 대상 목록 (우선순위 순)
| 순번 | 대상 | 문제 | 적용 기법 | 우선순위 |

## 리팩토링 후보 목록
R-U1: 입력 파싱을 InputParser로 분리
R-U2: 예외 메시지·코드 상수화
R-U3: 출력 포맷 생성을 OutputFormatter로 분리
R-L1: 변환 비율을 ConversionRule Value Object로 교체
R-L2: if-else 체인을 UnitRegistry(Map/Dictionary)로 교체
R-L3: 매직 넘버 3.28084/1.09361 → 상수 또는 설정 파일
R-L4: convert() 함수 추출 (meter 허브 변환 로직 단일화)

## 테스트 선행 필요 항목
## 리팩토링 후 검증 방법 (회귀 테스트 실행 명령어)


---

**Cursor**

이전 분석과 현재 코드·테스트 구성을 확인해 계획서를 작성합니다.


# UnitConverter 리팩토링 계획서

**전제:** 코드 스멜·ECB 분석 기준. **현재 `src/UnitConverter.cpp`는 부분 리팩터링 완료 상태**이며, 레거시 모놀리식 가정(비율·if-else가 `main`에 있음)과는 다릅니다. 아래 후보(R-U*, R-L*)는 **완료 / 잔여 / 신규**를 구분했습니다.

---

## 리팩토링 대상 목록 (우선순위 순)

| 순번 | 대상 | 문제 | 적용 기법 | 우선순위 |
|------|------|------|-----------|----------|
| 1 | `src/UnitConverter.cpp` `main()` (32–37, 13–15행) | Boundary(반올림·출력 포맷)가 진입점에 잔존 → SRP·BCE 위반 | **Extract Class**: `boundary::OutputFormatter` / `ConversionTableWriter` | **높음** |
| 2 | `src/UnitConverter.cpp` `main()` (19–44행) | 프롬프트·stdin·stderr·종료 코드·Control 조립이 한 함수에 혼재 | **Extract Class**: `control::Application` (또는 `ConversionService`) + **thin `main`** | **높음** |
| 3 | `src/UnitConverter.cpp` (23–24행) | EOF/`getline` 실패 시 메시지 없이 `return 1` | **Extract Method** + Boundary 에러 리포터 통일 | **중간** |
| 4 | 예외 메시지 산재 (`InputParser`, `UnitRegistry`, `Converter`) | `ERR-INPUT-*` 문자열 중복·prefix 불일치 위험 | **Replace Magic String**: `boundary/ErrorCodes.hpp` (R-U2) | **중간** |
| 5 | `UnitRegistry::loadDefaults()` vs `Converter.hpp` 상수 | 비율 이중 정의(`kFeetPerMeter` 등) | **Single Source of Truth**: JSON(`ConfigLoader`) 또는 Entity 전용 상수 파일 (R-L3) | **중간** |
| 6 | `UnitRegistry`의 `map<string,double>` | `ConversionRule` VO 미도입, 불변식이 map API에 분산 | **Replace Primitive with Object** (R-L1) — v1.1 확장 시 | **낮음** |
| 7 | `//refactoring` 주석, 미사용 `#include <sstream>` | 작업 잔여·불필요 include | **Remove Dead Code** | **낮음** |

**이미 완료되어 계획에서 “구현”이 아닌 “유지·검증”인 항목**

| 후보 | 상태 | 비고 |
|------|------|------|
| R-U1 | **완료** | `boundary::InputParser` |
| R-L2 | **완료** | `UnitRegistry` + `convertAll` 루프, 단위별 if-else 없음 |
| R-L4 | **완료** | `toMeterHub` / `fromMeterHub` / `convert` |
| R-L3 | **부분 완료** | `UnitRegistry.cpp` 상수화; 설정 파일 경로는 `data::ConfigLoader` 존재 |

---

## 리팩토링 후보 목록

| ID | 내용 | 현재 상태 | 계획 조치 |
|----|------|-----------|-----------|
| **R-U1** | 입력 파싱을 `InputParser`로 분리 | **완료** (`src/boundary/InputParser.cpp`) | `main`은 `parse()` 호출만 유지. 추가 작업 없음 |
| **R-U2** | 예외 메시지·코드 상수화 | **부분** (문자열에 `ERR-INPUT-00x` 포함, 헤더 상수 없음) | `ErrorCodes` + `formatError(code, detail)` 도입. Domain은 prefix 없는 도메인 예외, Boundary에서 prefix 부착 정책 통일 |
| **R-U3** | 출력 포맷을 `OutputFormatter`로 분리 | **미완료** (`roundHalfUp4`, `setprecision(4)`가 `UnitConverter.cpp`) | `boundary::roundHalfUp4`, `formatConversionLine()` 추출 → `main`은 `writer.write(results)`만 호출 |
| **R-L1** | 변환 비율을 `ConversionRule` VO로 교체 | **부분** (`map<string,double>`) | v1.0 인수 후: `{name, metersPerUnit}` VO + Registry API 정리 |
| **R-L2** | if-else → `UnitRegistry` | **완료** | 회귀만 유지 (`domain_tests` `[ratio]` `[register]`) |
| **R-L3** | `3.28084` / `1.09361` → 상수 또는 설정 | **부분** | `loadDefaults()` 단일 출처화; 장기적으로 `units.json` + `ConfigLoader` 주입으로 `defaultRegistry()` 대체 |
| **R-L4** | `convert()`·meter 허브 단일화 | **완료** (`Converter.cpp`) | `UnitConverter` 리팩터 시 **수정 금지** (회귀 위험만 모니터) |

---

## 테스트 선행 필요 항목

리팩터링 **전에 GREEN 빌드 + 전체 테스트 GREEN**이 선행 조건입니다 (`UNIT_CONVERTER_RED_PHASE=OFF`).

| 순번 | 항목 | 이유 | 권장 시점 |
|------|------|------|-----------|
| T-1 | **Golden Master 4건 GREEN** | `UnitConverter` stdout·round4·프롬프트가 E2E로 고정됨 (`tests/test_golden_master.cpp`) | R-U3, thin `main` **직전** |
| T-2 | **Domain 20건 GREEN** | `Converter` / `UnitRegistry` 회귀 방지 (R-L4, R-L2는 이미 구현됨) | R-L3, R-L1 **직전** |
| T-3 | **Boundary 15건 GREEN** | `InputParser` 계약 (R-U1 완료 상태 검증) | R-U2 **직전** |
| T-4 | **Data 10건 GREEN** | JSON/YAML Registry 로드 (R-L3 설정 파일화 시) | R-L3 **직전** |
| T-5 | **(선택) Boundary `[table]` 테스트 추가** | `roundHalfUp4` / `8.2021` HALF_UP을 단위 테스트로 고정 후 R-U3 추출 | R-U3 **직전** (`.cursorrules` T-B-05~07, 현재 GM에만 의존) |
| T-6 | **(선택) EOF/`getline` 실패 테스트** | 23–24행 동작 명세화 후 리팩터 | 순번 3(에러 처리 통일) **직전** |

**후보 ID별 테스트 선행 요약**

| 후보 | 테스트 선행 |
|------|-------------|
| R-U1 | 불필요 (이미 커버) |
| R-U2 | T-3 (ERR prefix assertion) |
| R-U3 | **T-1 필수**, T-5 권장 |
| R-L1 | T-2 + T-4 |
| R-L2 | T-2 (유지 검증만) |
| R-L3 | T-2, T-4 |
| R-L4 | T-2 (유지 검증만) |

---

## 리팩토링 후 검증 방법 (회귀 테스트 실행 명령어)

**1. GREEN phase로 configure (한 번만 또는 옵션 변경 시)**

```powershell
cmake -S C:\DEV\UnitConverter_08 -B C:\DEV\UnitConverter_08\build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build C:\DEV\UnitConverter_08\build
```

**2. 전체 회귀 (권장 — 인수 기준)**

```powershell
ctest --test-dir C:\DEV\UnitConverter_08\build --output-on-failure
```

**3. 레이어별 스모크 (리팩터 영향 범위별)**

```powershell
# Domain (R-L2, R-L3, R-L4)
C:\DEV\UnitConverter_08\build\unit_converter_tests.exe "[domain]"

# Boundary parse + ERR (R-U1, R-U2)
C:\DEV\UnitConverter_08\build\unit_converter_tests.exe "[boundary]"

# Data / config (R-L3)
C:\DEV\UnitConverter_08\build\unit_converter_tests.exe "[data]"

# CLI stdout Golden Master (R-U3, main 정리) — PRD R-01
C:\DEV\UnitConverter_08\build\unit_converter_tests.exe "[golden][regression][r01]"
```

**4. 실행 파일 수동 스모크 (선택)**

```powershell
echo meter:2.5 | C:\DEV\UnitConverter_08\build\UnitConverter.exe
```

**합격 기준:** `ctest` **0 failed**; Golden Master `[golden][regression][r01]` **4/4 PASS**; Domain ε(1e-9) vs Boundary round4(4자리) 동작이 `tests/golden_master_expected.txt`와 일치.

---

## 권장 실행 순서 (요약)

```
[사전] GREEN configure + ctest 전체 PASS
  → R-U3 (OutputFormatter) + thin main / Application  ← 높음
  → R-U2 (ErrorCodes 상수화)
  → R-L3 (비율 단일 출처 / ConfigLoader 주입)
  → R-L1 (ConversionRule VO) — v1.1
[사후] ctest 전체 + [golden] + (선택) [table] 신규 테스트
```

**한 줄 원칙:** `UnitConverter.cpp`는 **Boundary·Control 잔여물만 건드리고**, Entity/Control 핵심(`Converter`, `UnitRegistry`)은 **테스트 GREEN을 확인한 뒤 최소 diff**로 유지합니다.

---

**User**

Report 폴더에 09_REFACTORING 으로 시작하는 파일명으로 보고서를 작성해줘

---

**Cursor**

Ask 모드라 파일은 직접 생성할 수 없습니다. 기존 Report 형식을 확인한 뒤, `Report/09_REFACTORING_*.md`에 붙여 넣을 전체 보고서를 작성합니다.


Ask 모드에서는 `Report/`에 파일을 생성·저장할 수 없습니다. **Agent 모드로 전환**하시면 `Report/09_REFACTORING_UnitConverter_ECB_Plan_Report.md`를 바로 작성해 드릴 수 있습니다.

아래는 그때 저장할 **전체 보고서 초안**입니다. 복사해 `Report/09_REFACTORING_UnitConverter_ECB_Plan_Report.md`로 저장하거나, Agent 모드에서 “위 내용으로 09 파일 생성”이라고 요청하시면 됩니다.

---

```markdown
# UnitConverter ECB 리팩토링 계획 보고서

## 문서 관리

| 항목 | 내용 |
|------|------|
| **보고서 ID** | RPT-REF-001 |
| **문서 유형** | REFACTORING 계획 (코드 스멜·ECB·후보 매핑·테스트 게이트) |
| **버전** | 1.0 |
| **작성일** | 2026-05-21 |
| **프로젝트** | UnitConverter (C++17, CMake, Catch2 v3.5.4) |
| **정본 참조** | [../docs/PRD.md](../docs/PRD.md), [../.cursorrules](../.cursorrules), [05_GREEN_CATCH2_Test_Suite_Report.md](05_GREEN_CATCH2_Test_Suite_Report.md) |
| **선행 보고서** | [06_REFACTORING_Golden_Master_Regression_Report.md](06_REFACTORING_Golden_Master_Regression_Report.md), [08_REFACTORING_Golden_Master_README_Checklist_Report.md](08_REFACTORING_Golden_Master_README_Checklist_Report.md) |
| **대상 소스** | `src/UnitConverter.cpp`, `src/domain/*`, `src/boundary/*`, `src/data/*` |

---

## 요약 (Executive Summary)

`src/UnitConverter.cpp`는 레거시 모놀리식이 아니라 **부분 ECB 분리 완료** 상태이다. Entity·Control은 `domain/`으로 이전되었고, **Boundary(I/O·round4·출력 포맷)와 애플리케이션 조립이 `main()`에 잔존**한다. 리팩터링은 **R-U3(OutputFormatter) + thin main/Control**을 최우선으로 하며, Golden Master 4건을 회귀 게이트로 고정한다.

| 구분 | 내용 |
|------|------|
| **완료 후보** | R-U1, R-L2, R-L4 |
| **부분 완료** | R-U2, R-L3, R-L1 |
| **미완료·우선** | R-U3, `main()` SRP, EOF 오류 처리 통일 |
| **회귀 게이트** | GREEN 빌드 + `ctest` 전체 + `[golden][regression][r01]` |

---

## 1. 현황 분석

### 1.1 코드 스멜 (`src/UnitConverter.cpp`)

| 줄 | 스멜 | 우선순위 |
|----|------|----------|
| 19–44 | SRP 위반 — `main()`에 프롬프트·입력·변환 오케스트레이션·출력·에러 표시 혼재 | 높음 |
| 13–15, 32–36 | 표시 책임(`roundHalfUp4`, `setprecision(4)`)이 Boundary가 아닌 진입 TU에 존재 | 높음 |
| 23–24 | `getline` 실패 시 메시지 없이 `return 1` | 중간 |
| 14, 34 | 매직 넘버 `10000.0`, `4` (비율 3.28084/1.09361은 본 파일에 없음) | 중간 |
| 19–44 | `main()` 약 26줄 (20줄 초과 기준) | 중간 |
| 4 | 미사용 `#include <sstream>` | 낮음 |
| 46 | `//refactoring` 작업 잔여 주석 | 낮음 |

**본 파일에 해당 없음:** 단위별 if-else, 비율 `3.28084`/`1.09361` 하드코딩 → `src/domain/UnitRegistry.cpp`, `Converter.cpp`로 이전됨.

### 1.2 ECB 역할 분리 현황

| ECB | 현재 위치 | `main()` 잔존 |
|-----|-----------|---------------|
| **Entity** | `UnitRegistry` (비율 map, `loadDefaults`) | 없음 |
| **Control** | `Converter` (meter 허브, `convertAll`) | 조립만 (29–30행) |
| **Boundary (입력)** | `InputParser::parse` | 호출만 (28행) |
| **Boundary (출력·I/O)** | `main()` 20–25, 32–40, `roundHalfUp4` | **잔존** |

### 1.3 테스트 현황

| 파일 | 역할 |
|------|------|
| `tests/domain_tests.cpp` | Domain 20건 |
| `tests/boundary_tests.cpp` | Boundary 15건 |
| `tests/data_tests.cpp` | Data 10건 |
| `tests/test_golden_master.cpp` | CLI stdout Golden Master 4건 (GREEN 빌드만) |

---

## 2. 리팩토링 대상 목록 (우선순위 순)

| 순번 | 대상 | 문제 | 적용 기법 | 우선순위 |
|------|------|------|-----------|----------|
| 1 | `UnitConverter.cpp` 출력·반올림 (13–15, 32–37) | Boundary 미분리 | Extract Class: `OutputFormatter` / `ConversionTableWriter` | 높음 |
| 2 | `UnitConverter.cpp` `main()` (19–44) | Control·Boundary 조립 혼재 | Extract Class: `Application` + thin `main` | 높음 |
| 3 | `getline` 실패 (23–24) | 불완전 오류 처리 | Boundary `CliErrorReporter` 통일 | 중간 |
| 4 | ERR 메시지 산재 | 매직 문자열·prefix 불일치 위험 | `ErrorCodes.hpp` 상수화 (R-U2) | 중간 |
| 5 | 비율 이중 정의 | Registry vs 헤더 상수 | Single Source: JSON/`ConfigLoader` (R-L3) | 중간 |
| 6 | `map<string,double>` | VO 미도입 | Replace Primitive with Object (R-L1) | 낮음 |
| 7 | 미사용 include·주석 | 잡음 | Remove Dead Code | 낮음 |

---

## 3. 리팩토링 후보 목록 (R-U / R-L)

| ID | 내용 | 상태 | 계획 |
|----|------|------|------|
| **R-U1** | 입력 파싱 → `InputParser` | **완료** | 유지 |
| **R-U2** | 예외 메시지·코드 상수화 | **부분** | `boundary/ErrorCodes.hpp` 도입 |
| **R-U3** | 출력 포맷 → `OutputFormatter` | **미완료** | `roundHalfUp4`·`formatLine` 추출 |
| **R-L1** | `ConversionRule` VO | **부분** | v1.1에서 Registry API 정리 |
| **R-L2** | if-else → `UnitRegistry` | **완료** | 회귀만 |
| **R-L3** | 비율 → 상수/설정 | **부분** | `units.json` + `ConfigLoader` 주입 |
| **R-L4** | meter 허브 `convert()` | **완료** | 수정 금지, T-2로만 검증 |

---

## 4. 코드 이동 매핑 (ECB)

| 현재 코드 | 이동 대상 | ECB |
|-----------|-----------|-----|
| `roundHalfUp4`, `setprecision(4)`, 출력 한 줄 | `boundary::OutputFormatter` | Boundary |
| 프롬프트, `getline`, `cerr` | `boundary::CliReader` / `CliErrorReporter` | Boundary |
| `parse` → `convertAll` 조립 | `control::Application` | Control |
| `3.28084`, `1.09361` | `UnitRegistry` / `units.json` | Entity |
| `convert`, `convertAll` | `domain::Converter` (유지) | Control |

**새 단위 추가 시 변경 최소 구조:** Entity(`registerUnit` 또는 JSON) **1곳만** 변경 → `convertAll`이 `allUnits()` 순회 → Boundary 출력 루프 변경 없음 (OCP).

---

## 5. 테스트 선행 필요 항목

| ID | 항목 | 시점 |
|----|------|------|
| T-1 | Golden Master `[golden][regression][r01]` GREEN | R-U3, thin `main` 직전 **필수** |
| T-2 | Domain `[domain]` 20건 GREEN | R-L3, R-L1 직전 |
| T-3 | Boundary `[boundary]` 15건 GREEN | R-U2 직전 |
| T-4 | Data `[data]` 10건 GREEN | R-L3 직전 |
| T-5 | (권장) Boundary `[table]` round4 단위 테스트 | R-U3 직전 |
| T-6 | (선택) EOF/`getline` 실패 테스트 | 에러 통일 직전 |

**선행 조건:** `cmake -DUNIT_CONVERTER_RED_PHASE=OFF` 후 **전체 `ctest` PASS**.

---

## 6. 리팩토링 후 검증 (회귀 테스트 명령)

```powershell
cmake -S . -B build -DUNIT_CONVERTER_RED_PHASE=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

**범위별:**

```powershell
build\unit_converter_tests.exe "[domain]"
build\unit_converter_tests.exe "[boundary]"
build\unit_converter_tests.exe "[data]"
build\unit_converter_tests.exe "[golden][regression][r01]"
```

**합격 기준:** `ctest` 0 failed; Golden Master 4/4 PASS; `golden_master_expected.txt`와 stdout 일치.

---

## 7. 권장 실행 순서

```text
[사전] GREEN configure + ctest 전체 PASS
  → R-U3 (OutputFormatter) + thin main / Application
  → R-U2 (ErrorCodes)
  → R-L3 (비율 단일 출처 / ConfigLoader 주입)
  → R-L1 (ConversionRule VO, v1.1)
[사후] ctest + [golden] + (선택) [table] 추가
```

---

## 8. 리스크·완화

| 리스크 | 완화 |
|--------|------|
| stdout 포맷 변경 | Golden Master + (권장) `[table]` 테스트 |
| ERR prefix 회귀 | `boundary_tests` ERR assertion |
| Domain ε vs Boundary round4 혼동 | Domain 1e-9, Boundary HALF_UP 4자리 분리 유지 |
| RED 빌드에서 GM 미실행 | REFACTOR는 `UNIT_CONVERTER_RED_PHASE=OFF`만 |

---

## 9. 승인·다음 단계

| 체크 | 항목 |
|------|------|
| [ ] | T-1 Golden Master GREEN 확인 |
| [ ] | R-U3 `OutputFormatter` 추출 |
| [ ] | `control::Application` + thin `main` |
| [ ] | `ctest` 전체 PASS |
| [ ] | `Report/README.md`에 본 문서 링크 추가 |

---

## 부록 A: `main()` ECB 혼재 (참조)

```cpp
// Boundary: prompt, getline, roundHalfUp4, cout format, cerr
// Control (위임): InputParser::parse, Converter::convertAll
// Entity: (main에 없음 — UnitRegistry::defaultRegistry)
```

## 부록 B: 관련 보고서

| 문서 | 관계 |
|------|------|
| [06_REFACTORING_Golden_Master_Regression_Report.md](06_REFACTORING_Golden_Master_Regression_Report.md) | E2E 회귀 설계 |
| [05_GREEN_CATCH2_Test_Suite_Report.md](05_GREEN_CATCH2_Test_Suite_Report.md) | 45건 GREEN 전제 |
```

**`Report/README.md`에 추가할 한 줄:**

```markdown
| [09_REFACTORING_UnitConverter_ECB_Plan_Report.md](09_REFACTORING_UnitConverter_ECB_Plan_Report.md) | UnitConverter ECB·코드 스멜·R-U/L 후보·테스트 게이트 계획 (RPT-REF-001) |
```

Agent 모드로 전환해 주시면 위 내용을 `Report/09_REFACTORING_UnitConverter_ECB_Plan_Report.md`로 저장하고 `Report/README.md` 인덱스까지 반영하겠습니다.

