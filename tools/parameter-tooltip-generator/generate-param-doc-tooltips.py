"""
Grab all parameter files (markdown) and convert them to html files
"""
import os
import glob
import markdown


parameterDocsFolder = "AI-on-the-edge-device-docs/param-docs/parameter-pages"
#htmlTooltipFolder = "html"
configPage = "../../sd-card/html/edit_config_param.html"


htmlHeader = """
<head>
    <link rel="stylesheet" href="../css/theme.css?v=$COMMIT_HASH" />
    <link rel="stylesheet" href="../css/theme_extra.css?v=$COMMIT_HASH" />
    <link rel="stylesheet" href="../css/github.min.css?v=$COMMIT_HASH" />
</head>

<body class="wy-body-for-nav" role="document" style="margin: 10px;">
    <div class="rst-content">
"""

htmlFooter = """
    <p></p></div>
</body>
"""

htmlTooltipPrefix = """
    <div class="tooltip"><img src="help.png" width="32px"><span class="tooltiptext">
"""


htmlTooltipSuffix = """
    </span></div>
"""


folders = sorted( filter( os.path.isdir, glob.glob(parameterDocsFolder + '/*') ) )


def generateHtmlTooltip(section, parameter, markdownFile):
    #print(section, parameter, file)

    with open(markdownFile, 'r') as markdownFileHandle:
        markdownFileContent = markdownFileHandle.read()

#    if not os.path.exists(htmlTooltipFolder + "/" + section):
#        os.mkdir(htmlTooltipFolder + "/" + section)

    markdownFileContent = markdownFileContent.replace("# ", "### ") # Move all headings 2 level down

    htmlTooltip = markdown.markdown(markdownFileContent, extensions=['admonition'])

    # Make all links to be opened in a new page
    htmlTooltip = htmlTooltip.replace("a href", "a target=_blank href")

#    with open(htmlTooltipFolder + "/" + section + "/" + parameter + ".html", 'w') as htmlTooltipFileHandle:
#        htmlTooltipFileHandle.write(htmlHeader)
#        htmlTooltipFileHandle.write(htmlTooltip)
#        htmlTooltipFileHandle.write(htmlFooter)


    htmlTooltip = htmlTooltipPrefix + htmlTooltip + htmlTooltipSuffix

    # Add the tooltip to the config page
    with open(configPage, 'r') as configPageHandle:
        configPageContent = configPageHandle.read()

    print("replacing $TOOLTIP_" + section + "_" + parameter + " with the tooltip content...")
    configPageContent = configPageContent.replace("$TOOLTIP_" + section + "_" + parameter, htmlTooltip)

    with open(configPage, 'w') as configPageHandle:
        configPageHandle.write(configPageContent)


print("Generating Tooltips...")

# Create HTML folder if it does not exist yet
#if not os.path.exists(htmlTooltipFolder):
#    os.mkdir(htmlTooltipFolder)


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
