# 🛠️ WFC3D Visualizer 구현 로드맵

## 📋 목차
- [🔍 현재 코드 분석](#-현재-코드-분석)
- [🚀 Phase 1: ISMC 전환 (Week 1-3)](#-phase-1-ismc-전환-week-1-3)
- [⚡ Phase 2: HISMC 전환 (Week 4-7)](#-phase-2-hismc-전환-week-4-7)
- [🧪 테스트 및 검증 (Week 8)](#-테스트-및-검증-week-8)
- [📦 배포 계획 (Week 9)](#-배포-계획-week-9)
- [🎯 즉시 시작 가능한 작업](#-즉시-시작-가능한-작업)

---

## 🔍 현재 코드 분석

### 기존 아키텍처 구조
```cpp
// 현재 UWFC3DVisualizer.h/cpp의 핵심 구조
class UWFC3DVisualizer {
    // 📊 현재 문제점들
    TArray<UStaticMeshComponent*> SpawnedMeshComponents;  // 개별 컴포넌트 배열
    
    // 🔧 핵심 메서드들
    void CreateMeshesFromData(UWorld* World, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData);
    void CleanupSpawnedMeshes();
    
    // ⚡ 비동기 시스템 (이미 구현됨)
    TUniquePtr<FAsyncTask<FWFC3DVisualizeAsyncTask>> AsyncTask;
    std::atomic<bool> bIsRunningAtomic;
};
```

### 📊 현재 성능 특성 분석
| 항목 | 현재 구현 | 문제점 | 목표 개선치 |
|------|----------|--------|-------------|
| **메시 생성** | `NewObject<UStaticMeshComponent>()` 반복 | N개 개별 생성 | 메시당 1개 ISMC |
| **Draw Calls** | 타일 수만큼 (N개) | 5×5×5=125개, 10×10×10=1000개 | 메시 종류만큼 (~10개) |
| **메모리 사용** | 컴포넌트당 ~800바이트 | 대규모에서 기하급수적 증가 | 70% 절감 |
| **정리 방식** | `DestroyComponent()` 반복 | 느린 개별 정리 | 배치 정리 |

### 🎯 변경이 필요한 핵심 함수들
1. **`CreateMeshesFromData`** → ISMC 기반으로 재작성
2. **`CleanupSpawnedMeshes`** → ISMC 정리로 변경  
3. **`SpawnedMeshComponents` 배열** → 메시별 ISMC 맵으로 변경

---

## 🚀 Phase 1: ISMC 전환 (Week 1-3)

### 📅 Week 1: 기반 인프라 구축

#### Day 1-2: 인터페이스 설계 및 기본 구조 생성

**작업 1: 렌더링 전략 인터페이스 생성**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/IWFCRenderingStrategy.h
class PROCEDURALWORLD_API IWFCRenderingStrategy {
public:
    virtual ~IWFCRenderingStrategy() = default;
    
    // 🔧 핵심 인터페이스
    virtual void Initialize(AActor* OwnerActor, float TileSize) = 0;
    virtual void VisualizeGrid(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, const FRandomStream* RandomStream) = 0;
    virtual void ClearVisualization() = 0;
    virtual void SetTileSize(float NewTileSize) = 0;
    
    // 📊 성능 모니터링
    virtual FWFCRenderingStats GetRenderingStats() const = 0;
    virtual bool SupportsGridSize(const FIntVector& Dimensions) const = 0;
    
    // 📋 전략 정보
    virtual EWFCRenderingMode GetRenderingMode() const = 0;
    virtual FString GetStrategyName() const = 0;
};

// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFCRenderingTypes.h
UENUM(BlueprintType)
enum class EWFCRenderingMode : uint8 {
    StaticMesh = 0,           // 기존 방식
    InstancedStaticMesh = 1,  // Phase 1 목표
    HierarchicalISM = 2,      // Phase 2 목표
    Auto = 3                  // 자동 선택
};

USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFCRenderingStats {
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadOnly)
    int32 TotalInstances = 0;
    
    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls = 0;
    
    UPROPERTY(BlueprintReadOnly)
    int32 VisibleInstances = 0;
    
    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    float LastRenderTimeMs = 0.0f;
    
    UPROPERTY(BlueprintReadOnly)
    TMap<UStaticMesh*, int32> MeshInstanceCounts;
};
```

**작업 2: 기존 전략 클래스 생성 (호환성 유지)**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizer_Legacy.h
class PROCEDURALWORLD_API UWFC3DVisualizer_Legacy : public UObject, public IWFCRenderingStrategy {
    GENERATED_BODY()

public:
    // 🔄 기존 UWFC3DVisualizer의 로직을 그대로 이동
    virtual void Initialize(AActor* OwnerActor, float TileSize) override;
    virtual void VisualizeGrid(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, const FRandomStream* RandomStream) override;
    virtual void ClearVisualization() override;
    // ... 기존 구현을 그대로 복사

private:
    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedMeshComponents;  // 기존과 동일
    
    void CreateMeshesFromData(UWorld* World, UWFC3DGrid* Grid, const UWFC3DModelDataAsset* ModelData);
    void CleanupSpawnedMeshes();
};
```

#### Day 3-4: ISMC 전략 클래스 기본 구조

**작업 3: ISMC 전략 클래스 생성**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizer_ISMC.h
class PROCEDURALWORLD_API UWFC3DVisualizer_ISMC : public UObject, public IWFCRenderingStrategy {
    GENERATED_BODY()

public:
    UWFC3DVisualizer_ISMC();
    virtual ~UWFC3DVisualizer_ISMC() = default;

    // IWFCRenderingStrategy 구현
    virtual void Initialize(AActor* OwnerActor, float TileSize) override;
    virtual void VisualizeGrid(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, const FRandomStream* RandomStream) override;
    virtual void ClearVisualization() override;
    virtual void SetTileSize(float NewTileSize) override;
    virtual FWFCRenderingStats GetRenderingStats() const override;
    virtual bool SupportsGridSize(const FIntVector& Dimensions) const override;
    virtual EWFCRenderingMode GetRenderingMode() const override { return EWFCRenderingMode::InstancedStaticMesh; }
    virtual FString GetStrategyName() const override { return TEXT("InstancedStaticMesh"); }

protected:
    // 🔧 ISMC 관리 핵심 로직
    void CollectInstanceData(UWFC3DGrid* Grid, UWFC3DModelDataAsset* ModelData, const FRandomStream* RandomStream);
    void CreateISMCComponents();
    void PopulateInstanceData();
    void OptimizeMaterialBatching();

private:
    UPROPERTY()
    AActor* OwnerActor = nullptr;
    
    // 📊 ISMC 관리 구조체
    struct FISMCBatch {
        UPROPERTY()
        UInstancedStaticMeshComponent* Component = nullptr;
        
        TArray<FTransform> InstanceTransforms;
        TArray<UMaterialInterface*> Materials;
        bool bNeedsUpdate = false;
        
        void AddInstance(const FTransform& Transform);
        void FlushUpdates();
    };
    
    UPROPERTY()
    TMap<UStaticMesh*, FISMCBatch> ISMCBatches;
    
    float TileSize = 100.0f;
    mutable FWFCRenderingStats CachedStats;
    mutable float LastStatsUpdateTime = 0.0f;
};
```

#### Day 5: 기존 Visualizer 수정

**작업 4: UWFC3DVisualizer를 매니저 클래스로 변환**
```cpp
// 📁 수정: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizer.h
class PROCEDURALWORLD_API UWFC3DVisualizer : public UObject {
    GENERATED_BODY()

public:
    UWFC3DVisualizer();
    virtual ~UWFC3DVisualizer() = default;

    // 🔧 새로운 전략 기반 인터페이스
    UFUNCTION(BlueprintCallable, Category = "WFC Rendering")
    void SetRenderingMode(EWFCRenderingMode Mode);
    
    UFUNCTION(BlueprintCallable, Category = "WFC Rendering") 
    void SetAutoRenderingMode(bool bEnabled);

    // 📊 기존 인터페이스 유지 (하위 호환성)
    UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
    FWFC3DVisualizeResult Execute(const FWFC3DVisualizeContext& Context);
    
    UFUNCTION(BlueprintCallable, Category = "WFCVisualization")
    void ExecuteAsync(const FWFC3DVisualizeContext& Context);

protected:
    // 🎯 전략 패턴 적용
    UPROPERTY()
    TObjectPtr<UObject> CurrentRenderingStrategy;  // IWFCRenderingStrategy 구현체
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    EWFCRenderingMode RenderingMode = EWFCRenderingMode::Auto;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bAutoSelectRenderingMode = true;

private:
    void CreateRenderingStrategy(EWFCRenderingMode Mode);
    EWFCRenderingMode SelectOptimalRenderingMode(const FIntVector& GridSize);
    IWFCRenderingStrategy* GetRenderingStrategy() const;
};
```

### 📅 Week 2: ISMC 핵심 로직 구현

#### Day 6-8: 데이터 수집 및 그룹핑 로직

**작업 5: 인스턴스 데이터 수집 시스템**
```cpp
// 📁 Source/ProceduralWorld/Private/WFC/Visualization/WFC3DVisualizer_ISMC.cpp
void UWFC3DVisualizer_ISMC::CollectInstanceData(
    UWFC3DGrid* Grid, 
    UWFC3DModelDataAsset* ModelData, 
    const FRandomStream* RandomStream) {
    
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_CollectInstanceData);
    
    // 1️⃣ 기존 데이터 정리
    ISMCBatches.Empty();
    
    const TArray<FWFC3DCell>& Cells = Grid->GetCells();
    const FIntVector& Dimensions = Grid->GetDimensions();
    
    // 2️⃣ 메모리 사전 할당 (성능 최적화)
    int32 EstimatedInstances = Dimensions.X * Dimensions.Y * Dimensions.Z;
    ISMCBatches.Reserve(ModelData->GetTileInfosNum());
    
    // 3️⃣ 각 셀 순회하며 인스턴스 데이터 수집
    for (int32 CellIndex = 0; CellIndex < Cells.Num(); ++CellIndex) {
        const FWFC3DCell& Cell = Cells[CellIndex];
        
        if (!Cell.bIsCollapsed) continue;
        
        // 타일 정보 가져오기
        const FTileInfo* TileInfo = ModelData->GetTileInfo(Cell.CollapsedTileIndex);
        if (!TileInfo) continue;
        
        // 시각적 정보 가져오기 (기존 로직 활용)
        const FTileVisualInfo* VisualInfo = GetTileVisualInfo(Cell, ModelData, RandomStream);
        if (!VisualInfo || !VisualInfo->StaticMesh) continue;
        
        // Transform 계산 (기존 로직 활용)
        FVector WorldLocation = FVector(Cell.Location) * TileSize;
        FRotator Rotation = GetRotationFromRotationStep(GetRotationStep(Cell, ModelData));
        FTransform InstanceTransform(Rotation, WorldLocation, FVector::OneVector);
        
        // 4️⃣ 메시별 배치에 인스턴스 추가
        FISMCBatch& Batch = ISMCBatches.FindOrAdd(VisualInfo->StaticMesh);
        Batch.InstanceTransforms.Add(InstanceTransform);
        Batch.Materials = VisualInfo->Materials;  // 첫 번째 인스턴스의 머티리얼 사용
        Batch.bNeedsUpdate = true;
    }
    
    // 5️⃣ 통계 업데이트
    UpdateRenderingStats();
}

void UWFC3DVisualizer_ISMC::FISMCBatch::AddInstance(const FTransform& Transform) {
    InstanceTransforms.Add(Transform);
    bNeedsUpdate = true;
}

void UWFC3DVisualizer_ISMC::FISMCBatch::FlushUpdates() {
    if (!bNeedsUpdate || !Component) return;
    
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_ISMC_FlushUpdates);
    
    // 기존 인스턴스 제거
    Component->ClearInstances();
    
    if (InstanceTransforms.Num() > 0) {
        // 메모리 사전 할당 
        Component->PreAllocateInstancesMemory(InstanceTransforms.Num());
        
        // 배치 인스턴스 추가 (단일 호출로 성능 향상)
        Component->AddInstances(InstanceTransforms, false);
        
        // GPU 업데이트 요청
        Component->MarkRenderStateDirty();
    }
    
    bNeedsUpdate = false;
}
```

#### Day 9-10: ISMC 컴포넌트 생성 및 관리

**작업 6: ISMC 컴포넌트 생성 시스템**
```cpp
void UWFC3DVisualizer_ISMC::CreateISMCComponents() {
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_CreateISMCComponents);
    
    if (!OwnerActor) return;
    
    for (auto& [StaticMesh, Batch] : ISMCBatches) {
        if (!StaticMesh || Batch.InstanceTransforms.Num() == 0) continue;
        
        // ISMC 컴포넌트 생성
        UInstancedStaticMeshComponent* ISMC = NewObject<UInstancedStaticMeshComponent>(OwnerActor);
        if (!ISMC) continue;
        
        // 기본 설정
        ISMC->SetStaticMesh(StaticMesh);
        ISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 성능 최적화
        ISMC->SetCastShadow(true);
        ISMC->SetReceivesDecals(false);  // 성능 최적화
        
        // 루트 컴포넌트에 연결
        ISMC->AttachToComponent(OwnerActor->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);
        
        // 머티리얼 적용
        for (int32 MaterialIndex = 0; MaterialIndex < Batch.Materials.Num(); ++MaterialIndex) {
            ISMC->SetMaterial(MaterialIndex, Batch.Materials[MaterialIndex]);
        }
        
        // 배치 구조체에 컴포넌트 할당
        Batch.Component = ISMC;
        
        // 컴포넌트 등록 (게임 월드에 표시되도록)
        ISMC->RegisterComponent();
    }
}

void UWFC3DVisualizer_ISMC::PopulateInstanceData() {
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_PopulateInstanceData);
    
    // 모든 배치의 인스턴스 데이터를 GPU로 전송
    for (auto& [StaticMesh, Batch] : ISMCBatches) {
        Batch.FlushUpdates();
    }
}

void UWFC3DVisualizer_ISMC::VisualizeGrid(
    UWFC3DGrid* Grid, 
    UWFC3DModelDataAsset* ModelData, 
    const FRandomStream* RandomStream) {
    
    if (!Grid || !ModelData || !OwnerActor) return;
    
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_VisualizeGrid_ISMC);
    
    // 1️⃣ 기존 시각화 정리
    ClearVisualization();
    
    // 2️⃣ 인스턴스 데이터 수집
    CollectInstanceData(Grid, ModelData, RandomStream);
    
    // 3️⃣ ISMC 컴포넌트 생성
    CreateISMCComponents();
    
    // 4️⃣ 인스턴스 데이터 적용
    PopulateInstanceData();
    
    // 5️⃣ 추가 최적화 수행
    OptimizeMaterialBatching();
}
```

### 📅 Week 3: 통합 및 호환성 구현

#### Day 11-13: 기존 시스템과의 통합

**작업 7: 매니저 클래스 구현 완성**
```cpp
// 📁 Source/ProceduralWorld/Private/WFC/Visualization/WFC3DVisualizer.cpp 수정
FWFC3DVisualizeResult UWFC3DVisualizer::ExecuteInternal(const FWFC3DVisualizeContext& Context) {
    FScopeLock Lock(&CriticalSection);
    
    FWFC3DVisualizeResult Result;
    
    // 1️⃣ 렌더링 전략 선택 (자동 또는 수동)
    EWFCRenderingMode SelectedMode = RenderingMode;
    if (bAutoSelectRenderingMode || RenderingMode == EWFCRenderingMode::Auto) {
        SelectedMode = SelectOptimalRenderingMode(Context.Grid->GetDimensions());
    }
    
    // 2️⃣ 전략이 변경되었으면 새로 생성
    if (!CurrentRenderingStrategy || GetCurrentRenderingMode() != SelectedMode) {
        CreateRenderingStrategy(SelectedMode);
    }
    
    // 3️⃣ 전략을 통해 시각화 실행
    IWFCRenderingStrategy* Strategy = GetRenderingStrategy();
    if (Strategy) {
        Strategy->VisualizeGrid(Context.Grid, Context.ModelData, Context.RandomStream);
        
        // 성능 통계 수집
        FWFCRenderingStats Stats = Strategy->GetRenderingStats();
        Result.bSuccess = true;
        Result.TotalInstancesCreated = Stats.TotalInstances;
        Result.RenderingMode = SelectedMode;
    }
    
    return Result;
}

