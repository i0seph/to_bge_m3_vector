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

# 제약사항
완벽한 학습용 코드임으로 운영환경에서 알아서 잘 고쳐서 사용하시길


