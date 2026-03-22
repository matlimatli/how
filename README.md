# how

[![Build & Test](https://github.com/matlimatli/how/actions/workflows/build.yml/badge.svg)](https://github.com/matlimatli/how/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A modern take on `man`. Ask your terminal a question, get an answer you can copy-paste. Written in C++20, no dependencies beyond libcurl.

```
$ how to undo a git commit
Soft reset keeps your changes staged, hard reset discards them entirely.

​```
git reset --soft HEAD~1
​```
```

The binary name becomes part of the query. Symlinks give you more question words:

```
$ what is my ip address
$ when was the berlin wall torn down
$ why does the sky look red at sunset
```

## Build

C++20 compiler, cmake and libcurl required.

```sh
mkdir -p build && cd build
cmake ..
make
```

Install system-wide (creates `what`, `when`, `why` symlinks too):

```sh
sudo cmake --install build
```

Or install to your personal bin without sudo:

```sh
cmake --install build --prefix ~/.local
```

Make sure `~/.local/bin` is in your `$PATH`.

Run the test suite:

```sh
ctest --test-dir build --output-on-failure
```

## Configuration

The config lives at `~/.config/how/config` and must be `chmod 600` — the binary won't start otherwise, since it contains API keys.

```sh
mkdir -p ~/.config/how
cat > ~/.config/how/config << 'EOF'
default_provider = mistral

mistral_api_key = your-key-here
mistral_model = mistral-small-latest
EOF
chmod 600 ~/.config/how/config
```

### Providers

| Provider | Config prefix | Default model |
|----------|---------------|---------------|
| Mistral | `mistral_` | `mistral-small-latest` |
| OpenAI | `openai_` | `gpt-5.4-nano` |
| Anthropic | `anthropic_` | `claude-haiku-4-5-20251001` |
| Google | `google_` | `gemini-2.5-flash-lite` |
| Custom | `custom_` | (user-specified) |

Mistral is the default — it was the fastest in our benchmarks at a reasonable price. Override per-call with environment variables:

```sh
HOW_PROVIDER=openai how to find large files
HOW_MODEL=gpt-5-nano how to find large files
```

### Local / offline models

Point the `custom` provider at any OpenAI-compatible endpoint (Ollama, vLLM, llama.cpp, etc.):

```
default_provider = custom
custom_endpoint = http://localhost:11434/v1/chat/completions
custom_model = llama3.2
allow_insecure_ssl = true
```

`allow_insecure_ssl` disables certificate verification for self-signed certs. Off by default.

## Follow-ups

The last exchange is cached in `~/.cache/how/history`, so this works:

```
$ how to find files larger than 100MB
$ how but only in the home directory
```

## Note on question marks

`?` is a shell glob character. Quote it or just leave it off — makes no difference to the answer:

```sh
how "what time is it in Tokyo?"
how what time is it in Tokyo        # same result
```

## SBOM

Third-party dependencies and their licenses are listed in [SBOM.md](SBOM.md). CI generates a machine-readable SPDX artifact as well.

## License

[MIT](LICENSE)