void UWFC3DVisualizer::CreateRenderingStrategy(EWFCRenderingMode Mode) {
    // 기존 전략 정리
    if (CurrentRenderingStrategy) {
        IWFCRenderingStrategy* OldStrategy = GetRenderingStrategy();
        if (OldStrategy) {
            OldStrategy->ClearVisualization();
        }
    }
    
    // 새로운 전략 생성
    switch (Mode) {
        case EWFCRenderingMode::StaticMesh:
            CurrentRenderingStrategy = NewObject<UWFC3DVisualizer_Legacy>(this);
            break;
            
        case EWFCRenderingMode::InstancedStaticMesh:
            CurrentRenderingStrategy = NewObject<UWFC3DVisualizer_ISMC>(this);
            break;
            
        case EWFCRenderingMode::HierarchicalISM:
            // Phase 2에서 구현
            CurrentRenderingStrategy = NewObject<UWFC3DVisualizer_ISMC>(this); // 임시로 ISMC 사용
            break;
            
        default:
            CurrentRenderingStrategy = NewObject<UWFC3DVisualizer_Legacy>(this);
            break;
    }
    
    // 초기화
    IWFCRenderingStrategy* Strategy = GetRenderingStrategy();
    if (Strategy) {
        Strategy->Initialize(GetTypedOuter<AActor>(), TileSize);
    }
}

