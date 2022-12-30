## Preparing for release

1. [Changelog](https://github.com/jomjol/AI-on-the-edge-device/blob/rolling/Changelog.md) is merged back from `master` branch to `rolling` branch (should be the last step of the previous release creation)
1. All changes are documented in the [Changelog](https://github.com/jomjol/AI-on-the-edge-device/blob/rolling/Changelog.md) in `rolling` branch


## Release creation steps
1. Merge`rolling` into `master` branch
2. Best to wait for the GitHub action to run successfully 
3. On `master` branch tag the version like `v11.3.1` and don't forget to push it.
4. Wait for the GitHub-Action of release creation. After all is done:
    * the release should be created
    * the artifacts are downloadable from release 
    * The documented changes were applied to the release
5. Merge master back in `rolling`
1. In `rolling` create a folder `rolling/docs/releases/download/<VERSION>` and add the `firmware.bin` from one of the release artifacts.
1. Update `rolling/docs/manifest.json` with the new version (update the `version` and the last `path` fields)
