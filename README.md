# ProceduralWorld
## 절차적 지형 생성을 위해 WFC 알고리즘을 사용하여 3D 건물 및 던전을 생성하는 플러그인

Wave Function Collapse 알고리즘을 사용하여 던전 및 건물을 생성

### Wave Function Collapse Algorithm

​Wave Function Collapse(WFC) 알고리즘은 절차적 콘텐츠 생성 분야에서 널리 사용되는 제약 충족 알고리즘

이 알고리즘은 입력된 비트맵이나 타일맵을 분석하여, 그와 유사한 출력을 생성

이를 통해 게임 개발 등에서 던전이나 건물과 같은 3D 지형을 절차적으로 생성하는 데 활용할 수 있음

**Wave Function Collapse 알고리즘의 주요 단계:**

1. 입력 분석: 입력된 비트맵을 읽고 NxN 패턴을 추출하여 패턴의 빈도를 계산

2. 출력 초기화: 출력 비트맵의 각 픽셀을 모든 가능한 상태의 중첩(superposition)으로 초기화

3. 관찰 및 전파 반복: 가장 낮은 엔트로피를 가진 픽셀을 선택하여 특정 상태로 결정(관찰)하고, 이 결정된 정보를 주변 픽셀에 전파하여 가능한 상태를 갱신

4. 출력 생성: 모든 픽셀이 특정 상태로 결정되면, 최종 출력을 반환

추가적인 정보 링크: https://github.com/mxgmn/WaveFunctionCollapse

## 현재 구현 상태

Wave Function Collapse 2D 구현 완료

https://github.com/devouring123/WFC2D

Wave Function Collapse 3D 구현 중
- WFC3D 입력 분석(구현 완료)
- 출력 초기화(구현 완료)
- 관찰 및 전파(구현 완료)
- 논리적 출력 생성(구현 완료)
- 시각적 출력 생성(구현 완료)
- 3D 타일 실제로 적용 시켜보기(현재 진행 중)
