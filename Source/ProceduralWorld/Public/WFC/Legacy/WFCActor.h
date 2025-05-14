// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WFC/Legacy/WFC3DModel.h"
#include "GameFramework/Actor.h"
#include "WFCActor.generated.h"

/*
 * Cell 구조체
 * 자체적으로 메시를 생성하지는 않고, 메시에 대한 데이터만 가지고 있음
 * bool IsCollapsed 붕괴여부
 * FBaseTileInfo CollapsedTileInfo 붕괴 시 어떤 타일로 붕괴했는지 알려주는 변수
 * int32 Entropy 엔트로피 수(붕괴 시 1)
 * TArray<int32> PropagatedFaces 어떤 면에서 해당 면으로 전파 했는지 알려주는 배열
 * TBitArray<> RemainingTileOptionsBitset 현재 셀에 들어갈 수 있는 타일 정보 비트셋
 * TArray<TBitArray<>> MergedFaceOptionsBitset 각 타일 옵션의 6개의 면에 대한 병합 비트셋 배열
 */

USTRUCT(BlueprintType)
struct FCell3D
{
	GENERATED_BODY()

	FCell3D() = default;

	void Init(int32 TileInfoNum, int32 FaceInfoNum, const int32& Index, const FIntVector& Dimension);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsCollapsed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bIsPropagated = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Location;

	FBaseTileInfo* CollapsedTileInfo = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 Entropy = 0;

	// Propagated Faces에서 어느 면이 전파 받았는지 확인
	// 전파 받은 면 방향에서 해당 면으로 MergedFaceOptionBitset을 받아옴
	// FaceBitset이기 때문에 받아온 면에 대해서 해당 Bitset 인덱스를 전부 가져옴
	// 가져온 인덱스를 통해서 해당 면들이 가질 수 있는 모든 타일을 구할 수 있음
	// 이것에 대해서는 모두 OR 연산
	// 다른 면에서도 동일한 작업을 하면 타일비트셋이 전파 받은 면의 개수만큼 있는데 이것들을 And연산
	// 그렇게 해서 나온 결과를 RemainingTileOptionsBitset에 넣음
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D", EditFixedSize)
	uint8 PropagatedFaces = 0;

	TBitArray<> RemainingTileOptionsBitset = TBitArray<>();

	// RemainingTileOptionBitset에서 받아온 모든 타일 옵션에 대해서 각 면에 대한 비트셋을 OR 연산해서 해당 변수에 넣음
	TArray<TBitArray<>> MergedFaceOptionsBitset = {TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>(), TBitArray<>()};

	static FORCEINLINE FIntVector&& IndexToLocation(const int32& Index, const FIntVector& Dimension);
	bool GetPropagatedFaces(const EFace& Direction) const;
	void SetPropagatedFaces(const EFace& Direction);
};

/*
 * Grid 구조체
 * 셀 그리드
 * TArray<FCell3D> Grid 셀 배열
 * FIntVector Dimension 차원
 * Operator() 연산자 오버로딩
 * const Operator() const 연산자 오버로딩
 *
 * 실질적으로 여기서 Process를 수행
 *
 *	## 붕괴 과정
 *  	1. 어떤 한 셀이 붕괴
 *   		1. 가중치 랜덤을 사용해서 붕괴, 누적합 사용
 * 		2. 총 조각 수 n에 따라서 O(n)의 시간 복잡도를 가짐, 가능한 모든 조각을 모으는 행위 O(n)
 *  			조각의 가중치를 모두 합해서 누적 확률로 만드는 행위 O(n), 누적 확률에 기반해서 조각을 뽑는 행위 O(logN)
 *   	2. 붕괴 후 해당 셀과 인접한 셀로 전파
 *   		1. 각 전파는 자기 셀의 UBRLFD로 전파 큐에 삽입, 만약 이미 붕괴한 셀이면 삽입 하지 않음
 *   	3. 전파 받은 셀에서 가능한 각 면의 경우의 수를 재계산 후 만약 각 면에 대한 비트셋에 변화가 있다면 큐에 넣기, 최대 모든 셀의 개수만큼 반복
 *   		1. 각 면에 대한 비트셋 계산은 비트셋의 &, | 연산으로 계산
 *  			1. 먼저 들어온 면이 어딘지 파악한다, 들어온 면은 여러 개 일 수 있다.
 *  			2. 들어온 각 면 내에서는 | 연산, 각 면 끼리는 & 연산을 통해서, 가능한 모든 조각을 설정한다.
 *  			3. 현재 셀에 가능한 모든 조각 옵션이 설정 되면, 각 조각 옵션에 대해서 가능한 모든 면을 비트셋으로 다시 설정
 *  				1. 비트셋→인덱스로 변환
 *  				2. 인덱스를 넣으면 해당 조각 옵션에 대한 비트셋 6개를 가져옴
 *  					1. 이거도 비트셋으로 바꿔서 한번 더 할 수 있나?
 *  					2. 예를 들면 UBR이 이미 설정 되었다면 매개변수로 ‘101100’을 같이 보내면 비트셋 3개만 가져오게 한다던가
 *  						이건 그렇게 하면 총 63개의 비트셋 배열을 만들어 놔야함
 *  						1. 원래는 6개를 한번에 가져와서 원하는 것만 썼다면 이렇게 하면 그냥 한번에 다 할 수 있다 근데 메모리가 괜찮을까
 *  							63배의 메모리를 사용하게 되는데 시간은 얼마나 줄어들 지 모르겠음 
 *  						2. 하지 않는게 좋음 어차피 110110이런 비트셋을 만드는데 같은 시간을 씀?
 *  						3. 애초에 전파 받을 때 비트셋에 추가를 처음부터 받는다면
 *  							그래도 O(1)씩 들긴 하는데 그걸 결국 받은 횟수만큼 반복 할거고 근데 그래도 결국 비슷한 시간 만큼이 소요됨
 *  				3. 가져온 6개의 비트셋 중 들어온 면을 제외한 다른 모든 면의 비트셋을 각각 면에 해당하게 합침
 *  					합친 것이 재설정된 모든 조각 옵션에 대한 각 면의 경우의 수가 됨
 *  			4. 재설정된 모든 면에 대해서 기존 경우의 수와 차이점이 있다면 해당 셀을 큐에 삽입
 *  		2. 큐에 포함된 모든 셀이 전파될 때 까지 전파함, 전파 중 만약 엔트로피가 1개만 존재하는 셀이 있다면 해당 셀도 붕괴시킴 
 *  		3. 면에서 셀로, 변경된 셀에서 나머지 면으로 전파
 *  			BFS로 하기 때문에 순서는 항상 인접한 방향이 모두 업데이트 된 후 업데이트 되기 때문에 신경 안써도 됨
 *  	4. 전파가 끝나면 엔트로피의 개수가 가장 낮은 셀이 붕괴 됨
 *  	5. 위 과정을 모든 셀이 붕괴될 때 까지 반복 
 *  	6. 모든 셀이 붕괴되면, 각 셀에서 인접면에 대한 출입구 위치 고정
 *  	7. 각 셀에 대하여 하위 셀 붕괴(출입구 및 테두리가 고정되어 있기 때문에 병렬 처리 가능)
 *  	
 */


