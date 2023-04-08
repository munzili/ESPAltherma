
import json
import os
import shutil
import glob
import gzip

print('Generating X10A language files...')

data_src_dir = os.path.join(os.getcwd(), 'definitions/X10A/')

buildDir = os.path.join(os.getcwd(), 'build/')

if not os.path.exists(buildDir):
   os.makedirs(buildDir)

files_to_convert = []
files_to_convert.extend(glob.glob(os.path.join(data_src_dir, '*.json')))

language_directorys = [os.path.abspath(data_src_dir + name) for name in os.listdir(data_src_dir + '.') if os.path.isdir(data_src_dir + name)]

print('  files to convert: ' + str(files_to_convert))
print('  directorys to convert: ' + str(language_directorys))

for file in files_to_convert:
    print('  Converting file: ' + os.path.basename(file))

    modelFile = open(file, "r", encoding='utf8')
    modelDefinition = json.load(modelFile)
    modelFile.close()

    modelFileLanguageDir = buildDir + modelDefinition["Language"]

    if not os.path.exists(modelFileLanguageDir):
        os.makedirs(modelFileLanguageDir)

    shutil.copyfile(file, modelFileLanguageDir + '/' + os.path.basename(file))

    for langDirectory in language_directorys:
        languageFile = open(file, "r", encoding='utf8')
        languageDefinition = json.load(languageFile)
        languageFile.close()

        modelDefinition["Language"] = languageDefinition["Language"]

        languageFileDir = buildDir + languageDefinition["Language"] + '/'

        counter = 0
        for parameter in languageDefinition["Parameters"]:
            modelDefinition["Parameters"][counter][5] = languageDefinition["Parameters"][counter]
            counter += 1

        with open(languageFileDir + os.path.basename(file), "w", encoding='utf8') as outfile:
            json.dump(modelDefinition, outfile, ensure_ascii=False)


#commandsFile = open(data_src_dir + "commands_hpsu.json")
#commands = json.load(commandsFile)["commands"]
#commandsFile.close()
#
#for file in files_to_convert:
#    print('  Converting file: ' + file)
#    srcFile = os.path.join(data_src_dir, os.path.basename(file))
#    newFileName = os.path.splitext(srcFile)[0] + ".conv.json"
#
#    languageFile = open(srcFile, "r", encoding='utf8')
#    languageDefinition = json.load(languageFile)
#    languageFile.close()
#
#    counter = 0
#    for definition in languageDefinition["Commands"]:
#        defName = definition["name"]
#        command = commands[defName]
#
#        for c in command:
#            languageDefinition["Commands"][counter][c] = command[c]
#
#        counter += 1
#
#    if os.path.exists(newFileName):
#        os.remove(newFileName)
#
#    with open(newFileName, "w", encoding='utf8') as outfile:
#        json.dump(languageDefinition, outfile, ensure_ascii=False)

print('Finished generating X10A language files!')