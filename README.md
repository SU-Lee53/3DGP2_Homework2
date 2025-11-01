# 3DGP2_Homework2
3D Game Programming Homework 2 from TUKorea Game Engineering

- 요구사항

    - 다음 내용을 구현하라.

        1. 시작화면 및 메뉴 구현(텍스쳐 등을 사용)

        2. 헬리콥터/탱크/자동차/건물 등의 모델과 지형을 사용하여 게임 세상을 구축한다.

            모델의 법선 맵을 사용하여 조명 계산을 한다. 

            플레이어는 헬리콥터/탱크/자동차 중 하나이어야 하며 1인칭 플레이어를 OM 단계의 블렌딩으로 구현한다.

            지형에는 물, 나무, 풀 등을 다양하게 표현한다(빌보드 사각형은 기하 쉐이더를 사용하여 구현한다).

            건물에는 거울처럼 반사를 하는 유리창(또는 벽면)이 있도록 구현한다.

        3. 게임의 내용에 맞게 게임의 상태, 점수, 적의 위치 등을 UI(User Interface)로 구현한다.

        4. 충돌검사, 절두체 컬링 등을 구현한다. 

        5. 적절한 폭발 효과(Effect) 등을 구현한다.

    - 언급하지 않은 나머지 내용은 각자가 알아서 정하여 구현하고 설명 문서에 잘 서술한다.

- Requirements

    - Implement the start screen and menu (using textures, etc.).

        1. Build a game world using models such as helicopters, tanks, cars, buildings, and terrain.

        2. Use normal maps on models for lighting calculations.

            The player must be one of the following: helicopter, tank, or car.

            Implement the first-person player view using OM (Output Merger) stage blending.

            The terrain should feature various elements such as water, trees, and grass (implement billboard quads using a geometry shader).

            Buildings should include mirror-like reflective windows or walls.

        3. Implement a UI (User Interface) that displays the game’s state, score, and enemy positions according to the gameplay context.

        4. Implement collision detection and frustum culling.

        5. Implement appropriate visual effects, such as explosions.

    - Any other details not explicitly mentioned should be defined and implemented as you see fit, and described clearly in the accompanying documentation.