"""
For each parameter which can be found in the config file,
create a markdown file with a templated content if it does not exist yet.
The files are grouped in sub folders representing the config sections.
"""

import os
import configparser
import urllib.request


configFileUrl = "https://raw.githubusercontent.com/jomjol/AI-on-the-edge-device/rolling/sd-card/config/config.ini"

parameterDocsFolder = "parameter-pages"
parameterTemplateFile = "./templates/parameter.md"
expertParameterListFile = "./expert-params.txt"
hiddenInUiParameterListFile = "./hidden-in-ui.txt"


# Fetch default config file from URL
print("Fetching %r..." % configFileUrl)
with urllib.request.urlopen(configFileUrl) as response:
   content = response.read().decode("utf-8")

lines = str(content).split("\n")

for l in range(len(lines)):
    lines[l] = lines[l].strip() + "\n"
    if lines[l][0] == ";":
        lines[l] = lines[l][1:] # Remove comment

content = "".join(lines)

# Fetch list of expert parameters
with open(expertParameterListFile) as f:
    expertParameters = f.read().splitlines()

# Fetch list of parameters not available through the UI
with open(hiddenInUiParameterListFile) as f:
    hiddenInUiParameters = f.read().splitlines()


config = configparser.ConfigParser(allow_no_value=True)
config.optionxform = str # Make it case-insensitive
config.read_string(content)

#shutil.rmtree(parameterDocsFolder)
if not os.path.exists(parameterDocsFolder):
    os.mkdir(parameterDocsFolder)

with open(parameterTemplateFile, 'r') as parameterTemplateFileHandle:
    parameterTemplate = parameterTemplateFileHandle.read()


print("For each section/parameter, check if there is already a documentation page in the folder %r..." % (os.getcwd() + "/" + parameterDocsFolder))
for section in config:
    if section != "DEFAULT":
        #print(section)

        subFolder = parameterDocsFolder + "/" + section

        if not os.path.exists(subFolder):
            os.mkdir(subFolder)

        for parameter in config[section]:
            if not " " in parameter: # Ignore parameters with whitespaces in them (special format, not part of editable config)
                value = config[section][parameter]
                #print("  %s = %s" % (parameter, value))

                if "main." in parameter:
                    parameter = parameter.replace("main.", "NUMBER.")

                """
                For each config line, create a markdown file
                """
                parameterDocFile = subFolder + '/' + parameter + ".md"

                if not os.path.exists(parameterDocFile): # File does not exist yet, generate template
                    print("%r does not exit yet, generating a templated file for it" % (os.getcwd() + "/" + parameterDocFile))
                    with open(parameterDocFile, 'w') as paramFileHandle:
                        content = parameterTemplate
                        content = content.replace("$NAME", parameter)
                        content = content.replace("$DEFAULT", value)

                        if parameter in expertParameters:
                            content = content.replace("$EXPERT_PARAMETER", "!!! Warning\n    This is an **Expert Parameter**! Only change it if you understand what it does!") # Note: Needs a 4 whitespace Intent!
                        else:
                            content = content.replace("$EXPERT_PARAMETER", "")

                        if parameter in hiddenInUiParameters:
                            content = content.replace("$HIDDEN_IN_UI", "!!! Note\n    This parameter is not accessible through the Web Interface Configuration Page!") # Note: Needs a 4 whitespace Intent!
                        else:
                            content = content.replace("$HIDDEN_IN_UI", "")

                        paramFileHandle.write(content)
