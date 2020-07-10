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

//�Է���ġ �ִ��� Ȯ���ϰ� press/release ����
void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();	//�Է���ġ�� �ִ��� find

	if (InputComponent)//ũ������ �������� ������ġ
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);	//�Է���ġ�� ������ ��������->�Է¿��� ������� Ű������ bindAction ���ش�
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
}

// Physics Handle�� �ִ��� Ȯ��
void UGrabber::FindPhysicsHandle()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();	//initialize physicsHandle pointer

	if (!PhysicsHandle)	//ũ������ �������� ������ġ
	{
		UE_LOG(LogTemp, Error, TEXT("%s has no PhysicsHandleComponent in it"), *GetOwner()->GetName());
	}
}

//���ε��� Ű�� ������ �ش� �Լ��� �� �� InputComponent���� ��� 
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

//Grab �Է��� �ߵ��ϸ�...
//�÷��̾� ������Ʈ���� LineTraceEnd(���� Location+���� Rotation * ���� �Ÿ�)�� ã��(FVector)
//LineTraceSingleByObject()�� FResultHit�� Out�Ѵ�
//FResultHit�� ã���� Get.Actor()->GetName()���� Log����� Ȯ���Ѵ�
FHitResult UGrabber::GetFirstPhysicsBodyinReach() const
{
	//FVector LineTraceEnd = GetPlayerReach();

	FHitResult Hit;	//�Լ��� ���������� ��ȯ�ϴ� ��(ray-Tracing�� hit�� ��ü)

	FCollisionQueryParams TraceParams(FName(TEXT("")), false, GetOwner());

	//DrawDebugLine ����׿�. ������ ���� �������� ����(���)
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

	//������ �����ϴ� Ray-Tracing
	GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		GetPlayerWorldPos(),
		GetPlayerReach(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParams
	);

	//FHitResult Hit�� �̸������� ���� ���� �������ϻ� �̰� ������ �� ���� ����. �������� ��ȯ���� �ƴ�
	AActor* ActorHit = Hit.GetActor();
	if (ActorHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("LineTrace has hit %s"), *ActorHit->GetName());
	}

	return Hit; //FHitResult ��ȯ
}

//Getters

//���� �÷��̾��� ���� �����̼��� ��ȯ�ϴ� �Լ�
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

//���� �÷��̾��� FVector lineEnd�� ��ȯ�ϴ� �Լ�
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