EWFCRenderingMode UWFC3DVisualizer::SelectOptimalRenderingMode(const FIntVector& GridSize) {
    int32 TotalCells = GridSize.X * GridSize.Y * GridSize.Z;
    
    // 그리드 크기에 따른 자동 선택
    if (TotalCells <= 125) {  // 5×5×5 이하
        return EWFCRenderingMode::StaticMesh;
    }
    else if (TotalCells <= 8000) {  // ~20×20×10
        return EWFCRenderingMode::InstancedStaticMesh;
    }
    else {
        return EWFCRenderingMode::HierarchicalISM;  // Phase 2 준비
    }
}
```

#### Day 14-15: 성능 모니터링 및 디버깅 도구

**작업 8: 성능 프로파일링 시스템**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizerProfiler.h
UCLASS(BlueprintType)
class PROCEDURALWORLD_API UWFC3DVisualizerProfiler : public UObject {
    GENERATED_BODY()

public:
    struct FVisualizationMetrics {
        float ExecutionTimeMs = 0.0f;
        float MemoryUsageMB = 0.0f;
        int32 DrawCalls = 0;
        int32 TotalInstances = 0;
        EWFCRenderingMode UsedRenderingMode;
        FDateTime Timestamp;
    };
    
    UFUNCTION(BlueprintCallable, Category = "WFC Profiling")
    void StartProfiling();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Profiling")
    void StopProfiling();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Profiling")
    FVisualizationMetrics GetLastMetrics() const;
    
    UFUNCTION(BlueprintCallable, Category = "WFC Profiling")
    void LogPerformanceReport();

private:
    TArray<FVisualizationMetrics> MetricsHistory;
    FDateTime ProfilingStartTime;
    bool bIsProfiling = false;
    
    void CollectCurrentMetrics();
    void GeneratePerformanceReport();
};

// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizerDebugWidget.h
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API UWFC3DVisualizerDebugWidget : public UUserWidget {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "WFC Debug")
    void UpdateRenderingStats(const FWFCRenderingStats& Stats);
    
    UFUNCTION(BlueprintCallable, Category = "WFC Debug")
    void SetRenderingMode(EWFCRenderingMode Mode);
    
protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DrawCallsText;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* InstancesText;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* MemoryUsageText;
    
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* RenderingModeText;
    
    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* RenderingModeCombo;
};
```

