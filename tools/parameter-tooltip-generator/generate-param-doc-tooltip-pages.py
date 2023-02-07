"""
Grab all parameter files (markdown) and convert them to html files
"""

import os
import glob
import markdown


parameterDocsFolder = "AI-on-the-edge-device-docs/param-docs/parameter-pages"
htmlTooltipFolder = "html"


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

folders = sorted( filter( os.path.isdir, glob.glob(parameterDocsFolder + '/*') ) )


def generateHtmlFile(section, parameter, markdownFile):
    #print(section, parameter, file)

    with open(markdownFile, 'r') as markdownFileHandle:
        markdownFileContent = markdownFileHandle.read()

    if not os.path.exists(htmlTooltipFolder + "/" + section):
        os.mkdir(htmlTooltipFolder + "/" + section)

    markdownFileContent = markdownFileContent.replace("# ", "### ") # Move all headings 2 level down

    htmlContent = markdown.markdown(markdownFileContent, extensions=['admonition'])
    with open(htmlTooltipFolder + "/" + section + "/" + parameter + ".html", 'w') as htmlTooltipFileHandle:
        htmlTooltipFileHandle.write(htmlHeader)
        htmlTooltipFileHandle.write(htmlContent)
        htmlTooltipFileHandle.write(htmlFooter)



print("Generating Tooltip pages...")

# Create HTML folder if it does not exist yet
if not os.path.exists(htmlTooltipFolder):
    os.mkdir(htmlTooltipFolder)


"""
Generate a HTML page for each markdown page
"""
for folder in folders:
    folder = folder.split("/")[-1]

    files = sorted(filter(os.path.isfile, glob.glob(parameterDocsFolder + "/" + folder + '/*')))
    for file in files:
        parameter = file.split("/")[-1].replace(".md", "")
        parameter = parameter.replace("<", "").replace(">", "")
        generateHtmlFile(folder, parameter, file)
