# 준비

python venv 환경에서

```sh
pip install sentence-transformers torch numpy huggingface-hub
```

# download bge-m3 모델

```python
from huggingface_hub import snapshot_download

# 모델을 저장할 로컬 경로 지정
local_dir = "../my_bge_m3_model"

# 다운로드 실행 (필요한 파일만 깔끔하게 가져옵니다)
snapshot_download(
    repo_id="BAAI/bge-m3",
    local_dir=local_dir,
    local_dir_use_symlinks=False  # 실제 파일을 물리적으로 복사함
)

print(f"모델이 {local_dir} 폴더에 저장되었습니다!")
```

# 실행

```sh
nohup python -u server.py > server.log 2>&1 &
tail -f server.log
```