USTRUCT(BlueprintType)
struct FGrid
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	TArray<FCell3D> Grid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Dimension;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	TObjectPtr<UWFC3DModelDataAsset> WFC3DModel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 RandomSeed = 0;

private:
	int32 RemainingCells = 0;

	TQueue<FIntVector> PropagationQueue;

	FRandomStream RandomStream;

public:
	FGrid() = default;

	FGrid(const int32& X, const int32& Y, const int32& Z);

	FGrid(const FIntVector& Dimension);

	FGrid& operator=(const FGrid& Other);

	void Init(UWFC3DModelDataAsset* InWFCModel, const FIntVector& InDimension);

	bool GetCell(const FIntVector& Location, FCell3D*& OutCell);
	
	bool GetCell(const FIntVector& Location, const FCell3D*& OutCell) const;

	/**
	 * CollapseGrid
	 *	1. Random Collapse Cell by CollapseCell
	 *	2. PropagateGrid
	 *		while(!Queue.IsEmpty())
	 *		1. GetCell in Queue
	 *		2. Propagate Cell To Other Cell by PropagateCell
	 *			if Cell is Not Collapsed
	 *				Add Cell to Queue
	 */

	bool CollapseGrid();

private:
	// 셀이 붕괴 될 경우 불리는 함수, 붕괴되면 데이터만 갱신
	bool CollapseCell(const FIntVector& Location, const int32& SelectedTileInfoIndex);

	// 그리드 전파
	bool PropagateGrid();

	// 셀을 기준으로 전파하는 함수
	bool PropagateCell(const FIntVector& Location);

	// 랜덤 시드 생성
	bool CreateRandomSeed();

	// 시드 설정
	bool SetRandomSeed(int32 Seed);

	// RandomStream을 통해서 0~Max-1 사이의 랜덤한 정수를 반환
	FORCEINLINE int32 Rand0(int32 Max) const
	{
		return RandomStream.RandRange(0, Max - 1);
	}

	FORCEINLINE bool CheckLocation(FIntVector Location) const;

	// 비트셋에서 1인 모든 인덱스를 가져오는 함수
	static TArray<int32> GetAllIndexFromBitset(const TBitArray<>& Bitset);

	// UBRLFD
	static const TArray<FIntVector> PropagateDirection;
	static FCell3D OutEmptyCell;
};


UCLASS()
class PROCEDURALWORLD_API AWFCActor : public AActor
{
	GENERATED_BODY()
	
	// Debug
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bInitGrid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bCollapseGrid;

public:
	// Sets default values for this actor's properties
	AWFCActor();

	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void Init();
	
	UFUNCTION(BlueprintCallable, Category = "WFC3D")
	void Collapse();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D|Data")
	TObjectPtr<UWFC3DModelDataAsset> WFC3DModel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FGrid Grid;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bInitializeInstancedMeshes = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bCreateInstancedStaticMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	bool bCreateRandomSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 RandomSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	float TileSize = 400;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	TArray<UInstancedStaticMeshComponent*> InstancedMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	FIntVector Dimension = FIntVector(3, 3, 3);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 TryCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WFC3D")
	int32 TypeOfTiles = 25;




protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	// Random Stream
	FRandomStream TileOptionRandomStream;
};
