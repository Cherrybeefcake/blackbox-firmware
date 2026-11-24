# Development sequence

1. Review this source baseline and historical release measurements. Establish a separate diagnostics-only target with explicit source selection and reviewed startup, menu, shortcut and serial entry points. Keep the upstream baseline identifiable.
2. Build the approved diagnostic target in an isolated workspace. Record dependency revisions, warnings, raw and merged binary digests, static RAM and actual application partition margin. Repeat with clean caches to assess reproducibility.
3. Review the System dashboard proposal in ARCHITECTURE.md. Acceptance criteria include five legible rows, graceful handling of absent SD and unconfirmed ADV identity, immediate Back navigation, and no persistent writes or peripheral probing.
4. Implement the approved screen in a separate change. Compare flash and RAM to the same diagnostic target without the screen. Verify keyboard response, mount-state display and heap behavior on hardware in a separately authorized session.

Task 002 delivered documentation only. Dashboard implementation and device validation remain pending.
