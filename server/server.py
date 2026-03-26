import os
import json
import asyncio
import struct
import torch
import time
from datetime import datetime
from sentence_transformers import SentenceTransformer
import numpy as np  # 상단에 추가

# 설정
MODEL_PATH = '../my_bge_m3_model'
SOCKET_PATH = '/tmp/bge_m3_model.sock'
if torch.backends.mps.is_available():
    DEVICE = 'mps'
else:
    DEVICE = 'cpu'

# 모델 로드
print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Loading model on {DEVICE}...")
model = SentenceTransformer(MODEL_PATH, device=DEVICE, model_kwargs={"torch_dtype": torch.float16})
#model = SentenceTransformer(MODEL_PATH, device=DEVICE)
model.encode("warmup")
print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Model loaded and ready.")

async def handle_client(reader, writer):
    # 클라이언트 정보 (소켓 경로에서는 빈값이므로 카운터 등으로 대체 가능)
    client_id = id(writer)

    try:
        while True:
            # 1. 요청 헤더 읽기
            header = await reader.readexactly(4)
            if not header: break

            # 요청 시작 시각 기록
            start_time = time.perf_counter()
            log_start = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

            # 2. 본문 읽기
            length = struct.unpack('>I', header)[0]
            payload = await reader.readexactly(length)
            request = json.loads(payload.decode('utf-8'))
            query = request.get("query", "")
            query_preview = (query[:30] + '..') if len(query) > 30 else query

            # 3. 모델 추론 (Thread Executor 활용)
            loop = asyncio.get_running_loop()
            embedding = await loop.run_in_executor(None, lambda: model.encode(query, normalize_embeddings=True).tolist())

            # 4. 응답 전송 준비
            #resp_payload = json.dumps({"embedding": embedding}).encode('utf-8')
            #resp_header = struct.pack('>I', len(resp_payload))

            # 중요: embedding이 list인 경우 numpy array로 변환 후 바이너리화
            if isinstance(embedding, list):
                embedding = np.array(embedding)

            # 바이너리 변환: float32 배열을 바이트로 변환 (1024차원 = 4096바이트)
            resp_payload = embedding.astype('float32').tobytes()
            resp_header = struct.pack('>I', len(resp_payload))

            writer.write(resp_header + resp_payload)
            await writer.drain()

            # 5. 로그 출력
            end_time = time.perf_counter()
            duration = (end_time - start_time) * 1000 # ms 단위
            print(f"[{log_start}] Client({client_id}) | Process Time: {duration:.2f}ms | Query: {query_preview}")

    except asyncio.IncompleteReadError:
        pass
    except Exception as e:
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Error: {e}")
    finally:
        writer.close()
        await writer.wait_closed()

async def main():
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)

    server = await asyncio.start_unix_server(handle_client, path=SOCKET_PATH)
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] Server started on {SOCKET_PATH}")

    async with server:
        await server.serve_forever()

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nServer stopped.")
