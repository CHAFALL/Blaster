

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible

protected:
	virtual void BeginPlay() override;
	void SetHUDTime();

	/**
	* Sync time between client and server
	*/
	// 각각의 시간이 다름을 알 수 있음....
	// 로딩시간을 서버로 부터 GetServerTime()으로 불러와 서버 시간을 얻을 수 있음
	// 다만 아직도 부정확도가 있음, (rpc할 때 걸리는 시간.) -> 서버에서 클라로 rpc를 할 때 클라에서 서버로 rpc하는데 걸린 시간을 계산해서 보내줌.
	// 데이터가 네트워크로 이동하는 데 걸리는 시간을 고려하면 서버pc가 클라에 도달하는 데 걸리는 시간과 클라의 pc가 서버에 도달하는 시간을 비교 가능
	// 이 두 가지를 더하면 왕복 시간이 나옴. (RTT) , RTT 시간을 절반으로 줄임으로써 이를 추정가능. (비대칭 왕복 시간은 우얌?) (클라가 요청보낸 시간을 통해 계산.)

	// Requests the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClentRequest, float TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
	uint32 CountdownInt = 0;


	
};