---

## ⚡ Phase 2: HISMC 전환 (Week 4-7)

### 📅 Week 4: HISMC 기본 구조 구축

#### Day 16-18: HISMC 전략 클래스 생성

**작업 9: HISMC 전략 클래스 구현**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizer_HISMC.h
class PROCEDURALWORLD_API UWFC3DVisualizer_HISMC : public UWFC3DVisualizer_ISMC {
    GENERATED_BODY()

public:
    UWFC3DVisualizer_HISMC();
    virtual ~UWFC3DVisualizer_HISMC() = default;

    // IWFCRenderingStrategy 오버라이드
    virtual void Initialize(AActor* OwnerActor, float TileSize) override;
    virtual bool SupportsGridSize(const FIntVector& Dimensions) const override;
    virtual EWFCRenderingMode GetRenderingMode() const override { 
        return EWFCRenderingMode::HierarchicalISM; 
    }
    virtual FString GetStrategyName() const override { 
        return TEXT("HierarchicalInstancedStaticMesh"); 
    }

    // HISMC 전용 기능
    UFUNCTION(BlueprintCallable, Category = "WFC HISMC")
    void SetLODSettings(const FWFCHISMCLODSettings& Settings);
    
    UFUNCTION(BlueprintCallable, Category = "WFC HISMC")
    void SetCullingSettings(const FWFCHISMCCullingSettings& Settings);
    
    UFUNCTION(BlueprintCallable, Category = "WFC HISMC")
    void OptimizeForLargeScale(bool bEnable);

protected:
    virtual void CreateISMCComponents() override;  // HISMC로 변경
    
    void ConfigureLODSystem();
    void ConfigureCullingSystem();
    void ConfigureLargeScaleOptimizations(UHierarchicalInstancedStaticMeshComponent* HISMC, int32 InstanceCount);
    void ScheduleTreeRebuild();
    void PerformTreeOptimization();

private:
    struct FHISMCManager {
        UPROPERTY()
        UHierarchicalInstancedStaticMeshComponent* HISMC = nullptr;
        
        FWFCHISMCLODSettings LODSettings;
        FWFCHISMCCullingSettings CullingSettings;
        bool bNeedsTreeRebuild = false;
        float LastOptimizationTime = 0.0f;
        
        void UpdateLODDistances();
        void UpdateCullingSettings();
        void RebuildTreeIfNeeded();
    };
    
    UPROPERTY()
    TMap<UStaticMesh*, FHISMCManager> HISMCManagers;
    
    UPROPERTY(EditAnywhere, Category = "HISMC Settings")
    FWFCHISMCLODSettings DefaultLODSettings;
    
    UPROPERTY(EditAnywhere, Category = "HISMC Settings")
    FWFCHISMCCullingSettings DefaultCullingSettings;
    
    bool bLargeScaleOptimization = false;
    float LastGlobalOptimizationTime = 0.0f;
};

// HISMC 설정 구조체들
USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFCHISMCLODSettings {
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODDistances = {1000.0f, 3000.0f, 8000.0f};
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxDrawDistance = 10000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnablePerInstanceFade = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FadeOutStartDistance = 8000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FadeOutEndDistance = 10000.0f;
};

