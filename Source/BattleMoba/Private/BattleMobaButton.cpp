// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleMobaButton.h"
#include "Engine.h"
#include "BattleMobaCharacter.h"

UBattleMobaButton::UBattleMobaButton()
{
	OnPressed.AddDynamic(this, &UBattleMobaButton::OnPress);

	//Bind function
	//load.AddDynamic(this, &UBattleMobaButton::LoadSkillFile);
}

void UBattleMobaButton::OnPress()
{
	//load.Broadcast(this->GetName());
	LoadSkillFile(this->GetName());
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Blue, FString::Printf(TEXT("%s"), *this->GetName()));
}

//void UBattleMobaButton::LoadSkillFile(const FString &name)
//{
//	/*ABattleMobaCharacter* pawn = Cast<ABattleMobaCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
//	if (pawn)
//	{
//		pawn->GetButtonSkillAction(FKey(), name);
//	}*/
//}
