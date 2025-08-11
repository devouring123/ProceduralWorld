// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "WFC/Core/WFC3DController.h"
#include "WFC/Data/WFC3DModelDataAsset.h"
#include "WFC/Data/WFC3DTypes.h"
#include "WFC/Data/WFC3DCell.h"
#include "WFC3DActor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROCEDURALWORLD_API AWFC3DActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWFC3DActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor is being destroyed
	virtual void BeginDestroy() override;

	/** WFC3D 생성 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void ExecuteWFC3D();

	/** WFC3D 비동기 생성 실행 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void ExecuteWFC3DAsync();

	/** 생성된 메시들을 검증 */
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	bool VerifyGeneratedMeshes();

	/** 특정 위치의 StaticMesh 참조 가져오기 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	UStaticMeshComponent* GetMeshComponentAt(const FIntVector& GridPosition) const;

	/** 모든 생성된 메시 컴포넌트 가져오기 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	TArray<UStaticMeshComponent*> GetAllMeshComponents() const;

	/** 생성 완료 여부 확인 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	bool IsGenerationComplete() const;

	/** 생성 진행률 가져오기 */
	UFUNCTION(BlueprintPure, Category = "WFC3D")
	float GetGenerationProgress() const;

 public:
	/** WFC3D 컨트롤러 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WFC3D")
	UWFC3DController* WFC3DController;

	/** 루트 씬 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

	/** 생성된 메시 컴포넌트들을 담을 컨테이너 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WFC3D")
	TArray<UStaticMeshComponent*> GeneratedMeshComponents;

	/** WFC3D 실행 컨텍스트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D Settings")
	FWFC3DExecutionContext ExecutionContext;

	/** 게임 시작 시 자동으로 WFC3D 실행 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D Settings")
	bool bAutoExecuteOnBeginPlay = true;

	/** 원본 메시의 기본 타일 크기 (X, Y, Z) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D Settings")
	FVector BaseTileSize = FVector(400.0f, 400.0f, 400.0f);

	/** 목표 타일 크기 (X, Y, Z) - 실제 생성될 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WFC3D Settings")
	FVector TileSize = FVector(400.0f, 400.0f, 400.0f);

	/** 생성된 그리드와 메시 컴포넌트 매핑 */
	UPROPERTY()
	TMap<FIntVector, UStaticMeshComponent*> GridToMeshMap;

private:
	/** WFC3D 컨트롤러 델리게이트 바인딩 */
	void BindControllerDelegates();

	/** WFC3D 실행 완료 콜백 */
	UFUNCTION()
	void OnWFC3DExecutionCompleted(const FWFC3DAlgorithmResult& Result);

	/** WFC3D 실행 취소 콜백 */
	UFUNCTION()
	void OnWFC3DExecutionCancelled();

	/** WFC3D 실행 진행률 콜백 */
	UFUNCTION()
	void OnWFC3DExecutionProgress(int32 CurrentStep, int32 TotalSteps);

	/** 생성된 그리드를 바탕으로 StaticMesh 컴포넌트 생성 */
	void CreateMeshComponentsFromGrid();

	/** 특정 그리드 위치에 메시 컴포넌트 생성 */
	UStaticMeshComponent* CreateMeshComponentAtPosition(const FIntVector& GridPosition, const FWFC3DCell& Cell);

	/** 생성된 메시의 유효성 검증 */
	bool ValidateMeshComponent(UStaticMeshComponent* MeshComponent) const;

	/** 회전 스텝을 FRotator로 변환 */
	FRotator GetRotationFromRotationStep(int32 RotationStep) const;

	/** BaseTileSize와 TileSize 비교로 스케일 계산 */
	FVector CalculateTileScale() const;

	/** Actor를 그리드의 바닥 중심으로 이동 */
	void PositionActorAtGridCenter();
};