USTRUCT(BlueprintType)
struct PROCEDURALWORLD_API FWFCHISMCCullingSettings {
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    int32 MaxInstancesPerLeaf = 1024;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float ClusterCullDistance = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bUseAsyncBuildTree = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float TreeRebuildInterval = 5.0f;
};
```

### 📅 Week 5-6: HISMC 고급 기능 구현

#### Day 19-25: 공간 분할 및 LOD 시스템

**작업 10: HISMC 핵심 기능 구현**
```cpp
// 📁 Source/ProceduralWorld/Private/WFC/Visualization/WFC3DVisualizer_HISMC.cpp
void UWFC3DVisualizer_HISMC::CreateISMCComponents() {
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_CreateHISMCComponents);
    
    if (!OwnerActor) return;
    
    for (auto& [StaticMesh, Batch] : ISMCBatches) {
        if (!StaticMesh || Batch.InstanceTransforms.Num() == 0) continue;
        
        // HISMC 컴포넌트 생성 (ISMC 대신)
        UHierarchicalInstancedStaticMeshComponent* HISMC = 
            NewObject<UHierarchicalInstancedStaticMeshComponent>(OwnerActor);
        if (!HISMC) continue;
        
        // 기본 설정
        HISMC->SetStaticMesh(StaticMesh);
        HISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        HISMC->SetCastShadow(true);
        HISMC->SetReceivesDecals(false);
        
        // HISMC 전용 최적화 설정
        HISMC->SetCullDistances(DefaultLODSettings.FadeOutStartDistance, DefaultLODSettings.MaxDrawDistance);
        HISMC->bUseAsyncBuildTree = DefaultCullingSettings.bUseAsyncBuildTree;
        
        // 클러스터링 설정
        HISMC->SetClusterSettings(
            DefaultCullingSettings.MaxInstancesPerLeaf,  // MaxInstancesPerLeaf
            16,                                           // MaxClustersPerLevel  
            true                                          // bDisableCollision
        );
        
        // 대규모 최적화 적용
        if (bLargeScaleOptimization) {
            ConfigureLargeScaleOptimizations(HISMC, Batch.InstanceTransforms.Num());
        }
        
        // 루트 컴포넌트에 연결
        HISMC->AttachToComponent(OwnerActor->GetRootComponent(), 
            FAttachmentTransformRules::KeepRelativeTransform);
        
        // 머티리얼 적용
        for (int32 MaterialIndex = 0; MaterialIndex < Batch.Materials.Num(); ++MaterialIndex) {
            HISMC->SetMaterial(MaterialIndex, Batch.Materials[MaterialIndex]);
        }
        
        // 메모리 사전 할당
        HISMC->PreAllocateInstancesMemory(Batch.InstanceTransforms.Num());
        
        // 인스턴스 배치 추가
        HISMC->AddInstances(Batch.InstanceTransforms, false);
        
        // 매니저 생성 및 등록
        FHISMCManager& Manager = HISMCManagers.Add(StaticMesh);
        Manager.HISMC = HISMC;
        Manager.LODSettings = DefaultLODSettings;
        Manager.CullingSettings = DefaultCullingSettings;
        
        // 배치 업데이트 (ISMC 인터페이스 호환성)
        Batch.Component = HISMC;  // UInstancedStaticMeshComponent의 부모 클래스이므로 호환
        
        // 컴포넌트 등록
        HISMC->RegisterComponent();
        
        // 공간 분할 트리 비동기 빌드
        if (DefaultCullingSettings.bUseAsyncBuildTree && Batch.InstanceTransforms.Num() > 100) {
            HISMC->BuildTreeAnyThread(true, true);
            Manager.bNeedsTreeRebuild = false;
        }
    }
    
    // 전역 최적화 스케줄링
    ScheduleTreeRebuild();
}

void UWFC3DVisualizer_HISMC::ConfigureLargeScaleOptimizations(
    UHierarchicalInstancedStaticMeshComponent* HISMC, 
    int32 InstanceCount) {
    
    if (InstanceCount > 10000) {
        // 매우 큰 규모
        HISMC->bUseAsyncBuildTree = true;
        HISMC->SetClusterSettings(2048, 8, true);  // 더 큰 클러스터
        HISMC->SetCullDistances(2000.0f, 15000.0f);
        
        // UE5.1+ GPU 기반 선택
        #if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
        HISMC->bUseGPUBasedSelection = true;
        #endif
    }
    else if (InstanceCount > 5000) {
        // 중간 규모
        HISMC->SetClusterSettings(1536, 12, true);
        HISMC->SetCullDistances(1500.0f, 12000.0f);
    }
    else if (InstanceCount > 1000) {
        // 소규모
        HISMC->SetClusterSettings(1024, 16, true);
        HISMC->SetCullDistances(1000.0f, 10000.0f);
    }
}

void UWFC3DVisualizer_HISMC::PerformTreeOptimization() {
    SCOPE_CYCLE_COUNTER(STAT_WFC3D_HISMC_TreeOptimization);
    
    float CurrentTime = FPlatformTime::Seconds();
    
    if (CurrentTime - LastGlobalOptimizationTime < 2.0f) {
        return;  // 2초마다만 실행
    }
    
    for (auto& [StaticMesh, Manager] : HISMCManagers) {
        if (!Manager.HISMC) continue;
        
        // 트리 리빌드가 필요한 경우
        if (Manager.bNeedsTreeRebuild) {
            Manager.RebuildTreeIfNeeded();
        }
        
        // LOD 거리 동적 조정
        if (bLargeScaleOptimization) {
            Manager.UpdateLODDistances();
        }
    }
    
    LastGlobalOptimizationTime = CurrentTime;
}

