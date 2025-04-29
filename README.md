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

- **퀘스트 시스템**


## 마무리
