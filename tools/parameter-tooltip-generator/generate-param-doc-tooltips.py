"""
Grab all parameter files (markdown) and convert them to html files
"""
import os
import glob
import markdown


parameterDocsFolder = "../../param-docs/parameter-pages"
docsMainFolder = "../../sd-card/html"
configPageTemplate = "edit_config_template.html"
configPage = "edit_config.html"
refImagePage = "edit_reference.html"

htmlTooltipPrefix = """
    <div class="rst-content"><div class="tooltip"><img src="help.png" width="32px"><span class="tooltiptext">
"""


htmlTooltipSuffix = """
    </span></div></div>
"""


os.system("cp " + docsMainFolder + "/" + configPageTemplate + " " + docsMainFolder + "/" + configPage)

folders = sorted( filter( os.path.isdir, glob.glob(parameterDocsFolder + '/*') ) )


def generateHtmlTooltip(section, parameter, markdownFile):
    # print(section, parameter, markdownFile)

    with open(markdownFile, 'r') as markdownFileHandle:
        markdownFileContent = markdownFileHandle.read()

    markdownFileContent = markdownFileContent.replace("# ", "### ") # Move all headings 2 level down

    htmlTooltip = markdown.markdown(markdownFileContent, extensions=['admonition'])

    # Make all links to be opened in a new page
    htmlTooltip = htmlTooltip.replace("a href", "a target=_blank href")

    # Replace relative documentation links with absolute ones pointing to the external documentation
    htmlTooltip = htmlTooltip.replace("href=\"../", "href=\"https://jomjol.github.io/AI-on-the-edge-device-docs/")

    # Add custom styles
    htmlTooltip = htmlTooltip.replace("<h3>", "<h3 style=\"margin: 0\">")

    # Update image paths and copy images to right folder
    if "../img/" in htmlTooltip:
        htmlTooltip = htmlTooltip.replace("../img/", "/")

    htmlTooltip = htmlTooltipPrefix + htmlTooltip + htmlTooltipSuffix

    # Add the tooltip to the config page
    with open(docsMainFolder + "/" + configPage, 'r') as configPageHandle:
        configPageContent = configPageHandle.read()
    configPageContent = configPageContent.replace("<td>$TOOLTIP_" + section + "_" + parameter + "</td>", "<td>" + htmlTooltip + "</td>")
    with open(docsMainFolder + "/" + configPage, 'w') as configPageHandle:
        configPageHandle.write(configPageContent)

    # Add the tooltip to the reference image page
    with open(docsMainFolder + "/" + refImagePage, 'r') as refImagePageHandle:
        refImagePageContent = refImagePageHandle.read()
    refImagePageContent = refImagePageContent.replace("<td>$TOOLTIP_" + section + "_" + parameter + "</td>", "<td>" + htmlTooltip + "</td>")
    with open(docsMainFolder + "/" + refImagePage, 'w') as refImagePageHandle:
        refImagePageHandle.write(refImagePageContent)

print("Generating Tooltips...")

"""
Generate a HTML tooltip for each markdown page
"""
for folder in folders:
    folder = folder.split("/")[-1]

    files = sorted(filter(os.path.isfile, glob.glob(parameterDocsFolder + "/" + folder + '/*')))
    for file in files:
        if not ".md" in file: # Skip non-markdown files
            continue

        parameter = file.split("/")[-1].replace(".md", "")
        parameter = parameter.replace("<", "").replace(">", "")
        generateHtmlTooltip(folder, parameter, file)

"""
Copy images to main folder
"""
os.system("cp " + parameterDocsFolder + "/img/* " + docsMainFolder + "/")