void UWFC3DVisualizer_HISMC::FHISMCManager::RebuildTreeIfNeeded() {
    if (!HISMC || !bNeedsTreeRebuild) return;
    
    float CurrentTime = FPlatformTime::Seconds();
    
    if (CurrentTime - LastOptimizationTime < CullingSettings.TreeRebuildInterval) {
        return;
    }
    
    // 비동기 트리 리빌드
    HISMC->BuildTreeAnyThread(true, true);
    
    bNeedsTreeRebuild = false;
    LastOptimizationTime = CurrentTime;
}
```

### 📅 Week 7: 성능 최적화 및 안정성 개선

#### Day 26-30: 메모리 최적화 및 안전장치

**작업 11: 메모리 최적화 및 안전장치 시스템**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizerSafetySystem.h
UCLASS(BlueprintType)
class PROCEDURALWORLD_API UWFC3DVisualizerSafetySystem : public UObject {
    GENERATED_BODY()

public:
    struct FPerformanceThresholds {
        float MinAcceptableFPS = 20.0f;
        float MaxAcceptableMemoryMB = 2000.0f;
        int32 MaxConsecutivePoorFrames = 180;  // 3초
        float MaxAcceptableFrameTimeMs = 50.0f;  // 20 FPS
    };
    
    UFUNCTION(BlueprintCallable, Category = "WFC Safety")
    void StartMonitoring(UWFC3DVisualizer* Visualizer);
    
    UFUNCTION(BlueprintCallable, Category = "WFC Safety")
    void StopMonitoring();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Safety")
    bool ShouldTriggerRollback() const;
    
    UFUNCTION(BlueprintCallable, Category = "WFC Safety")
    void TriggerEmergencyRollback(const FString& Reason);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety Settings")
    FPerformanceThresholds Thresholds;
    
    UPROPERTY(BlueprintAssignable, Category = "Safety Events")
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceWarning, FString, Warning);
    
    UPROPERTY(BlueprintAssignable, Category = "Safety Events") 
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmergencyRollback, FString, Reason, EWFCRenderingMode, FallbackMode);

private:
    UPROPERTY()
    TWeakObjectPtr<UWFC3DVisualizer> MonitoredVisualizer;
    
    TArray<float> RecentFrameTimes;
    int32 PoorPerformanceFrameCount = 0;
    float LastMemoryCheckTime = 0.0f;
    bool bIsMonitoring = false;
    
    void OnFrameEnd();
    void CheckMemoryUsage();
    void EvaluatePerformance();
    void ExecuteRollback(EWFCRenderingMode FallbackMode);
    
    UFUNCTION()
    void HandleEndFrameCallback();
};

// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizerMemoryManager.h
class PROCEDURALWORLD_API FWFC3DMemoryManager {
public:
    struct FMemoryStats {
        float TotalAllocatedMB = 0.0f;
        float ISMCInstanceDataMB = 0.0f;
        float HISMCTreeDataMB = 0.0f;
        float MeshAssetsMB = 0.0f;
        float MaterialDataMB = 0.0f;
    };
    
    static FMemoryStats GetCurrentMemoryUsage();
    static void OptimizeMemoryUsage();
    static bool IsMemoryUsageCritical();
    static void PreallocateInstanceBuffers(int32 EstimatedInstanceCount);
    static void FlushUnusedMemory();

private:
    static TMap<UStaticMesh*, int32> MeshUsageCount;
    static TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> TrackedISMCs;
    
    static float CalculateInstanceDataMemory(UInstancedStaticMeshComponent* ISMC);
    static float CalculateTreeDataMemory(UHierarchicalInstancedStaticMeshComponent* HISMC);
    static void OptimizeISMCMemory(UInstancedStaticMeshComponent* ISMC);
};
```

---

## 🧪 테스트 및 검증 (Week 8)

### 자동화된 테스트 시스템 구축

#### Day 31-33: 성능 벤치마크 시스템

**작업 12: 종합 벤치마크 시스템**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Testing/WFC3DVisualizerBenchmark.h
UCLASS(BlueprintType)
class PROCEDURALWORLD_API UWFC3DVisualizerBenchmark : public UObject {
    GENERATED_BODY()

public:
    USTRUCT(BlueprintType)
    struct FBenchmarkScenario {
        GENERATED_BODY()
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString ScenarioName = TEXT("Default");
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FIntVector GridSize = FIntVector(10, 10, 10);
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<EWFCRenderingMode> RenderingModesToTest = {
            EWFCRenderingMode::StaticMesh,
            EWFCRenderingMode::InstancedStaticMesh,
            EWFCRenderingMode::HierarchicalISM
        };
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 TestDurationSeconds = 30;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 WarmupFrames = 60;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        TArray<FVector> CameraPositions = {
            FVector(0, 0, 1000),
            FVector(1000, 1000, 1000),
            FVector(-1000, -1000, 1000)
        };
    };
    
    UFUNCTION(BlueprintCallable, Category = "WFC Benchmark")
    void RunBenchmark(const FBenchmarkScenario& Scenario);
    
    UFUNCTION(BlueprintCallable, Category = "WFC Benchmark") 
    void RunComprehensiveBenchmark();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Benchmark")
    FWFCBenchmarkResults GetLastResults() const;
    
    UFUNCTION(BlueprintCallable, Category = "WFC Benchmark")
    void ExportResultsToCSV(const FString& FilePath);

protected:
    UPROPERTY(BlueprintAssignable, Category = "Benchmark Events")
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBenchmarkProgress, FString, CurrentTest, float, Progress);
    
    UPROPERTY(BlueprintAssignable, Category = "Benchmark Events")
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBenchmarkCompleted, FWFCBenchmarkResults, Results);

private:
    UPROPERTY()
    TObjectPtr<UWFC3DVisualizer> TestVisualizer;
    
    UPROPERTY()
    TObjectPtr<UWFC3DGrid> TestGrid;
    
    UPROPERTY()
    TObjectPtr<UWFC3DModelDataAsset> TestModelData;
    
    FWFCBenchmarkResults LastResults;
    TArray<FWFCFrameMetrics> CurrentTestFrameData;
    
    void SetupTestEnvironment(const FBenchmarkScenario& Scenario);
    void CleanupTestEnvironment();
    void CollectFrameMetrics(FWFCFrameMetrics& OutMetrics);
    void AnalyzeResults(const TArray<FWFCFrameMetrics>& FrameData, FWFCBenchmarkResults& OutResults);
    void RunSingleRenderingModeTest(EWFCRenderingMode Mode, const FBenchmarkScenario& Scenario);
    void MoveCameraToPosition(const FVector& Position);
    void WaitForStableFrameRate();
};

// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Testing/WFC3DVisualizerUnitTests.h
#if WITH_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Tests/AutomationCommon.h"
#include "WFC/Visualization/WFC3DVisualizer.h"

