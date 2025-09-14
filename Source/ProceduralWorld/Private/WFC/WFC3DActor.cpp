// Fill out your copyright notice in the Description page of Project Settings.

#include "WFC/WFC3DActor.h"
#include "WFC/Data/WFC3DGrid.h"
#include "WFC/Visualization/WFC3DVisualizer.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AWFC3DActor::AWFC3DActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 설정
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// WFC3D 컨트롤러 생성
	WFC3DController = CreateDefaultSubobject<UWFC3DController>(TEXT("WFC3DController"));

	// 기본 실행 컨텍스트 설정
	ExecutionContext.GridDimension = FIntVector(5, 5, 5);
	ExecutionContext.bEnableVisualization = true;
	ExecutionContext.bRealTimeGeneration = true;
	ExecutionContext.RandomSeed = 0;
	ExecutionContext.MaxRetryCount = 5;
	
	// 기본 ModelData 설정 (경로는 블루프린트에서 수정 가능)
	static ConstructorHelpers::FObjectFinder<UWFC3DModelDataAsset> DefaultModelDataFinder(TEXT("/Game/WFC3D/Data/DA_WFC3DModel"));
	if (DefaultModelDataFinder.Succeeded())
	{
		ExecutionContext.ModelData = DefaultModelDataFinder.Object;
		UE_LOG(LogTemp, Log, TEXT("Default ModelData loaded successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load default ModelData. Please set ModelData manually in ExecutionContext."));
	}
}

// Called when the game starts or when spawned
void AWFC3DActor::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeAndExecuteWFC3D();
}

void AWFC3DActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// 무한 루프 방지
	if (bIsExecutingConstruction)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnConstruction already executing, skipping to prevent infinite loop"));
		return;
	}
	
	if(bCreateMeshComponents)
	{
		UE_LOG(LogTemp, Log, TEXT("OnConstruction: bCreateMeshComponents is true, executing WFC3D"));
		bIsExecutingConstruction = true;
		
		InitializeAndExecuteWFC3D();
		
		bCreateMeshComponents = false;
		bIsExecutingConstruction = false;
		
		UE_LOG(LogTemp, Log, TEXT("OnConstruction: WFC3D execution completed"));
	}
}

void AWFC3DActor::BeginDestroy()
{
	// 델리게이트 언바인딩
	if (WFC3DController && IsValid(WFC3DController) && bDelegatesBound)
	{
		WFC3DController->OnExecutionCompleted.RemoveDynamic(this, &AWFC3DActor::OnWFC3DExecutionCompleted);
		WFC3DController->OnExecutionCancelled.RemoveDynamic(this, &AWFC3DActor::OnWFC3DExecutionCancelled);
		WFC3DController->OnExecutionProgress.RemoveDynamic(this, &AWFC3DActor::OnWFC3DExecutionProgress);
		bDelegatesBound = false;
	}
	
	// 생성된 메시 컴포넌트들 정리
	for (UStaticMeshComponent* MeshComponent : GeneratedMeshComponents)
	{
		if (MeshComponent && IsValid(MeshComponent))
		{
			MeshComponent->DestroyComponent();
		}
	}
	GeneratedMeshComponents.Empty();
	GridToMeshMap.Empty();
	
	// WFC3DController는 CreateDefaultSubobject로 생성되었으므로 자동으로 정리됨
	
	Super::BeginDestroy();
}

void AWFC3DActor::ExecuteWFC3D()
{
	UE_LOG(LogTemp, Log, TEXT("ExecuteWFC3D() called"));
	
	if (!WFC3DController)
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3DController is null!"));
		return;
	}

	if (!ExecutionContext.ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelData is not set in ExecutionContext!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting synchronous WFC3D execution with grid size: %s"), *ExecutionContext.GridDimension.ToString());
	
	// 동기적 실행
	FWFC3DExecutionResult Result = WFC3DController->Execute(ExecutionContext);
	
	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("WFC3D generation completed successfully!"));
		CreateMeshComponentsFromGrid();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3D generation failed: %s"), *Result.ErrorMessage);
	}
}

void AWFC3DActor::ExecuteWFC3DAsync()
{
	UE_LOG(LogTemp, Log, TEXT("ExecuteWFC3DAsync() called"));
	
	if (!WFC3DController)
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3DController is null!"));
		return;
	}

	if (!ExecutionContext.ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelData is not set in ExecutionContext!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting asynchronous WFC3D execution with grid size: %s, ModelData: %s"), 
		*ExecutionContext.GridDimension.ToString(), 
		ExecutionContext.ModelData ? *ExecutionContext.ModelData->GetName() : TEXT("NULL"));
	
	// 비동기적 실행
	WFC3DController->ExecuteAsync(ExecutionContext);
	UE_LOG(LogTemp, Log, TEXT("WFC3D async generation started!"));
}

