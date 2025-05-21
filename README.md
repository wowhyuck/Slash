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
     - AttackMontageSections 배열에 AttackMontage SectionName 등록
     - 캐릭터 생성될 때 CurrentCombo = 0 초기화
       
     ![연속 공격1](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_01_%EC%97%B0%EC%86%8D%EA%B3%B5%EA%B2%A91.png)  
     ```cpp
     // SlashCharacter.cpp
     int32 ASlashCharacter::PlayAttackMontage()
     {
         // AttackMontageSections의 CurrentCombo번째 AttackMontage 재생
         PlayMontageSection(AttackMontage, AttackMontageSections[CurrentCombo]);
         return CurrentCombo;
     }
     ```
     - Attack Animation 끝날 때(AttackEnd), ResetComboTimer 작동
       - ResetComboTime 안에 공격: CurrentCombo + 1 -> 다음 Section Attack Animation 재생
       - ResetcomboTime 뒤에 공격 || CurrentCombo가 마지막 공격: CurrentCombo = 0 -> AttackMontageSections[0] Animation 재생
         
     ![연속 공격2](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_01_%EC%97%B0%EC%86%8D%EA%B3%B5%EA%B2%A92.png)
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
     - 결과 ( 공격 / 연속공격 )
     
     |![공격](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/01_%EA%B3%B5%EA%B2%A9.gif)|![연속공격](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/01_%EC%97%B0%EC%86%8D%EA%B3%B5%EA%B2%A9.gif)|
     |---|---|
   
1. 피격 방향에 따라 HitReact
    - 구상: 캐릭터 Forward Vector 기준으로 ImpactPoint 지점 각도에 따라 HitReact Montage 재생
      - 캐릭터 Forward Vector와 캐릭터->ImpactPoint Vector와의 각도(Theata) 구하기
        
      ![HitReact](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_02_HitReact.png)
      ```cpp
      // BaseCharacter.cpp
      double ABaseCharacter::GetThetaImpactPoint(const FVector& ImpactPoint)
      {
          const FVector Forward = GetActorForwardVector();

          // ImpactPoint와 캐릭터 Location.Z 일치
          // 캐릭터->ImpactPoint의 유닛벡터 구하기
          const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
          const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

          // Forward * ToHit = |Forward||ToHit| * cos(theta)
          // |Forward| = 1, |ToHit| = 1 -> Forward * ToHit = cos(theta)
          const double CosTheta = FVector::DotProduct(Forward, ToHit);

          // Theta 구하기
          // theta = acos(cos(theta))
          double Theta = FMath::Acos(CosTheta);

          // Radian을 degree로 변환
          Theta = FMath::RadiansToDegrees(Theta);

          // CrossProduct.Z가 -일 경우, ImpactPoint가 캐릭터 왼쪽 위치 (언리얼에서 외적은 왼손법칙)
          const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
          if (CrossProduct.Z < 0)
          {
              Theta *= -1.f;
          }

          return Theta;
      }
      ```
   - Theta에 따라 HitReactMontage SectionName 설정 및 재생
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
      - 결과 ( Front / Back / Left / Right )
    
      |![Front](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/02_HitReact_Front.gif)|![Back](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/02_HitReact_Back.gif)|
      |---|---|
      |![Left](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/02_HitReact_Left.gif)|![Right](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/02_HitReact_Right.gif)|

