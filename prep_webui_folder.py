Import("env")

try:
    from css_html_js_minify import process_single_html_file, process_single_js_file, process_single_css_file
except ImportError:
    env.Execute("$PYTHONEXE -m pip install css-html-js-minify htmlmin")    
    
from css_html_js_minify import process_single_html_file, process_single_js_file, process_single_css_file
import htmlmin 
import os
import shutil
import glob
import gzip

filetypes_to_gzip = ['js', 'html', 'css']

print('Generating gzip webui files...')

data_src_dir = os.path.join(env.get('PROJECT_DIR'), 'webui')

files_to_gzip = []
for extension in filetypes_to_gzip:
    files_to_gzip.extend(glob.glob(os.path.join(data_src_dir, '*.' + extension)))

print('  files to gzip: ' + str(files_to_gzip))

for file in files_to_gzip:
    print('  Minifying file: ' + file)
    srcFile = os.path.join(data_src_dir, os.path.basename(file))
    tmpFile = srcFile + ".tmp"
    gzFile = srcFile + ".gz"
    extension = file.split(".")[-1]    
    
    shutil.copyfile(srcFile, tmpFile)

    if extension == "js":
        process_single_js_file(tmpFile, overwrite=True)
    elif extension == "html":                
        with open(tmpFile,'r') as fileHandler:
            htmlContent = fileHandler.read()
        
        htmlContent = htmlmin.minify(htmlContent)

        with open(tmpFile,'w') as fileHandler:
            fileHandler.write(htmlContent)
    elif extension == "css":
        process_single_css_file(tmpFile, overwrite=True)
        
    print('  GZipping file: ' + file)
    with open(tmpFile, "rb") as src, gzip.open(gzFile, 'wb') as dst:        
        dst.writelines(src)

    os.remove(tmpFile)

print('Finished generating gzip webui files!')