# to_bge_m3_vector
Embedding extension implemented using the BAAI/bge-m3 model for PostgreSQL

# 전체 구성

<img src="tobgem3-diagram.svg" width="800" height="600">

# 사용법
1. bge-m3 임베딩 처리를 할 서버 실행 (server 폴더에 있음)
2. pg_extension 폴더에 있는 확장 모듈 빌드 및 설치
3. DB 서버에서 CREATE EXTENSION tobgem3_client

# 사용예
```
postgres=# select tobgem3vector('나는 너를 사랑한다.') <=> tobgem3vector('내가 너를 사랑한다.');
       ?column?
-----------------------
 0.0074454112230664116
(1개 행)

작업시간: 72.279 ms
postgres=# select tobgem3vector('나는 너를 사랑한다.') <=> tobgem3vector('나는 사과를 사랑한다.');
      ?column?
---------------------
 0.28823513104475296
(1개 행)

작업시간: 71.335 ms
```

# 주의사항
* 이 코드와 예제는 학습 차원에서 100% 구글 AI 모드에서 생성된 코드임을 밝힙니다. 
