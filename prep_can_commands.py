
import json
import os
import shutil
import glob
import gzip

print('Generating CAN language files...')

data_src_dir = os.path.join(os.getcwd(), 'definitions/CAN/')

files_to_convert = []
files_to_convert.extend(glob.glob(os.path.join(data_src_dir, 'commands_hpsu_*.json')))

files_to_convert = filter(lambda x: ".conv." not in x, files_to_convert)

print('  files to convert: ' + str(files_to_convert))

commandsFile = open(data_src_dir + "commands_hpsu.json")
commands = json.load(commandsFile)["commands"]
commandsFile.close()  

for file in files_to_convert:
    print('  Converting file: ' + file)
    srcFile = os.path.join(data_src_dir, os.path.basename(file))
    newFileName = os.path.splitext(srcFile)[0] + ".conv.json"

    languageFile = open(srcFile)
    languageDefinition = json.load(languageFile)
    languageFile.close() 

    counter = 0
    for definition in languageDefinition["Commands"]:
        defName = definition["name"]
        command = commands[defName]

        for c in command:            
            languageDefinition["Commands"][counter][c] = command[c]
        
        counter += 1

    if os.path.exists(newFileName):
        os.remove(newFileName)

    with open(newFileName, "w") as outfile:
        json.dump(languageDefinition, outfile)

print('Finished generating CAN language files!')