bool AWFC3DActor::VerifyGeneratedMeshes()
{
	bool bAllValid = true;
	int32 ValidCount = 0;
	int32 InvalidCount = 0;

	for (UStaticMeshComponent* MeshComponent : GeneratedMeshComponents)
	{
		if (ValidateMeshComponent(MeshComponent))
		{
			ValidCount++;
		}
		else
		{
			InvalidCount++;
			bAllValid = false;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Mesh Verification Results - Valid: %d, Invalid: %d, Total: %d"), 
		ValidCount, InvalidCount, GeneratedMeshComponents.Num());

	return bAllValid;
}

UStaticMeshComponent* AWFC3DActor::GetMeshComponentAt(const FIntVector& GridPosition) const
{
	if (GridToMeshMap.Contains(GridPosition))
	{
		return GridToMeshMap[GridPosition];
	}
	return nullptr;
}

TArray<UStaticMeshComponent*> AWFC3DActor::GetAllMeshComponents() const
{
	return GeneratedMeshComponents;
}

bool AWFC3DActor::IsGenerationComplete() const
{
	if (!WFC3DController)
	{
		return false;
	}
	return WFC3DController->IsComplete();
}

float AWFC3DActor::GetGenerationProgress() const
{
	if (!WFC3DController)
	{
		return 0.0f;
	}
	return WFC3DController->GetProgress();
}

void AWFC3DActor::BindControllerDelegates()
{
	if (!WFC3DController)
	{
		return;
	}

	// 이미 바인딩되어 있다면 스킵
	if (bDelegatesBound)
	{
		UE_LOG(LogTemp, Log, TEXT("Delegates already bound, skipping"));
		return;
	}

	// 기존 바인딩이 있다면 제거 (안전장치)
	WFC3DController->OnExecutionCompleted.RemoveDynamic(this, &AWFC3DActor::OnWFC3DExecutionCompleted);
	WFC3DController->OnExecutionCancelled.RemoveDynamic(this, &AWFC3DActor::OnWFC3DExecutionCancelled);
	WFC3DController->OnExecutionProgress.RemoveDynamic(this, &AWFC3DActor::OnWFC3DExecutionProgress);

	// 델리게이트 바인딩
	WFC3DController->OnExecutionCompleted.AddDynamic(this, &AWFC3DActor::OnWFC3DExecutionCompleted);
	WFC3DController->OnExecutionCancelled.AddDynamic(this, &AWFC3DActor::OnWFC3DExecutionCancelled);
	WFC3DController->OnExecutionProgress.AddDynamic(this, &AWFC3DActor::OnWFC3DExecutionProgress);
	
	bDelegatesBound = true;
	UE_LOG(LogTemp, Log, TEXT("Controller delegates bound successfully"));
}

void AWFC3DActor::InitializeAndExecuteWFC3D()
{
	// 에디터 모드 확인
	bool bIsInEditor = false;
#if WITH_EDITOR
	bIsInEditor = (GetWorld() && !GetWorld()->IsGameWorld());
#endif

	UE_LOG(LogTemp, Log, TEXT("InitializeAndExecuteWFC3D called - IsInEditor: %s"), bIsInEditor ? TEXT("TRUE") : TEXT("FALSE"));

	// 컨트롤러 델리게이트 바인딩 (런타임에서만)
	if (!bIsInEditor)
	{
		BindControllerDelegates();
	}

	// ModelData 체크 및 초기화
	if (ExecutionContext.ModelData)
	{
		if (ExecutionContext.ModelData->InitializeData())
		{
			UE_LOG(LogTemp, Display, TEXT("ModelData initialized successfully in WFC3DActor"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ModelData initialization failed, but continuing with WFC3D execution"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ModelData is null! Cannot execute WFC3D."));
		return;
	}
	
	// 자동 실행 옵션이 활성화되어 있으면 WFC3D 실행
	if (bAutoExecuteOnBeginPlay)
	{
		UE_LOG(LogTemp, Log, TEXT("Auto-executing WFC3D"));
		
		if (bIsInEditor)
		{
			// 에디터에서는 전용 함수 사용 (델리게이트 없음)
			UE_LOG(LogTemp, Log, TEXT("Editor mode: executing with editor-specific function"));
			ExecuteWFC3DForEditor();
		}
		else
		{
			// 런타임에서는 비동기로 실행하여 게임 시작을 방해하지 않도록 함
			UE_LOG(LogTemp, Log, TEXT("Runtime mode: executing asynchronously"));
			ExecuteWFC3DAsync();
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Auto-execution is disabled. Call ExecuteWFC3D() manually to start generation."));
	}
}

void AWFC3DActor::ExecuteWFC3DForEditor()
{
	UE_LOG(LogTemp, Log, TEXT("ExecuteWFC3DForEditor() called"));
	
	if (!WFC3DController)
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3DController is null!"));
		return;
	}

	if (!ExecutionContext.ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelData is not set in ExecutionContext!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Starting editor WFC3D execution with grid size: %s"), *ExecutionContext.GridDimension.ToString());
	
	// 동기적 실행 (에디터에서는 델리게이트 콜백을 기대하지 않음)
	FWFC3DExecutionResult Result = WFC3DController->Execute(ExecutionContext);
	
	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("WFC3D generation completed successfully in editor!"));
		// 에디터에서는 직접 메시 컴포넌트 생성
		CreateMeshComponentsFromGrid();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3D generation failed in editor: %s"), *Result.ErrorMessage);
	}
}

void AWFC3DActor::OnWFC3DExecutionCompleted(const FWFC3DAlgorithmResult& Result)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnWFC3DExecutionCompleted CALLED ==="));
	UE_LOG(LogTemp, Log, TEXT("WFC3D execution completed in Actor! Success: %s"), Result.bSuccess ? TEXT("TRUE") : TEXT("FALSE"));
	
	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Result success - calling CreateMeshComponentsFromGrid()"));
		// 비동기 실행의 경우 여기서 메시 컴포넌트 생성
		CreateMeshComponentsFromGrid();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3D execution failed: %s"), *Result.ErrorMessage);
	}
}

