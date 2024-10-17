


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubSystem.h"
#include "GameFramework/GameModeBase.h"
#include "Blaster/Character/MyBlasterCharacter.h"

void UReturnToMainMenu::MenuSetup()
{
	// 이렇게 해야 메뉴를 볼 수 있음.
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ?  World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			// 현재 UI 위젯(UReturnToMainMenu 클래스)이 Slate 위젯으로 변환되었을 때, 그 위젯을 포커스할 대상으로 설정
			InputModeData.SetWidgetToFocus(TakeWidget()); // TakeWidget(): 기본 slate 위젯을 가져오거나 존재하지 않는 경우 구성함.
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (ReturnButton && !ReturnButton->OnClicked.IsBound()) // 이미 바인딩되지 않은 경우에만 바인딩 됨.
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		//
		// 세션을 삭제하면 해당 세션이 적극적?으로 삭제될 때까지 기다려야 하기 때문에 delegate를 하위 시스템에 바인딩하고 이에 응답.
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	return true;

}

// 
void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	// 실패했으면 버튼을 다시 누를 수 있도록 함.
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		// GetAuthGameMode: 현재 사용 중인 게임 모드(Game Mode)를 가져옴
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost(); // 메인메뉴로 돌아감.
		}
		else // 게임모드가 유효하지 않다면 클라에 있다는 뜻.
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				// 클라가 메인 메뉴로 돌아가기 위해 호출할 수 있는 함수.
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent(); // viewport에서 제거
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound()) 
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
	
}

void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			AMyBlasterCharacter* BlasterCharacter = Cast<AMyBlasterCharacter>(FirstPlayerController->GetPawn());
			if (BlasterCharacter)
			{
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
	
}


void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession(); // 세션이 파괴될 때까지 기다림. (파괴되면 UReturnToMainMenu::OnDestroySession 실행됨.) (그렇게 콜백을 해놨으니.)
	}
}