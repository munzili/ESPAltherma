
import json
import os
import shutil
import glob
import gzip

print('Generating CAN language files...')

data_src_dir = os.path.join(os.getcwd(), 'definitions/CAN/')

buildDir = os.path.join(os.getcwd(), 'build', 'CAN')

if not os.path.exists(buildDir):
   os.makedirs(buildDir)

files_to_convert = []
files_to_convert.extend(glob.glob(os.path.join(data_src_dir, 'commands_hpsu_*.json')))

print('  files to convert: ' + str(files_to_convert))

commandsFile = open(data_src_dir + "commands_hpsu.json")
commands = json.load(commandsFile)["commands"]
commandsFile.close()

for file in files_to_convert:
    print('  Converting file: ' + file)
    srcFile = os.path.join(data_src_dir, os.path.basename(file))
    targetFile = os.path.join(buildDir, os.path.basename(file))

    languageFile = open(srcFile, "r", encoding='utf8')
    languageDefinition = json.load(languageFile)
    languageFile.close()

    counter = 0
    for definition in languageDefinition["Commands"]:
        defName = definition["name"]
        command = commands[defName]

        for c in command:
            languageDefinition["Commands"][counter][c] = command[c]

        counter += 1

    with open(targetFile, "w", encoding='utf8') as outfile:
        json.dump(languageDefinition, outfile, ensure_ascii=False)

print('Finished generating CAN language files!')