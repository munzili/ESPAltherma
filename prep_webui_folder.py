Import("env")

import os
import gzip
import glob

filetypes_to_gzip = ['js', 'html', 'css']

print('Generating gzip webui files...')

data_src_dir = os.path.join(env.get('PROJECT_DIR'), 'webui')

files_to_gzip = []
for extension in filetypes_to_gzip:
    files_to_gzip.extend(glob.glob(os.path.join(data_src_dir, '*.' + extension)))

print('  files to gzip: ' + str(files_to_gzip))

for file in files_to_gzip:
    print('  GZipping file: ' + file)
    with open(file, "rb") as src, gzip.open(os.path.join(data_src_dir, os.path.basename(file) + '.gz'), 'wb') as dst:        
        dst.writelines(src)

print('Finished generating gzip webui files!')