1. 막기(Blocking)
    - 구상: Blocking 상태일 때 피격 각도에 따라 Blocking 성공 여부
      - 막기 가능(-105 ~ 105) 범위 설정
        
      ![막기](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_03_%EB%A7%89%EA%B8%B0.png)
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
      - 캐릭터가 막기 중 && 적의 공격 위치가 막기 범위 안 -> 막기 성공   
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
      - ParryEnd Notify 기준
        - ParryEnd 전 막기 성공 -> 패링
        - ParryEnd 후 막기 성공 -> 기본 막기
          
      ![패링](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_04_%ED%8C%A8%EB%A7%81.png)
      ```cpp
      // SlashCharacter.cpp
      void ASlashCharacter::Block()
      {
          if (IsOccupied() || IsFalling() || !HasEnoughStamina(StartBlockCost) || CharacterState == ECharacterState::ECS_Unequipped) return;

          // 막기 시작했을 때 bCanParry = true 설정
          bCanParry = true;

          ...
      }
      ```

      ```cpp
      // SlashCharacter.cpp
      void ASlashCharacter::ParryEnd()
      {
          // ParryEnd Notify가 호출될 때 bCanParry = false 설정
          bCanParry = false;
      }
      ```
      - 패링 성공했을 때 적의 Attack Animation 중지
      ```cpp
      // SlashCharacter.cpp
      void ASlashCharacter::ParryingSuccess(AActor* EnemyWeapon)
      {
          ...
      
          // 적이 패링 당했을 때 Attack Animation 중지시키기
          IHitInterface* HitInterface = Cast<IHitInterface>(EnemyWeapon->GetOwner());
          if (HitInterface)
          {
              HitInterface->Execute_GetHit(EnemyWeapon->GetOwner(), EnemyWeapon->GetActorLocation(), this);
          }
      }
      ```
      - 결과 ( 막기 / 패링 )
      
      |![막기](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/03_%EB%A7%89%EA%B8%B0.gif)|![패링](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/03_%ED%8C%A8%EB%A7%81.gif)|
      |---|---|

1. 반격
    - 구상: 막기/패링 성공 후 타이머 시간 내에 공격
      - 적의 공격을 막았을 때 -> bCanCounter = true 설정 및 CanCounterTimer 작동
        
      ![반격](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_05_%EB%B0%98%EA%B2%A9.png)
      ```cpp
      // SlashCharacter.cpp
      float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
      {
          ...

          // 적의 공격을 막기 성공했을 경우
          if (bBlockAttack)
          {
              // 반격 가능 여부 변수인 bCanCounter = true 설정
              // CanCounterTimer 지나면 bCanCounter = false 설정
              bCanCounter = true;
              SetCanCounterTimer();
          }
      
          ...

      }
      ```
      - bCanCounter -> True: 반격 / False: 일반 공격
      ```cpp
      // SlashCharacter.cpp
      void ASlashCharacter::Attack()
      {
          ...

          if (CanAttack())
          {
              // True -> 반격
              // False -> 일반 공격
              bCanCounter ? Counter() : PlayAttackMontage();
          }
      }
      ```
      - 결과

      |![막기반격](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/05_%EB%A7%89%EA%B8%B0%EB%B0%98%EA%B2%A9.gif)|![패링반격](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/05_%ED%8C%A8%EB%A7%81%EB%B0%98%EA%B2%A9.gif)|
      |---|---|
    