// 기본 기능 테스트
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWFC3DVisualizerBasicTest, 
    "WFC3D.Visualizer.BasicFunctionality", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWFC3DVisualizerBasicTest::RunTest(const FString& Parameters) {
    // Given: 기본 테스트 데이터 생성
    auto TestWorld = CreateTestWorld();
    auto TestActor = TestWorld->SpawnActor<AActor>();
    auto Visualizer = NewObject<UWFC3DVisualizer>(TestActor);
    
    // When: 각 렌더링 모드로 시각화 실행
    TArray<EWFCRenderingMode> ModesToTest = {
        EWFCRenderingMode::StaticMesh,
        EWFCRenderingMode::InstancedStaticMesh,
        EWFCRenderingMode::HierarchicalISM
    };
    
    for (auto Mode : ModesToTest) {
        Visualizer->SetRenderingMode(Mode);
        
        FWFC3DVisualizeContext Context = CreateTestContext();
        FWFC3DVisualizeResult Result = Visualizer->Execute(Context);
        
        // Then: 성공적으로 실행되었는지 확인
        TestTrue(FString::Printf(TEXT("Rendering mode %d should succeed"), (int32)Mode), Result.bSuccess);
        TestTrue(TEXT("Should create instances"), Result.TotalInstancesCreated > 0);
    }
    
    return true;
}

// 성능 비교 테스트
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWFC3DVisualizerPerformanceTest, 
    "WFC3D.Visualizer.Performance", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWFC3DVisualizerPerformanceTest::RunTest(const FString& Parameters) {
    struct FPerformanceResult {
        EWFCRenderingMode Mode;
        float ExecutionTimeMs;
        int32 DrawCalls;
        float MemoryUsageMB;
    };
    
    TArray<FPerformanceResult> Results;
    
    // 각 렌더링 모드의 성능 측정
    TArray<EWFCRenderingMode> ModesToTest = {
        EWFCRenderingMode::StaticMesh,
        EWFCRenderingMode::InstancedStaticMesh,
        EWFCRenderingMode::HierarchicalISM
    };
    
    for (auto Mode : ModesToTest) {
        FPerformanceResult Result = MeasureRenderingModePerformance(Mode);
        Results.Add(Result);
    }
    
    // 성능 개선 검증
    TestTrue(TEXT("ISMC should have fewer draw calls than SMC"), 
        Results[1].DrawCalls < Results[0].DrawCalls);
    
    TestTrue(TEXT("HISMC should have fewer draw calls than ISMC"), 
        Results[2].DrawCalls <= Results[1].DrawCalls);
    
    TestTrue(TEXT("ISMC should use less memory than SMC"), 
        Results[1].MemoryUsageMB < Results[0].MemoryUsageMB);
    
    return true;
}

// 안정성 테스트
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWFC3DVisualizerStabilityTest, 
    "WFC3D.Visualizer.Stability", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FWFC3DVisualizerStabilityTest::RunTest(const FString& Parameters) {
    // 메모리 누수 테스트
    float InitialMemory = GetCurrentMemoryUsage();
    
    for (int32 i = 0; i < 100; ++i) {
        auto Visualizer = NewObject<UWFC3DVisualizer>();
        // ... 시각화 실행 및 정리 ...
        Visualizer = nullptr;
        
        if (i % 10 == 0) {
            CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
        }
    }
    
    float FinalMemory = GetCurrentMemoryUsage();
    float MemoryIncrease = FinalMemory - InitialMemory;
    
    TestTrue(TEXT("Memory increase should be minimal"), MemoryIncrease < 100.0f);  // 100MB 이하
    
    return true;
}

#endif // WITH_AUTOMATION_TESTS
```

#### Day 34-35: 통합 테스트 및 최종 검증

**작업 13: 실제 환경 테스트**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Testing/WFC3DVisualizerIntegrationTest.h
UCLASS(BlueprintType)
class PROCEDURALWORLD_API UWFC3DVisualizerIntegrationTest : public UObject {
    GENERATED_BODY()

public:
    USTRUCT(BlueprintType)
    struct FIntegrationTestScenario {
        GENERATED_BODY()
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FString ScenarioName;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FIntVector GridSize;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 ExpectedMinFPS = 30;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        float MaxAllowedMemoryMB = 1000.0f;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        int32 MaxAllowedDrawCalls = 200;
        
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        bool bTestAllRenderingModes = true;
    };
    
    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    void RunIntegrationTest(const FIntegrationTestScenario& Scenario);
    
    UFUNCTION(BlueprintCallable, Category = "Integration Test") 
    void RunRegressionTest();
    
    UFUNCTION(BlueprintCallable, Category = "Integration Test")
    bool ValidateBackwardCompatibility();

private:
    TArray<FString> TestResults;
    int32 PassedTests = 0;
    int32 FailedTests = 0;
    
    void TestRenderingModeCompatibility();
    void TestAsyncVisualization(); 
    void TestMemoryManagement();
    void TestLargeScalePerformance();
    void GenerateTestReport();
};
```

---

## 📦 배포 계획 (Week 9)

### 단계적 배포 전략

#### Day 36-37: 프로덕션 준비

**작업 14: 배포 설정 및 문서화**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizerConfig.h
UCLASS(Config=Game, BlueprintType)
class PROCEDURALWORLD_API UWFC3DVisualizerConfig : public UObject {
    GENERATED_BODY()

public:
    // 전역 설정
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    EWFCRenderingMode DefaultRenderingMode = EWFCRenderingMode::Auto;
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableAutomaticRollback = true;
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnablePerformanceMonitoring = true;
    
    // 성능 임계값
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinAcceptableFPS = 30.0f;
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 2000.0f;
    
    // 자동 선택 설정
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Auto Selection")
    int32 StaticMeshModeMaxCells = 125;      // 5×5×5
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Auto Selection")
    int32 ISMCModeMaxCells = 8000;           // ~20×20×10
    
    // 디버그 설정
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowRenderingStats = false;
    
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDetailedProfiling = false;

public:
    static UWFC3DVisualizerConfig* Get();
};

// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DVisualizerModule.h
class PROCEDURALWORLD_API FWFC3DVisualizerModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
private:
    void RegisterRenderingStrategies();
    void InitializePerformanceMonitoring();
    void SetupDefaultSettings();
};
```

#### Day 38-40: 문서 및 예제 생성

**작업 15: 사용자 가이드 및 예제**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Examples/WFC3DVisualizerExamples.h
UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API AWFC3DVisualizerExamples : public AActor {
    GENERATED_BODY()

public:
    AWFC3DVisualizerExamples();

    // 📚 예제 메서드들
    UFUNCTION(BlueprintCallable, Category = "WFC Examples")
    void Example01_BasicUsage();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Examples")
    void Example02_PerformanceComparison();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Examples")
    void Example03_LargeScaleVisualization();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Examples")
    void Example04_CustomRenderingSettings();
    
    UFUNCTION(BlueprintCallable, Category = "WFC Examples")
    void Example05_RuntimeModeSwitching();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UWFC3DVisualizer> Visualizer;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TObjectPtr<UWFC3DModelDataAsset> ExampleModelData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FIntVector ExampleGridSize = FIntVector(10, 10, 10);

private:
    void SetupExampleEnvironment();
    void RunPerformanceBenchmark();
    void DemonstrateRenderingModes();
};
```

---

## 🎯 즉시 시작 가능한 작업

### 📅 오늘 시작할 수 있는 작업들

#### 🏃‍♂️ Quick Win 작업 (30분-2시간)

**1. 성능 측정 도구 추가**
```cpp
// 📁 기존 WFC3DVisualizer.h에 간단한 메트릭 추가
UFUNCTION(BlueprintCallable, Category = "WFC Debug")
void LogCurrentPerformanceStats();

// 📁 기존 WFC3DVisualizer.cpp에 구현
void UWFC3DVisualizer::LogCurrentPerformanceStats() {
    int32 ComponentCount = SpawnedMeshComponents.Num();
    float MemoryEstimate = ComponentCount * 0.8f;  // 컴포넌트당 약 0.8KB
    
    UE_LOG(LogTemp, Warning, TEXT("WFC3D Performance Stats:"));
    UE_LOG(LogTemp, Warning, TEXT("- Components: %d"), ComponentCount);
    UE_LOG(LogTemp, Warning, TEXT("- Estimated Memory: %.2f MB"), MemoryEstimate);
    UE_LOG(LogTemp, Warning, TEXT("- Draw Calls (Est): %d"), ComponentCount);
}
```

**2. 렌더링 모드 컴파일 플래그**
```cpp
// 📁 새 파일: Source/ProceduralWorld/Public/WFC/Visualization/WFC3DRenderingConfig.h
#pragma once

// 컴파일 타임 렌더링 모드 선택
#ifndef WFC3D_RENDERING_MODE
#define WFC3D_RENDERING_MODE 0  // 0=SMC, 1=ISMC, 2=HISMC
#endif

#if WFC3D_RENDERING_MODE >= 1
#define WFC3D_ENABLE_ISMC 1
#else
#define WFC3D_ENABLE_ISMC 0
#endif

#if WFC3D_RENDERING_MODE >= 2
#define WFC3D_ENABLE_HISMC 1
#else
#define WFC3D_ENABLE_HISMC 0
#endif
```

**3. 기본 벤치마크 함수**
```cpp
// 📁 기존 WFC3DVisualizer.h에 추가
UFUNCTION(BlueprintCallable, Category = "WFC Benchmark")
void RunQuickPerformanceTest();

// 📁 기존 WFC3DVisualizer.cpp에 구현
void UWFC3DVisualizer::RunQuickPerformanceTest() {
    FDateTime StartTime = FDateTime::Now();
    
    // 현재 시각화 재실행
    if (GetWorld()) {
        // ... 기존 시각화 로직 ...
    }
    
    FDateTime EndTime = FDateTime::Now();
    float ExecutionTimeMs = (EndTime - StartTime).GetTotalMilliseconds();
    
    UE_LOG(LogTemp, Warning, TEXT("WFC3D Quick Test Results:"));
    UE_LOG(LogTemp, Warning, TEXT("- Execution Time: %.2f ms"), ExecutionTimeMs);
}
```

#### 🛠️ 하루 작업 (4-8시간)

**1. 기본 ISMC 프로토타입 생성**
- `IWFCRenderingStrategy` 인터페이스 생성
- `UWFC3DVisualizer_Legacy` 클래스 생성 (기존 코드 이동)
- 기본 `UWFC3DVisualizer_ISMC` 클래스 뼈대 생성

**2. 성능 모니터링 시스템**
- 기본 FPS/메모리 모니터링 클래스 생성
- UE5 에디터 위젯으로 실시간 표시
- CSV 로그 출력 기능

**3. 자동화된 테스트 기본 구조**
- Google Test 또는 UE5 Automation Test 설정
- 기본 기능 테스트 케이스 작성
- CI/CD 파이프라인 연결 준비

---

## 🎯 결론 및 다음 단계

### 예상 성과 요약
- **Phase 1 완료 후**: 10×10×10 그리드에서 15-30 FPS → 60 FPS, 70% 메모리 절약
- **Phase 2 완료 후**: 32×32×32 그리드에서 30+ FPS, 99.7% Draw Call 감소
- **전체 완료 후**: 생산성 향상, 확장 가능한 아키텍처, 완전한 하위 호환성

### 🚀 권장 시작 순서
1. **오늘**: Quick Win 작업들로 현재 성능 기준선 확립
2. **내일**: Phase 1 Day 1-2 작업 시작 (인터페이스 설계)
3. **이번 주**: Week 1 완료 (기반 인프라 구축)
4. **다음 주**: Week 2 시작 (ISMC 핵심 로직 구현)

### 🎯 성공 지표
- **Week 3 종료 시**: 10×10×10 그리드 60 FPS 달성
- **Week 7 종료 시**: 32×32×32 그리드 30+ FPS 달성
- **Week 9 종료 시**: 프로덕션 배포 준비 완료

어떤 작업부터 시작하시겠습니까? 즉시 Quick Win 작업들을 구현해드릴 수도 있습니다!

---

*문서 버전: v1.0*  
*작성일: 2025-08-11*  
*예상 완료: 2025-10-13 (9주 계획)*