void AWFC3DActor::OnWFC3DExecutionCancelled()
{
	UE_LOG(LogTemp, Warning, TEXT("WFC3D execution was cancelled!"));
}

void AWFC3DActor::OnWFC3DExecutionProgress(int32 CurrentStep, int32 TotalSteps)
{
	float Progress = TotalSteps > 0 ? (float)CurrentStep / (float)TotalSteps : 0.0f;
	UE_LOG(LogTemp, Log, TEXT("WFC3D Progress: %d/%d (%.1f%%)"), CurrentStep, TotalSteps, Progress * 100.0f);
}

void AWFC3DActor::CreateMeshComponentsFromGrid()
{
	UE_LOG(LogTemp, Warning, TEXT("=== CreateMeshComponentsFromGrid() CALLED ==="));
	
	// 에디터 모드 확인
	bool bIsInEditor = false;
#if WITH_EDITOR
	bIsInEditor = (GetWorld() && !GetWorld()->IsGameWorld());
#endif

	UE_LOG(LogTemp, Log, TEXT("CreateMeshComponentsFromGrid - IsInEditor: %s"), bIsInEditor ? TEXT("TRUE") : TEXT("FALSE"));
	
	if (!WFC3DController)
	{
		UE_LOG(LogTemp, Error, TEXT("WFC3DController is null!"));
		return;
	}

	UWFC3DGrid* GeneratedGrid = WFC3DController->GetGeneratedGrid();
	if (!GeneratedGrid)
	{
		UE_LOG(LogTemp, Error, TEXT("No generated grid found!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("GeneratedGrid found, dimensions: %s"), *GeneratedGrid->GetDimension().ToString());

	// 기존 메시 컴포넌트 정리 (에디터에서는 더 안전하게)
	for (UStaticMeshComponent* MeshComp : GeneratedMeshComponents)
	{
		if (MeshComp && IsValid(MeshComp))
		{
#if WITH_EDITOR
			if (bIsInEditor)
			{
				// 에디터에서는 DestroyComponent 대신 다른 방법 사용
				MeshComp->DestroyComponent();
			}
			else
#endif
			{
				MeshComp->DestroyComponent();
			}
		}
	}
	GeneratedMeshComponents.Empty();
	GridToMeshMap.Empty();

	// 새로운 메시 컴포넌트 생성
	FIntVector GridDimensions = GeneratedGrid->GetDimension();
	int32 CreatedMeshCount = 0;

	for (int32 X = 0; X < GridDimensions.X; X++)
	{
		for (int32 Y = 0; Y < GridDimensions.Y; Y++)
		{
			for (int32 Z = 0; Z < GridDimensions.Z; Z++)
			{
				FIntVector GridPosition(X, Y, Z);
				FWFC3DCell* Cell = GeneratedGrid->GetCell(GridPosition);
				
				if (Cell && Cell->bIsCollapsed && Cell->CollapsedTileInfo)
				{
					// 셀 정보와 회전 정보를 함께 전달
					UStaticMeshComponent* MeshComponent = CreateMeshComponentAtPosition(GridPosition, *Cell);
					
					if (MeshComponent)
					{
						GeneratedMeshComponents.Add(MeshComponent);
						GridToMeshMap.Add(GridPosition, MeshComponent);
						CreatedMeshCount++;
					}
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Created %d mesh components from grid"), CreatedMeshCount);
}

UStaticMeshComponent* AWFC3DActor::CreateMeshComponentAtPosition(const FIntVector& GridPosition, const FWFC3DCell& Cell)
{
	if (!ExecutionContext.ModelData)
	{
		UE_LOG(LogTemp, Error, TEXT("ModelData is null!"));
		return nullptr;
	}

	// 셀의 타일 정보 확인
	if (!Cell.CollapsedTileInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("Cell has no collapsed tile info!"));
		return nullptr;
	}

	const FTileInfo& TileInfo = *Cell.CollapsedTileInfo;
	
	// 타일 시각 정보 가져오기
	const FTileVariantInfo* TileVariantInfo = ExecutionContext.ModelData->GetTileVariant(TileInfo.BaseTileID);
	if (!TileVariantInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get tile variant info for BaseTileID: %d"), TileInfo.BaseTileID);
		return nullptr;
	}

	// 기본 바이옴 선택 ("Default" 또는 첫 번째 바이옴)
	FString BiomeName = TEXT("Default");
	if (TileVariantInfo->Biomes.Num() > 0)
	{
		if (TileVariantInfo->Biomes.Contains(BiomeName))
		{
			// "Default" 바이옴 사용
		}
		else
		{
			// 첫 번째 바이옴 사용
			for (const auto& BiomePair : TileVariantInfo->Biomes)
			{
				BiomeName = BiomePair.Key;
				break;
			}
		}
	}

	const FTileByBiome* BiomeData = TileVariantInfo->Biomes.Find(BiomeName);
	if (!BiomeData || BiomeData->Tiles.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No tile data found for biome: %s"), *BiomeName);
		return nullptr;
	}

	// 첫 번째 타일 변형 사용
	const FTileVisualInfo& VisualInfo = BiomeData->Tiles[0];
	if (!VisualInfo.StaticMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("StaticMesh is null for tile at position (%d, %d, %d)"), 
			GridPosition.X, GridPosition.Y, GridPosition.Z);
		return nullptr;
	}

	// 메시 컴포넌트 생성 (에디터와 런타임 모두 지원)
	FString ComponentName = FString::Printf(TEXT("MeshComponent_%d_%d_%d"), 
		GridPosition.X, GridPosition.Y, GridPosition.Z);
	
	UStaticMeshComponent* MeshComponent = nullptr;
	
	// 에디터에서는 CreateDefaultSubobject 스타일로, 런타임에서는 NewObject 사용
#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		// 에디터 모드: Construction Script에서 호출
		MeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), *ComponentName, RF_Transactional);
	}
	else
#endif
	{
		// 게임 런타임 모드
		MeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), *ComponentName);
	}
	
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create mesh component!"));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Created mesh component successfully: %s"), *ComponentName);

	// 메시 설정
	MeshComponent->SetStaticMesh(VisualInfo.StaticMesh);
	UE_LOG(LogTemp, Log, TEXT("Set StaticMesh: %s"), VisualInfo.StaticMesh ? *VisualInfo.StaticMesh->GetName() : TEXT("NULL"));

	// 위치 계산 (Actor 기준 상대 좌표)
	// Actor를 그리드 바닥 중심으로 하여 상대적 위치 계산
	FVector RelativeLocation = FVector(
		(GridPosition.X - (ExecutionContext.GridDimension.X - 1) * 0.5f) * TileSize.X,
		(GridPosition.Y - (ExecutionContext.GridDimension.Y - 1) * 0.5f) * TileSize.Y,
		GridPosition.Z * TileSize.Z
	);
	
	// 회전 계산
	FRotator Rotation = FRotator::ZeroRotator;
	if (ExecutionContext.ModelData)
	{
		const FTileRotationInfo* RotationInfo = ExecutionContext.ModelData->GetTileRotationInfo(Cell.CollapsedTileInfoIndex);
		int32 RotationStep = RotationInfo ? RotationInfo->RotationStep : 0;
		Rotation = GetRotationFromRotationStep(RotationStep);
		UE_LOG(LogTemp, Log, TEXT("Applied rotation: RotationStep=%d, Rotation=%s"), RotationStep, *Rotation.ToString());
	}

	// 스케일 계산 (BaseTileSize 대비 TileSize 비율)
	FVector Scale = CalculateTileScale();

	// 트랜스폼 설정 (Actor 기준 상대 위치, 회전, 스케일)
	FTransform RelativeTransform(Rotation, RelativeLocation, Scale);

	// 루트 컴포넌트에 연결 및 트랜스폼 설정
	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	MeshComponent->SetRelativeTransform(RelativeTransform);
	UE_LOG(LogTemp, Log, TEXT("Attached to RootComponent and set transform - Location: %s, Rotation: %s, Scale: %s"), *RelativeLocation.ToString(), *Rotation.ToString(), *Scale.ToString());

	// 컴포넌트 등록 (에디터에서는 조건부)
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		MeshComponent->RegisterComponent();
		UE_LOG(LogTemp, Log, TEXT("Registered mesh component (Runtime)"));
	}
	else
	{
		// 에디터에서는 RegisterComponent를 호출하지 않음
		UE_LOG(LogTemp, Log, TEXT("Mesh component created in editor mode"));
	}