1. 적의 시야, 소리 감지
    - 구상: PawnSensingComponent / AIPerceptionComponent를 활용하여 적의 시야 / 소리 감지
      - PawnSensingComponent / AIPerceptionComponent 생성 및 초기화
      ```cpp
      // Enemy.cpp
      AEnemy::AEnemy()
      {
          ...

          // PawnSensing Component(시야 감지) 생성 및 초기화
          PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
          PawnSensing->SightRadius = 4000.f;
          PawnSensing->SetPeripheralVisionAngle(45.f);

          // AIPerception Component(소리 감지) 생성 및 초기화
          AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
          HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
          AIPerception->ConfigureSense(*HearingConfig);
      }
      ```
      
      ![적시야소리](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_06_%EC%A0%81%EC%8B%9C%EC%95%BC%EC%86%8C%EB%A6%AC.png)
      - PawnSensing / AIPerception 호출 함수 등록
        - PawnSeing->OnSeePawn에 AEnemy::PawnSeen 함수 등록
        - AIPerception->OnTargetPerceptionUpdated에 AEnemy::SeneNoise 함수 등록
      ```cpp
      // Enemy.cpp
      void AEnemy::BeginPlay()
      {
          ...

          // 시야, 소리 감지할 때 호출할 함수 등록
          // PawnSensing->OnSeePawn / AIPerception->OnTargetPerceptionUpdated
          if (PawnSensing)
          {
              PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
          }
          if (AIPerception)
          {
              AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemy::SenseNoise);
          }
      }
      ```
      - PawnSeen 함수
      ```cpp
      // Enemy.cpp
      void AEnemy::PawnSeen(APawn* SeenPawn)
      {
          bSeeTarget = true;	// Pawn 볼 때, true로 설정

          // SeenPawn이 적의 시야각 범위 밖에 있으면 bSeeTarget = false로 초기화
          GetWorldTimerManager().SetTimer(SeeTargetTimer, this, &AEnemy::NotSeeTarget, 0.5);

          // 감지된 Pawn이 플레이어고 Enemy가 공격 시작할 수 있는지
          const bool bShouldChaseTarget =
              EnemyState != EEnemyState::EES_Dead &&
              EnemyState != EEnemyState::EES_Chasing &&
              EnemyState < EEnemyState::EES_Attacking&&
              SeenPawn->ActorHasTag(FName("EngageableTarget"));

          // 적이 공격 가능하면 Patrol 중지하고,
          // SeenPawn을 CobatTarget으로 세팅하고 추격하기
          if (bShouldChaseTarget)
          {
              CombatTarget = SeenPawn;
              ClearPatrolTimer();
              ChaseTarget();
          }
      }
      ```

      - SenseNoise 함수
      ```cpp
      // Enemy.cpp
      void AEnemy::SenseNoise(AActor* NoiseActor, FAIStimulus Stimulus)
      {
          ...

          // 소리 감지 시, 소리나는 지점으로 정찰
          EnemyState = EEnemyState::EES_Searching;
          LocationSearched = NoiseActor->GetActorLocation();
      }
      ```
      - 결과 ( 시야 감지 / 소리 감지 )

      |![시야](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/06_%EC%8B%9C%EC%95%BC.gif)|![소리](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/06_%EC%86%8C%EB%A6%AC.gif)|
      |---|---|
      
1. 적의 공격 모션 워핑(Motion Warping)
    - 구상: 적의 Attack Montage에 무기의 Collision이 활성화 되기 전에 플레이어를 바라보는 Motion Warping 넣기
      
      ![모션워핑1](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_07_%EB%AA%A8%EC%85%98%EC%9B%8C%ED%95%911.png)
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

          // 블루프린트에서 Add or Update Warp Target from Location 함수의 Target Location 값을 넣기 위해 
          return CombatTargetLocation + TargetToMe;
      }
      ```
      ```cpp
      // BaseCharacter.cpp
      FVector ABaseCharacter::GetRotationWarpTarget()
      {
          if (CombatTarget)
          {
              // 블루프린트에서 Add or Update Warp Target from Location 함수의 Target Location 값을 넣기 위해
              return CombatTarget->GetActorLocation();
          }

          return FVector();
      }
      ```
      - Notify에 GetWarpTarget 함수 연결
        
      ![모션워핑2](https://github.com/wowhyuck/Slash/blob/main/github_assets/02_07_%EB%AA%A8%EC%85%98%EC%9B%8C%ED%95%912.png)
      - 결과

      ![모션워핑](https://github.com/wowhyuck/Slash/blob/main/github_assets/gif/07_%EB%AA%A8%EC%85%98%EC%9B%8C%ED%95%91.gif)

## 마무리
이상으로 3인칭 액션RPG을 언리얼 엔진5.0버전으로 C++과 블루프린트 활용해서 만들었습니다.

이 프로젝트의 핵심 구현이 캐릭터와 전투 시스템이여서 이 둘을 위주로 설명했습니다.

그 외, 설명하지 않은 아이템과 HUD 등 구현 기능들은 소스 파일을 통해 충분히 이해할 수 있을거라 생각합니다.  

다만, 록온 시스템과 퀘스트 시스템은 블루프린트, 플러그인으로 통해 구현됐고, 적의 공격 패턴은 C++로 단순 구현이 된 부분도 있습니다.

다음 프로젝트를 할 때, 이 부족한 부분들을 C++를 통해 시스템 구축하고 적의 공격을 더 복잡한 패턴으로 만듦으로써 개선하려고 합니다.
