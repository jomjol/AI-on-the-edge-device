"""
Grab all parameter files (markdown) and convert them to html files
"""
import os
import glob
import markdown


parameterDocsFolder = "AI-on-the-edge-device-docs/param-docs/parameter-pages"
#htmlTooltipFolder = "html"
configPage = "../../sd-card/html/edit_config_param.html"

htmlTooltipPrefix = """
    <div class="rst-content"><div class="tooltip"><img src="help.png" width="32px"><span class="tooltiptext">
"""


htmlTooltipSuffix = """
    </span></div></div>
"""


folders = sorted( filter( os.path.isdir, glob.glob(parameterDocsFolder + '/*') ) )


def generateHtmlTooltip(section, parameter, markdownFile):
    #print(section, parameter, file)

    with open(markdownFile, 'r') as markdownFileHandle:
        markdownFileContent = markdownFileHandle.read()

    markdownFileContent = markdownFileContent.replace("# ", "### ") # Move all headings 2 level down

    htmlTooltip = markdown.markdown(markdownFileContent, extensions=['admonition'])

    # Make all links to be opened in a new page
    htmlTooltip = htmlTooltip.replace("a href", "a target=_blank href")


    htmlTooltip = htmlTooltipPrefix + htmlTooltip + htmlTooltipSuffix

    # Add the tooltip to the config page
    with open(configPage, 'r') as configPageHandle:
        configPageContent = configPageHandle.read()

    print("replacing $TOOLTIP_" + section + "_" + parameter + " with the tooltip content...")
    configPageContent = configPageContent.replace("$TOOLTIP_" + section + "_" + parameter, htmlTooltip)

    with open(configPage, 'w') as configPageHandle:
        configPageHandle.write(configPageContent)


print("Generating Tooltips...")

"""
Generate a HTML tooltip for each markdown page
"""
for folder in folders:
    folder = folder.split("/")[-1]

    files = sorted(filter(os.path.isfile, glob.glob(parameterDocsFolder + "/" + folder + '/*')))
    for file in files:
        parameter = file.split("/")[-1].replace(".md", "")
        parameter = parameter.replace("<", "").replace(">", "")
        generateHtmlTooltip(folder, parameter, file)
