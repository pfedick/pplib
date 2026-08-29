
{
        "name": "Unsloth Studio Qwen3.8-27B",
        "vendor": "customendpoint",
        "apiKey": "${input:chat.lm.secret.622f12ac}",
        "apiType": "chat-completions",
        "models": [
            {
                "id": "unsloth/Qwen3.8-27B-GGUF",
                "name": "Qwen3.8-27B-GGUF",
                "url": "http://192.168.0.8:8888/v1",
                "apiKey": "${input:chat.lm.secret.-32ea2db0}",
                "maxInputTokens": 100000,
                "maxOutputTokens": 10000,
                "toolCalling": true,
                "vision": false,
                "capabilities": {
                    "tools": true
                }
            }
        ]
    }



    name: Local Config
version: 1.0.0
schema: v1
models:
  - name: Unsloth Studio Qwen3.8-27B
    provider: openai
    model: unsloth/Qwen3.8-27B-GGUF
    apiKey: sk-unsloth-a4beb181c6b035d5aa057cde6daecbbd
    apiBase: http://192.168.0.8:8888/v1
    capabilities:
      - tool_use
    defaultCompletionOptions:
      contextLength: 100000
      maxTokens: 50000


