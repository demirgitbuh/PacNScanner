# Contributing to PacNScanner

Thanks for your interest! Contributions are welcome.

## Getting started

1. Read [docs/BUILD.md](docs/BUILD.md) and get a local build + tests passing.
2. Skim [docs/DEVELOPER_GUIDE.md](docs/DEVELOPER_GUIDE.md) for the architecture.

## Workflow

- Branch from `main`; keep changes focused.
- Match the existing style — run `clang-format` (`.clang-format` is provided).
- Add or update **GoogleTest** coverage for behavioural changes (`tests/`). The engine is
  testable offline via `FakeProbeFactory` + `FakePlatform`.
- Ensure `ctest --test-dir build` is green and the project builds on your platform. CI builds
  and tests on Windows, macOS and Linux.
- Keep the core layer GUI-free (only `Qt::Core`/`Qt::Network`).

## Commit & PR

- Write clear commit messages.
- Describe the change and testing in the PR.
- Reference any related issue.

## Reporting bugs

Open an issue with: OS + version, Qt version, steps to reproduce, expected vs actual, and the
relevant lines from `pacnscanner.log` (see **View → Log** for its path).

## Security & ethics

PacNScanner is a defensive/inventory tool for **authorized** networks only. Please do not file
requests that facilitate unauthorized scanning or attacks.

By contributing you agree your contributions are licensed under the [MIT License](LICENSE).
