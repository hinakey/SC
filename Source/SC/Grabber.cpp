// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Grabber.h"

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UGrabber::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Grabber Ready"));

	FindPhysicsHandle();
	SetupInputComponent();
}

//입력장치 있는지 확인하고 press/release 세팅
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();	//입력장치가 있는지 find

	if (InputComponent)//크래쉬를 방지해줄 안전장치
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);	//입력장치가 있으면 플젝세팅->입력에서 만들어준 키매핑을 bindAction 해준다
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
}

// Physics Handle이 있는지 확인
void UGrabber::FindPhysicsHandle()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();	//initialize physicsHandle pointer

	if (!PhysicsHandle)	//크래쉬를 방지해줄 안전장치
	{
		UE_LOG(LogTemp, Error, TEXT("%s has no PhysicsHandleComponent in it"), *GetOwner()->GetName());
	}
}

//바인딩한 키를 누르면 해당 함수를 콜 해 InputComponent에서 사용 
void UGrabber::Grab()
{
	FHitResult HitResult = GetFirstPhysicsBodyinReach();
	UPrimitiveComponent* ComponentToGrab = HitResult.GetComponent();
	AActor* ActorHit = HitResult.GetActor();

	if (ActorHit)
	{
		if (!PhysicsHandle)
		{
			return;
		}
		PhysicsHandle->GrabComponentAtLocation
		(
			ComponentToGrab,
			NAME_None,
			GetPlayerReach()
		);
	}
}

void UGrabber::Release()
{
	if (!PhysicsHandle)
	{
		return;
	}
	if (PhysicsHandle->GrabbedComponent)
	{
		//PhysicsHandle->SetTargetLocation(LineTraceEnd);
		PhysicsHandle->ReleaseComponent();
	}
}

void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle)
	{
		return;
	}
	if (PhysicsHandle->GrabbedComponent)
	{
		PhysicsHandle->SetTargetLocation(GetPlayerReach());
	}
}

//Grab 입력이 발동하면...
//플레이어 뷰포인트에서 LineTraceEnd(현재 Location+현재 Rotation * 뻗는 거리)를 찾고(FVector)
//LineTraceSingleByObject()로 FResultHit를 Out한다
//FResultHit를 찾으면 Get.Actor()->GetName()으로 Log결과를 확인한다
FHitResult UGrabber::GetFirstPhysicsBodyinReach() const
{
	//FVector LineTraceEnd = GetPlayerReach();

	FHitResult Hit;	//함수가 최종적으로 반환하는 값(ray-Tracing에 hit한 물체)

	FCollisionQueryParams TraceParams(FName(TEXT("")), false, GetOwner());

	//DrawDebugLine 디버그용. 실제로 뭔가 동작하진 않음(백업)
	//FVector LineTraceEnd = PlayerViewPointLocation + FVector(0.f, 100.f, 0.f);
	// DrawDebugLine(
	// 	GetWorld(),
	// 	PlayerViewPointLocation,
	// 	LineTraceEnd,
	// 	FColor(0,255,255),
	// 	false,
	// 	0.0f,
	// 	0,
	// 	5.0f
	// );

	//실제로 동작하는 Ray-Tracing
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		GetPlayerWorldPos(),
		GetPlayerReach(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParams
	);

	//FHitResult Hit의 이름정보를 보기 위한 포인터일뿐 이걸 가지고 뭘 하진 않음. 실질적인 반환값이 아님
	AActor* ActorHit = Hit.GetActor();
	if (ActorHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace has hit %s"), *ActorHit->GetName());
	}

	return Hit; //FHitResult 반환
}

//Getters

//현재 플레이어의 월드 로케이션을 반환하는 함수
FVector UGrabber::GetPlayerWorldPos() const
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return PlayerViewPointLocation;
}

//현재 플레이어의 FVector lineEnd를 반환하는 함수
FVector UGrabber::GetPlayerReach() const
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;

	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return PlayerViewPointLocation + PlayerViewPointRotation.Vector() * Reach;
}
