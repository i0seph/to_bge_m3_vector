# 문자열을 pgvector 확장 모듈의 vector 자료형으로 바꾸는 PostgreSQL용 함수
사용한 임베딩 모델은 bge-m3 이고, unix domain socket을 이용해서 임베딩 처리를 하는 함수에 요청하고, 그 결과를 받아서 vector으로 바꾸는 일을 함

# 설치 방법
* pg_config 명령을 앞에 디렉터리명 지정 없이 실행했을 때 실행 가능한 환경
* PostgreSQL 확장 모듈의 소스를 구해서 빌드할 수 있는 환경

```sh
make all
make install
```

# 참고
확장 모듈 형태로 제공됨으로 아래 SQL 명령어로 설치해서 사용하면 됨 
(make install 작업을 했음에도 해당 모듈이 없다고 나오면 DB 서버 재실행)
```SQL
CREATE EXTENSION tobgem3_client
```

# 사용법
```SQL
postgres=# \dt+ news_chunks
                                테이블 목록
 스키마 |    이름     |  형태  | 소유주 | 지속성 | 접근 방법 | 크기  | 설명
--------+-------------+--------+--------+--------+-----------+-------+------
 public | news_chunks | 테이블 | ioseph | 영구   | heap      | 19 GB |
(1개 행)

postgres=# \d news_chunks
                                 "public.news_chunks" 테이블
    필드명     |     형태     | 정렬규칙 | NULL허용 |                 초기값
---------------+--------------+----------+----------+-----------------------------------------
 id            | integer      |          | not null | nextval('news_chunks_id_seq'::regclass)
 article_id    | integer      |          |          |
 chunk_content | text         |          | not null |
 chunk_index   | integer      |          |          |
 embedding     | vector(1024) |          |          |
인덱스들:
    "news_chunks_pkey" PRIMARY KEY, btree (id)
    "idx_news_chunks_article_id" btree (article_id)
    "news_chunks_embedding_idx" hnsw (embedding vector_cosine_ops) WITH (m='16', ef_construction='64')

postgres=# \di+ news_chunks_embedding_idx
                                              인덱스 목록
 스키마 |           이름            |  형태  | 소유주 |   테이블    | 지속성 | 접근 방법 | 크기  | 설명
--------+---------------------------+--------+--------+-------------+--------+-----------+-------+------
 public | news_chunks_embedding_idx | 인덱스 | ioseph | news_chunks | 영구   | hnsw      | 24 GB |

postgres=# select count(*) from news_chunks;
  count
---------
 3163508
(1개 행)

postgres=# \timing
작업수행시간 보임
postgres=# select id from news_chunks order by embedding <=>  tobgem3vector('동해물과 백두산이 마르고 닳도록') limit 5;
   id
---------
  344185
 2455010
 2410622
  341961
 2799403
(5개 행)

작업시간: 68.539 ms

postgres=# explain (analyze, buffers)select id from news_chunks order by embedding <=>  tobgem3vector('동해물과 백두산이 마르고 닳도록') limit 5;
 Limit  (cost=3536.42..3610.36 rows=10 width=12) (actual time=0.836..0.889 rows=10.00 loops=1)
   Buffers: shared hit=1116
   ->  Index Scan using news_chunks_embedding_idx on news_chunks  (cost=3536.42..23417699.78 rows=3166389 width=12) (actual time=0.836..0.887 rows=10.00 loops=1)
         Order By: (embedding <=> '[0.026855469,0.06286621,-0.07348633,-0.06854248,-0.03665161,...중간생략...,-0.011833191,0.034179688,0]'::vector)
         Index Searches: 1
         Buffers: shared hit=1116
 Planning:
   Buffers: shared hit=1
 Planning Time: 70.841 ms
 Execution Time: 0.903 ms
(10개 행)

작업시간: 72.167 ms
```

# 제약사항
완벽한 학습용 코드임으로 운영환경에서 알아서 잘 고쳐서 사용하시길


