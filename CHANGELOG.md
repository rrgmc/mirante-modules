# Changelog

All notable changes to these modules are documented in this file. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the repository follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). While the version is 0.x, a minor version may break apps.

A release is tagged when a merged pull request raises `VERSION`. See [decision 0134](https://github.com/rrgmc/mirante-core/blob/master/docs/decisions/0134-release-tags-from-a-version-file.md) in the core.

## [Unreleased]

## [0.8.1] - 2026-11-04

### Changed

- CI builds against the core's v0.8.1, and the release workflow publishes a GitHub release with each tag.

## [0.8.0] - 2026-11-03

The first release of the modules on their own, after they were split from `mirante` for custom builds.

### Added

- The modules `document`, `folder`, `image`, `notes` and `web`, with their history from `mirante`.
- Each module states the version of the module interface it needs, `API 0.1`.
- Each module carries its own translations, in `<module>/translations/`.
- The repository builds alone, as an app of its own folders, with CI on Windows, macOS and Linux that checks the modules' includes and translations.

### Removed

- The modules `claude`, `github`, `gitlab` and `jira`, which moved to [`mirante-modules-work`](https://github.com/rrgmc/mirante-modules-work) with their history.

[Unreleased]: https://github.com/rrgmc/mirante-modules/compare/v0.8.1...HEAD
[0.8.1]: https://github.com/rrgmc/mirante-modules/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/rrgmc/mirante-modules/tree/v0.8.0
