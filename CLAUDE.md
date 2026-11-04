# Mirante modules

The first-party modules of Mirante, one per folder, in Qt 6 (C++17, Qt Widgets, CMake). The design documents, the module interface and the plan for this repository are in `mirante-core`: see its `docs/modules.md` and `docs/plans/custom-builds.md`.

## Git workflow

Several sessions may work on this repository at the same time, so each piece of work happens in its own Git worktree, with its own branch.

- Before the first edit, create a worktree with its own branch: `EnterWorktree` in Claude Code, or `git worktree add -b <branch> .claude/worktrees/<branch> origin/master`. A new branch in the main checkout is not enough, so never run `git checkout` or `git switch` there.
- Never edit files in the main checkout on `master` directly. It stays clean, so other sessions can branch from it.

Changes reach `master` only through a pull request. Never merge a pull request on your own: only the user merges. When the user asks for the merge, use a merge commit (`gh pr merge <number> --merge`).

## Rules

- A module includes only its own files and `<mirante/...>`, never a core header.
- Keep platform-specific code out of modules where possible. Changes must build on Windows, macOS and Linux.
