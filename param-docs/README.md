# Parameter Documentation
Each parameter which is listed in the [configfile](https://github.com/jomjol/AI-on-the-edge-device/blob/rolling/sd-card/config/config.ini) has its own description page in the folder `parameter-pages` (grouped by the config sections).
Those pages can be edited as needed.

During a Github action build, those parameter pages will be used to generate the tooltips in the web interface. And they also are used to build the [Online Documentation](https://jomjol.github.io/AI-on-the-edge-device-docs/Parameters).

If you create or rename a parameter, make sure to also update its description page!

## Template Generator
The script `generate-template-param-doc-pages.py` should be run whenever a new parameter gets added to the config file.
It then checks if there is already a page for each of the parameters.
 - If no page exists yet, a templated page gets generated.
 - Existing pages do not get modified.

If the parameter is listed in `expert-params.txt`, an **Expert warning** will be shown.

If the parameter is listed in `hidden-in-ui.txt`, a **Note**  will be shown.