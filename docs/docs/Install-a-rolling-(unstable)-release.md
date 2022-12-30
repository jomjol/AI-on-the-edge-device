# :bangbang: Living on the edge :bangbang:
:bangbang: The branch [rolling](https://github.com/jomjol/AI-on-the-edge-device/tree/rolling) contains the latest version of the Firmware and the Web Interface. It is  work in progress, don't expect it to work stable or be an improvement for your AI-on-the-edge-device! Also it might break the OTA Update and then require manual flashing over USB! :bangbang:

# Still here?

Grab the latest build from https://github.com/jomjol/AI-on-the-edge-device/actions and proceed as following:
1. Pick the most top successful (green) build.
2. Download the `firmware__extract_before_upload__only_needed_for_migration_from_11.2.0` and extract it (its a zip file).
3. Flash that binary as new firmware.
4. Download the `html__only_needed_for_migration_from_11.2.0__2022-09-15_19-13-37__rolling_(042ff18)`. It is also a zip file but you should **not** extract it!
5. Flash the zip file als html part.

The filenames have changed, e.g. right now it is:
* AI-on-the-edge-device__manual-setup__rolling_(4b23e0c)
* AI-on-the-edge-device__remote-setup__rolling_(4b23e0c)  
* AI-on-the-edge-device__update__rolling_(4b23e0c)

Github bot-reply Rolling Build has the following info at the moment:

You can use the latest [Automatic Build](https://github.com/jomjol/AI-on-the-edge-device/actions/workflows/build.yaml?query=branch%3Arolling) of the the rolling branch. It might already contain a fix for your issue.
Pick the most top passing entry (it has a green circle with a tick in it), then scroll down to the Artifacts and download the file named update_*. So I do not know what the manual-setup and remote-setup are used for.