#else
	MeshComponent->RegisterComponent();
	UE_LOG(LogTemp, Log, TEXT("Registered mesh component (Runtime)"));
#endif

	// 매테리얼 설정
	for (int32 i = 0; i < VisualInfo.Materials.Num(); i++)
	{
		if (VisualInfo.Materials[i])
		{
			MeshComponent->SetMaterial(i, VisualInfo.Materials[i]);
			UE_LOG(LogTemp, Log, TEXT("Set material %d: %s"), i, *VisualInfo.Materials[i]->GetName());
		}
	}

	// 가시성 및 렌더링 설정
	MeshComponent->SetVisibility(true);
	MeshComponent->SetHiddenInGame(false);
	MeshComponent->SetCastShadow(true);
	UE_LOG(LogTemp, Log, TEXT("Set visibility and rendering settings"));

	UE_LOG(LogTemp, Log, TEXT("Created mesh component at grid position (%d, %d, %d) with relative location %s, mesh: %s"), 
		GridPosition.X, GridPosition.Y, GridPosition.Z, *RelativeLocation.ToString(),
		VisualInfo.StaticMesh ? *VisualInfo.StaticMesh->GetName() : TEXT("NULL"));

	return MeshComponent;
}

bool AWFC3DActor::ValidateMeshComponent(UStaticMeshComponent* MeshComponent) const
{
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshComponent is null!"));
		return false;
	}

	if (!MeshComponent->GetStaticMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshComponent has no StaticMesh!"));
		return false;
	}

	if (!MeshComponent->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshComponent is not valid!"));
		return false;
	}

	// 메시의 바운딩 박스 확인
	FBoxSphereBounds Bounds = MeshComponent->GetStaticMesh()->GetBounds();
	if (Bounds.BoxExtent.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("MeshComponent has zero bounds!"));
		return false;
	}

	return true;
}

