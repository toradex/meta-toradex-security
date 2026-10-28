# Maintainers and Governance

This document defines the shared ownership, maintenance responsibilities, and contribution process for `meta-toradex-security`. It applies to all maintained branches of the repository.

## Project ownership

`meta-toradex-security` is jointly maintained by two Toradex teams, with equal ownership of the project. The **Devices** team develops the [Toradex Embedded Linux BSP](https://developer.toradex.com/linux-bsp/) and the support for Toradex SoMs. The **Torizon** team develops [Torizon OS](https://developer.toradex.com/torizon/), which also runs on non-Toradex hardware. Generic and shared code is owned and maintained by both teams, while each hardware platform is owned by the team responsible for it.

The repository accepts security-related metadata for both Toradex and non-Toradex hardware, subject to the requirements below. Supporting a platform does not make the other team responsible for maintaining or validating it, and this ownership continues after the initial contribution is merged.

## Maintainers

| Area | Team | Maintainer | Scope |
| --- | --- | --- | --- |
| Toradex hardware | Devices | Sergio Prado (@sergioprado) | Toradex SoMs and their BSP integrations |
| Non-Toradex hardware | Torizon | Rogério Borin (@rborn-tx) | Third-party platforms, including x86 and NVIDIA |
| Generic and shared code | Both | Both maintainers | Shared classes, public interfaces, and common functionality |

Each team should designate a backup reviewer for periods when its maintainer is unavailable. Maintainers may delegate reviews and day-to-day work to other contributors within their teams. Changes to maintainer assignments or areas of responsibility must be agreed upon by both teams and documented here.

## Maintenance responsibilities

For the hardware and functionality under its ownership, each team is responsible for:

- Reviewing contributions and maintaining the relevant code.
- Triaging and resolving bugs, regressions, GitHub issues, and security vulnerabilities.
- Keeping platform documentation and the [feature support matrix](https://developer.toradex.com/linux-bsp/os-development/security/introduction-to-meta-toradex-security#feature-support-matrix) up to date.
- Maintaining compatibility with supported Yocto and BSP releases.
- Providing the hardware, CI infrastructure, and test coverage required for its platforms.

If a change to shared code causes a regression on a supported platform, the author of the change is responsible for fixing or reverting it. The team responsible for the affected platform assists with the investigation.

Resource allocation and budget commitments remain the responsibility of the respective team leads. This document defines technical responsibilities but does not, by itself, commit either team's budget or engineering capacity.

### Unmaintained platforms

A platform that no longer has an active maintainer or working CI coverage may be marked as unmaintained and removed from the repository. The removal must be announced to both teams at least 30 working days in advance, and does not happen if a maintainer takes responsibility for the platform within that period.

## Contributions and code review

Contributions are evaluated on technical merit, maintainability, compatibility, test coverage, and compliance with project conventions, regardless of which team or platform they originate from.

- **Platform-specific changes** are reviewed and maintained by the responsible team. The other team is involved if shared behavior or interfaces may be affected. A maintainer may approve and merge these changes once the review and CI requirements have been met.
- **Generic or shared changes** are reviewed by both teams, following the review window defined below.
- **Cross-platform changes** must identify the affected platforms and include appropriate regression testing.
- **External contributions** follow the same requirements and must have a maintainer willing to take responsibility for ongoing support.

Platform-specific logic belongs in platform-specific files, such as machine includes and dynamic layers, and not in conditionals inside shared classes. This keeps the ownership of each change clear and limits joint reviews to code that is actually shared.

### Review window for shared changes

To prevent one team's availability from indefinitely blocking the other, the following process applies to routine changes in generic or shared code:

1. The contributor requests review from both teams when the PR is ready for review, with a clear description of the change, its impact, and relevant test results. The **five-working-day review window** starts at that point, not when a draft PR is opened.
2. Both teams are expected to provide a review or concrete technical feedback within that window. A response from a team's backup reviewer counts as a response from that team.
3. If one team has not responded after five working days, the other team may merge the PR, provided it has been approved by its maintainer or backup reviewer (who must not be the author of the PR), all required CI checks have passed, there are no known regressions, and the change does not fall under the exceptions below. The lack of a response must be noted in the PR.
4. If a reviewer raises a substantive technical concern within the window, the PR must address that concern or reach an agreed resolution before merging. An expired review window does not override an unresolved technical objection. If the teams cannot agree, the issue follows the escalation process below.

The review window prevents delays caused by silence. It does not replace technical review or allow unilateral architectural decisions. Changes substantially revised after a review request should be resubmitted for review.

**Exceptions:** Major new features, significant architectural changes, repository restructuring, and incompatible changes to shared public interfaces require advance discussion and agreement from both teams. They cannot be merged solely because the review window has expired.

## CI and validation

Every supported platform must have automated CI coverage, with **build validation as the minimum requirement**. Runtime tests must also be included where technically feasible, particularly for security mechanisms that cannot be adequately validated at build time.

Before adding a platform, the responsible team must identify the CI targets and configurations that will be maintained, the automated tests that validate its security functionality, and who provides the CI resources and handles failures.

Changes to shared code must pass the CI checks for **all supported platforms**, and no change may be merged with known regressions on the other team's platforms. If CI coverage is missing, broken, or temporarily unavailable, both maintainers must explicitly agree on the exception and document any remaining validation work. The review window does not waive these requirements.

## Shared technical decisions and coordination

Both teams have equal input into decisions affecting:

- Generic security functionality and shared classes.
- Public configuration interfaces and backward compatibility.
- Supported Yocto branches and common maintenance policies.
- Common CI standards and infrastructure.
- Major features, refactoring, repository restructuring, or architectural changes.

Major features and architectural proposals should be announced in advance, with their goals, expected impact, and proposed approach. The maintainers should hold a **regular coordination meeting, initially monthly**, to discuss upcoming changes, roadmap dependencies, and decisions affecting both teams. Urgent matters and time-sensitive reviews are handled asynchronously, without waiting for the next meeting. Routine platform-specific decisions remain with the responsible maintainer, provided they do not negatively affect shared interfaces or other supported platforms.

When consensus cannot be reached, maintainers should document the technical trade-offs and escalate the issue to the respective engineering leads for a management-level decision. Until a decision is reached, neither team may unilaterally merge a disputed shared change. Urgent security fixes may use an expedited review process, with both teams notified and any follow-up validation tracked.

## Updating this document

Changes to project ownership, maintenance scope, or review policy require agreement from both teams and should be proposed through a pull request.
