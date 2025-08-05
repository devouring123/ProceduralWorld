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
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	
	// 컨트롤러 델리게이트 바인딩
	BindControllerDelegates();

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
		UE_LOG(LogTemp, Log, TEXT("Auto-executing WFC3D on BeginPlay"));
		
		// 비동기로 실행하여 게임 시작을 방해하지 않도록 함
		ExecuteWFC3DAsync();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Auto-execution is disabled. Call ExecuteWFC3D() manually to start generation."));
	}
}

// Called every frame
void AWFC3DActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWFC3DActor::BeginDestroy()
{
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

	// 델리게이트 바인딩
	WFC3DController->OnExecutionCompleted.AddDynamic(this, &AWFC3DActor::OnWFC3DExecutionCompleted);
	WFC3DController->OnExecutionCancelled.AddDynamic(this, &AWFC3DActor::OnWFC3DExecutionCancelled);
	WFC3DController->OnExecutionProgress.AddDynamic(this, &AWFC3DActor::OnWFC3DExecutionProgress);
}

void AWFC3DActor::OnWFC3DExecutionCompleted(const FWFC3DAlgorithmResult& Result)
{
	UE_LOG(LogTemp, Log, TEXT("WFC3D execution completed in Actor!"));
	
	if (Result.bSuccess)
	{
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

	// 기존 메시 컴포넌트 정리
	for (UStaticMeshComponent* MeshComp : GeneratedMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
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

	// 메시 컴포넌트 생성 (런타임에서는 NewObject 사용)
	FString ComponentName = FString::Printf(TEXT("MeshComponent_%d_%d_%d"), 
		GridPosition.X, GridPosition.Y, GridPosition.Z);
	
	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), *ComponentName);
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create mesh component!"));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Created mesh component successfully: %s"), *ComponentName);

	// 메시 설정
	MeshComponent->SetStaticMesh(VisualInfo.StaticMesh);
	UE_LOG(LogTemp, Log, TEXT("Set StaticMesh: %s"), VisualInfo.StaticMesh ? *VisualInfo.StaticMesh->GetName() : TEXT("NULL"));

	// 루트 컴포넌트에 연결
	MeshComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
	UE_LOG(LogTemp, Log, TEXT("Attached to RootComponent"));

	// 컴포넌트 등록 (매우 중요!)
	MeshComponent->RegisterComponent();
	UE_LOG(LogTemp, Log, TEXT("Registered mesh component"));

	// 위치 계산
	FVector Location = FVector(GridPosition.X * TileSize, GridPosition.Y * TileSize, GridPosition.Z * TileSize);
	
	// 회전 계산
	FRotator Rotation = FRotator::ZeroRotator;
	if (ExecutionContext.ModelData)
	{
		const FTileRotationInfo* RotationInfo = ExecutionContext.ModelData->GetTileRotationInfo(Cell.CollapsedTileInfoIndex);
		int32 RotationStep = RotationInfo ? RotationInfo->RotationStep : 0;
		Rotation = GetRotationFromRotationStep(RotationStep);
		UE_LOG(LogTemp, Log, TEXT("Applied rotation: RotationStep=%d, Rotation=%s"), RotationStep, *Rotation.ToString());
	}

	// 트랜스폼 설정 (위치, 회전, 스케일)
	FTransform Transform(Rotation, Location, FVector::OneVector);
	MeshComponent->SetWorldTransform(Transform);
	UE_LOG(LogTemp, Log, TEXT("Set world transform - Location: %s, Rotation: %s"), *Location.ToString(), *Rotation.ToString());

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

	UE_LOG(LogTemp, Log, TEXT("Created mesh component at position (%d, %d, %d) with mesh: %s"), 
		GridPosition.X, GridPosition.Y, GridPosition.Z, 
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