FRotator AWFC3DActor::GetRotationFromRotationStep(int32 RotationStep) const
{
	// 회전 스텝을 FRotator로 변환 (Y축 기준 90도씩 회전)
	float YawRotation = RotationStep * 90.0f;
	return FRotator(0.0f, YawRotation, 0.0f);
}

FVector AWFC3DActor::CalculateTileScale() const
{
	// BaseTileSize 대비 TileSize의 비율을 계산
	FVector Scale = FVector::OneVector;
	
	if (BaseTileSize.X > 0.0f) Scale.X = TileSize.X / BaseTileSize.X;
	if (BaseTileSize.Y > 0.0f) Scale.Y = TileSize.Y / BaseTileSize.Y;
	if (BaseTileSize.Z > 0.0f) Scale.Z = TileSize.Z / BaseTileSize.Z;
	
	return Scale;
}

void AWFC3DActor::PositionActorAtGridCenter()
{
	// Actor는 (0,0,0)에 고정, 메시 컴포넌트 위치 계산에서 중심화 처리
	// 그리드 크기 정보는 로깅용으로만 사용
	FVector GridSize = FVector(
		ExecutionContext.GridDimension.X * TileSize.X,
		ExecutionContext.GridDimension.Y * TileSize.Y,
		ExecutionContext.GridDimension.Z * TileSize.Z
	);
	
	UE_LOG(LogTemp, Log, TEXT("Actor remains at (0,0,0), GridSize: %s"), *GridSize.ToString());
}

