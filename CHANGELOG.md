# Changelog

All notable changes to these modules are documented in this file. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the repository follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html). While the version is 0.x, a minor version may break apps.

A release is tagged when a merged pull request raises `VERSION`. See [decision 0134](https://github.com/rrgmc/mirante-core/blob/master/docs/decisions/0134-release-tags-from-a-version-file.md) in the core. A pull request does not edit this file. It adds a fragment to [changelog.d](changelog.d/README.md), and the release moves the fragments here. See [decision 0154](https://github.com/rrgmc/mirante-core/blob/master/docs/decisions/0154-changelog-fragments.md) in the core.

## [Unreleased]

## [0.15.0] - 2026-11-09

Changelog fragments, and CI for the merge queue. It needs the core's v0.15.0.

### Changed

- Pull requests add their changelog entries as fragments in `changelog.d/`, and a release moves them into this file with the core's `tools/release-changelog.py`. See [decision 0154](https://github.com/rrgmc/mirante-core/blob/master/docs/decisions/0154-changelog-fragments.md) in the core.

## [0.13.0] - 2026-11-05

A more reliable test of the system module. It needs the core's v0.12.0.

### Fixed

- The system module's view test reads the CPU again for up to 5 seconds. Before, it could fail on a busy runner, where the CPU counters had not moved after 300 ms.

## [0.12.0] - 2026-11-04

The system module with its System Resources item type, and web links in the cell header. It needs the core's v0.12.0.

### Added

- The `system` module, with the System Resources item type: this computer's CPU use, free memory and free disk space, each with a bar, read again every few seconds while the item is visible. The item's settings choose which ones to show, and which disks. See [the system module](system/README.md).

### Changed

- The cell header's **Open in browser** button opens a Web document's address and an image's link. The views give them with `ItemContext::setWebLink()`, and a click on an image still opens its link. Web page items get the button from the core. See [decision 0148](https://github.com/rrgmc/mirante-core/blob/master/docs/decisions/0148-web-link-in-the-cell-header.md) in the core.
- CI builds against the core's master at the merge of rrgmc/mirante-core#49, which adds `ItemContext::setWebLink()`.

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

[Unreleased]: https://github.com/rrgmc/mirante-modules/compare/v0.15.0...HEAD
[0.15.0]: https://github.com/rrgmc/mirante-modules/compare/v0.13.0...v0.15.0
[0.13.0]: https://github.com/rrgmc/mirante-modules/compare/v0.12.0...v0.13.0
[0.12.0]: https://github.com/rrgmc/mirante-modules/compare/v0.8.1...v0.12.0
[0.8.1]: https://github.com/rrgmc/mirante-modules/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/rrgmc/mirante-modules/tree/v0.8.0
