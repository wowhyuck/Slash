# 언리얼 엔진5 포트폴리오
언리얼 엔진5를 이용한 3인칭 액션RPG(게임명: SLASH) 구현


## 목차
- 프로젝트와 게임 소개
- 주요 기능 구현 설명
- 마무리


## 프로젝트와 게임 소개
- **소개**

    언리얼 엔진5를 이용해 3인칭 액션RPG를 C++과 블루프린트로 구현했습니다.

    기본적으로 캐릭터와 아이템을 구현했으며, 추가로 액션RPG 요소인 전투 시스템과 퀘스트 시스템을 넣었습니다.

    아래 설명은 캐릭터 설계와 전투 시스템입니다.

- **게임 플레이 영상**

  /* 게임 영상 링크 넣기 */


## 주요 기능 구현 설명
- **캐릭터 설계**
     
    ![캐릭터 클래스 상속](https://github.com/wowhyuck/Slash/blob/main/github_assets/01_%EC%BA%90%EB%A6%AD%ED%84%B0%20%ED%81%B4%EB%9E%98%EC%8A%A4%20%EC%83%81%EC%86%8D.png)
    - BaseCharacter Class
      - 캐릭터가 공통적으로 갖고 있는 함수와 변수 (Ex. Attack(), Die(), Montage와 관련된 함수와 변수)
      - IHitInterface 상속 받음
      - 캐릭터의 체력과 최대 체력을 갖고 있는 AttributeComponent 생성
        
    - SlashCharacter Class
      - 플레이어블 캐릭터가 갖고 있는 함수와 변수 (Ex. 캐릭터 이동, 카메라)
      - IPickupInterface 상속 받음
      - SlashAnimInstance와 연결
        
    - Enemy Class
      - AIController와 관련된 함수
      - Enemy의 체력관련 Widget을 갖고 있는 HealthBarComponent 생성

    - Boss Class
      - Enemy Class의 함수를 Boss Class에 맞게 재정의
      - Boss의 체력관련 Widget을 갖고 있는 WBP_BossHealthBar 사용

- **전투 시스템**
1. 공격 & 연속 공격
   - 구상
       1. 공격: 무기와 Overlap될 때, ExecuteGetHit 함수 -> HitReact Montage 재생 / ApplyDamage 함수 -> 피격 캐릭터 Health 감소 
       1. 연속 공격: 공격 후 타이머 시간 내에 공격을 하면, 다음 공격
   - 공격

   ```cpp
    // Weapon.cpp
    void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
    {
        ...
    
    	if (BoxHit.GetActor())
    	{
                ...
    
                // ApplyDamage 실행 시, 피격 캐릭터는 TakeDamage 호출해서 Damage를 Health에 적용
                UGameplayStatics::ApplyDamage(
                    BoxHit.GetActor(),
                    Damage,
                    GetInstigator()->GetController(),
                    this,
                    UDamageType::StaticClass());
    
                // ExecuteGetHit 실핼 시, 피격 캐릭터는 GetHit_Implementation 호출해서 HitReact Montage 재생
                ExecuteGetHit(BoxHit);
    
                ...
    	}
    }
    ```

   - 연속 공격
    ```cpp
    // SlashCharacter.cpp
    int32 ASlashCharacter::PlayAttackMontage()
    {
        // AttackMontageSections의 CurrentCombo번째 AttackMontage 재생
        PlayMontageSection(AttackMontage, AttackMontageSections[CurrentCombo]);
        return CurrentCombo;
    }
    ```
    ```cpp
    // SlashCharacter.cpp
    void ASlashCharacter::AttackEnd()
    {
        ActionState = EActionState::EAS_Unoccupied;
    
        // 공격이 끝나면, ResetComboTime까지 콤보 가능
        SetResetComboTimer();
    	
        // CurrentCombo == MaxCombo일 때, CurrentCombo를 0으로 초기화
        // CurrentCombo != MaxCombo일 때, CurrentCombo에 1 더하기
        CurrentCombo = (CurrentCombo == MaxCombo) ? 0 : FMath::Clamp(CurrentCombo + 1, 0, MaxCombo);
    }
    ```
1. 피격 방향에 따라 HitReact
    - 구상: 캐릭터 Forward Vector 기준으로 피격 지점 각도에 따라 HitReact Montage 재생
    ```cpp
    // BaseCharacter.cpp
    FVector ABaseCharacter::GetTranslationWarpTarget()
    {
        if (CombatTarget == nullptr) return FVector();
	
        const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
        const FVector Location = GetActorLocation();

        // CombatTarget->캐릭터 방향의 거리
        FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
        TargetToMe *= WarpTargetDistance;

        return CombatTargetLocation + TargetToMe;
    }
    ```
    ```cpp
    // BaseCharacter.cpp
    void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint)
    {
        double Theta = GetThetaImpactPoint(ImpactPoint);
    
        // 피격 각도에 따라 HitReactMontaget SectionName 설정
        FName Section("FromBack");
        if (Theta < 45.f && Theta >= -45.f)
        {
        	Section = FName("FromFront");
        }
        else if (Theta < -45.f && Theta >= -135.f)
        {
        	Section = FName("FromLeft");
        }
        else if (Theta < 135.f && Theta >= 45.f)
        {
        	Section = FName("FromRight");
        }
    
        PlayHitReactMontage(Section);
    }
    ```

1. 막기(Blocking)
    - 구상: Blocking 상태일 때 피격 각도에 따라 Blocking 성공 여부
    ```cpp
    // BaseCharacter.cpp
    bool ABaseCharacter::IsFront(const FVector& ImpactPoint)
    {
        double Theta = GetThetaImpactPoint(ImpactPoint);

        // ImpactPoint 위치가 캐릭터 앞 -> true
        if (Theta < 105.f && Theta >= -105.f)
    {
        return true;
    }

        // ImpactPoint 위치가 캐릭터 뒤 -> false
        return false;
    }
    ```
    ```cpp
    // SlashCharacter.cpp
    float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
    {
        // 적의 공격을 받을 때, 적의 무기가 캐릭터 앞에 있을 때 && 캐릭터가 막기 중일 때 -> 막기 성공
        bBlockAttack = IsFront(DamageCauser->GetActorLocation()) && ActionState == EActionState::EAS_Blocking;
	
	    // 공격 막기 성공했을 경우 -> Weapon Location = ImpactPoint 해서 bBlockAttack = true로 만들기
        if (bBlockAttack)
        {
            ...

            // True -> 적의 공격을 패링했을 때, 패링 관련 함수 실행
            // False -> 적의 공격을 막았을 때, 막기 관련 함수 실행
            bCanParry ? ParryingSuccess(DamageCauser) : BlockingSuccess(DamageCauser);

            // 막았을 때 DamageBlocked(현재 데미지 없음) 적용
            HandleDamage(DamageBlocked);

            ...
        }
    }

    ```
1. 패링(Parrying)
    - 구상: Blocking Animation 앞부분 패링 성공 Notify 두고 Notify 전후로 패링 성공 여부
    - 
1. 반격
    - 구상: 막기/패링 성공 후 타이머 시간 내에 공격
    - 
1. 적의 시야, 소리 감지
    - 구상: PawnSensingComponent / AIPerceptionComponent를 활용하여 적의 시야 / 소리 감지
    - 
1. 적의 공격 모션 워핑(Motion Warping)
    - 구상: 적의 Attack Animation 중 무기의 Collision이 활성화 되기 전에 플레이어를 바라보는 Motion Warping 넣기
    - 

## 마무리
